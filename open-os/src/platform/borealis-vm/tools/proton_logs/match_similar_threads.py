#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Match up the per-thread logs extracted from two Proton logs.

   Per-thread logs should be extracted with ./extract_threads.py.

   The matching is based on the Jaccard similarity index, which is
   defined as the ratio between the intersection of two sets to the
   union of the two sets.

   In this code, the sets are made of the words contained within each
   per-thread log extracted from the two Proton logs.
"""

import argparse
import os
import re


def jaccard_index(x, y):
    """Computes the Jaccard similarity index between two sets"""
    intersection = len(x.intersection(y))
    union = len(x.union(y))
    return intersection / float(union)


def extract_words(file_path):
    """Returns a set of words in a file, allowing underscores and dashes in words"""
    words = []
    with open(file_path, encoding="utf-8", errors="backslashreplace") as file:
        for line in file.readlines():
            words.extend(re.findall(r"\b[a-zA-Z_-]+\b", line))
    return set(words)


def parse_per_thread_proton_logs(dir_path):
    """Parses a directory of per-thread proton logs into a dictionary.

    The dictionary has file paths to each per-thread log file (keys)
    associated with a set of words extracted from each file (value)
    """
    tid_words = {}
    for file_name in os.listdir(dir_path):
        file_path = os.path.join(dir_path, file_name)
        if not os.path.isfile(file_path):
            continue
        tid_words[file_path] = extract_words(file_path)
    return tid_words


def analyze_and_print(bad_to_good_similarities, good_to_bad_similarities):
    """Analyzes the calculated similarities and prints a summary report"""
    similarity_below_80 = {}
    similarity_below_90 = {}
    for tid_similarities in [
        good_to_bad_similarities,
        bad_to_good_similarities,
    ]:
        for src_case_path, similarities in tid_similarities.items():
            print(f"\n{src_case_path} top matches:")
            print_cnt = 0
            sorted_similarity_scores = sorted(similarities.keys(), reverse=True)
            if sorted_similarity_scores[0] < 0.8:
                similarity_below_80[src_case_path] = sorted_similarity_scores[0]
            elif sorted_similarity_scores[0] < 0.9:
                similarity_below_90[src_case_path] = sorted_similarity_scores[0]
            # Print at least the top three matches for each file in the bad_case
            # (more matches printed for ties in similarity scores)
            for similarity_score in sorted_similarity_scores:
                similarity_percent = round(similarity_score * 100.0, 2)
                for dst_case_path in similarities[similarity_score]:
                    print(f"    {dst_case_path} => {similarity_percent}%")
                    print_cnt = print_cnt + 1
                if print_cnt >= 3:
                    break
    if len(similarity_below_90) > 0:
        print(
            "\n\nThese threads only have partial matches. "
            "They may be of interest to root-cause issues."
        )
        for path, similarity_score in sorted(
            similarity_below_90.items(), key=lambda item: item[1]
        ):
            similarity_percent = round(similarity_score * 100.0, 2)
            print(f"    {path} ({similarity_percent}%)")
    if len(similarity_below_80) > 0:
        print(
            "\n\nThese threads only have poor matches. "
            "They are likely relevant to root-cause issues."
        )
        for path, similarity_score in sorted(
            similarity_below_80.items(), key=lambda item: item[1]
        ):
            similarity_percent = round(similarity_score * 100.0, 2)
            print(f"    {path} ({similarity_percent}%)")


def main():
    """Command-line entry-point"""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--bad_case_dir_path",
        "--bad",
        "-b",
        help="Path with the per-thread Proton logs for a bad or failing case.",
    )
    parser.add_argument(
        "--good_case_dir_path",
        "--good",
        "-g",
        help="Path with the per-thread Proton logs for a good or working case.",
    )
    args = parser.parse_args()

    bad_case_tid_words = parse_per_thread_proton_logs(args.bad_case_dir_path)
    good_case_tid_words = parse_per_thread_proton_logs(args.good_case_dir_path)

    # Maps files from the bad case to files from the good case and vice versa.
    # Note that multiple pairings may result in the same similarity index
    # so these are N->M mappings. For example (bad->good mappings):
    # {bad_case_path: { similarityA: [good_case_path1, good_case_path2, ...] } }
    bad_to_good_similarities = {}
    good_to_bad_similarities = {}
    for bad_case_path, bad_case_words in bad_case_tid_words.items():
        for good_case_path, good_case_words in good_case_tid_words.items():
            similarity = jaccard_index(bad_case_words, good_case_words)
            bad_to_good_similarities.setdefault(bad_case_path, {}).setdefault(
                similarity, []
            ).append(good_case_path)
            good_to_bad_similarities.setdefault(good_case_path, {}).setdefault(
                similarity, []
            ).append(bad_case_path)
    analyze_and_print(bad_to_good_similarities, good_to_bad_similarities)


if __name__ == "__main__":
    main()
