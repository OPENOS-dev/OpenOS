#!/usr/bin/python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Driver to download an image from a path/server to the 'image usbkey'."""

import argparse
import contextlib
import logging
import os
import re
import shutil
import subprocess
import sys
from urllib.error import ContentTooShortError
from urllib.error import URLError
from urllib.request import urlopen

import requests
from requests.exceptions import RequestException


LOG_FORMAT = "%(levelname)s - %(filename)s:%(lineno)d:%(funcName)s : %(message)s"


class ImageDownloaderException(Exception):
    """Error class for ImageDownloaderException errors."""


class ImageDownloader:
    """Downloads an image to a USB device."""

    _HTTP_REGEX = "https?://"

    def __init__(self, usb_dev, image_path):
        """Creates a new image downloader

        Args:
            usb_dev (string): Path to USB device
            image_path (string): URL or local path to image file
        """
        self._logger = logging.getLogger(type(self).__name__)
        self.usb_dev = usb_dev
        self.image_path = image_path

    def download(self):
        """Downloads the image to the USB device.

        Raises:
            ImageDownloaderException: If there is an error during the download process.
        """
        errormsgs = []

        # Verify we can write to the USB drive
        try:
            self._check_usb_stick()
        except ImageDownloaderException as err:
            raise err

        try:
            # Determine which type of image path is being used
            if re.match(self._HTTP_REGEX, self.image_path):
                self._logger.info("Image path is a URL, downloading image")
                self._download_url()
            else:
                self._logger.info("Image path is a local, copying image")
                self._copy_file()

        # Catch various exceptions that may occur during the download process
        except ImageDownloaderException as e:
            errstr = f"ImageDownloaderException: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        except URLError as e:
            errstr = f"URLError: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        except RequestException as e:
            errstr = f"RequestException: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        except IOError as e:
            errstr = f"IOError: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        except OSError as e:
            errstr = f"OSError: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        except BaseException as e:
            errstr = f"Unexpected Exception: {e}"
            self._logger.error(errstr)
            errormsgs.append(errstr)
        finally:
            # We just plastered the partition table for a block device.
            # Pass or fail, we mustn't go without telling the kernel about
            # the change, or it will punish us with sporadic, hard-to-debug
            # failures.
            self._logger.debug("USB Device is at %s", self.usb_dev)
            if self.usb_dev:
                self._logger.debug("Calling Sync")
                proc = subprocess.run(
                    ["sync", self.usb_dev], capture_output=True, text=True
                )
                if proc.returncode:
                    errstr = f"Sync failed: {proc.stderr}"
                    self._logger.error(errstr)
                    errormsgs.append(errstr)

                self._logger.debug("Calling blockdev")
                proc = subprocess.run(
                    ["sudo", "blockdev", "--rereadpt", self.usb_dev],
                    capture_output=True,
                    text=True,
                )
                if proc.returncode:
                    errstr = f"Blockdev failed: {proc.stderr}"
                    self._logger.error(errstr)
                    errormsgs.append(errstr)

        if errormsgs:
            raise ImageDownloaderException("\n".join(errormsgs))

    def _check_usb_stick(self):
        """Verify we have write access to the USB stick

        Raises:
            ImageDownloaderException: If access failed either do to it not existing or us not having premission
        """

        self._logger.debug("Testing device %r", self.usb_dev)

        # Alert if the path does not exist
        if not os.path.exists(self.usb_dev):
            errormsg = "Device does not exist"
            self._logger.error(errormsg)
            raise ImageDownloaderException(errormsg)
        self._logger.debug("Path exists %r", self.usb_dev)

        # Verify we have write access to it
        try:
            with open(self.usb_dev, "wb") as f:
                f.write(b"000000000000000000000000000")
            self._logger.debug("Device testing pass")
        except (IOError, OSError) as e:
            errormsg = f"Unable to access device {e}"
            self._logger.error(errormsg)
            raise ImageDownloaderException(errormsg) from err

    def _copy_file(self):
        """Copies the image file from the local filesystem to the USB device."""
        shutil.copyfile(self.image_path, self.usb_dev)

    def _download_url(self):
        """Downloads the image file from the network to the USB device."""

        # Check the webserver is working by getting the first 100 bytes of the file.
        # If the image path requests extraction use the archive instead.
        self._logger.debug("Testing webserver")
        base_image_path = re.sub('\?file=.*$', '', self.image_path)
        base_image_path = re.sub('/extract/', '/download/', base_image_path)
        response = requests.head(base_image_path, timeout=900)
        response.raise_for_status()
        self._logger.debug("Test Headers %s" % response.headers)
        self._logger.debug("Webserver test pass")

        def progress_bar(block_num, block_size, total_size):
            """Displays a progress bar in the logs."""
            if block_num and block_num % 10000 == 0:
                self._logger.debug(
                    "Show progress Block Num %d Block Size %d  Total %d"
                    % (block_num, block_size, total_size)
                )
                self._logger.debug(
                    "Urlretrieve Progress %d%%"
                    % (((block_num * block_size) / total_size) * 100)
                )

        self._logger.debug("Copy Started %s %s" % (self.image_path, self.usb_dev))
        self._urlretrieve(progress_bar)
        self._logger.debug("Copy Ended")

    def _urlretrieve(self, reporthook=None):
        """
        Retrieves a URL into a temporary location on disk.

        Requires a URL argument. If a filename is passed, it is used as
        the temporary file location.

        The reporthook argument should be a callable that accepts a block
        number, a read size, and the total file size of the URL target.
        The data argument should be valid URL encoded data.
        Returns a tuple containing the path to the newly created
        data file as well as the resulting HTTPMessage object.
        """

        # Get the block size of the device so we can write in the same chunk size.
        bs = os.statvfs(self.usb_dev).f_bsize
        if bs <= 0:
            bs = 4096

        with contextlib.closing(
            requests.get(self.image_path, timeout=3000, stream=True)
        ) as resp:
            headers = resp.headers
            self._logger.debug("Block size %d", bs)
            self._logger.debug("Request Get Headers %s", headers)
            resp.raise_for_status()
            with open(self.usb_dev, "wb") as f:
                result = self.usb_dev, headers
                size = -1
                read = 0
                blocknum = 0
                if "content-length" in headers:
                    size = int(headers["Content-Length"])
                if reporthook:
                    reporthook(blocknum, bs, size)

                for block in resp.iter_content(chunk_size=bs):
                    if not block:
                        break
                    read += len(block)
                    f.write(block)
                    f.flush()
                    blocknum += 1
                    if reporthook:
                        reporthook(blocknum, bs, size)
                f.flush()
                self._logger.debug("Closing handle to block file")
            self._logger.debug("Closing urlopen")
        if size >= 0 and read < size:
            raise ContentTooShortError(
                "Retrieval incomplete: got only %i out of %i bytes" % (read, size),
                result,
            )
        self._logger.debug("All done....")
        return result


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--device",
        "-d",
        required=True,
        help="Destination drive path",
    )
    parser.add_argument(
        "--image_path",
        "-i",
        required=True,
        help="Image path on local system or http url",
    )
    logging.basicConfig(level=logging.DEBUG, format=LOG_FORMAT)
    args = parser.parse_args(argv)
    downloader = ImageDownloader(args.device, args.image_path)
    downloader.download()


if __name__ == "__main__":
    main(sys.argv[1:])
