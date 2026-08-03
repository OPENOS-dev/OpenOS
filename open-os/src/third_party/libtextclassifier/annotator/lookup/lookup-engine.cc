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

#include "annotator/lookup/lookup-engine.h"

#include <algorithm>
#include <iterator>

#include "utils/utf8/unicodetext.h"
#include "third_party/absl/container/flat_hash_set.h"

namespace libtextclassifier3 {

void LookupEngine::AddEntry(const std::vector<std::string>& ngrams,
                            ClassificationResult entry) {
  const int entry_index = entries_.size();
  entries_.push_back(std::move(entry));
  for (const std::string& ngram : ngrams) {
    if (ngram.empty()) {
      continue;
    }
    const UnicodeText ngram_unicode =
        UTF8ToUnicodeText(ngram, /*do_copy=*/false);
    if (!ngram_unicode.is_valid()) {
      TC3_LOG(WARNING) << ngram << " failed to convert to unicode.";
      continue;
    }
    auto start_it = ngram_unicode.begin();
    auto end_it = ngram_unicode.end();
    CodepointSpan ngram_span = {0, ngram_unicode.size_codepoints()};
    auto stripped_ngram =
        StripBoundaryCodepointsAndNormalize(&start_it, &end_it, &ngram_span);
    if (!stripped_ngram.empty()) {
      // TODO(b/120870643): Add transcription and lemmatization.
      std::vector<int>* entry_indices = &ngram_to_entry_index_[stripped_ngram];
      if (entry_indices->empty() || entry_indices->back() != entry_index) {
        entry_indices->push_back(entry_index);
      }
    }
  }
}

bool LookupEngine::ClassifyTextInternal(
    const std::string& context, CodepointSpan selection_indices,
    ClassificationResult* classification_result) const {
  const UnicodeText context_unicode =
      UTF8ToUnicodeText(context, /*do_copy=*/false);

  auto start_it = context_unicode.begin();
  std::advance(start_it, selection_indices.first);
  auto end_it = start_it;
  std::advance(end_it, selection_indices.second - selection_indices.first);

  std::vector<ClassificationResult> classification_results = FindMatches(
      /*max_num_matches=*/1, start_it, end_it, &selection_indices);

  if (!classification_results.empty()) {
    *classification_result = std::move(classification_results[0]);
    return true;
  } else {
    return false;
  }
}

bool LookupEngine::ChunkInternal(const UnicodeText& context_unicode,
                                 const std::vector<Token>& tokens,
                                 int max_num_tokens, int max_num_matches,
                                 std::vector<AnnotatedSpan>* result) const {
  CodepointIndex start_codepoint_idx = 0;
  auto start_it = context_unicode.begin();
  TokenIndex minimum_start = 0;

  // Iterate over all the possible starts (token indices) of a match.
  for (TokenIndex start = 0; start < tokens.size(); ++start) {
    if (start < minimum_start) {
      // There is an overlap with the previous detected match.
      continue;
    }

    // Update the codepoint index and the iterator.
    while (start_codepoint_idx < tokens[start].start) {
      ++start_codepoint_idx;
      ++start_it;
    }

    CodepointIndex end_codepoint_idx = start_codepoint_idx;
    auto end_it = start_it;
    std::vector<TokenIndex> end_candidates;
    std::vector<CodepointIndex> end_codepoint_idx_candidates;
    std::vector<UnicodeText::const_iterator> end_it_candidates;
    // Iterate over all the possible ends (token indices) of a match.
    for (TokenIndex end = start + 1;
         end <= start + max_num_tokens && end <= tokens.size(); ++end) {
      // Update the codepoint index and the iterator.
      while (end_codepoint_idx < tokens[end - 1].end) {
        ++end_codepoint_idx;
        ++end_it;
      }

      // Store the possible ends in the vectors.
      end_candidates.push_back(end);
      end_codepoint_idx_candidates.push_back(end_codepoint_idx);
      end_it_candidates.push_back(end_it);
    }

    // Iterate over the possible ends backwards, to find the longest match
    // starting at the current start token.
    for (int i = end_codepoint_idx_candidates.size() - 1; i >= 0; --i) {
      CodepointSpan stripped_codepoint_span = {start_codepoint_idx,
                                               end_codepoint_idx_candidates[i]};
      AnnotatedSpan annotated_span;
      annotated_span.classification =
          FindMatches(max_num_matches, start_it, end_it_candidates[i],
                      &stripped_codepoint_span);

      if (!annotated_span.classification.empty()) {
        // At least one match was found, add all the matches to the result.
        annotated_span.span = stripped_codepoint_span;
        result->push_back(std::move(annotated_span));

        // Avoid finding an overlapping match.
        minimum_start = end_candidates[i];

        break;
      }
    }
  }

  return true;
}

std::string LookupEngine::StripBoundaryCodepointsAndNormalize(
    UnicodeText::const_iterator* start_it, UnicodeText::const_iterator* end_it,
    CodepointSpan* span) const {
  // Skip strip boundary codepoints if |feature_processor_| is missing.
  // TODO(http://b/150347764): Discuss if it makes sense to create a feature
  // processor in case it is missing.
  if (feature_processor_ != nullptr) {
    const CodepointSpan stripped_span =
        feature_processor_->StripBoundaryCodepoints(*start_it, *end_it, *span);
    std::advance(*start_it, stripped_span.first - span->first);
    std::advance(*end_it, stripped_span.second - span->second);
    *span = stripped_span;
  }
  if (span->first == span->second) {
    return "";
  }
  return normalizer_.Normalize(UnicodeText::UTF8Substring(*start_it, *end_it),
                               /*fold_case=*/true);
}

std::vector<ClassificationResult> LookupEngine::FindMatches(
    int max_num_matches, const UnicodeText::const_iterator& start_it,
    const UnicodeText::const_iterator& end_it, CodepointSpan* span) const {
  UnicodeText::const_iterator after_strip_start_it = start_it;
  UnicodeText::const_iterator after_strip_end_it = end_it;
  absl::flat_hash_set<int> index;
  std::vector<ClassificationResult> results;
  FindTokenMatches(StripBoundaryCodepointsAndNormalize(
                       &after_strip_start_it, &after_strip_end_it, span),
                   max_num_matches, &index, &results);
  return results;
}

void LookupEngine::FindTokenMatches(
    const std::string& token, int max_num_matches,
    absl::flat_hash_set<int>* result_index,
    std::vector<ClassificationResult>* results) const {
  const auto it = ngram_to_entry_index_.find(token);
  if (it == ngram_to_entry_index_.end()) {
    return;
  }
  for (const int entry_idx : it->second) {
    if (results->size() == max_num_matches) {
      break;
    }
    // Avoid selecting the same result more than once.
    if (result_index->contains(entry_idx)) {
      continue;
    }
    results->emplace_back(entries_[entry_idx]);
    results->back().collection = collection_;
    result_index->emplace(entry_idx);
  }
}

}  // namespace libtextclassifier3
