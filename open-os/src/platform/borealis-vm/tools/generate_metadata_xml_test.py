#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for generate_metadata_xml.py."""

import io
import textwrap
import unittest

import generate_metadata_xml


class TestParsing(unittest.TestCase):
    """Test parsing existing metadata.xml files."""

    def test_get_tags_from_metadata_xml(self):
        """Verify that we can parse existing metadata.xml files."""
        sio = io.StringIO(
            textwrap.dedent(
                """\
                    <?xml version="1.0" encoding="UTF-8"?>
                    <!DOCTYPE pkgmetadata SYSTEM "https://www.gentoo.org/dtd/metadata.dtd">
                    <pkgmetadata>
                        <maintainer type="project">
                            <email>test@google.com</email>
                        </maintainer>
                        <upstream>
                            <remote-id type="cpe">cpe:/a:acl_project:acl:2.3.1</remote-id>
                            <remote-id type="cpe">cpe:/a:airtable:airtable:1.8.10</remote-id>
                            <remote-id type="cpe">cpe:/a:alsa-project:alsa-lib:1.2.10</remote-id>
                        </upstream>
                    </pkgmetadata>
                """
            )
        )
        tags = generate_metadata_xml.get_tags_from_metadata_file(sio)
        self.assertEqual(
            set(
                [
                    "cpe:/a:acl_project:acl:2.3.1",
                    "cpe:/a:airtable:airtable:1.8.10",
                    "cpe:/a:alsa-project:alsa-lib:1.2.10",
                ]
            ),
            tags,
        )


if __name__ == "__main__":
    unittest.main()
