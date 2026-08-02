// Copyright 2020 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TOKEN_REPLACER_H__
#define __TOKEN_REPLACER_H__

#include <iostream>
#include <regex>
#include <string>

class TokenReplacer {
 public:
  TokenReplacer() = delete;
  TokenReplacer(const std::string& host,
                const std::string& user,
                const std::string& title,
                const std::string& copies);

  // For testing purposes
  const std::string& GetTitle() const { return title_; };

  // Replaces tokens in the @PJL section of the print job as follows:
  // 1) GETMYHOST is replaced with host
  // 2) GEYMYUSERNAME is replaced with user
  // 3) GETMYJOBNAME is replaced with an escaped title
  // 4) GETMYCOPIES is replaced with copies
  const std::string TokenizeLine(std::string line) const;

 private:
  // Escape a string according to the following rules (see myjob in
  // fax-pnh-filter for the rules):
  // 1) One or two backslashes in sequence are replaced with one backslash
  // 2) double-quotes are replaced with single-quotes
  //
  // Note that fax-pnh-filter has additional rules for ampersands & slashes, but
  // these are only to deal with the fact that the title string is being passed
  // to sed on the command-line, so those characters are being escaped because
  // they would be interpreted on the command-line as bash control characters.
  // The outputted title from the filter would revert \& and \/ to & and /
  // respectively. Therefore there is no need to escape those characters here.
  static std::string EscapeTitle(std::string title);

  const std::string title_;
  const std::regex re_host_;
  const std::string host_replacement_;
  const std::regex re_user_;
  const std::string user_replacement_;
  const std::regex re_title_;
  const std::string title_replacement_;
  const std::regex re_copies_;
  const std::string copies_replacement_;
};

void transform(const TokenReplacer& replacer,
               std::istream& in,
               std::ostream& out);

#endif // __TOKEN_REPLACER_H__