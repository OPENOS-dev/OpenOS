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

#ifndef LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_NORMALIZER_H_
#define LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_NORMALIZER_H_

#include <map>
#include <string>
#include <vector>

#include "utils/utf8/unilib.h"
#include "third_party/absl/strings/string_view.h"

namespace libtextclassifier3 {

// A lightweight utility for normalizing Latin text.
// It uses a fixed substitution table generated with ICU.
// This class is thread-compatible.
class Normalizer {
 public:
  explicit Normalizer(const UniLib* unilib);

  // Default copy/assign/move.
  Normalizer(const Normalizer& rhs) = default;
  Normalizer& operator=(const Normalizer& rhs) = default;
  Normalizer(Normalizer&& rhs) = default;
  Normalizer& operator=(Normalizer&& rhs) = default;

  // Returns a normalized version of the string. If fold_case, then normalized
  // text will be case-folded.
  // If original_indices is non-null, it will be cleared and populated with a
  // mapping from normalized byte index to input byte index. These indices map
  // to the first byte of each respective code point. There's an additional
  // index mapping from output text size to original input size, so
  // original_indices.size() == return_string.size() + 1.
  ::std::string Normalize(
      absl::string_view input, bool fold_case = false,
      std::vector<::std::string::size_type>* original_indices = nullptr) const;

 private:
  const UniLib* unilib_;
};

}  // namespace libtextclassifier3

#endif  // LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_NORMALIZER_H_
