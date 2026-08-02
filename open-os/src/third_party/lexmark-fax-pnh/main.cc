// Copyright 2020 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fstream>
#include <iostream>

#include "token_replacer.h"

int main(int argc, char* argv[]) {
  if (argc < 6 || argc > 7) {
    std::cerr << "ERROR: " << argv[0]
              << " job-id user title copies options [file]" << std::endl;
    return 1;
  }

  std::string host = "localhost";
  std::string user = argv[2];
  std::string title = argv[3];
  std::string copies = argv[4];

  TokenReplacer replacer(host, user, title, copies);

  if (argc < 7) {
    transform(replacer, std::cin, std::cout);
  } else {
    std::ifstream fstream(argv[6]);
    transform(replacer, fstream, std::cout);
  }

  return 0;
}
