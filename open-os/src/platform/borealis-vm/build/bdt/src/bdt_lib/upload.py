# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Upload game test results to Borealis developer storage."""

import hashlib
import json
import os
import stat
import subprocess
import sys
import tempfile
import time
import traceback

# pylint: disable=import-error
import google.auth.transport.requests
import google.oauth2.credentials
import google_auth_oauthlib
import requests
from requests_toolbelt.multipart.encoder import MultipartEncoder


# pylint: enable=import-error


# App Engine rejects requests larger than 32MB, so chunk files
# to keep below this.
CHUNKSIZE = 30 * 1024 * 1024

# Upload anything larger than this using the App Engine Blobstore,
# which requires an extra request but has much better performance.
LARGE_FILE_THRESHOLD = 512 * 1024


class UploadError(Exception):
    """Exception occurring during a bdt upload session."""


class Uploader:
    """Client for the Borealis log uploader."""

    # OAuth scopes to request.
    SCOPES = ["https://www.googleapis.com/auth/userinfo.email"]

    def __init__(self, app_env="prod"):
        self.creds = None

        # Which app to contact.
        self.app_env = os.environ.get("LOG_UPLOADER_ENV", app_env)

        # Cloud Project ID for this app
        self.project_id = f"borealis-log-uploader-{self.app_env}"

        # Receiver app URL.
        self.server = f"https://{self.project_id}.appspot.com"

        # Where to store OAuth credentials.
        self.creds_path = (
            f'{os.environ["HOME"]}/.borealis-uploader-credentials.json'
        )

        # OAuth-authenticated session, used to communicate with the receiver app.
        self.oauth_session = None

        # Uploaded report ID on receiver app.
        self.upload_id = None

    def enable_debugging(self):  # pylint: disable=no-self-use
        """Enable verbose HTTP/OAuth debugging."""
        # pylint: disable=import-outside-toplevel
        import http.client
        import logging

        # pylint: enable=import-outside-toplevel
        logging.basicConfig()
        logging.getLogger().setLevel(logging.DEBUG)
        http.client.HTTPConnection.debuglevel = 1

    def login(self):  # pylint: disable=no-self-use
        """Log in to Google and get OAuth credentials for the uploader app."""

        print("Logging in to the upload server.\n")

        # Set up OAuth flow.
        client_config = {
            "installed": {
                "project_id": self.project_id,
                "auth_uri": "https://accounts.google.com/o/oauth2/auth",
                "token_uri": "https://oauth2.googleapis.com/token",
                "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
                "redirect_uris": ["http://localhost"],
                # Note that despite the name "client_secret", these are "installed app"
                # credentials, and hence safe to include directly in source code.
                "client_id": (
                    "663895284698-sunqvbqr6bf3br1m2hij5a0olnsebgea"
                    ".apps.googleusercontent.com"
                ),
                "client_secret": "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",
            }
        }

        appflow = google_auth_oauthlib.flow.InstalledAppFlow.from_client_config(
            client_config,
            scopes=self.SCOPES,
            redirect_uri="urn:ietf:wg:oauth:2.0:oob",
        )

        # Fetch and open the authorization URL.
        auth_url, _ = appflow.authorization_url(prompt="consent")

        # The user will get an authorization code. This code is used to get the
        # access token.
        code = self.read_auth_code(auth_url)

        # Don't warn if Google gives us the 'openid' permission as well as 'userinfo.email'.
        os.environ["OAUTHLIB_RELAX_TOKEN_SCOPE"] = "1"

        appflow.fetch_token(code=code)
        creds = appflow.credentials
        with open(self.creds_path, "w", encoding="utf-8") as f:
            f.write(creds.to_json())
            print(f"Credentials written to {self.creds_path}")

    def read_auth_code(self, auth_url):  # pylint: disable=no-self-use
        """Open auth URL from the server, and request auth code from the user."""
        # This is in a function so it can be stubbed out for testing.
        try:
            subprocess.check_call(["/usr/bin/garcon-url-handler", auth_url])
        except subprocess.CalledProcessError:
            traceback.print_exc()
            print(
                "\n\n"
                "Failed to open browser; please open this URL and"
                " follow the auth flow to get a code:\n"
                "\n"
                f"  {auth_url}\n"
            )
        return input("Enter the authorization code: ")

    def load_creds(self):
        """Load OAuth credentials if necessary."""

        if self.creds:
            return

        if not os.path.exists(self.creds_path):
            raise UploadError(
                f"No credentials found in {self.creds_path}; "
                "please run bdt login."
            )

        print(f"Loading uploader credentials from {self.creds_path}")
        self.creds = (
            google.oauth2.credentials.Credentials.from_authorized_user_file(
                self.creds_path, scopes=self.SCOPES
            )
        )

    def _post(self, context, *args, **kwargs):
        """Make an OAuth-authorized HTTPS request and handle common errors."""
        r = self.oauth_session.post(*args, **kwargs)
        if r.status_code != 200:
            print(repr(r))
            raise UploadError(
                f"{context}: Failed with HTTP response {r.status_code}"
            )
        try:
            rj = r.json()
        except requests.exceptions.JSONDecodeError as e:
            raise UploadError(
                f"{context}: Failed to decode JSON response {r.text}"
            ) from e
        print(rj)
        if rj["code"] != 0:
            raise UploadError(f'{context}: Failed: {rj["message"]}')
        return rj

    def _start_oauth_session(self):
        """Test credentials and start an OAuth-authorized session."""

        self.load_creds()

        self.oauth_session = google.auth.transport.requests.AuthorizedSession(
            self.creds
        )

        print(f"Testing that we can talk to the upload server ({self.server}).")
        self._post("Pinging server", f"{self.server}/hello")

    def _upload_file(self, full_path):
        """Upload a single file and return metadata."""

        # Assume that only the filename, not the path, is important.
        filename = os.path.basename(full_path)

        file_size = os.stat(full_path)[stat.ST_SIZE]
        print(
            f"Uploading file {full_path} as {filename} ({file_size} bytes)..."
        )

        # Time the entire upload process, including multiple requests.
        start_time = time.time()

        # Hash the data as we go.
        hash_md5 = hashlib.md5()
        hash_blake2b = hashlib.blake2b()

        if file_size > LARGE_FILE_THRESHOLD:
            # For files that'll take more than a second to upload, use the
            # blobstore upload method.
            print("Getting large file upload URL.")
            resp = self._post(
                "Getting large file URL",
                f"{self.server}/prep-large-file",
                headers={
                    "Content-Type": "application/json",
                    "X-Borealis-Upload-Id": self.upload_id,
                    "X-Borealis-Upload-Filename": filename,
                },
            )
            upload_url = resp["upload_url"]

            # Send the whole file in one go, to the App Engine blobstore endpoint.
            print("Uploading file.")
            with open(full_path, "rb") as f:
                mp_encoder = MultipartEncoder(
                    fields={
                        "file": (
                            filename,
                            f,
                            "application/octet-stream",
                        )
                    }
                )
                resp = self._post(
                    "Uploading large file",
                    upload_url,
                    headers={
                        "Content-Type": mp_encoder.content_type,
                        "X-Borealis-Upload-Type": "blobstore",
                        "X-Borealis-Upload-Id": self.upload_id,
                        "X-Borealis-Upload-Filename": filename,
                    },
                    data=mp_encoder,
                )
                stored_path = resp["path"]

            # We reread the entire file to generate hashes; this is inefficient
            # but more straightforward than making a 'tee' file object to grab
            # the bytes as the MultipartEncoder above reads them.
            print("Hashing data")
            with open(full_path, "rb") as f:
                while True:
                    chunk_bytes = f.read(CHUNKSIZE)
                    if not chunk_bytes:
                        break
                    hash_blake2b.update(chunk_bytes)
                    hash_md5.update(chunk_bytes)
            if hash_md5.hexdigest() != resp["md5_hash"]:
                raise UploadError(
                    f'MD5 {resp["md5_hash"]} calculated by server '
                    f"does not match local hash {hash_md5.hexdigest()}"
                )
        else:
            # File is small enough to upload in a single request.
            with open(full_path, "rb") as f:
                start_time = time.time()
                data = f.read()
                self._post(
                    "Uploading small file",
                    f"{self.server}/add-file",
                    headers={
                        "Content-Type": "application/octet-stream",
                        "X-Borealis-Upload-Type": "direct",
                        "X-Borealis-Upload-Id": self.upload_id,
                        "X-Borealis-Upload-Filename": filename,
                    },
                    data=data,
                )
                stored_path = filename
                hash_blake2b.update(data)
                hash_md5.update(data)

        time_taken = time.time() - start_time
        mib_s = file_size / 1024 / 1024 / time_taken
        print(
            f"Uploaded {file_size} bytes in {time_taken:.2f} s; {mib_s:.2f} MiB/s"
        )

        return {
            "path": full_path,
            "size": file_size,
            "stored_path": stored_path,
            "hash_blake2b": hash_blake2b.hexdigest(),
            "hash_md5": hash_md5.hexdigest(),
        }

    def _upload_report(self, files, steam_id, comment):
        self._start_oauth_session()

        print("Starting upload.")
        metadata = {"steam_id": str(steam_id) if steam_id else "Unavailable"}
        if comment:
            metadata["comment"] = comment
        resp = self._post(
            "Setting up new upload", f"{self.server}/new-upload", json=metadata
        )
        self.upload_id = resp["upload_id"]

        print(f"Upload ID on remote server: {self.upload_id}")

        # Upload all files, broken into chunks as necessary, then upload a manifest showing which
        # files need to be reassembled later.
        manifest = [self._upload_file(fn) for fn in files]

        # Write out JSON manifest and upload it.
        with tempfile.TemporaryDirectory() as tempdir:
            manifest_fn = f"{tempdir}/manifest.json"
            with open(manifest_fn, "w", encoding="utf-8") as f:
                json.dump(manifest, f)
            self._upload_file(manifest_fn)

        print("Ending upload session")
        self._post(
            "Finalizing upload",
            f"{self.server}/finish-upload",
            headers={
                "X-Borealis-Upload-Id": self.upload_id,
            },
        )

    def upload_report(self, files, steam_id, comment=None):
        """Upload a BDT report folder."""

        try:
            self._upload_report(files, steam_id, comment)
        except:  # pylint: disable=bare-except
            print("*" * 80)
            print(
                "The report could not be sent; please try again.  The following error occurred:"
            )
            print("*" * 80)
            traceback.print_exc(file=sys.stdout)
            print("*" * 80)
