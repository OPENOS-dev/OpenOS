// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#define MAIN_VERSION 1
#define SUB_VERSION 8
#define LAST_VERSION 20
#define DESC "HIDRAW"

void printVersion() {
#ifndef DESC
  printf("V%d.%d.%d\n", MAIN_VERSION, SUB_VERSION, LAST_VERSION);
#else
  printf("V%d.%d.%d %s\n", MAIN_VERSION, SUB_VERSION, LAST_VERSION, DESC);
#endif
}

#endif  // SRC_VERSION_H_
