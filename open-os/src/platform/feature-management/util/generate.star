# Copyright 2023 The ChromiumOS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions to generate proto payloads."""

def _generate_include(bundle, output = "output.txt"):
    """Serializes a Bundle to a file.

    A json proto is written. Note that there is some post processing done
    by the generate_include_file.sh script to convert this json output into a json
    output that uses ints for encoding enums.
    """

    def _generate_impl(ctx):
        ctx.output[output] = proto.to_wirepb(bundle)

    lucicfg.generator(impl = _generate_impl)

def _gen_file(content, output = "new_generated_file"):
    """Create a new file with open file name and free content.

    We can define the file name and file content freely.
    The new file will be created in the same directory of jsonproto files
    in each project repo. Example: project/fake/fake/generated/newfile
    """

    def _gen_file_impl(ctx):
        ctx.output[output] = content

    lucicfg.generator(impl = _gen_file_impl)

generate = struct(
    generate_include = _generate_include,
    gen_file = _gen_file,
)
