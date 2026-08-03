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

#include "annotator/lookup/normalizer.h"

#include <algorithm>
#include <map>
#include <string>

#include "knowledge/cerebra/sense/annotation/normalization-table.h"
#include "utils/strings/utf8.h"
#include "utils/utf8/unicodetext.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/absl/strings/string_view.h"

namespace libtextclassifier3 {

namespace {

// Given the first byte of a UTF-8 character, figures out the character's number
// of bytes by looking at the number of consecutive most significant one-bits.
// Always returns a positive value.
int Utf8CharLength(char first_byte) {
  if (IsTrailByte(first_byte)) {
    // Invalid input, return 1 as a fallback.
    return 1;
  }
  return GetNumBytesForUTF8Char(&first_byte);
}

}  // namespace

Normalizer::Normalizer(const UniLib* unilib) : unilib_(unilib) {}

::std::string Normalizer::Normalize(
    absl::string_view input, bool fold_case,
    std::vector<::std::string::size_type>* original_indices) const {
  const char* input_left = input.data();
  ::std::string::size_type bytes_left = input.length();

  ::std::string output;
  if (original_indices != nullptr) {
    original_indices->clear();
  }
  ::std::string::size_type codepoint_start_index = 0;
  while (bytes_left > 0) {
    const int utf8_char_length = Utf8CharLength(*input_left);
    if (utf8_char_length > bytes_left) {
      // Invalid utf-8 bytes. We strip these from the output.
      break;
    }
    const ::std::string utf8_char(input_left, utf8_char_length);

    // The normalization table might map the character to multiple characters.
    const auto it = cerebra_sense::GetNormalizationTable().find(utf8_char);
    ::std::string normalized_chars =
        it != cerebra_sense::GetNormalizationTable().end() ? it->second
                                                           : utf8_char;

    if (fold_case) {
      const UnicodeText unfolded =
          UTF8ToUnicodeText(normalized_chars, /*do_copy=*/false);
      UnicodeText folded;
      for (const char32 codepoint : unfolded) {
        folded.push_back(unilib_->ToLower(codepoint));
      }
      normalized_chars = folded.ToUTF8String();
    }

    output += normalized_chars;
    if (original_indices != nullptr) {
      for (int i = 0; i < normalized_chars.size(); i++) {
        original_indices->push_back(codepoint_start_index);
      }
    }

    codepoint_start_index += utf8_char_length;
    input_left += utf8_char_length;
    bytes_left -= utf8_char_length;
  }

  if (original_indices != nullptr) {
    // Add mapping from normalized size to input size.
    original_indices->push_back(input.size());
  }
  return output;
}

}  // namespace libtextclassifier3
