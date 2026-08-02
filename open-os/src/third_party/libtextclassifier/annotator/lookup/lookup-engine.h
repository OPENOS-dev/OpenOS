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

#ifndef LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_LOOKUP_ENGINE_H_
#define LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_LOOKUP_ENGINE_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "annotator/feature-processor.h"
#include "annotator/lookup/normalizer.h"
#include "annotator/types.h"
#include "utils/utf8/unicodetext.h"
#include "utils/utf8/unilib.h"
#include "third_party/absl/container/flat_hash_set.h"

namespace libtextclassifier3 {

// A common implementation of annotation engines that annotate by looking up
// n-grams in an in-memory database.
class LookupEngine {
 public:
  // The collection specified here is set on all the returned classification
  // results.
  LookupEngine(const std::string& collection,
               const FeatureProcessor* feature_processor, const UniLib* unilib)
      : collection_(collection),
        feature_processor_(feature_processor),
        normalizer_(unilib) {}

  virtual ~LookupEngine() = default;

 protected:
  // Adds an entry (in the form of a classification result) to the database.
  // This result will be returned for n-grams in text matching those in the
  // given list. Duplicates and empty strings in the list are ignored.
  void AddEntry(const std::vector<std::string>& ngrams,
                ClassificationResult entry);

  // Classifies the span and returns at most one result, the one added earlies
  // to the database.
  // TODO(b/120870643): Return multiple entries when the API allows it.
  bool ClassifyTextInternal(const std::string& context,
                            CodepointSpan selection_indices,
                            ClassificationResult* classification_result) const;

  // Returns a list of annotations for n-grams found in the tokenized context.
  // Never returns overlapping spans. One span might still correspond to several
  // matching results, up to the given limit. These results come in the order
  // they were inserted into the database.
  // Looks for matching spans greedily, considering all the start positions in
  // the natural order, and for each of them, all the end positions in reversed
  // order (starting with the given maximum number of tokens), to prefer longer
  // matches. Once a match is found, moves on beyond it.
  bool ChunkInternal(const UnicodeText& context_unicode,
                     const std::vector<Token>& tokens, int max_num_tokens,
                     int max_num_matches,
                     std::vector<AnnotatedSpan>* result) const;

  // Looks for matching n-grams for the given string (expressed both as an
  // iterator span and a codepoint span, for efficiency). Modifies |span| to
  // represent the found match. |results| preserves matching order.
  // This method needs to be virtual for customization in classes such as
  // |ContactEngine|.
  // result_index: collects the indexes associated with the list of |results|.
  virtual std::vector<ClassificationResult> FindMatches(
      int max_num_matches, const UnicodeText::const_iterator& start_it,
      const UnicodeText::const_iterator& end_it, CodepointSpan* span) const;

  // Looks for matching n-grams for the given |token|.
  void FindTokenMatches(const std::string& token, int max_num_matches,
                        absl::flat_hash_set<int>* result_index,
                        std::vector<ClassificationResult>* results) const;

  // Returns the string after stripping boundary codepoints (if
  // |feature_processor_| is present) and normalization. For efficiency, the
  // span is expressed both as a pair of iterators and a codepoint span.
  // Modifies the iterators and the |span| to match the result. See also
  // |StripBoundaryCodepoints| methods in the FeatureProcessor.
  std::string StripBoundaryCodepointsAndNormalize(
      UnicodeText::const_iterator* start_it,
      UnicodeText::const_iterator* end_it, CodepointSpan* span) const;

 private:
  friend class LookupEngineTest;
  const std::string collection_;
  const FeatureProcessor* feature_processor_;
  const Normalizer normalizer_;

  std::vector<ClassificationResult> entries_;
  std::unordered_map<std::string, std::vector<int>> ngram_to_entry_index_;
};

}  // namespace libtextclassifier3

#endif  // LIBTEXTCLASSIFIER_ANNOTATOR_LOOKUP_LOOKUP_ENGINE_H_
