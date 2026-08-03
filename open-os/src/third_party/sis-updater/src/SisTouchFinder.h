// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

#ifndef SRC_SISTOUCHFINDER_H_
#define SRC_SISTOUCHFINDER_H_

// #define SIS_TOUCH_FINDER_DEBUG
#ifdef SIS_TOUCH_FINDER_DEBUG
#define STF_DEBUG if (true)
#else
#define STF_DEBUG if (false)
#endif

class SisTouchFinder {
 public:
  /* Sis Touch Type Define */
  enum DeviceType {
    UNKNOW = -1,
    USB_817 = 0,
    I2C_817 = 1,
  };

  SisTouchFinder();
  ~SisTouchFinder();

  char* autoDetectDevicePath();
  SisTouchFinder::DeviceType getDeviceType();
  bool isSisTouchHid(const char* deviceName);

  bool isRecoveryDevice;

 private:
  /* Detect hidraw* */
  char* autoDetectHidDevicePath();

  /*=====================================================*/
  /* hid-example.c */
  /*
   * Hidraw Userspace Example
   *
   * Copyright (c) 2010 Alan Ott <alan@signal11.us>
   * Copyright (c) 2010 Signal 11 Software
   *
   * The code may be used by anyone for any purpose,
   * and can serve as a starting point for developing
   * applications using hidraw.
   */
  bool getHidInfo(const char* deviceName);
  const char* bus_str(int bus);
  /*=====================================================*/

  /* Detect i2c-* */
  const char* autoDetectI2cDevicePath();
  bool isSisTouchI2c(const char* deviceName);

 private:
  int m_vId;
  char m_rawName[256];
  SisTouchFinder::DeviceType m_deviceType;
};

#endif  // SRC_SISTOUCHFINDER_H_
