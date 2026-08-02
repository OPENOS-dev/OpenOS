// Copyright 2020 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "token_replacer.h"

#include <gtest/gtest.h>

namespace {
constexpr char kHostName[] = "hostname";
constexpr char kUserName[] = "user@host.com";
constexpr char kTitle[] = R"("This" & /that\\)";
constexpr char kCopies[] = "42";
} // namespace

class TokenReplacerTest : public testing::Test {
 protected:
  TokenReplacerTest() : replacer_(kHostName, kUserName, kTitle, kCopies) {}

  TokenReplacer replacer_;
};

TEST(Title, NoChange) {
  TokenReplacer replacer("unused", "unused", "unchanged", "unused");

  EXPECT_EQ(replacer.GetTitle(), "unchanged");
}

// One backslash => one backslash
TEST(Title, Backslash) {
  TokenReplacer replacer("unused", "unused", R"(this \ that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), R"(this \ that)");
}

// Two backslashes => one backslash
TEST(Title, Backslash2) {
  TokenReplacer replacer("unused", "unused", R"(this \\ that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), R"(this \ that)");
}

// Three backslashes => two backslashes
TEST(Title, Backslash3) {
  TokenReplacer replacer("unused", "unused", R"(this \\\ that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), R"(this \\ that)");
}

// Four backslashes => two backslashes
TEST(Title, Backslash4) {
  TokenReplacer replacer("unused", "unused", R"(this \\\\ that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), R"(this \\ that)");
}

// Five backslashes => three backslashes
TEST(Title, Backslash5) {
  TokenReplacer replacer("unused", "unused", R"(this \\\\\ that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), R"(this \\\ that)");
}

TEST(Title, Ampersand) {
  TokenReplacer replacer("unused", "unused", "this & that", "unused");

  EXPECT_EQ(replacer.GetTitle(), "this & that");
}

TEST(Title, DoubleQuotes) {
  TokenReplacer replacer("unused", "unused", R"("this" and that)", "unused");

  EXPECT_EQ(replacer.GetTitle(), "'this' and that");
}

TEST(Title, ForwardSlash) {
  TokenReplacer replacer("unused", "unused", "this / that", "unused");

  EXPECT_EQ(replacer.GetTitle(), "this / that");
}

TEST(Title, Transform) {
  TokenReplacer replacer("hostname", "user@host.com", "this / that", "42");
  std::istringstream input(
    "@PJL SET STATIONID = GETMYHOST\n"
    "@PJL SET USERNAME = GEYMYUSERNAME\n"
    "@PJL SET JOBNAME = GETMYJOBNAME\n"
    "@PJL SET QTY = GETMYCOPIES\n"
  );
  std::ostringstream output;
  std::string expected =
    "@PJL SET STATIONID = \"hostname\"\n"
    "@PJL SET USERNAME = \"user@host.com\"\n"
    "@PJL SET JOBNAME = \"this / that\"\n"
    "@PJL SET QTY = 42\n";

  transform(replacer, input, output);
  EXPECT_EQ(output.str(), expected);
}

TEST_F(TokenReplacerTest, UnchangedLine) {
  EXPECT_EQ(replacer_.TokenizeLine("Unchanged"), "Unchanged");
}

TEST_F(TokenReplacerTest, ChangedHost) {
  EXPECT_EQ(replacer_.TokenizeLine(
      "@PJL SET STATIONID = GETMYHOST"),
    R"(@PJL SET STATIONID = "hostname")");
}

TEST_F(TokenReplacerTest, ChangedUser) {
  EXPECT_EQ(replacer_.TokenizeLine(
      "@PJL SET USERNAME = GEYMYUSERNAME"),
    R"(@PJL SET USERNAME = "user@host.com")");
}

TEST_F(TokenReplacerTest, ChangedTitle) {
  EXPECT_EQ(replacer_.TokenizeLine(
      "@PJL SET JOBNAME = GETMYJOBNAME"),
    R"(@PJL SET JOBNAME = "'This' & /that\")");
}

TEST_F(TokenReplacerTest, ChangedCopies) {
  EXPECT_EQ(replacer_.TokenizeLine(
    "@PJL SET QTY = GETMYCOPIES"),
    "@PJL SET QTY = 42");
}

TEST_F(TokenReplacerTest, ChangedJobInfo) {
  EXPECT_EQ(replacer_.TokenizeLine(
      "@PJL LJOBINFO USERID = GEYMYUSERNAME HOSTID = GETMYHOST"),
    R"(@PJL LJOBINFO USERID = "user@host.com" HOSTID = "hostname")");
}
