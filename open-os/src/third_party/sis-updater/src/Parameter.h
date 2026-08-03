// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

#ifndef SRC_PARAMETER_H_
#define SRC_PARAMETER_H_

#include <cstdio>
#include <cstring>

#include <linux/limits.h>

#define AUTO "auto"
#define ZEUS_I2C "zeus_i2c"
#define ZEUS_USB "zeus_usb"
#define ZEUS_INTERNALUSB "zeus_internalusb"
#define AEGIS_I2C "aegis_i2c"
#define AEGIS_USB "aegis_usb"
#define AEGIS_INTERNALUSB "aegis_internalusb"
#define AEGIS_SLAVE "aegis_slave"
#define AEGIS_MULTI "aegis_multi"
#define CUSTOM "custom"

#define MAX_SLAVE_NUM 7

const int MAX_TRANS_LENGTH = 17;
const int MAX_DEVNAME_LENGTH = 128;
const int MAX_CONNECT_CHAR = 32;

const int DEFAULT_DELAY = 1000;
const int DEFAULT_RETRY = 100;
const int DEFAULT_IO_DELAY = 0;

const int DEFAULT_SLAVE_ADDR = 5;

const int DEFAULT_MULTI_NUM = 1;

const int DEFAULT_VERBOSE = 0;
const int DEFAULT_CHANGE_DIAGMODE = 1;
const int DEFAULT_CHANGE_PWRMODE = 1;

const int DEFAULT_DETECT_HIDRAW_FLAG = 0;

struct Parameter {
  int con;
  char connect[MAX_CONNECT_CHAR];
  int max_retry;
  int delay;  // us.
  int verbose;
  int io_Delay;  // ms
  int slave_Addr;
  int multi_Num;
  int changeDiagMode;
  int changePWRMode;
  char devName[MAX_DEVNAME_LENGTH];
  char log_to[PATH_MAX];

  int m_detectHidrawFlag;
  bool m_isRecoceryDevice;

  Parameter();
  ~Parameter();
  int parse(int argc, char **argv);
  bool parseArgument(char *arg);
  int check();
  int setActiveDeviceName();
  void setConnectType(char *connectName);
  void print_sep();
};

#endif  // SRC_PARAMETER_H_
