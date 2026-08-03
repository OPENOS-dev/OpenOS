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

#include "utils/utf8/unilib-icu.h"

#include <functional>
#include <utility>

#include "utils/base/logging.h"
#include "utils/base/statusor.h"
#include "utils/utf8/unilib-common.h"
#include "unicode/unistr.h"
#include "unicode/utext.h"

namespace libtextclassifier3 {

bool UniLibBase::ParseInt32(const UnicodeText& text, int32* result) const {
  return ParseInt(text, result);
}

bool UniLibBase::ParseInt64(const UnicodeText& text, int64* result) const {
  return ParseInt(text, result);
}

bool UniLibBase::ParseDouble(const UnicodeText& text, double* result) const {
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

  auto it_dot = text.begin();
  for (; it_dot != text.end() && !IsDot(*it_dot); it_dot++) {
  }

  int64 integer_part;
  if (!ParseInt(UnicodeText::Substring(text.begin(), it_dot, /*do_copy=*/false),
                &integer_part)) {
    return false;
  }

  int64 fractional_part = 0;
  if (it_dot != text.end()) {
    if (!ParseInt(
            UnicodeText::Substring(++it_dot, text.end(), /*do_copy=*/false),
            &fractional_part)) {
      return false;
    }
  }

  double factional_part_double = fractional_part;
  while (factional_part_double >= 1) {
    factional_part_double /= 10;
  }
  *result = integer_part + factional_part_double;

  return true;
}

bool UniLibBase::IsOpeningBracket(char32 codepoint) const {
  return u_getIntPropertyValue(codepoint, UCHAR_BIDI_PAIRED_BRACKET_TYPE) ==
         U_BPT_OPEN;
}

bool UniLibBase::IsClosingBracket(char32 codepoint) const {
  return u_getIntPropertyValue(codepoint, UCHAR_BIDI_PAIRED_BRACKET_TYPE) ==
         U_BPT_CLOSE;
}

bool UniLibBase::IsWhitespace(char32 codepoint) const {
  return u_isWhitespace(codepoint);
}

bool UniLibBase::IsDigit(char32 codepoint) const {
  return u_isdigit(codepoint);
}

bool UniLibBase::IsLower(char32 codepoint) const {
  return u_islower(codepoint);
}

bool UniLibBase::IsUpper(char32 codepoint) const {
  return u_isupper(codepoint);
}

bool UniLibBase::IsPunctuation(char32 codepoint) const {
  return u_ispunct(codepoint);
}

char32 UniLibBase::ToLower(char32 codepoint) const {
  return u_tolower(codepoint);
}

char32 UniLibBase::ToUpper(char32 codepoint) const {
  return u_toupper(codepoint);
}

char32 UniLibBase::GetPairedBracket(char32 codepoint) const {
  return u_getBidiPairedBracket(codepoint);
}

StatusOr<int32> UniLibBase::Length(const UnicodeText& text) const {
  return icu::UnicodeString::fromUTF8({text.data(), text.size_bytes()})
      .countChar32();
}

UniLibBase::RegexMatcher::RegexMatcher(icu::RegexPattern* pattern,
                                       icu::UnicodeString text)
    : text_(std::move(text)),
      last_find_offset_(0),
      last_find_offset_codepoints_(0),
      last_find_offset_dirty_(true) {
  UErrorCode status = U_ZERO_ERROR;
  matcher_.reset(pattern->matcher(text_, status));
  if (U_FAILURE(status)) {
    matcher_.reset(nullptr);
  }
}

UniLibBase::RegexPattern::RegexPattern(const UnicodeText& pattern, bool lazy)
    : initialized_(false),
      initialization_failure_(false),
      pattern_text_(pattern) {
  if (!lazy) {
    LockedInitializeIfNotAlready();
  }
}

void UniLibBase::RegexPattern::LockedInitializeIfNotAlready() const {
  std::lock_guard<std::mutex> guard(mutex_);
  if (initialized_ || initialization_failure_) {
    return;
  }

  UErrorCode status = U_ZERO_ERROR;
  pattern_.reset(icu::RegexPattern::compile(
      icu::UnicodeString::fromUTF8(
          icu::StringPiece(pattern_text_.data(), pattern_text_.size_bytes())),
      /*flags=*/UREGEX_MULTILINE, status));
  if (U_FAILURE(status) || pattern_ == nullptr) {
    initialization_failure_ = true;
    pattern_.reset();
    return;
  }
  initialized_ = true;
  pattern_text_.clear();  // We don't need this anymore.
}

std::unique_ptr<UniLibBase::RegexMatcher> UniLibBase::RegexPattern::Matcher(
    const UnicodeText& input) const {
  LockedInitializeIfNotAlready();  // Possibly lazy initialization.
  if (initialization_failure_) {
    return nullptr;
  }
  return std::unique_ptr<UniLibBase::RegexMatcher>(new UniLibBase::RegexMatcher(
      pattern_.get(), icu::UnicodeString::fromUTF8(
                          icu::StringPiece(input.data(), input.size_bytes()))));
}

constexpr int UniLibBase::RegexMatcher::kError;
constexpr int UniLibBase::RegexMatcher::kNoError;

bool UniLibBase::RegexMatcher::Matches(int* status) const {
  if (!matcher_) {
    *status = kError;
    return false;
  }

  UErrorCode icu_status = U_ZERO_ERROR;
  const bool result = matcher_->matches(/*startIndex=*/0, icu_status);
  if (U_FAILURE(icu_status)) {
    *status = kError;
    return false;
  }
  *status = kNoError;
  return result;
}

bool UniLibBase::RegexMatcher::ApproximatelyMatches(int* status) {
  if (!matcher_) {
    *status = kError;
    return false;
  }

  matcher_->reset();
  *status = kNoError;
  if (!Find(status) || *status != kNoError) {
    return false;
  }
  const int found_start = Start(status);
  if (*status != kNoError) {
    return false;
  }
  const int found_end = End(status);
  if (*status != kNoError) {
    return false;
  }
  if (found_start != 0 || found_end != text_.countChar32()) {
    return false;
  }
  return true;
}

bool UniLibBase::RegexMatcher::UpdateLastFindOffset() const {
  if (!last_find_offset_dirty_) {
    return true;
  }

  // Update the position of the match.
  UErrorCode icu_status = U_ZERO_ERROR;
  const int find_offset = matcher_->start(0, icu_status);
  if (U_FAILURE(icu_status)) {
    return false;
  }
  last_find_offset_codepoints_ +=
      text_.countChar32(last_find_offset_, find_offset - last_find_offset_);
  last_find_offset_ = find_offset;
  last_find_offset_dirty_ = false;

  return true;
}

bool UniLibBase::RegexMatcher::Find(int* status) {
  if (!matcher_) {
    *status = kError;
    return false;
  }
  UErrorCode icu_status = U_ZERO_ERROR;
  const bool result = matcher_->find(icu_status);
  if (U_FAILURE(icu_status)) {
    *status = kError;
    return false;
  }

  last_find_offset_dirty_ = true;
  *status = kNoError;
  return result;
}

int UniLibBase::RegexMatcher::Start(int* status) const {
  return Start(/*group_idx=*/0, status);
}

int UniLibBase::RegexMatcher::Start(int group_idx, int* status) const {
  if (!matcher_ || !UpdateLastFindOffset()) {
    *status = kError;
    return kError;
  }

  UErrorCode icu_status = U_ZERO_ERROR;
  const int result = matcher_->start(group_idx, icu_status);
  if (U_FAILURE(icu_status)) {
    *status = kError;
    return kError;
  }
  *status = kNoError;

  // If the group didn't participate in the match the result is -1 and is
  // incompatible with the caching logic bellow.
  if (result == -1) {
    return -1;
  }

  return last_find_offset_codepoints_ +
         text_.countChar32(/*start=*/last_find_offset_,
                           /*length=*/result - last_find_offset_);
}

int UniLibBase::RegexMatcher::End(int* status) const {
  return End(/*group_idx=*/0, status);
}

int UniLibBase::RegexMatcher::End(int group_idx, int* status) const {
  if (!matcher_ || !UpdateLastFindOffset()) {
    *status = kError;
    return kError;
  }
  UErrorCode icu_status = U_ZERO_ERROR;
  const int result = matcher_->end(group_idx, icu_status);
  if (U_FAILURE(icu_status)) {
    *status = kError;
    return kError;
  }
  *status = kNoError;

  // If the group didn't participate in the match the result is -1 and is
  // incompatible with the caching logic bellow.
  if (result == -1) {
    return -1;
  }

  return last_find_offset_codepoints_ +
         text_.countChar32(/*start=*/last_find_offset_,
                           /*length=*/result - last_find_offset_);
}

UnicodeText UniLibBase::RegexMatcher::Group(int* status) const {
  return Group(/*group_idx=*/0, status);
}

UnicodeText UniLibBase::RegexMatcher::Group(int group_idx, int* status) const {
  if (!matcher_) {
    *status = kError;
    return UTF8ToUnicodeText("", /*do_copy=*/false);
  }
  std::string result = "";
  UErrorCode icu_status = U_ZERO_ERROR;
  const icu::UnicodeString result_icu = matcher_->group(group_idx, icu_status);
  if (U_FAILURE(icu_status)) {
    *status = kError;
    return UTF8ToUnicodeText("", /*do_copy=*/false);
  }
  result_icu.toUTF8String(result);
  *status = kNoError;
  return UTF8ToUnicodeText(result, /*do_copy=*/true);
}

constexpr int UniLibBase::BreakIterator::kDone;

UniLibBase::BreakIterator::BreakIterator(const UnicodeText& text)
    : text_(icu::UnicodeString::fromUTF8(
          icu::StringPiece(text.data(), text.size_bytes()))),
      last_break_index_(0),
      last_unicode_index_(0) {
  icu::ErrorCode status;
  break_iterator_.reset(
      icu::BreakIterator::createWordInstance(icu::Locale("en"), status));
  if (!status.isSuccess()) {
    break_iterator_.reset();
    return;
  }
  break_iterator_->setText(text_);
}

int UniLibBase::BreakIterator::Next() {
  const int break_index = break_iterator_->next();
  if (break_index == icu::BreakIterator::DONE) {
    return BreakIterator::kDone;
  }
  last_unicode_index_ +=
      text_.countChar32(last_break_index_, break_index - last_break_index_);
  last_break_index_ = break_index;
  return last_unicode_index_;
}

std::unique_ptr<UniLibBase::RegexPattern> UniLibBase::CreateRegexPattern(
    const UnicodeText& regex) const {
  return std::unique_ptr<UniLibBase::RegexPattern>(
      new UniLibBase::RegexPattern(regex));
}

std::unique_ptr<UniLibBase::RegexPattern> UniLibBase::CreateLazyRegexPattern(
    const UnicodeText& regex) const {
  return std::unique_ptr<UniLibBase::RegexPattern>(
      new UniLibBase::RegexPattern(regex, /*lazy=*/true));
}

std::unique_ptr<UniLibBase::BreakIterator> UniLibBase::CreateBreakIterator(
    const UnicodeText& text) const {
  return std::unique_ptr<UniLibBase::BreakIterator>(
      new UniLibBase::BreakIterator(text));
}

}  // namespace libtextclassifier3
