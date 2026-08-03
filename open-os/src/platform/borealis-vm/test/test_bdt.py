# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for /usr/bin/bdt and /usr/lib/python3.*/bdt_lib."""

import hashlib
import json
import logging
import os.path
import random
import subprocess
import tempfile
import unittest
from unittest import mock

import bdt_lib  # pylint: disable=import-error


# Disable some pylint warnings that aren't useful for tests.
# pylint: disable=no-self-use  # For test cases which don't call methods.
# pylint: disable=no-value-for-parameter  # Because pylint doesn't understand mock.patch.
# pylint: disable=protected-access  # For overriding hidden things.
# pylint: disable=redefined-outer-name  # For "json" parameters, to match the API we're mocking.


class TestBDT(unittest.TestCase):
    """Test that required libraries are installed."""

    def test_bdt_tests(self):
        """Run /usr/bin/bdt's internal tests."""
        subprocess.check_call(["/usr/bin/bdt", "test", "--", "-b"])


class TestQuirks(unittest.TestCase):
    """Validate the quirks config shipped in the rootfs."""

    def test_quirks_config_exists(self):
        """Test the shipped quirks config exists."""
        self.assertTrue(os.path.isfile(bdt_lib.quirks.SHIPPED_QUIRKS_CONFIG))

    def test_quirks_config_is_valid(self):
        """Test the shipped quirks config is valid."""
        p = subprocess.run(
            [
                "/usr/bin/bdt",
                "validate-quirks",
                f"--config={bdt_lib.quirks.SHIPPED_QUIRKS_CONFIG}",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
        print(p.stdout)
        self.assertEqual(p.returncode, 0)


class TestUploader(bdt_lib.upload.Uploader):
    """Test-aware version of bdt_lib.Uploader."""

    def __init__(self):
        super().__init__(app_env="unittest")

    def read_auth_code(self, auth_url):
        """Skip opening the auth page and reading response code from user."""
        logging.info("Auth URL: %s", auth_url)
        return "fake-auth-code-from-oauth-success-page"


class FakeFlow:
    """Fake Flow class for `bdt login` testing."""

    def __init__(self):
        self.credentials = None

    def authorization_url(self, prompt):
        """Generate fake auth URL, which won't be used in the test."""
        return (f"http://example.com/the-prompt-was-{prompt}", None)

    def fetch_token(self, code):
        """Pretend to fetch a Google OAuth token."""

        class Credentials:
            """Fake Credentials."""

            def to_json(self):
                """JSONified fake credentials."""
                return json.dumps({"what": f"fake creds from code {code}"})

        self.credentials = Credentials()


class FakeResponse:
    """Fake HTTP response from a mocked server endpoint."""

    def __init__(self, data=None, status_code=200):
        self.data = {
            "code": 0,
            "msg": "fake response message",
        } | (data if data else {})
        self.status_code = status_code

    def json(self):
        """Accessor for JSONified data."""
        return self.data


class FakeSession:
    """Fake AuthorizedSession for `bdt report -u` testing."""

    # Counter for generated filenames.
    last_file_serial = 1

    def __init__(self):
        self.log = []
        self.files = {}

    def request(self, method, url, data=None, headers=None, json=None):
        """Fake an OAuth-protected HTTPS request."""
        self.log.append(
            {
                "method": method,
                "url": url,
                "data": data,
                "headers": headers,
                "json": json,
            }
        )
        if url.endswith("/new-upload"):
            return FakeResponse({"upload_id": "fake_upload"})
        if url.endswith("/add-file"):
            rj = {}
            if headers["X-Borealis-Upload-Type"] == "blobstore":
                # Hackily extract file content from the MultipartEncoder. Ideally we would
                # call data.to_string(), then decode the file content back out of the payload,
                # but as long as requests_toolbelt is doing its job, this is okay.
                file_content = data.fields["file"][1].read()
                # Blobstore uploads generate an MD5 hash.
                rj["md5_hash"] = hashlib.md5(file_content).hexdigest()
                stored_path = rj[
                    "path"
                ] = f"_blob/{FakeSession.last_file_serial}"
                FakeSession.last_file_serial += 1
            else:
                stored_path = headers["X-Borealis-Upload-Filename"]
                file_content = data
            self.files[stored_path] = file_content
            return FakeResponse(data=rj)
        if url.endswith("/prep-large-file"):
            return FakeResponse({"upload_url": "/add-file"})
        return FakeResponse()

    def get(self, url):
        """Fake an OAuth-protected GET request."""
        return self.request("GET", url)

    def post(self, url, data=None, headers=None, json=None):
        """Fake an OAuth-protected POST request."""
        return self.request("POST", url, data, headers, json)


class TestBDTUploader(unittest.TestCase):
    """Test bdt_lib.Uploader, the client to the Borealis log upload service."""

    def setUp(self):
        self.uploader = TestUploader()

    @mock.patch("google_auth_oauthlib.flow.InstalledAppFlow.from_client_config")
    def test_login(self, mock_from_client_config):
        """Test `bdt login`."""
        mock_from_client_config.return_value = FakeFlow()
        self.uploader.login()

    @mock.patch("google.auth.transport.requests.AuthorizedSession")
    def _upload(self, files, mock_session):
        """Start a session and upload some files."""
        session = FakeSession()
        mock_session.return_value = session
        with tempfile.TemporaryDirectory() as tempdir:
            # Make some minimal fake credentials for the upload.
            self.uploader.creds_path = f"{tempdir}/fake_creds.json"  # pylint: disable=attribute-defined-outside-init
            with open(self.uploader.creds_path, "w", encoding="utf-8") as f:
                json.dump(
                    {
                        "client_id": "fake-client-id",
                        "client_secret": "fake-client-secret",
                        "refresh_token": "fake-refresh-token",
                    },
                    f,
                )
            # Upload test_bdt.py and manifest.json.
            self.uploader.upload_report(files=files, steam_id=1234)
        return session

    def test_upload(self):
        """Test `bdt report --upload`."""
        session = self._upload([__file__])

        # Verify that the right URLs were requested.
        self.assertEqual(
            [
                ["POST", "hello"],
                ["POST", "new-upload"],
                ["POST", "add-file"],
                ["POST", "add-file"],
                ["POST", "finish-upload"],
            ],
            [[log["method"], log["url"].split("/")[-1]] for log in session.log],
        )

        # Verify that we have the expected files.
        self.assertEqual(
            {"test_bdt.py", "manifest.json"}, set(session.files.keys())
        )

    def test_file_sizes(self):
        """Test uploading files of various sizes."""
        with tempfile.TemporaryDirectory() as tempdir:
            tempfn = f"{tempdir}/test.bin"
            for size in (
                0,
                1,
                1024,
                bdt_lib.upload.LARGE_FILE_THRESHOLD - 1,
                bdt_lib.upload.LARGE_FILE_THRESHOLD,
                bdt_lib.upload.LARGE_FILE_THRESHOLD + 1,
                100 * 1024 * 1024,
            ):
                with self.subTest(msg=f"Testing a {size} B file"):
                    # Write out and upload a random file of the desired size.
                    # pylint: disable=no-member
                    content = random.randbytes(size)
                    # pylint: enable=no-member
                    with open(tempfn, "wb") as f:
                        f.write(content)
                    session = self._upload([tempfn])

                    # Find the manifest entry for the single file.
                    manifest = json.loads(session.files["manifest.json"])
                    (entry,) = manifest

                    # Verify reported size.
                    self.assertEqual(size, entry["size"])

                    # Verify reported hash.
                    self.assertEqual(
                        hashlib.md5(content).hexdigest(), entry["hash_md5"]
                    )
                    self.assertEqual(
                        hashlib.blake2b(content).hexdigest(),
                        entry["hash_blake2b"],
                    )

                    # Verify uploaded content.
                    self.assertEqual(
                        content, session.files[entry["stored_path"]]
                    )


if __name__ == "__main__":
    unittest.main()
