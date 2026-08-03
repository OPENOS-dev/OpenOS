// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// UniLib implementation with the help of ICU. UniLib is basically a wrapper
// around the ICU functionality.

#ifndef LIBTEXTCLASSIFIER_UTILS_UTF8_UNILIB_ICU_H_
#define LIBTEXTCLASSIFIER_UTILS_UTF8_UNILIB_ICU_H_

#include <cmath>
#include <functional>
#include <memory>
#include <mutex>  // NOLINT(build/c++11)

#include "utils/base/integral_types.h"
#include "utils/base/statusor.h"
#include "utils/utf8/unicodetext.h"
#include "unicode/brkiter.h"
#include "unicode/errorcode.h"
#include "unicode/regex.h"
#include "unicode/uchar.h"
#include "unicode/unum.h"

namespace libtextclassifier3 {

class UniLibBase {
 public:
  bool ParseInt32(const UnicodeText& text, int32* result) const;
  bool ParseInt64(const UnicodeText& text, int64* result) const;
  bool ParseDouble(const UnicodeText& text, double* result) const;

  bool IsOpeningBracket(char32 codepoint) const;
  bool IsClosingBracket(char32 codepoint) const;
  bool IsWhitespace(char32 codepoint) const;
  bool IsDigit(char32 codepoint) const;
  bool IsLower(char32 codepoint) const;
  bool IsUpper(char32 codepoint) const;
  bool IsPunctuation(char32 codepoint) const;

  char32 ToLower(char32 codepoint) const;
  char32 ToUpper(char32 codepoint) const;
  char32 GetPairedBracket(char32 codepoint) const;

  StatusOr<int32> Length(const UnicodeText& text) const;

  // Forward declaration for friend.
  class RegexPattern;

  class RegexMatcher {
   public:
    static constexpr int kError = -1;
    static constexpr int kNoError = 0;

    // Checks whether the input text matches the pattern exactly.
    bool Matches(int* status) const;

    // Approximate Matches() implementation implemented using Find(). It uses
    // the first Find() result and then checks that it spans the whole input.
    // NOTE: Unlike Matches() it can result in false negatives.
    // NOTE: Resets the matcher, so the current Find() state will be lost.
    bool ApproximatelyMatches(int* status);

    // Finds occurrences of the pattern in the input text.
    // Can be called repeatedly to find all occurrences. A call will update
    // internal state, so that 'Start', 'End' and 'Group' can be called to get
    // information about the match.
    // NOTE: Any call to ApproximatelyMatches() in between Find() calls will
    // modify the state.
    bool Find(int* status);

    // Gets the start offset of the last match (from  'Find').
    // Sets status to 'kError' if 'Find'
    // was not called previously.
    int Start(int* status) const;

    // Gets the start offset of the specified group of the last match.
    // (from  'Find').
    // Sets status to 'kError' if an invalid group was specified or if 'Find'
    // was not called previously.
    int Start(int group_idx, int* status) const;

    // Gets the end offset of the last match (from  'Find').
    // Sets status to 'kError' if 'Find'
    // was not called previously.
    int End(int* status) const;

    // Gets the end offset of the specified group of the last match.
    // (from  'Find').
    // Sets status to 'kError' if an invalid group was specified or if 'Find'
    // was not called previously.
    int End(int group_idx, int* status) const;

    // Gets the text of the last match (from 'Find').
    // Sets status to 'kError' if 'Find' was not called previously.
    UnicodeText Group(int* status) const;

    // Gets the text of the specified group of the last match (from 'Find').
    // Sets status to 'kError' if an invalid group was specified or if 'Find'
    // was not called previously.
    UnicodeText Group(int group_idx, int* status) const;

    // Returns the matched text (the 0th capturing group).
    std::string Text() const {
      std::string result;
      return text_.toUTF8String(result);
    }

   private:
    friend class RegexPattern;
    explicit RegexMatcher(icu::RegexPattern* pattern, icu::UnicodeString text);
    bool UpdateLastFindOffset() const;

    std::unique_ptr<icu::RegexMatcher> matcher_;
    icu::UnicodeString text_;
    mutable int last_find_offset_;
    mutable int last_find_offset_codepoints_;
    mutable bool last_find_offset_dirty_;
  };

  class RegexPattern {
   public:
    std::unique_ptr<RegexMatcher> Matcher(const UnicodeText& input) const;

   private:
    friend class UniLibBase;
    explicit RegexPattern(const UnicodeText& pattern, bool lazy = false);
    void LockedInitializeIfNotAlready() const;

    // These members need to be mutable because of the lazy initialization.
    // NOTE: The Matcher method first ensures (using a lock) that the
    // initialization was attempted (by using LockedInitializeIfNotAlready) and
    // then can access them without locking.
    mutable std::mutex mutex_;
    mutable bool initialized_;
    mutable bool initialization_failure_;
    mutable UnicodeText pattern_text_;
    mutable std::unique_ptr<icu::RegexPattern> pattern_;
  };

  class BreakIterator {
   public:
    int Next();

    static constexpr int kDone = -1;

   private:
    friend class UniLibBase;
    explicit BreakIterator(const UnicodeText& text);

   private:
    std::unique_ptr<icu::BreakIterator> break_iterator_;
    icu::UnicodeString text_;
    int last_break_index_;
    int last_unicode_index_;
  };

  std::unique_ptr<RegexPattern> CreateRegexPattern(
      const UnicodeText& regex) const;
  std::unique_ptr<RegexPattern> CreateLazyRegexPattern(
      const UnicodeText& regex) const;
  std::unique_ptr<BreakIterator> CreateBreakIterator(
      const UnicodeText& text) const;

 private:
  template <class T>
  bool ParseInt(const UnicodeText& text, T* result) const;
};

template <class T>
bool UniLibBase::ParseInt(const UnicodeText& text, T* result) const {
  // Fail fast if the text is unlikely to be a number (consistency with
  // javaicu).
  if (!PassesIntPreChesks(text, result)) {
    return false;
  }

  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UNumberFormat, std::function<void(UNumberFormat*)>>
      format_alias(
          unum_open(/*style=*/UNUM_DECIMAL, /*pattern=*/nullptr,
                    /*patternLength=*/0, /*locale=*/"en_US_POSIX",
                    /*parseErr=*/nullptr, &status),
          [](UNumberFormat* format_alias) { unum_close(format_alias); });
  if (U_FAILURE(status)) {
    return false;
  }
  icu::UnicodeString utf8_string = icu::UnicodeString::fromUTF8(
      icu::StringPiece(text.data(), text.size_bytes()));
  int parse_index = 0;

  // Using the unum_parseDouble here because unum_parse called for an input
  // like "1.23" returns 1, parse_index == utf8_string.length() and the status
  // is not error. Consequently there is no indication about floating numbers.
  const double double_parse =
      unum_parseDouble(format_alias.get(), utf8_string.getBuffer(),
                       utf8_string.length(), &parse_index, &status);
  // N.B., the `numeric_limits` numbers are casted because Clang emits errors
  // here for sufficiently wide types (e.g.,
  // "error: implicit conversion from 'type' (aka 'long long') to 'double'
  // changes value from 9223372036854775807 to 9223372036854775808").
  //
  // These checks are actually unnecessary, since we have
  // `*result != double_parse` below, which will verify that the double
  // conversion didn't result in an overflow.
  if (isnan(double_parse) || isinf(double_parse) ||
      double_parse > static_cast<double>(std::numeric_limits<T>::max()) ||
      double_parse < static_cast<double>(std::numeric_limits<T>::min())) {
    return false;
  }
  *result = std::trunc(double_parse);
  if (U_FAILURE(status) || parse_index != utf8_string.length() ||
      *result != double_parse) {
    return false;
  }
  return true;
}

}  // namespace libtextclassifier3

#endif  // LIBTEXTCLASSIFIER_UTILS_UTF8_UNILIB_ICU_H_
