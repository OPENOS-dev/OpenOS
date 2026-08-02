// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

/*
 * SiSTouchDeviceName.h
 *
 *  Created on: 2013/6/28
 *      Author: swing
 */

#ifndef SRC_SISTOUCHDEVICENAME_H_
#define SRC_SISTOUCHDEVICENAME_H_

class SiSTouchDeviceName {
 public:
  SiSTouchDeviceName();
  virtual ~SiSTouchDeviceName();

  bool compareDeviceName(char* devName1, const char* devName2);
  int getDeviceTypeByTable(char* devName);
  int getDeviceType(char* devName, int max_retry, int retry_delay, int verbose,
                    int ioDelay, int changeDiagMode, int changePWRMode);
};

#endif  // SRC_SISTOUCHDEVICENAME_H_
