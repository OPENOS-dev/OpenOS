#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Update and verify cros_borealis Archlinux DB files."""

import argparse
import json
import logging
import os
import re
import subprocess
import textwrap
from typing import List, Optional, Set, TextIO
from xml.etree import ElementTree


logger = logging.getLogger("generate_metadata_xml")


def get_arch_packages(container_name: str) -> dict:
    """Get a list of Arch packages installed on a given Docker container."""
    packages = {}
    logger.info("Getting packages list from container: %s", container_name)
    result = subprocess.check_output(
        ["sudo", "docker", "run", container_name, "pacman", "-Q"]
    )
    pacman_output = result.decode("utf-8").strip()

    for line in pacman_output.splitlines():
        package, version = line.split(" ")
        # Trim lib32- prefix for 32-bit versions of packages.
        package = re.sub("^lib32-", "", package)
        # Trim -debug suffix for debug symbol packages.
        package = re.sub("-debug$", "", package)
        # Trim Arch epoch from package version.
        version = re.sub(r"^\d+:", "", version)
        # Trim Arch release count from package version.
        version = re.sub(r"\-\d+$", "", version)
        assert ":" not in version, (
            f"Version {version} for package {package} contains"
            " embedded : character that is invalid in a CPE tag"
        )
        packages[package] = version

    return packages


def get_tags_from_metadata_file(metadata_xml_file: TextIO) -> Set[str]:
    """Extract CPE tags with versions from a metadata.xml file."""
    tags = set()
    tree = ElementTree.parse(metadata_xml_file)
    for element in tree.findall("./upstream/remote-id[@type='cpe']"):
        tags.add(element.text)
    return tags


def generate_metadata_xml(
    docker_image: str,
    exclude_existing_metadata_paths: Optional[List[str]] = None,
) -> bytes:
    """Generate metadata.xml file listing packages in the Docker image.

    Args:
        docker_image: Docker image containing packages to list.
        exclude_existing_metadata_paths: Optional paths to existing
            metadata.xml; if provided, exclude any tags that are present
            in these files.
    """

    # Extract package list from Docker image.
    arch_packages = get_arch_packages(docker_image)

    # Read known package-to-cpe mappings.
    with open(
        os.path.join(os.path.dirname(__file__), "cpe", "cpe_mapping.json"),
        "r",
        encoding="utf-8",
    ) as f:
        cpe_by_package = json.load(f)

    # Read existing metadata.xml files, if given.
    exclude_tags = set()
    exclude_all_tags = False
    if exclude_existing_metadata_paths:
        for mp in exclude_existing_metadata_paths:
            if mp == "*":
                # Magic path "*" means exclude everything.
                exclude_all_tags = True
            else:
                with open(mp, "r", encoding="utf-8") as mf:
                    exclude_tags |= get_tags_from_metadata_file(mf)

    todo = set()  # Packages missing from cpe_mapping.json.
    cpe_tags = set()  # Tags to write into metadata.xml.
    unknown_package_count = 0  # Packages with "null" in cpe_mapping.json.

    for package, version in sorted(arch_packages.items()):
        if package in cpe_by_package:
            known_tags = cpe_by_package[package]
            if known_tags:
                # We have one or more CPE tags for this package.
                for cpe in (
                    [known_tags] if isinstance(known_tags, str) else known_tags
                ):
                    cpe_tags.add(f"{cpe}:{version}")
            else:
                # This package has a null entry in cpe_mapping.json.
                unknown_package_count += 1
        else:
            logger.warning("Need CPE mapping for %s", package)
            todo.add(package)

    # Build metadata.xml tree.
    xml_prefix = textwrap.dedent(
        """\
            <?xml version="1.0" encoding="UTF-8"?>
            <!DOCTYPE pkgmetadata SYSTEM "https://www.gentoo.org/dtd/metadata.dtd">
        """
    )
    root = ElementTree.fromstring(
        textwrap.dedent(
            """\
                <pkgmetadata>
                    <maintainer type="project">
                        <email>chromeos-gaming-core@google.com</email>
                    </maintainer>
                    <upstream>
                    </upstream>
                </pkgmetadata>
            """
        )
    )
    upstream = root.find("upstream")

    # Pretty-print: indent each <remote-id/>.
    WHITESPACE_BETWEEN_ELEMENTS = "\n        "
    upstream.text = WHITESPACE_BETWEEN_ELEMENTS

    if not exclude_all_tags:
        # Add a <remote-id/> for each CPE tag.
        for cpe in sorted(cpe_tags - exclude_tags):
            # <remote-id type="cpe">cpe:/a:valvesoftware:steam_client:1.2.3</remote-id>
            remote_id = ElementTree.SubElement(
                upstream, "remote-id", {"type": "cpe"}
            )
            remote_id.text = cpe
            remote_id.tail = WHITESPACE_BETWEEN_ELEMENTS

    # Properly indent the </upstream> tag.
    if len(upstream):
        upstream[-1].tail = upstream[-1].tail[:-4]
    else:
        upstream.text = upstream.text[:-4]

    logger.info(
        "%d packages with tags, %d packages with no tags",
        len(arch_packages) - unknown_package_count,
        unknown_package_count,
    )

    if todo:
        logger.info(
            "%d packages were not found in cpe_mapping.json: %s",
            len(todo),
            " ".join(sorted(todo)),
        )

    metadata_xml = (
        xml_prefix.encode("utf-8") + ElementTree.tostring(root) + b"\n"
    )

    return metadata_xml, todo


def main() -> None:
    """Main function."""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--image",
        default="cros_borealis",
        help="Docker image for Borealis rootfs",
    )
    parser.add_argument("--output", "-o", help="Path to XML output file")
    parser.add_argument(
        "--missing-output", help="Path to missing mappings output file"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Make more noise"
    )
    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.INFO)

    metadata_xml, missing_mappings = generate_metadata_xml(args.image)

    # Write metadata.xml if we have a path, or print to stdout.
    if args.output:
        with open(args.output, "wb") as f:
            f.write(metadata_xml)
    else:
        print(metadata_xml.decode("utf-8"))

    # Write packages with missing mappings if we have a path to write to.
    if args.missing_output:
        with open(args.missing_output, "w", encoding="utf-8") as f:
            f.write("".join(f"{package}\n" for package in missing_mappings))


if __name__ == "__main__":
    logging.basicConfig(level=logging.WARNING)
    main()
