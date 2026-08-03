// Copyright 2020 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "token_replacer.h"

TokenReplacer::TokenReplacer(const std::string& host,
                             const std::string& user,
                             const std::string& title,
                             const std::string& copies)
  : title_(EscapeTitle(title)),
    re_host_("(STATIONID|PJL(.+?HOSTID)) = GETMYHOST"),
    host_replacement_(R"($1 = ")" + host + R"(")"),
    // GEYMYUSERNAME is not a typo (see fax-pnh-filter)
    re_user_("(PJL (SET USERNAME|(.+?USERID))) = GEYMYUSERNAME"),
    user_replacement_(R"($1 = ")" + user + R"(")"),
    re_title_("(PJL SET JOBNAME) = GETMYJOBNAME"),
    title_replacement_(R"($1 = ")" + title_ + R"(")"),
    re_copies_("(PJL SET QTY) = GETMYCOPIES"),
    copies_replacement_("$1 = " + copies ) {}

const std::string TokenReplacer::TokenizeLine(std::string line) const {
  line = std::regex_replace(line, re_host_, host_replacement_);
  line = std::regex_replace(line, re_user_, user_replacement_);
  line = std::regex_replace(line, re_title_, title_replacement_);
  line = std::regex_replace(line, re_copies_, copies_replacement_);
  return line;
}

std::string TokenReplacer::EscapeTitle(std::string title) {
  // Replace each one or two backslash sequence with one backslash
  std::regex slash(R"(\\{1,2})");
  title = std::regex_replace(title, slash, R"(\)");

  // Replace double-quotes with single-quotes
  std::regex quote(R"(")");
  title = std::regex_replace(title, quote, "'");

  return title;
}

// Read in the file, line-by-line, apply the transformation to each line, and
// then write the line to out.
void transform(const TokenReplacer& replacer,
               std::istream& in,
               std::ostream& out) {
  std::string line;
  while(std::getline(in, line)) {
    out << replacer.TokenizeLine(line) << std::endl;
  }
}
