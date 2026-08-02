# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math
import re

class Table(object):
  """
  This class allows you to format tables for console output.
  Example:
      table = Table(prefix="    ")
      table.header("equation", "result")
      table.row("1+2", "3")
      table.row("21*2", "42")
      print table
  The column width will automatically be determined to be fitting for every
  cell.
  """

  def __init__(self, prefix=""):
    """
    creates an empty table. Prefix will be printed before any new line
    of the table (i.e. it can be used to indent the table)
    """
    self.prefix = prefix
    self.title = None
    self.footer = None
    self.entries = []

  def header(self, *args):
    """
    Add a new table header row. Every parameter will be the title of one
    cell. Equivalent of <tr><th>arg1</th><th>arg2</th>...</tr>
    """
    self.entries.append(("header", args))

  def row(self, *args):
    """
    Add a new table data row. Every parameter will be the title of one
    cell. Equivalent of <tr><td>arg1</td><td>arg2</td>...</tr>
    """
    self.entries.append(("row", args))

  def __str__(self):
    """
    format the table using all entries added using header/row.
    """

    # The entries are structured in the following way:
    # self.entries = [
    #     ("header", ["header1", "header2", ...]),
    #     ("row", ["cell1", "cell2", ...]),
    #     ("row", ["cell1", "cell2", ...])
    # ]

    # calculate dimensions and number of columns
    column_count = max([len(entry[1]) for entry in self.entries])
    column_sizes = []
    for i in range(column_count):
      column_size = 2 + max([len(str(entry[1][i]))
                             for entry in self.entries
                             if i < len(entry[1])])
      column_sizes.append(column_size)

    # overall width of table including all decorations
    width = 4 + sum(column_sizes) + 3 * (column_count - 1);

    # format strings for all parts of the table
    title_format = self.prefix + "{0:^%d}" % width
    footer_format = (self.prefix + "| {0:^%d} |" % (width - 4) + "\n" +
                     self.prefix + "+-" + ("-" * (width - 4)) + "-+")

    th_format = (self.prefix + "+-" +
                 "-+-".join(["{%d:-^%d}" % (i, c)
                             for i, c in enumerate(column_sizes)]) +
                 "-+")
    td_format = (self.prefix + "| " +
                 " | ".join(["{%d:<%d}" % (i, c)
                             for i, c in enumerate(column_sizes)]) +
                 " |\x1b[0m")
    tf_format = (self.prefix + "+-" +
                 "-+-".join(["-"*c for c in column_sizes]) +
                 "-+")

    # build table from entries
    parts = []

    if self.title:
      parts.append(title_format.format(self.title))

    for entry in self.entries:
      if entry[0] == "header":
        format = th_format
      if entry[0] == "row":
        format = td_format

      # pad every element with one space
      data = []
      color = "0"
      for i in range(column_count):
        if i < len(entry[1]):
          value = str(entry[1][i])

          # add padding for console escape characters
          value = re.sub("(\x1b\[.m)",   "     \\1", value)
          value = re.sub("(\x1b\[..m)",  "    \\1", value)
          value = re.sub("(\x1b\[...m)", "   \\1", value)

          if "success" in value:
            color = "32"
          elif "failure" in value:
            color = "31"
          elif "bad" in value:
            color = "33"
          elif "disabled" in value or "incomplete" in value:
            color = "34"

          data.append(" " + value + " ")
        else:
          data.append("")

      line = format.format(*data)
      if color != "0":
        line = line.replace("|", "\x1b[m|\x1b[" + color + "m")

      parts.append(line)

    parts.append(tf_format)

    if self.footer:
      parts.append(footer_format.format(self.footer))

    return "\n".join(parts)

