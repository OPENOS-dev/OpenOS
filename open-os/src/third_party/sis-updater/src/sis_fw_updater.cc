// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

/* Linux */
#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/types.h>

#include <base/files/file_util.h>
#include <base/logging.h>
#include <base/strings/stringprintf.h>
#include <brillo/syslog_logging.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <vector>

#include "./AegisSiSTouchAdapter.h"
#include "./ApplicationParameter.h"
#include "./ExitStatus.h"
#include "./MultiOpenShortData.h"
#include "./MultiOpenShortResult.h"
#include "./OpenShortConfig.h"
#include "./OpenShortData.h"
#include "./OpenShortResult.h"
#include "./Parameter.h"
#include "./SiSLogger.h"
#include "./SiSTouchAdapter.h"
#include "./SiSTouchDeviceName.h"
#include "./SiSTouchIO.h"
#include "./SisTouchFinder.h"
#include "./UpdateFWParameter.h"
#include "./csv_parser.hpp"
#include "./version.h"

using base::StringPrintf;

int m_LogInterface = 0;

int (*m_vLOGIpointer)(const char* format, va_list args) = 0;
int (*m_vLOGEpointer)(const char* format, va_list args) = 0;

/*============================================================================*/
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

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 *
 * If you need this, please have your distro update the kernel headers.
 */
#ifndef HIDIOCSFEATURE
#define HIDIOCGFEATURE(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x07, len)
#endif

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

/* C -- included above */

/*============================================================================*/

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define DEVICE_CHANGEMODE_RETRY_TIMES 40
#define DEVICE_CHANGEMODE_RETRY_INTERVAL 100

#define DEVICE_CLOSE_RETRY_TIMES 20
#define DEVICE_CLOSE_RETRY_INTERVAL 1000

#define DUMMYBYTE 0
#define HEADER_LENGTH 1

#define FW_ID_LENGTH 3

// 0x81 delay time (ms)
#define DELAY_FOR_GENEARL 2
#define DELAY_FOR_81 500
#define TIMEOUT_FOR_87 12000

// AP_MODE & TOUCH_MODE is replaced by DIAGNOSIS MODE
// #define AP_MODE 0x00000f00
// #define TOUCH_MODE 0x00000f0

#define DIAGNOSIS_MODE

#ifdef DIAGNOSIS_MODE
#define ENABLE_DIAGNOSIS_MODE 0x00000121
#define DISABLE_DIAGNOSIS_MODE 0x00000120
#define DELAY_FOR_DIAGNOSIS_MODE_CHANGE 1000
#endif

#define PWR_MODE

#ifdef PWR_MODE
#define PW_CMD_FWCTRL 0x00000950
#define PW_CMD_ACTIVE 0x00000951
#define PW_CMD_SLEEP 0x00000952

// #define PWR_MODE_QUERY

#define DEVICE_CHANGEPWR_RETRY_TIMES 10
#define DEVICE_CHANGEPWR_RETRY_INTERVAL 20

#endif

#define QUERY_STATUS_ADDRESS 0x50000000

#define CLOSE_AFTER_RESET

#define WAIT_DEVICE_REMOVE 3000

#ifdef VIA_IOCTL
#define DEVICE_NAME "/dev/hidraw*"  // same as master
#else
// #error "SiS817 not support system call"
#endif

#define DIV_ROUND_UP(n, d) ((n + d - 1) / d)

// Add a helper function to set break points during upgrate.
#define SET_TEST_POINT(C)              \
  {                                    \
    if (C) {                           \
      fprintf(stderr, "interrupt!\n"); \
      std::abort();                    \
    }                                  \
  }

static int interrupt_phase = -1;
static int current_phase = 0;

enum {
  OUTPUT_REPORT_ID = 0x9,
  INPUT_REPORT_ID = 0xA,

  DATA_UNIT_9257 = 52,
};

static const uint16_t crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};
// 20120925
/*
static const unsigned short wrong_crc16tab[256]=
{
    0x0001,0x1022,0x2043,0x3064,0x4085,0x50a6,0x60c7,0x70e8,
    0x8109,0x912a,0xa14b,0xb16c,0xc18d,0xd1ae,0xe1cf,0xf1e0,
    0x1232,0x0211,0x3274,0x2253,0x52b6,0x4295,0x72f8,0x62d7,
    0x933a,0x8319,0xb37c,0xa35b,0xd3be,0xc39d,0xf3f0,0xe3df,
    0x2463,0x3444,0x0421,0x1402,0x64e7,0x74c8,0x44a5,0x5486,
    0xa56b,0xb54c,0x8529,0x950a,0xe5ef,0xf5c0,0xc5ad,0xd58e,
    0x3654,0x2673,0x1612,0x0631,0x76d8,0x66f7,0x5696,0x46b5,
    0xb75c,0xa77b,0x971a,0x8739,0xf7d0,0xe7ff,0xd79e,0xc7bd,
    0x48c5,0x58e6,0x6887,0x78a8,0x0841,0x1862,0x2803,0x3824,
    0xc9cd,0xd9ee,0xe98f,0xf9a0,0x8949,0x996a,0xa90b,0xb92c,
    0x5af6,0x4ad5,0x7ab8,0x6a97,0x1a72,0x0a51,0x3a34,0x2a13,
    0xdbfe,0xcbdd,0xfbb0,0xeb9f,0x9b7a,0x8b59,0xbb3c,0xab1b,
    0x6ca7,0x7c88,0x4ce5,0x5cc6,0x2c23,0x3c04,0x0c61,0x1c42,
    0xedaf,0xfd80,0xcded,0xddce,0xad2b,0xbd0c,0x8d69,0x9d4a,
    0x7e98,0x6eb7,0x5ed6,0x4ef5,0x3e14,0x2e33,0x1e52,0x0e71,
    0xff90,0xefbf,0xdfde,0xcffd,0xbf1c,0xaf3b,0x9f5a,0x8f79,
    0x9189,0x81aa,0xb1cb,0xa1ec,0xd10d,0xc12e,0xf14f,0xe160,
    0x1081,0x00a2,0x30c3,0x20e4,0x5005,0x4026,0x7047,0x6068,
    0x83ba,0x9399,0xa3fc,0xb3db,0xc33e,0xd31d,0xe370,0xf35f,
    0x02b2,0x1291,0x22f4,0x32d3,0x4236,0x5213,0x6278,0x7257,
    0xb5eb,0xa5cc,0x95a9,0x858a,0xf56f,0xe540,0xd52d,0xc50e,
    0x34e3,0x24c4,0x14a1,0x0482,0x7467,0x6448,0x5425,0x4406,
    0xa7dc,0xb7fb,0x879a,0x97b9,0xe750,0xf77f,0xc71e,0xd73d,
    0x26d4,0x36f3,0x0692,0x16b1,0x6658,0x7677,0x4616,0x5635,
    0xd94d,0xc96e,0xf90f,0xe920,0x99c9,0x89ea,0xb98b,0xa9ac,
    0x5845,0x4866,0x7807,0x6828,0x18c1,0x08e2,0x3883,0x28a4,
    0xcb7e,0xdb5d,0xeb30,0xfb1f,0x8bfa,0x9bd9,0xabbc,0xbb9b,
    0x4a76,0x5a55,0x6a38,0x7a17,0x0af2,0x1ad1,0x2ab4,0x3a93,
    0xfd2f,0xed00,0xdd6d,0xcd4e,0xbdab,0xad8c,0x9de9,0x8dca,
    0x7c27,0x6c08,0x5c65,0x4c46,0x3ca3,0x2c84,0x1ce1,0x0cc2,
    0xef10,0xff3f,0xcf5e,0xdf7d,0xaf9c,0xbfbb,0x8fda,0x9ff9,
    0x6e18,0x7e37,0x4e56,0x5e75,0x2e94,0x3eb3,0x0ed2,0x1ef1
};
*/
// for custom device name/info setting
char SiSTouchIO::m_activeDeviceName[] = "";

bool SiSTouchIO::m_isCustomDevice = false;
int SiSTouchIO::m_customDeviceType = CON_NONE;  // CON_NONE
char SiSTouchIO::m_customDeviceName[] = "";

void printVersion();
void print_sep();
FILE* open_firmware_bin(const char* filename, int* file_size);

const char SiSTouchAdapter::phase2StatusStr[STATUS_INDEX_MAX][64] = {
    "Enable Palm Rejection : ",   "Enable Water Proof : ",
    "Enable Moiset : ",           "Enable Press Detect : ",
    "Initial Palm is OK ? ",      "Enable Dynamic Calibrate : ",
    "Enable Frequency Hopping : "};

bool SiSTouchAdapter::phase2Status[STATUS_INDEX_MAX];

bool isLittleEndian() {
  int tmp = 1;
  return (*(char*)&tmp == 1);
}

int main(int argc, char** argv) {
  // Parse command line flags.
  ApplicationParameter* param_tmp =
      ApplicationParameter::create(ApplicationParameter::UPDATE_FW);
  UpdateFWParameter& param = dynamic_cast<UpdateFWParameter&>(*param_tmp);

  if (!param.parse(argc, argv)) {
    param.print_usage();
    return EXIT_BADARGU;
  }

  // Configure logging settings.
  if (std::string(param.conParameter.log_to) == "") {
    // Default log to syslog.
    brillo::InitLog(brillo::InitFlags::kLogToSyslog);
  } else if (std::string(param.conParameter.log_to) == "stdout") {
    // Print log to stdout.
    logging::LoggingSettings logging_settings;
    logging::InitLogging(logging_settings);
  } else {
    // Log to a particular file.
    logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_FILE;
    logging_settings.log_file_path = param.conParameter.log_to;
    logging_settings.lock_log = logging::DONT_LOCK_LOG_FILE;
    logging::InitLogging(logging_settings);
  }

  // If running on big endian, just exit.
  if (!isLittleEndian()) {
    LOG(INFO) << "Do not support running on big endian system.";
    return EXIT_ERR;
  }

  int exitValue = EXIT_OK;

  printVersion();

  interrupt_phase = param.interrupt_point;
  if (interrupt_phase < 0 || interrupt_phase > 6) {
    LOG(WARNING) << "invalid interrupt point, must between 0 and 6.";
    interrupt_phase = 0;
  }

  print_sep();

  param.conParameter.setActiveDeviceName();

  // SisTouchFinder finder;
  // finder.autoDetectDevicePath();

  const bool isRecoveryDevice = param.conParameter.m_isRecoceryDevice;

  SiSTouchAdapter* adapter =
      SiSTouchAdapter::prepareAdapter(param.conParameter);

  if (adapter != 0) {
    int multiNum = SiSTouchAdapter::DEFALUT_SINGLE_NUM;

    multiNum = (!param.jump_check && !param.single_device) ?
                                      (adapter->doDetectSlaveNum() + 1)
                                    : (param.conParameter.multi_Num);

    if (typeid(*adapter) == typeid(AegisMultiSiSTouchAdapter)) {
      LOG(INFO) << "multi..";
      if (param.filenames.size() == 1) {
        multiNum = 1;
      }

      if (!param.jump_check) {
        if (multiNum != param.conParameter.multi_Num) {
          LOG(ERROR) << "Input data count " << param.conParameter.multi_Num
                     << "do not match chip count " << multiNum;
          delete adapter;
          return EXIT_ERR;
        }
      }

#define NUM_OF_FILES (7)
      FILE* multi_input_files[NUM_OF_FILES] = {0};
      int multi_files_size[NUM_OF_FILES] = {0};
#undef NUM_OF_FILES

      if (multiNum >= 1) {
        multi_input_files[0] =
            open_firmware_bin(param.filenames[0].c_str(), &multi_files_size[0]);

        if (!multi_input_files[0]) {
          return EXIT_ERR;
        }
      }

      for (int i = 1; i < multiNum; i++) {
        multi_input_files[i] = open_firmware_bin(param.filenames[i].c_str(),
                                                 &(multi_files_size[i]));
        if (!multi_input_files[i]) {
          for (int j = 0; j < i; j++) {
            if (multi_input_files[j]) {
              fclose(multi_input_files[j]);
            }
          }

          return EXIT_ERR;
        }
      }

      int ret =
          (dynamic_cast<AegisMultiSiSTouchAdapter*>(adapter))
              ->doUpdate(multi_input_files, multiNum, param, isRecoveryDevice);

      switch (ret) {
        case SiSTouchAdapter::SUCCESS: {
          exitValue = EXIT_OK;
          LOG(INFO) << "update firmware complete";
          break;
        }
        case SiSTouchAdapter::RESULT_SAME_FIRMWARE: {
          exitValue = EXIT_OK;
          std::string version_string = "";
          // TODO(frankhu): work with Mimo to have a better index of fw version.
          for (int i = 0; i < param.filenames.size(); ++i) {
            version_string.append(param.filenames[i] + ", ");
          }
          LOG(INFO)
              << "device has same firmware with system, no need for updating: "
              << version_string.substr(0, version_string.length() - 2);
          break;
        }
        default: {
          exitValue = EXIT_ERR;
          LOG(ERROR) << "some error occurs, please check the output. Err = "
                     << ret;
        }
      }

      for (int i = 0; i < multiNum; i++) {
        if (multi_input_files[i]) {
          fclose(multi_input_files[i]);
        }
      }
    } else {
      LOG(INFO) << "single..";
      if (param.conParameter.multi_Num != 1) {
        LOG(ERROR) << "input data count incorrect, expecting 1, got "
                   << param.conParameter.multi_Num;
        delete adapter;
        return EXIT_ERR;
      }

      if (typeid(*adapter) == typeid(AegisSlaveSiSTouchAdapter)) {
        if (!param.jump_check) {
          if (param.conParameter.slave_Addr <
                  AegisMultiSiSTouchAdapter::SLAVE_DEVICE0_ADDR ||
              param.conParameter.slave_Addr >
                  AegisMultiSiSTouchAdapter::getStaticSlaveAddr(multiNum - 1)) {
            LOG(ERROR) << "invalid slave addr "
                       << param.conParameter.slave_Addr;

            delete adapter;
            return EXIT_ERR;
          }
        }
        (reinterpret_cast<AegisSlaveSiSTouchAdapter*>(adapter))
            ->setSlaveAddr(param.conParameter.slave_Addr);
      }

      int file_size;
      FILE* input_file = 0;

      input_file = open_firmware_bin(param.filenames[0].c_str(), &file_size);
      if (!input_file) {
        delete adapter;
        return EXIT_ERR;
      }

      int ret = adapter->doUpdate(
          input_file, param.update_bootloader, param.update_bootloader_auto,
          param.reserve_RODATA, param.update_parameter, param.force_update,
          param.jump_check, isRecoveryDevice);

      if (ret != SiSTouchAdapter::SUCCESS) {
        exitValue = EXIT_ERR;
        LOG(ERROR) << "some error occurs, please check the output. Err = "
                   << ret;
      } else {
        exitValue = EXIT_OK;
        LOG(INFO) << "update firmware complete";
      }

      fclose(input_file);
    }

    delete adapter;
  } else {
    exitValue = EXIT_NODEV;
  }

  delete param_tmp;

  return exitValue;
}

void print_sep() { LOG(INFO) << "-----"; }

FILE* open_firmware_bin(const char* filename, int* size) {
  LOG(INFO) << "opening firmware bin: " << filename;

  FILE* input_file = fopen(filename, "r");

  if (!input_file) {
    LOG(ERROR) << "ERROR: file not found";
    return 0;
  }

  fseek(input_file, 0, SEEK_END);

  int file_size = ftell(input_file);
  *size = file_size;
  LOG(INFO) << "Pattern file contains " << file_size << " bytes";

  fseek(input_file, 0, SEEK_SET);
  return input_file;
}

/*===========================================================================*/
ApplicationParameter* ApplicationParameter::create(ApplicationType type) {
  ApplicationParameter* p = 0;

  switch (type) {
    case UPDATE_FW:
      p = new UpdateFWParameter();
      break;

    default:
      p = 0;
      break;
  }

  return p;
}
/*===========================================================================*/
enum MultiChipSelectiveID {
  ID_INVALID = 0xff,
  ID_NONE = 0x0,
  ID_SINGLE = 0x0,
  ID_MASTER = 0x1,
  ID_SLAVE1 = 0x5,
  ID_SLAVE2 = 0x6,
  ID_SLAVE3 = 0x7,
  ID_SLAVE4 = 0x8,
  ID_BRIDGE = 0xB,
  ID_DUMMY = 0xD,
};
/*===========================================================================*/
static bool sortFwBinfileOrder(std::vector<std::string>* filenames) {
  int bin_files_num = filenames->size();
  int slave_num = 0;
  int multichip_selective_id = 0;
  int num_master_cnt = 0;

  std::vector<int> chip_order(0);

  chip_order.clear();

  for (int idx = 0; idx < bin_files_num; idx++) {
    FILE* fp = NULL;

    if ((fp = fopen((*filenames)[idx].c_str(), "rb")) != NULL) {
#define MULTICHIP_SELECTIVE_ID_ADDR (0xc027)
      if (0 != fseek(fp, MULTICHIP_SELECTIVE_ID_ADDR, SEEK_SET)) {
        LOG(ERROR) << "file " << (*filenames)[idx].c_str()
                   << " is not correct sis firmware file";
        return false;
      }
#undef MULTICHIP_SELECTIVE_ID_ADDR
      multichip_selective_id = fgetc(fp);

      switch (multichip_selective_id) {
        case ID_INVALID:
          LOG(ERROR) << (*filenames)[idx].c_str()
                     << ": old firmware, cannot check multi-chip relation\n";
          return false;
          break;

        case ID_NONE:
          LOG(INFO) << (*filenames)[idx].c_str() << ": single chip firmware";
          chip_order.push_back(multichip_selective_id);
          break;

        case ID_MASTER:
        case ID_BRIDGE:
          chip_order.push_back(0);
          slave_num = fgetc(fp);
          num_master_cnt++;
          break;

        case 0x5:
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
          chip_order.push_back(multichip_selective_id - 4);
          break;

        default:
          LOG(ERROR) << (*filenames)[idx].c_str()
                     << ": FW format error, slave id of FW = "
                     << multichip_selective_id;
          return false;
          break;
      }
    } else {
      LOG(ERROR) << "can't open file " << (*filenames)[idx].c_str();
      exit(-1);
    }

    fclose(fp);
  }

  if (multichip_selective_id == ID_SINGLE) {
    return true;
  }

  // false if more than one master fw bin file
  if (num_master_cnt > 1) {
    LOG(ERROR) << "number of master FW > 1:" << num_master_cnt;
    return false;
  }

  // only update master

  if ((num_master_cnt == 1) && (chip_order.size() == 1)) {
    return true;
  }

  std::vector<std::string> tmp_filename(*filenames);

  if ((chip_order.size() != 0) && (slave_num == 0)) {
    LOG(ERROR) << "number of FW file doesn't not match";
    return false;
  }

  if (slave_num != (bin_files_num - 1)) {
    LOG(ERROR) << "number of FW file doesn't not match";
    return false;
  }
  // sort chip order to Master->Slave0->...->SlaveN
  for (int idx = 0; idx < bin_files_num; idx++) {
    (*filenames)[chip_order[idx]] = tmp_filename[idx];
  }

  return true;
}
/*===========================================================================*/
static std::vector<std::string> filenames_ini(0);
UpdateFWParameter::UpdateFWParameter()
    : filenames(filenames_ini),
      update_bootloader(false),
      update_bootloader_auto(false),
      reserve_RODATA(false),
      update_parameter(false),
      force_update(false),
      jump_check(false),
      single_device(false),
      wait_time(10),
      interrupt_point(0) {
  this->filenames.clear();
}
/*===========================================================================*/
UpdateFWParameter::~UpdateFWParameter() {}
/*===========================================================================*/
int UpdateFWParameter::parse(int argc, char** argv) {
  if (argc <= 1) {
    return 0;
  }

  LOG(INFO) << "parsing command parameter";

  for (int i = 1; i < argc; i++) {
    LOG(INFO) << i << ": "
              << "[" << argv[i] << "]";

    if (!parseArgument(argv[i])) {
      LOG(ERROR) << "unknown parameter: " << argv[i];
      return 0;
    }
  }

  return check();
}
/*===========================================================================*/
bool UpdateFWParameter::parseArgument(char* arg) {
  if (conParameter.parseArgument(arg)) {
    return true;
  }

  if (strcmp(arg, "-b") == 0) {
    update_bootloader = true;
    LOG(INFO) << "update bootloader";
  } else if (strcmp(arg, "-ba") == 0) {
    update_bootloader_auto = true;
    LOG(INFO) << "update bootloader automatically";
  } else if (strcmp(arg, "-g") == 0) {
    reserve_RODATA = true;
    LOG(INFO) << "reserve RO data";
  } else if (strcmp(arg, "-r") == 0) {
    update_parameter = true;
    LOG(INFO) << "only update parameter";
  } else if (strcmp(arg, "--force") == 0) {
    force_update = true;
    LOG(INFO) << "force update";
  } else if (strcmp(arg, "--jump") == 0) {
    jump_check = true;
    LOG(INFO) << "jump parameter validation";
  } else if (strcmp(arg, "--single") == 0) {
    single_device = true;
    LOG(INFO) << "assume single device";
  } else if (strstr(arg, "-w=") == arg) {
    wait_time = atoi(arg + 3);
    LOG(INFO) << "wait time set: " << wait_time;
  } else if (strstr(arg, "-interrupt=") == arg) {
    interrupt_point = atoi(arg + 11);
    LOG(INFO) << "set test interrupt point " << interrupt_point;
  } else {
    filenames.push_back(arg);
  }

  return true;
}
/*===========================================================================*/
int UpdateFWParameter::check() {
  if (update_parameter && update_bootloader) {
    LOG(WARNING) << "-r and -b conflict";
    return 0;
  }

  if (update_parameter && reserve_RODATA) {
    LOG(WARNING) << "-r and -g conflict";
    return 0;
  }

  if (force_update && jump_check) {
    LOG(WARNING) << "--force and --jump conflict";
    return 0;
  }

  if (single_device && (filenames.size() != 1)) {
    LOG(WARNING) << "--single requires one single filename";
    return 0;
  }

  if (conParameter.check() == 0) {
    return 0;
  }

  int bin_files_num = this->filenames.size();

  if (bin_files_num == 0) {
    LOG(WARNING) << "no fw bin file specified";
    return 0;
  }

  if (sortFwBinfileOrder(&filenames) == false) {
    exit(-1);
  }

  this->conParameter.multi_Num = filenames.size();

  return 1;
}
/*===========================================================================*/
void UpdateFWParameter::print_usage() {
  printf("\n");
  printf(
      "Usage: sis-updater [-i=CONNECT] [-n=DEVICE_NAME] [-w=WAIT_TIME] "
      "[-slave=SLAVEADDR] [-t=TIMEOUT] [-d=DELAY] [-io-delay=IO_DELAY] "
      "[-log_to=LOG_FILE_PATH] [-l] [-b] [-ba] [-g] [-r] [--recovery] "
      "[--force] [--jump] [--no-diag] [--no-power] [--single] FIRMWARE_PATH\n");
  printf("Update firmware from specified file.\n");
  printf(
      "    CONNECT:     "
      "'auto'|'zeus_i2c'|'zeus_usbbridge'|'zeus_internalusb'|'aegis_i2c'|'"
      "aegis_usbbridge'|'aegis_internalusb'|'aegis_slave'|'aegis_multi', "
      "default 'auto'\n");
  printf(
      "    DEVICE_NAME: the device name to be operate, ex. "
      "sis_aegis_hid_bridge_touch_device\n");
  printf(
      "    WAIT_TIME:   when FW is broken, update master first and wait for "
      "WAIT_TIME secs.\n");
  printf(
      "    SLAVEADDR:   specify slave address used in aegis_slave, default is "
      "5\n");
  printf("    TIMEOUT:     integer, retry times for NACK, default 100\n");
  printf(
      "    DELAY:       integer, delay time between every each query retry in "
      "millisecond, default 1\n");
  printf(
      "    IO_DELAY:    integer, delay time after each query in millisecond, "
      "default 0\n");
  printf("    LOG_FILE_PATH:    the file to write log to, can be 'stdout'.\n");
  printf("    -l:          verbose mode\n");
  printf("    -b:          update bootloader\n");
  printf("    -ba:         update bootloader automatically\n");
  printf("    -g:          reserve RO data\n");
  printf("    -r:          only update parameter\n");
  printf("    --force:     force update\n");
  printf("    --recovery:  device in recovery mode\n");
  printf("    --jump:      jump parameter validation\n");
  printf("    --single:    assume a single device without slaves\n");
  printf(
      "    --no-diag:   disable diagnosis mode changing automatically *** only "
      "for experiment, don't use in normal flow ***\n");
  printf(
      "    --no-power:  disable power mode changing automatically *** only for "
      "experiment, don't use in normal flow ***\n");
  printf("    ps. don't use space around '='\n");
  printf("    \n");
  printf("        -r and -b is conflict\n");
  printf("        -r and -g is conflict\n");
  printf("        --force and --jump is conflict\n");
  printf(
      "    'aegis_slave' won't be detected in aegis_auto, only can be "
      "specified manually\n");
}

/*===========================================================================*/
Parameter::Parameter()
    : con(SiSTouchIO::CON_AUTO),
      max_retry(DEFAULT_RETRY),
      delay(DEFAULT_DELAY),
      verbose(DEFAULT_VERBOSE),
      io_Delay(DEFAULT_IO_DELAY),
      slave_Addr(DEFAULT_SLAVE_ADDR),
      multi_Num(DEFAULT_MULTI_NUM),
      changeDiagMode(DEFAULT_CHANGE_DIAGMODE),
      changePWRMode(DEFAULT_CHANGE_PWRMODE),
      m_detectHidrawFlag(DEFAULT_DETECT_HIDRAW_FLAG),
      m_isRecoceryDevice(false) {
  snprintf(connect, MAX_CONNECT_CHAR, "auto");
  snprintf(devName, MAX_DEVNAME_LENGTH, "%s", "");
  snprintf(log_to, PATH_MAX, "%s", "");
}
/*===========================================================================*/
Parameter::~Parameter() {}
/*===========================================================================*/
int Parameter::parse(int argc, char** argv) {
  if (argc <= 1) {
    return 0;
  }

  LOG(INFO) << "parsing command parameter";

  for (int i = 1; i < argc; i++) {
    LOG(INFO) << i << ": "
              << "[" << argv[i] << "]";

    if (!parseArgument(argv[i])) {
      LOG(ERROR) << "unknown parameter";
      return 0;
    }
  }

  return check();
}
/*===========================================================================*/
bool Parameter::parseArgument(char* arg) {
  if (strstr(arg, "-i=") == arg) {
    setConnectType(arg + 3);
  } else if (strstr(arg, "-t=") == arg) {
    sscanf(arg + 3, "%d", &max_retry);
    LOG(INFO) << "set max number of retries: " << max_retry;
  } else if (strstr(arg, "-d=") == arg) {
    sscanf(arg + 3, "%d", &delay);
    LOG(INFO) << "set delay in ms: " << delay;
    delay *= 1000;
  } else if (strstr(arg, "-n=") == arg) {
    // strcpy(devName, arg + 3);
    snprintf(devName, MAX_DEVNAME_LENGTH, "%s", arg + 3);
    LOG(INFO) << "device name: " << devName;
  } else if (strcmp(arg, "-l") == 0) {
    verbose = 1;
    LOG(INFO) << "verbose mode";
  } else if (strstr(arg, "-io_delay=") == arg) {
    sscanf(arg + 10, "%d", &io_Delay);
    LOG(INFO) << "set io_delay in ms: " << io_Delay;
  } else if (strstr(arg, "-slave=") == arg) {
    sscanf(arg + 8, "%d", &slave_Addr);
    LOG(INFO) << "slave addr :" << slave_Addr;
  } else if (strstr(arg, "-log_to=") == arg) {
    snprintf(log_to, PATH_MAX, "%s", arg + 8);
    LOG(INFO) << "log to :" << log_to;
  } else if (strcmp(arg, "--no-diag") == 0) {
    changeDiagMode = 0;
    LOG(INFO) << "disable diagnosis mode changing automatically "
              << "*** only for experiment, don't use in normal flow ***";
  } else if (strcmp(arg, "--no-power") == 0) {
    changePWRMode = 0;
    LOG(INFO) << "disable power mode changing automatically "
              << "*** only for experiment, don't use in normal flow  ***";
  } else if (strcmp(arg, "--hidraw") == 0) {
    m_detectHidrawFlag = 1;
    LOG(INFO) << "Enable detect hidraw device";
  } else if (strcmp(arg, "--recovery") == 0) {
    m_isRecoceryDevice = true;
    LOG(INFO) << "Recovery mode device";
  } else {
    return false;
  }
  return true;
}
/*===========================================================================*/
int Parameter::check() {
  if (strcmp(connect, AUTO) != 0 && strcmp(connect, ZEUS_I2C) != 0 &&
      strcmp(connect, ZEUS_USB) != 0 &&
      strcmp(connect, ZEUS_INTERNALUSB) != 0 &&
      strcmp(connect, AEGIS_I2C) != 0 && strcmp(connect, AEGIS_USB) != 0 &&
      strcmp(connect, AEGIS_INTERNALUSB) != 0 &&
      strcmp(connect, AEGIS_SLAVE) != 0 && strcmp(connect, AEGIS_MULTI) != 0) {
    LOG(ERROR) << "error: connect type not supported: " << connect;
    return 0;
  }

  return 1;
}
/*===========================================================================*/
int Parameter::setActiveDeviceName() {
  if (strlen(devName) > 0) {
    if (SiSTouchIO::setActiveDeviceName(devName, strlen(devName)) != 1) {
      LOG(WARNING) << "Can't find device: " << devName;
      return 0;
    }
  }

  return 1;
}
/*===========================================================================*/
void Parameter::setConnectType(char* connectName) {
  snprintf(connect, MAX_CONNECT_CHAR, "%s", connectName);

  if (strcmp(connectName, AUTO) == 0) {
    con = SiSTouchIO::CON_AUTO;
    LOG(INFO) << "target select: " << AUTO;
  } else if (strcmp(connectName, ZEUS_I2C) == 0) {
    con = SiSTouchIO::CON_ZEUS_I2C;
    LOG(INFO) << "target select: " << ZEUS_I2C;
  } else if (strcmp(connectName, ZEUS_USB) == 0) {
    con = SiSTouchIO::CON_ZEUS_USB;
    LOG(INFO) << "target select: " << ZEUS_USB;
  } else if (strcmp(connectName, ZEUS_INTERNALUSB) == 0) {
    con = SiSTouchIO::CON_ZEUS_INTERUSB;
    LOG(INFO) << "target select: " << ZEUS_INTERNALUSB;
  } else if (strcmp(connectName, AEGIS_I2C) == 0) {
    con = SiSTouchIO::CON_AEGIS_I2C;
    LOG(INFO) << "target select: " << AEGIS_I2C;
  } else if (strcmp(connectName, AEGIS_USB) == 0) {
    con = SiSTouchIO::CON_AEGIS_USB;
    LOG(INFO) << "target select: " << AEGIS_USB;
  } else if (strcmp(connectName, AEGIS_INTERNALUSB) == 0) {
    con = SiSTouchIO::CON_AEGIS_INTERUSB;
    LOG(INFO) << "target select: " << AEGIS_INTERNALUSB;
  } else if (strcmp(connectName, AEGIS_SLAVE) == 0) {
    con = SiSTouchIO::CON_AEGIS_SLAVE;
    LOG(INFO) << "target select: " << AEGIS_SLAVE;
  } else if (strcmp(connectName, AEGIS_MULTI) == 0) {
    con = SiSTouchIO::CON_AEGIS_MULTI;
    LOG(INFO) << "target select: " << AEGIS_MULTI;
  } else if (strcmp(connectName, CUSTOM) == 0) {
    con = SiSTouchIO::CON_CUSTOM;
    LOG(INFO) << "target select: " << CUSTOM;
  } else {
    con = SiSTouchIO::CON_AUTO;
    snprintf(connect, MAX_CONNECT_CHAR, "%s", AUTO);
    LOG(INFO) << "target select: " << AUTO;
  }

  return;
}
/*===========================================================================*/
void Parameter::print_sep() { LOG(INFO) << "-----"; }
/*===========================================================================*/

int SiSTouchAdapter::invert_endian(int data) {
  int out = 0;

  for (unsigned int i = 0; i < sizeof(int); i++) {
    int shift = (sizeof(int) - i - 1) * 8;
    out |= ((data >> i * 8) & 0xff) << shift;
  }

  return out;
}

bool SiSTouchAdapter::read_from_address(unsigned int addr, int* buffer,
                                        int size) {
  int read_Length = get_READ_LENGTH();
  bool isVirtual = isVirtualAddr(addr);

  for (int i = 0; i < size; i += read_Length) {
    int length = ((size - i) >= read_Length) ? read_Length : (size - i);
    int ret = 0;

    // sometimes device is in sleep
    for (int retry = 0; retry < SLEEP_RETRY; retry++) {
      ret = m_io->call_86(addr, buffer + i, length);
      if (ret >= 0) {
        break;
      }

      // 100ms
      usleep(SLEEP_RETRY_TIME);
    }

    if (ret < 0) {
      LOG(ERROR) << "Read(0x86) Fail";
      return false;
    }

    if (!isVirtual) {
      addr += (length * sizeof(int));
    } else {
      addr += length;
    }
  }

  return true;
}

bool SiSTouchAdapter::isVirtualAddr(unsigned int addr) {
  if ((addr & getAddressBlockMask()) == getVirtualAddressMask()) {
    return true;
  } else {
    return false;
  }
}

int SiSTouchAdapter::read_file_to_buffer(FILE* file, int32_t offset, int size,
                                         int* buf) {
  fseek(file, offset, SEEK_SET);
  return fread(buf, sizeof(int), size, file);
}

bool SiSTouchAdapter::write_buffer_to_address(int* buf, int offset,
                                              int buf_size, int length,
                                              unsigned int addr) {
  int read_Length = get_READ_LENGTH();
  int* temp = new int[read_Length];

  int ret = 0;

  int send_legnth = 0;
  int actual_pending_byte_length = 0;
  int actual_pending_int_length = 0;

  while (send_legnth < length) {
    unsigned int inc = std::min(int(_12K), length - send_legnth);
    // unsigned int inc =
    //     length > send_legnth + _12K ? _12K : length - send_legnth;

    send_legnth += inc;
    int pack_num = DIV_ROUND_UP(inc, (read_Length * sizeof(int)));
    // int pack_num = inc / (read_Length * sizeof(int)) +
    //               ((inc % (read_Length * sizeof(int))) == 0 ? 0 : 1);

    int idx_base = offset / sizeof(int);

    for (int block_retry = 0; block_retry < 33; block_retry++) {
      ret = 0;
      LOG(INFO) << "block_retry: " << block_retry + 1 << " of max 33";

      LOG(INFO) << StringPrintf("Write to addr = %08x pack_num = %d", addr,
                                pack_num);

      ret = m_io->call_83(addr, pack_num);
      if (ret < 0) {
        LOG(ERROR) << "Write Header(0x83) fail";
        continue;
      }

      for (int i = 0; i < pack_num; i++) {
        int cpy_length = std::min(
            read_Length,
            std::min(buf_size - idx_base - i * read_Length,
                     int(DIV_ROUND_UP(inc, sizeof(int))) - i * read_Length));
        memset(temp, 0, read_Length * sizeof(int));
        memcpy(temp, &buf[i * read_Length + idx_base],
               cpy_length * sizeof(int));

        actual_pending_byte_length =
            get_actual_pending_byte_length(inc - i * read_Length * sizeof(int));
        actual_pending_int_length =
            DIV_ROUND_UP(actual_pending_byte_length, sizeof(int));

        printf(".");
        fflush(stdout);

        ret = m_io->call_84(temp, actual_pending_int_length);

        if (ret < 0) {
          break;
        }
      }

      printf("\n");

      SET_TEST_POINT(interrupt_phase == current_phase);

      if (ret < 0) {
        LOG(ERROR) << "Write(0x84) fail";
        continue;
      }

      ret = m_io->call_04();
      LOG(INFO) << "waiting for Query(0x04)";
      usleep(1 * 1000 * 1000);

      if (ret < 0) {
        LOG(ERROR) << "Query(0x04) fail";
        continue;
      } else {
        LOG(INFO) << "write all " << pack_num << " packets successfully";
        break;
      }
    }

    if (ret < 0) {
      LOG(ERROR) << "retry time out";
      break;
    }

    addr += inc;
    offset += inc;
  }

  delete[] temp;

  return (ret < 0) ? false : true;
}

int32_t SiSTouchAdapter::getFileLength(FILE* file) {
  int32_t file_length = 0;

  if (file == 0) {
    return 0;
  }

  fseek(file, 0L, SEEK_END);
  file_length = ftell(file);
  fseek(file, 0L, SEEK_SET);

  return file_length;
}

bool SiSTouchAdapter::compareId(int* idA, int* idB, int length) {
  if (idA == nullptr || idB == nullptr) return false;
  if (length <= 0) return false;
  for (int i = 0; i < length; i++) {
    if (idA[i] != idB[i]) {
      return false;
    }
  }
  return true;
}

int SiSTouchAdapter::doCheckFWState() {
  int input_boot_flag[BOOT_FLAG_LENGTH] = {0};
  int device_boot_flag[BOOT_FLAG_LENGTH] = {0};

  input_boot_flag[0] = getBootFlagValue();

  if (getBootFlag(device_boot_flag, BOOT_FLAG_LENGTH) != SUCCESS) {
    return ERROR_FAIL_BOOT_FLAG;
  }

  bool result = compareId(input_boot_flag, device_boot_flag, BOOT_FLAG_LENGTH);

  return result ? RESULT_SAME : RESULT_DIFFERENT;
}

int SiSTouchAdapter::getDeviceId(int* device_firmware_id, int id_Length) {
  int ret = SUCCESS;

  if (!device_firmware_id) {
    LOG(ERROR) << "null device_firmware_id array";
    return ERROR_NULL_INPUT_ARRAY;
  }

  if (id_Length > FIRMWARE_ID_LENGTH) {
    id_Length = FIRMWARE_ID_LENGTH;
  }

  if (m_io->stop_driver() < 0) {
    LOG(ERROR) << "stop driver fail";
    return ERROR_STOP_DRIVER;
  }

  bool result =
      read_from_address(getFirmwareIdAddr(), device_firmware_id, id_Length);
  if (result) {
    LOG(INFO) << "device firmware id: ";
    for (int i = 0; i < id_Length; i++) {
      device_firmware_id[i] = invert_endian(device_firmware_id[i]);
      LOG(INFO) << StringPrintf("%08x", device_firmware_id[i]);
    }
  } else {
    ret = ERROR_IN_READ_DATA;
    LOG(ERROR) << "cannot read firmware id";
  }

  if (m_io->start_driver() < 0) {
    LOG(ERROR) << "start driver fail";

    if (ret == SUCCESS) {
      ret = ERROR_START_DRIVER;
    }
  }

  return ret;
}

int SiSTouchAdapter::getBootFlag(int* boot_flag, int flag_Length) {
  int ret = SUCCESS;

  if (!boot_flag) {
    LOG(ERROR) << "null *boot_flag entry";
    return ERROR_NULL_INPUT_ARRAY;
  }

  if (flag_Length > BOOT_FLAG_LENGTH) {
    flag_Length = BOOT_FLAG_LENGTH;
  }

  if (m_io->stop_driver() < 0) {
    LOG(ERROR) << "stop driver fail";
    return ERROR_STOP_DRIVER;
  }

  bool result = read_from_address(getBootFlagAddr(), boot_flag, flag_Length);
  if (result) {
    LOG(INFO) << "fw boot flag: ";
    for (int i = 0; i < flag_Length; i++) {
      boot_flag[i] = invert_endian(boot_flag[i]);
      LOG(INFO) << StringPrintf("%08x", boot_flag[i]);
    }
    printf("\n");
  } else {
    ret = ERROR_IN_READ_DATA;
    LOG(ERROR) << "cannot read boot flag";
  }

  if (m_io->start_driver() < 0) {
    LOG(ERROR) << "start driver fail";

    if (ret == SUCCESS) {
      ret = ERROR_START_DRIVER;
    }
  }

  return ret;
}

bool SiSTouchAdapter::checkFileBufIsValid(int file_length, int* buf,
                                          int buf_length) {
  if (file_length > _64K) {
    LOG(ERROR) << "file length exceed 64K: " << file_length;
    return false;
  }

  if (!isSiSFWFile(buf, buf_length)) {
    LOG(ERROR) << "file is not sis fw file";
    return false;
  }

  return true;
}

int SiSTouchAdapter::doUpdate(FILE* file, bool update_bootloader,
                              bool update_bootloader_auto, bool reserve_RODATA,
                              bool update_parameter, bool force_update,
                              bool jump_check, const bool isRecoveryDevice) {
  if (!file) {
    LOG(ERROR) << "null file descriptor";
    return ERROR_NULL_FILE;
  }

  int buf[MAX_BUF_LENGTH] = {0};

  int file_length = getFileLength(file);

  /* int read_Length = */ read_file_to_buffer(file, 0, MAX_BUF_LENGTH, buf);

  // int index = 0;

  int ret = SUCCESS;

  bool result = false;

  bool* isNewBootloader = new bool(false);

  if (!jump_check) {
    if (!checkFileBufIsValid(file_length, buf, MAX_BUF_LENGTH)) {
      return ERROR_INVALID_FILE;
    }
  }

  int dev_fw_id[FIRMWARE_ID_LENGTH] = {0};
  prepareBuffer(buf, MAX_BUF_LENGTH, dev_fw_id);

  if (m_io->stop_driver() < 0) {
    LOG(ERROR) << "stop driver fail";
    return ERROR_STOP_DRIVER;
  }

  if (jump_check) {
    LOG(INFO) << "skip parameter validation.";
    *isNewBootloader = true;
    ret = SUCCESS;
  } else {
    ret = parameterCheck(&update_bootloader, update_bootloader_auto,
                         isNewBootloader, &reserve_RODATA, &update_parameter,
                         force_update, isRecoveryDevice, buf, file_length);
  }

  if (ret == SUCCESS) {
    result = updateFlow(buf, MAX_BUF_LENGTH, file_length, update_bootloader,
                        *isNewBootloader, reserve_RODATA, update_parameter);
    if (!result) {
      ret = ERROR_IN_UPDATEFLOW;
    }
  }

  if (m_io->start_driver() < 0) {
    LOG(ERROR) << "start driver fail";

    if (ret == SUCCESS) {
      ret = ERROR_START_DRIVER;
    }
  }

  return ret;
}

int SiSTouchAdapter::setActiveDeviceName(char* devName, int devNameLen) {
  return SiSTouchIO::setActiveDeviceName(devName, devNameLen);
}

SiSTouchAdapter* SiSTouchAdapter::prepareAdapter(Parameter param) {
  int con = param.con;
  int max_retry = param.max_retry;
  int retry_delay = param.delay;
  int verbose = param.verbose;
  int ioDelay = param.io_Delay;
  int changeDiagMode = param.changeDiagMode;
  int changePWRMode = param.changePWRMode;

  // force to only detect hidraw
  int detectHidrawFlag = 1;  // param.m_detectHidrawFlag;

  if (con == CON_AUTO) {
    con = SiSTouchIO::getDeviceType(param.devName, max_retry, retry_delay,
                                    verbose, ioDelay, changeDiagMode,
                                    changePWRMode, detectHidrawFlag);
  }

  LOG(INFO) << "con=" << con;

  // only detect hidraw
  if (con == SiSTouchIO::CON_AEGIS_MULTI_FOR_NEW_DRIVER) {
    return new AegisMultiSiSTouchAdapter(max_retry, retry_delay, verbose,
                                         ioDelay, changeDiagMode, changePWRMode,
                                         detectHidrawFlag);
  } else {
    // no-op
  }

  LOG(ERROR) << "not suitable sis touch device";
  return 0;
}

int SiSTouchAdapter::setIODelay(int delayms) {
  return m_io->setIODelay(delayms);
}

int SiSTouchAdapter::doDetectSlaveNum() { return 0; }

static inline unsigned char BCD(unsigned char x) {
  return ((x / 10) << 4) | (x % 10);
}

static int getTimestamp() {
  time_t rawtime;
  struct tm timeinfo;
  int mmddHHMM = 0;

  time(&rawtime);
  localtime_r(&rawtime, &timeinfo);
  mmddHHMM = BCD(static_cast<unsigned char>(timeinfo.tm_mon + 1));
  mmddHHMM |= BCD(static_cast<unsigned char>(timeinfo.tm_mday)) << 8;
  mmddHHMM |= BCD(static_cast<unsigned char>(timeinfo.tm_hour)) << 16;
  mmddHHMM |= BCD(static_cast<unsigned char>(timeinfo.tm_min)) << 24;

  return mmddHHMM;
}

int AegisSiSTouchAdapter::updateFlow(int* buf, int buf_length, int file_length,
                                     bool update_bootloader,
                                     bool isNewBootloader, bool reserve_RODATA,
                                     bool update_parameter, bool restart) {
  if (isNewBootloader) {
    if (update_parameter) {
      LOG(INFO) << "write Parameters...";
      current_phase = 1;
      if (!write_buffer_to_address(buf, FIRMWARE_INFO_BASE, buf_length, _4K,
                                   FIRMWARE_INFO_BASE)) {
        LOG(ERROR) << "write Parameters fail";
        return 0;
      }
    } else {
      if (reserve_RODATA) {
        int RODataBuf[_8K / sizeof(int)] = {0};
        memset(RODataBuf, 0, _8K / sizeof(int) * sizeof(int));

        LOG(INFO) << "acquire RO data...";
        if (!read_from_address(DEVICE_CALIBRATION_DATA_ADDRESS, RODataBuf,
                               _8K / sizeof(int))) {
          LOG(ERROR) << "cannot acquire RO data";
          return 0;
        }

        memcpy(buf + CALIBRATION_BASE / sizeof(int), RODataBuf,
               _8K / sizeof(int) * sizeof(int));

        buf[BOOT_FLAG_BASE / sizeof(int)] = 0x0;

        LOG(INFO) << "clear boot flag...";
        if (!write_buffer_to_address(buf, CALIBRATION_BASE, buf_length, _8K,
                                     CALIBRATION_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }

        buf[BOOT_FLAG_BASE / sizeof(int)] = invert_endian(BOOT_FLAG_VALUE);
      } else {
        int fakeBuf[_8K / sizeof(int)] = {0};
        memset(fakeBuf, 0, _8K / sizeof(int) * sizeof(int));

        LOG(INFO) << "release RO data...";
        current_phase = 2;
        if (!write_buffer_to_address(fakeBuf, 0, _8K / sizeof(int), _8K,
                                     CALIBRATION_BASE)) {
          LOG(ERROR) << "release RO fail";
          return 0;
        }
      }

      if (update_bootloader) {
        LOG(INFO) << "write FW...";
        current_phase = 3;
        if (!write_buffer_to_address(buf, MAINCODE_BASE, buf_length, _48K,
                                     MAINCODE_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }

        LOG(INFO) << "write bootloader...";
        current_phase = 4;
        if (!write_buffer_to_address(buf, INITIAL_BASE, buf_length, _4K,
                                     INITIAL_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }

        LOG(INFO) << "write update code...";
        current_phase = 5;
        if (!write_buffer_to_address(buf, UPDATE_BASE, buf_length,
                                     file_length - _56K - _4K, UPDATE_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }

        LOG(INFO) << "write RO data...";
        current_phase = 6;
        if (!write_buffer_to_address(buf, CALIBRATION_BASE, buf_length, _8K,
                                     CALIBRATION_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }

      } else {
        LOG(INFO) << "write FW...";
        if (!write_buffer_to_address(buf, MAINCODE_BASE, buf_length, _56K,
                                     MAINCODE_BASE)) {
          LOG(ERROR) << "write FW fail";
          return 0;
        }
      }
    }
  } else {
    if (reserve_RODATA) {
      int RODataBuf[_8K / sizeof(int)] = {0};
      memset(RODataBuf, 0, _8K / sizeof(int) * sizeof(int));

      LOG(INFO) << "acquire RO data...";
      if (!read_from_address(DEVICE_CALIBRATION_DATA_ADDRESS, RODataBuf,
                             _8K / sizeof(int))) {
        LOG(ERROR) << "cannot acquire RO data";
        return 0;
      }

      memcpy(buf + CALIBRATION_BASE / sizeof(int), RODataBuf,
             _8K / sizeof(int) * sizeof(int));
    }

    if (update_parameter) {
      LOG(WARNING) << "Update parameter is not effect in old bootloader";
    }

    // original flow
    LOG(INFO) << "write FW...";
    current_phase = 3;
    if (!write_buffer_to_address(buf, 0, buf_length, file_length, 0)) {
      LOG(ERROR) << "write FW fail";
      return 0;
    }
  }

  if (restart) {
    if (m_io->call_82() < 0) {
      LOG(ERROR) << "Reset(0x82) fail";
      return 0;
    }
  }

  return 1;
}

void AegisSiSTouchAdapter::prepareBuffer(int* buf, int buf_length,
                                         int *device_firmware_id) {
  int index = 0;
  index = AEGIS_BASE / sizeof(int);
  if (index < buf_length) {
    buf[index] = invert_endian(0x50383132);
  }

  int getDeviceIdResult = getDeviceId(device_firmware_id, FIRMWARE_ID_LENGTH);
  if (getDeviceIdResult != SUCCESS && getDeviceIdResult != ERROR_START_DRIVER) {
    LOG(ERROR) << "ERROR: Fail to Get Device ID";
    exit(-1);
  }

  for (int i = 0; i < 3; i++) {
    buf[0xc0b0 / sizeof(int) + i] = invert_endian(device_firmware_id[i]);
  }

  buf[0xc0bc / sizeof(int)] = buf[0xc0ac / sizeof(int)];

  for (int i = 0; i < 3; i++) {
    buf[0xc0a0 / sizeof(int) + i] = buf[0xc004 / sizeof(int) + i];
  }

  buf[0xc0ac / sizeof(int)] = getTimestamp();
}

int AegisSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  if (buf[BUILDER_CHIPINFO_ADDRESS / sizeof(int)] != 0x73696765 ||
      buf[BUILDER_SISMARK_ADDRESS / sizeof(int)] != 0x53695300) {
    return 0;
  } else {
    return 1;
  }
}

int AegisI2CSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  int ret = AegisSiSTouchAdapter::isSiSFWFile(buf, buf_length);
  if (ret) {
    unsigned int interfaceData =
        invert_endian(buf[INTERFACE_FLAG_BASE / sizeof(int)]);
    unsigned int interfaceId =
        (interfaceData >> INTERFACE_SHIFT) & INTERFACE_MASK;
    unsigned int multichip_Selective_ChipId =
        (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    if (interfaceId != I2C_INTERFACE ||
        multichip_Selective_ChipId != NON_MULTI_DEVICE_FLAG) {
      LOG(ERROR) << StringPrintf(
          "interface not match. interface: %02x, multi_select: %02x",
          interfaceId, multichip_Selective_ChipId);
      ret = 0;
    } else {
      ret = 1;
    }
  }

  return ret;
}

int AegisUSBSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  int ret = AegisSiSTouchAdapter::isSiSFWFile(buf, buf_length);
  if (ret) {
    unsigned int interfaceData =
        invert_endian(buf[INTERFACE_FLAG_BASE / sizeof(int)]);
    unsigned int interfaceId =
        (interfaceData >> INTERFACE_SHIFT) & INTERFACE_MASK;
    unsigned int multichip_Selective_ChipId =
        (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    if (interfaceId != I2C_INTERFACE ||
        multichip_Selective_ChipId != NON_MULTI_DEVICE_FLAG) {
      LOG(ERROR) << StringPrintf(
          "interface not match. interface: %02x, multi_select: %02x",
          interfaceId, multichip_Selective_ChipId);
      ret = 0;
    } else {
      ret = 1;
    }
  }

  return ret;
}

int AegisInterUSBSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  int ret = AegisSiSTouchAdapter::isSiSFWFile(buf, buf_length);
  if (ret) {
    unsigned int interfaceData =
        invert_endian(buf[INTERFACE_FLAG_BASE / sizeof(int)]);
    unsigned int interfaceId =
        (interfaceData >> INTERFACE_SHIFT) & INTERFACE_MASK;
    unsigned int multichip_Selective_ChipId =
        (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    if (interfaceId != USB_INTERFACE ||
        multichip_Selective_ChipId == SLAVE_DEVICE0_FLAG ||
        multichip_Selective_ChipId == SLAVE_DEVICE1_FLAG ||
        multichip_Selective_ChipId == SLAVE_DEVICE2_FLAG) {
      LOG(ERROR) << StringPrintf(
          "interface not match. interface: %02x, multi_select: %02x",
          interfaceId, multichip_Selective_ChipId);
      ret = 0;
    } else {
      ret = 1;
    }
  }

  return ret;
}

int AegisSlaveSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  int ret = AegisSiSTouchAdapter::isSiSFWFile(buf, buf_length);
  if (ret) {
    unsigned int interfaceData =
        invert_endian(buf[INTERFACE_FLAG_BASE / sizeof(int)]);
    unsigned int interfaceId =
        (interfaceData >> INTERFACE_SHIFT) & INTERFACE_MASK;
    unsigned int multichip_Selective_ChipId =
        (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    unsigned int slaveAddr = getSlaveAddr();

    if (interfaceId != I2C_INTERFACE ||
        (multichip_Selective_ChipId != slaveAddr)) {
      LOG(ERROR) << StringPrintf(
          "interface not match. interface: %02x, multi_select: %02x",
          interfaceId, multichip_Selective_ChipId);
      ret = 0;
    } else {
      ret = 1;
    }
  }

  return ret;
}

int AegisMultiSiSTouchAdapter::isSiSFWFile(int* buf, int buf_length) {
  int ret = AegisSiSTouchAdapter::isSiSFWFile(buf, buf_length);
  if (ret) {
    unsigned int interfaceData =
        invert_endian(buf[INTERFACE_FLAG_BASE / sizeof(int)]);
    unsigned int interfaceId =
        (interfaceData >> INTERFACE_SHIFT) & INTERFACE_MASK;
    unsigned int multichip_Selective_ChipId =
        (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    unsigned int slaveAddr = getSlaveAddr();

    // if(multichip_Selective_ChipId == NON_MULTI_DEVICE_FLAG || ((slaveAddr >
    // 0) && (multichip_Selective_ChipId != slaveAddr)) || ((slaveAddr <= 0) &&
    // (multichip_Selective_ChipId != MASTER_DEVICE_FLAG) &&
    // (multichip_Selective_ChipId != BRIDGE_DEVICE_FLAG) ))
    if (((slaveAddr > 0) &&
         (multichip_Selective_ChipId != slaveAddr))) {  // remove master == 0x00
      LOG(ERROR) << StringPrintf(
          "interface not match. interface: %02x, multi_select: %02x",
          interfaceId, multichip_Selective_ChipId);
      ret = 0;
    } else {
      ret = 1;
    }
  }

  return ret;
}

void AegisMultiSiSTouchAdapter::setSlaveAddr(int slaveAddr) {
  return reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->setSlaveAddr(slaveAddr);
}

int AegisMultiSiSTouchAdapter::getSlaveAddr() {
  return reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->getSlaveAddr();
}

int AegisMultiSiSTouchAdapter::getStaticSlaveAddr(int idx) {
  switch (idx) {
    case INDEX_MASTER:
      return MASTER_DEVICE_ADDR;
    case INDEX_SLAVE0:
      return SLAVE_DEVICE0_ADDR;
    case INDEX_SLAVE1:
      return SLAVE_DEVICE1_ADDR;
    case INDEX_SLAVE2:
      return SLAVE_DEVICE2_ADDR;
    case INDEX_SLAVE3:
      return SLAVE_DEVICE3_ADDR;
    case INDEX_SLAVE4:
      return SLAVE_DEVICE4_ADDR;
    case INDEX_SLAVE5:
      return SLAVE_DEVICE5_ADDR;
    default:
      return NON_MULTI_DEVICE_ADDR;
  }
}

static inline void wait(const int sec) {
  LOG(INFO) << "wait for " << sec << " secs";
  for (int i = 0; i < sec; i++) {
    printf("...");
    fflush(stdout);
    sleep(1);
  }
  printf("\n");
}

static inline bool Is817MasterFWBroken(AegisMultiSiSTouchAdapter* adapter) {
  return adapter->doCheckFWState() != SiSTouchAdapter::RESULT_SAME;
}

int AegisMultiSiSTouchAdapter::doUpdate(FILE** multi_files, int multi_Num,
                                        const UpdateFWParameter& param,
                                        const bool isRecoveryDevice) {
  bool is817MasterFWBroken = Is817MasterFWBroken(this);

  if (!multi_files) {
    LOG(ERROR) << "null file descriptor";
    return ERROR_NULL_FILE;
  }
#define NUM_OF_FILES (7)

  int multi_files_buf[NUM_OF_FILES][MAX_BUF_LENGTH] = {{0}};

  int multi_files_length[NUM_OF_FILES] = {0};

  for (int i = 0; i < multi_Num; i++) {
    multi_files_length[i] = getFileLength(multi_files[i]);
  }

  // int multi_files_read_length[NUM_OF_FILES] = {0};

  for (int i = 0; i < multi_Num; i++) {
    /* multi_files_read_length[i] = */ read_file_to_buffer(
        multi_files[i], 0, MAX_BUF_LENGTH, multi_files_buf[i]);
  }

  // int index = 0;

  int ret = SUCCESS;

  bool result = false;

  bool multi_update_bootloader[NUM_OF_FILES] = {0};

  bool multi_update_bootloader_auto[NUM_OF_FILES] = {0};

  bool multi_isNewBootloader[NUM_OF_FILES] = {0};

  bool multi_reserve_RODATA[NUM_OF_FILES] = {0};

  bool multi_update_parameter[NUM_OF_FILES] = {0};

  bool multi_force_update[NUM_OF_FILES] = {0};
#undef NUM_OF_FILES

  if (!param.jump_check) {
    for (int i = 0; i < multi_Num; i++) {
      setSlaveAddr(getStaticSlaveAddr(i));
      if (!checkFileBufIsValid(multi_files_length[i], multi_files_buf[i],
                               MAX_BUF_LENGTH)) {
        return ERROR_INVALID_FILE;
      }
    }
  }

  int dev_fw_id[FIRMWARE_ID_LENGTH] = {0};
  for (int i = 0; i < multi_Num; i++) {
    prepareBuffer(multi_files_buf[i], MAX_BUF_LENGTH, dev_fw_id);
  }

  if (multi_Num == 1 && !isRecoveryDevice) {
    int new_fw_id[FIRMWARE_ID_LENGTH] = {0};
    int index = getFirmwareIdBase() / sizeof(int);
    new_fw_id[0] = invert_endian(multi_files_buf[0][index]);
    new_fw_id[1] = invert_endian(multi_files_buf[0][index + 1]);
    new_fw_id[2] = invert_endian(multi_files_buf[0][index + 2]);

    if (compareId(new_fw_id, dev_fw_id, FW_ID_LENGTH)) {
      LOG(INFO) << "The device has the same FW as system";
      return RESULT_SAME_FIRMWARE;
    }
  }

  if (m_io->stop_driver() < 0) {
    LOG(ERROR) << "stop driver fail";
    return ERROR_STOP_DRIVER;
  }

  for (int i = 0; i < multi_Num; i++) {
    if (i == 0) {
      LOG(INFO) << "master:";
    } else {
      LOG(INFO) << "slave :" << (i - 1);
    }

    multi_update_bootloader[i] = param.update_bootloader;

    multi_update_bootloader_auto[i] = param.update_bootloader_auto;

    multi_isNewBootloader[i] = false;

    multi_reserve_RODATA[i] = param.reserve_RODATA;

    multi_update_parameter[i] = param.update_parameter;

    multi_force_update[i] = param.force_update;

    setSlaveAddr(getStaticSlaveAddr(i));

    if (param.jump_check) {
      LOG(INFO) << "skip parameter validataion.";
      multi_isNewBootloader[i] = true;
      ret = SUCCESS;
    } else {
      ret = parameterCheck(
          &multi_update_bootloader[i], multi_update_bootloader_auto[i],
          &multi_isNewBootloader[i], &multi_reserve_RODATA[i],
          &multi_update_parameter[i], multi_force_update[i], isRecoveryDevice,
          multi_files_buf[i], multi_files_length[i]);
    }
  }

  if (ret == SUCCESS) {
    if (is817MasterFWBroken) {
      // update master first and wait for param.wait_time
      LOG(INFO) << "master:";

      setSlaveAddr(getStaticSlaveAddr(0));

      result =
          updateFlow(multi_files_buf[0], MAX_BUF_LENGTH, multi_files_length[0],
                     multi_update_bootloader[0], multi_isNewBootloader[0],
                     multi_reserve_RODATA[0], multi_update_parameter[0]);
      if (!result) {
        ret = ERROR_IN_UPDATEFLOW;
      }
      if (ret == SUCCESS) {
        // wait for master reboot
        wait(param.wait_time);

        for (int i = 1; i < multi_Num; i++) {
          LOG(INFO) << "slave :" << (i - 1);

          setSlaveAddr(getStaticSlaveAddr(i));

          // must restart when finish update all slaves FW
          result = updateFlow(multi_files_buf[i], MAX_BUF_LENGTH,
                              multi_files_length[i], multi_update_bootloader[i],
                              multi_isNewBootloader[i], multi_reserve_RODATA[i],
                              multi_update_parameter[i],
                              (i != multi_Num - 1) ? false : true);
          if (!result) {
            ret = ERROR_IN_UPDATEFLOW;
            break;
          }
        }
      }
    } else {
      // update slave first
      for (int i = 1; i < multi_Num; i++) {
        LOG(INFO) << "slave :" << (i - 1);

        setSlaveAddr(getStaticSlaveAddr(i));

        if (ret == SUCCESS) {
          result = updateFlow(multi_files_buf[i], MAX_BUF_LENGTH,
                              multi_files_length[i], multi_update_bootloader[i],
                              multi_isNewBootloader[i], multi_reserve_RODATA[i],
                              multi_update_parameter[i], false);
          if (!result) {
            ret = ERROR_IN_UPDATEFLOW;
            break;
          }
        }
      }

      if (ret == SUCCESS) {
        LOG(INFO) << "master:";

        setSlaveAddr(getStaticSlaveAddr(0));

        result = updateFlow(multi_files_buf[0], MAX_BUF_LENGTH,
                            multi_files_length[0], multi_update_bootloader[0],
                            multi_isNewBootloader[0], multi_reserve_RODATA[0],
                            multi_update_parameter[0]);
        if (!result) {
          ret = ERROR_IN_UPDATEFLOW;
        }
      }
    }
  }

  if (m_io->start_driver() < 0) {
    LOG(WARNING) << "start driver fail";

    if (ret == SUCCESS) {
      ret = ERROR_START_DRIVER;
    }
  }

  return ret;
}

int AegisSiSTouchAdapter::parameterCheck(
    bool* update_bootloader, const bool update_bootloader_auto,
    bool* isNewBootloader, bool* reserve_RODATA, bool* update_parameter,
    bool force_update, const bool isRecoveryDevice, int* buf, int file_Length) {
  int firmwareInfo[FIRMWARE_INFO_LENGTH] = {0};

  int bootInfo[BOOT_INFO_LENGTH] = {0};

  int mainCode[MAIN_CODE_LENGTH] = {0};

  int index = 0;

  int cur_fw_id[FW_ID_LENGTH] = {0};
  int new_fw_id[FW_ID_LENGTH] = {0};

  // read id info
  LOG(INFO) << "reading firmware info...";
  bool result = read_from_address(getFirmwareInfoAddr(), firmwareInfo,
                                  FIRMWARE_INFO_LENGTH);
  if (result) {
    LOG(INFO) << "reading firmware info : success";
    index = (getFirmwareIdBase() - getFirmwareInfoBase()) / sizeof(int);
    cur_fw_id[0] = invert_endian(firmwareInfo[index]);
    cur_fw_id[1] = invert_endian(firmwareInfo[index + 1]);
    cur_fw_id[2] = invert_endian(firmwareInfo[index + 2]);

    LOG(INFO) << StringPrintf("current firmware id: %08x, %08x, %08x",
                              cur_fw_id[0], cur_fw_id[1], cur_fw_id[2]);

    index = (getBootloaderIdBase() - getFirmwareInfoBase()) / sizeof(int);
    LOG(INFO) << StringPrintf("current bootloader id: %08x",
                              invert_endian(firmwareInfo[index]));

    index = (getMainCodeCrcBase() - getFirmwareInfoBase()) / sizeof(int);
    LOG(INFO) << StringPrintf("current maincode crc: %08x",
                              invert_endian(firmwareInfo[index]));

    index = (getBootloaderCrcBase() - getFirmwareInfoBase()) / sizeof(int);
    LOG(INFO) << StringPrintf("current bootloader crc: %08x",
                              invert_endian(firmwareInfo[index]));
  } else {
    LOG(ERROR) << "reading firmware info : fail";
  }

  index = getFirmwareIdBase() / sizeof(int);
  new_fw_id[0] = invert_endian(buf[index]);
  new_fw_id[1] = invert_endian(buf[index + 1]);
  new_fw_id[2] = invert_endian(buf[index + 2]);

  LOG(INFO) << StringPrintf("new firmware id: %08x, %08x, %08x", new_fw_id[0],
                            new_fw_id[1], new_fw_id[2]);

  index = getBootloaderIdBase() / sizeof(int);
  LOG(INFO) << StringPrintf("new bootloader id: %08x",
                            invert_endian(buf[index]));

  index = getMainCodeCrcBase() / sizeof(int);
  LOG(INFO) << StringPrintf("new maincode crc: %08x",
                            invert_endian(buf[index]));

  index = getBootloaderCrcBase() / sizeof(int);
  LOG(INFO) << StringPrintf("new bootloader crc: %08x",
                            invert_endian(buf[index]));

  // read calibration flag
  LOG(INFO) << "reading calibration and boot info";
  result = read_from_address(getBootFlagAddr(), bootInfo, BOOT_INFO_LENGTH);
  if (result) {
    LOG(INFO) << "reading calibration and boot info : success";
    index = (getBootFlagBase() - getBootFlagBase()) / sizeof(int);
    LOG(INFO) << StringPrintf("current boot flag: %08x",
                              invert_endian(bootInfo[index]));

    index = (getCalibrationFlagBase() - getBootFlagBase()) / sizeof(int);
    LOG(INFO) << StringPrintf("current calibration flag: %08x",
                              invert_endian(bootInfo[index]));
  } else {
    LOG(ERROR) << "reading calibration and boot info : fail";
  }

  index = getBootFlagBase() / sizeof(int);
  LOG(INFO) << StringPrintf("new boot flag: %08x", invert_endian(buf[index]));

  index = getCalibrationFlagBase() / sizeof(int);
  LOG(INFO) << StringPrintf("new calibration flag: %08x",
                            invert_endian(buf[index]));

  index = (getBootFlagBase() - getBootFlagBase()) / sizeof(int);
  unsigned int bootFlag = static_cast<unsigned>(invert_endian(bootInfo[index]));
  index = (getBootloaderIdBase() - getFirmwareInfoBase()) / sizeof(int);
  int deviceBootloaderId = invert_endian(firmwareInfo[index]);
  index = getBootloaderIdBase() / sizeof(int);
  int fileBootloaderId = invert_endian(buf[index]);
  int main = (deviceBootloaderId >> 16) & 0xffff;
  int sub = deviceBootloaderId & 0xffff;

  index = getMainCodeCrcBase() / sizeof(int);
  unsigned int mainCodeCrc =
      static_cast<unsigned int>(invert_endian(buf[index]));
  index = (getMainCodeCrcBase() - getFirmwareInfoBase()) / sizeof(int);
  unsigned int deviceMainCodeCrc =
      static_cast<unsigned int>(invert_endian(firmwareInfo[index]));
  index = getBootloaderCrcBase() / sizeof(int);
  int bootLoaderCrc = invert_endian(buf[index]);
  index = (getBootloaderCrcBase() - getFirmwareInfoBase()) / sizeof(int);
  int deviceBootLoaderCrc = invert_endian(firmwareInfo[index]);

  if (compareId(cur_fw_id, new_fw_id, FW_ID_LENGTH) && !isRecoveryDevice) {
    LOG(INFO) << "The device has the same FW as system.";
    return RESULT_SAME_FIRMWARE;
  }

  if (bootFlag == getBootFlagValue()) {
    if ((main == 0xffff && sub == 0xffff) || (main < 0x0001) ||
        (main == 0x0001 && sub <= 0x0001)) {
      *isNewBootloader = false;
      LOG(INFO) << "Old bootloader. Only can update whole fw.";
      *update_bootloader = true;
      if (*update_parameter) {
        LOG(INFO) << "Disable only update parameters";
        *update_parameter = false;
      }
    } else {
      *isNewBootloader = true;
      if (!*update_bootloader) {
        if (deviceBootloaderId != fileBootloaderId) {
          if (update_bootloader_auto) {
            LOG(INFO) << "Different bootloader id. "
                      << "Automatically update with -ba parameter.";
            *update_bootloader = true;
          } else {
            if (force_update) {
              LOG(INFO) << "Different bootloader id. "
                        << "Force updating with -b parameter.";
              *update_bootloader = true;
              if (*update_parameter) {
                LOG(INFO) << "Disable only update parameters";
                *update_parameter = false;
              }
            } else {
              LOG(INFO) << "Different bootloader id. "
                        << "Please update with -b parameter.";
              return ERROR_IN_COMPARE_BOOTLOADER;
            }
          }
        } else {
          if (bootLoaderCrc != deviceBootLoaderCrc) {
            if (update_bootloader_auto) {
              LOG(INFO) << "Different bootloader id. "
                        << "Automatically update with -ba parameter.";
              *update_bootloader = true;
            } else {
              if (force_update) {
                LOG(INFO) << "Different bootloader crc. "
                          << "Force updating with -b parameter.";
                *update_bootloader = true;
                if (*update_parameter) {
                  LOG(INFO) << "Disable only update parameters.";
                  *update_parameter = false;
                }
              } else {
                LOG(ERROR) << "Different bootloader crc. "
                           << "Please update with -b parameter.";
                return ERROR_IN_COMPARE_BOOTLOADER;
              }
            }
          }
        }
      }
    }
  } else {
    *isNewBootloader = true;
    LOG(INFO) << "Broken fw. take new flow";
    if (*update_parameter) {
      LOG(INFO) << "Disable only update parameters";
      *update_parameter = false;
    }
  }

  if (*update_parameter) {
    if (mainCodeCrc != deviceMainCodeCrc) {
      if (force_update) {
        LOG(INFO) << "Different main code crc. Disable only update parameters.";
        *update_parameter = false;
      } else {
        LOG(ERROR) << "Different main code crc. Cannot only update parameters.";
        return ERROR_IN_COMPARE_MAINCODE;
      }
    } else {
      if (mainCodeCrc == 0xffffffff) {
        LOG(INFO)
            << "The is no information in crc = 0xffffffff. Compare main code.";
        LOG(INFO) << "reading main code";

        for (unsigned int i = 0; i < MAIN_CODE_LENGTH;
             i += (0x0500 / sizeof(int))) {
          result = read_from_address(MAINCODE_ADDRESS + i * sizeof(int),
                                     mainCode + i, READ_LENGTH);
          if (!result) {
            break;
          }
        }

        if (result) {
          LOG(INFO) << "reading main code : success";
        } else {
          LOG(WARNING) << "reading main code : fail";
        }

        for (unsigned int i = 0; i < MAIN_CODE_LENGTH;
             i += (0x0500 / sizeof(int))) {
          for (int j = 0; j < READ_LENGTH; j++) {
            if (mainCode[i + j] != buf[(MAINCODE_BASE / sizeof(int)) + i + j]) {
              if (force_update) {
                LOG(INFO)
                    << "Different main code. Disable only update parameters";
                *update_parameter = false;
                break;
              } else {
                LOG(ERROR)
                    << "Different main code. Cannot only update parameters.";
                return ERROR_IN_COMPARE_MAINCODE;
              }
            }
          }

          if (!(*update_parameter)) {
            break;
          }
        }
      }
    }
  }

  if (*reserve_RODATA) {
    index = (getCalibrationFlagBase() - getBootFlagBase()) / sizeof(int);
    unsigned int calibrationFlag =
        static_cast<unsigned int>(invert_endian(bootInfo[index]));
    if (calibrationFlag != getCalibrationFlagValue()) {
      LOG(WARNING) << "Invalid RO data, drop RO data";
      *reserve_RODATA = false;
    }
  }

  return SUCCESS;
}

int AegisMultiSiSTouchAdapter::doDetectSlaveNum() {
  int oldSlaveAddr =
      reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->getSlaveAddr();

  int slaveNum = DEFALUT_SLAVE_NUM;
  int ret = 0;
  bool result = false;

  reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->setSlaveAddr(
      MASTER_DEVICE_ADDR);

  ret = m_io->stop_driver();
  if (ret < 0) {
    reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->setSlaveAddr(oldSlaveAddr);
    LOG(INFO) << "[SiSTouch][AegisMultiSiSTouchAdapter::doDetectSlaveNum] "
              << "stop_driver fail, slaveNum=" << slaveNum;
    return slaveNum;
  }

  int data[2] = {0};

  result = read_from_address(INTERFACE_FLAG_ADDR, data, 2);

  if (result) {
    bool isMultiDevice = false;

    unsigned int interfaceData = invert_endian(data[0]);
    int multiSelect = (interfaceData >> MULTI_SHIFT) & MULTI_MASK;

    if ((multiSelect == MASTER_DEVICE_FLAG) ||
        (multiSelect == BRIDGE_DEVICE_FLAG)) {
      isMultiDevice = true;
    }

    if (isMultiDevice) {
      unsigned int slaveNumData = invert_endian(data[1]);
      int num = (slaveNumData >> SLAVE_NUM_SHIFT) & SLAVE_NUM_MASK;
      slaveNum = (num >= INDEX_MASTER && num <= INDEX_SLAVE5) ? num : 1;
    } else {
      slaveNum = 0;
    }
  }

  m_io->start_driver();

  reinterpret_cast<SiSTouch_Aegis_Multi*>(m_io)->setSlaveAddr(oldSlaveAddr);

  m_io->setChipNum(slaveNum + 1);  // [20150304]
  return slaveNum;
}

void SiSTouchIO::AppendCRC(char data) {
  m_crc = (m_crc << 8) ^ crc16tab[((m_crc >> 8) ^ data) & 0x00FF];
}

int SiSTouchIO::make_83_buffer(int base, int pack_num) {
  int buf_idx = 0;
  unsigned char temp;

  m_crc = 0;

  m_buffer[buf_idx++] = 0x83;

  if (m_add_bytecount) m_buffer[buf_idx++] = 8;

  for (int i = 3; i >= 0; i--) {
    temp = (base >> (8 * i)) & 0xff;
    AppendCRC(temp);
    m_buffer[buf_idx++] = temp;
  }

  for (int i = 1; i >= 0; i--) {
    temp = (pack_num >> (8 * i)) & 0xff;
    AppendCRC(temp);
    m_buffer[buf_idx++] = temp;
  }

  for (int i = 1; i >= 0; i--) {
    temp = (m_crc >> (8 * i)) & 0xff;
    m_buffer[buf_idx++] = temp;
  }

  if (m_verbose) dbg_print_buffer_hex(m_buffer, buf_idx);

  return buf_idx;
}

int SiSTouchIO::make_84_buffer(const int* data, int size) {
  int buf_idx = 0;
  char temp;

  m_crc = 0;

  m_buffer[buf_idx++] = 0x84;

  if (m_add_bytecount) m_buffer[buf_idx++] = 14;

  for (int k = 0; k < size; k++) {
    // for ( int i = 3; i >= 0; i-- )
    for (int i = 0; i < 4; i++) {
      temp = (data[k] >> (8 * i)) & 0xff;
      AppendCRC(temp);
      m_buffer[buf_idx++] = temp;
    }
  }

  for (int i = 1; i >= 0; i--) {
    temp = (m_crc >> (8 * i)) & 0xff;
    m_buffer[buf_idx++] = temp;
  }

  if (m_verbose) dbg_print_buffer_hex(m_buffer, buf_idx);

  return buf_idx;
}

int SiSTouchIO::make_85_buffer(unsigned char data) {
  int buf_idx = 0;
  m_buffer[buf_idx++] = 0x85;
  if (m_add_bytecount) m_buffer[buf_idx++] = 1;
  m_buffer[buf_idx++] = data;

  if (m_verbose) dbg_print_buffer_hex(m_buffer, buf_idx);

  return buf_idx;
}

int SiSTouchIO::make_86_buffer(int addr) {
  int buf_idx = 0;
  unsigned char temp;

  m_crc = 0;

  m_buffer[buf_idx++] = 0x86;

  if (m_add_bytecount) m_buffer[buf_idx++] = 6;

  for (int i = 3; i >= 0; i--) {
    temp = (addr >> (8 * i)) & 0xff;
    AppendCRC(temp);
    m_buffer[buf_idx++] = temp;
  }

  for (int i = 1; i >= 0; i--) {
    temp = (m_crc >> (8 * i)) & 0xff;
    m_buffer[buf_idx++] = temp;
  }

  if (m_verbose) dbg_print_buffer_hex(m_buffer, buf_idx);

  return buf_idx;
}

int SiSTouchIO::setIODelay(int delayms) {
  if (delayms >= 0) {
    m_IODelayms = delayms;
  } else {
    m_IODelayms = 0;
  }
  LOG(INFO) << StringPrintf("IODelay set to %d ms", m_IODelayms);
  return m_IODelayms;
}

// 20130904 for multi device - begin
int SiSTouchIO::device_Exist() {
#ifdef VIA_IOCTL
  struct stat s;
  if (stat(m_activeDeviceName, &s) == 0) {
    if (S_ISBLK(s.st_mode) || S_ISCHR(s.st_mode)) {
      return 1;
    }
  }
  return 0;
#else
  return 0;
#endif
}

int SiSTouchIO::setActiveDeviceName(char* devName, int devNameLen) {
  if (devNameLen > DEVNAME_MAX_LEN) {
    return ERROR;
  }

  // snprintf(m_activeDeviceName, DEVNAME_MAX_LEN, "/dev/%s", devName);
  snprintf(m_activeDeviceName, DEVNAME_MAX_LEN, "%s", devName);
  return device_Exist();  // 1:find device, 0:not found
}

char* SiSTouchIO::getActiveDeviceName() {
  if (strlen(m_activeDeviceName) > 0)
    return m_activeDeviceName;
  else
    return NULL;
}

int SiSTouchIO::getDeviceType(char* device_name, int max_retry, int retry_delay,
                              int verbose, int ioDelay, int changeDiagMode,
                              int changePWRMode) {
  return getDeviceType(device_name, max_retry, retry_delay, verbose, ioDelay,
                       changeDiagMode, changePWRMode, 0);
}

int SiSTouchIO::getDeviceType(char* device_name, int max_retry, int retry_delay,
                              int verbose, int ioDelay, int changeDiagMode,
                              int changePWRMode, int detectHidrawFlag) {
  int con = SiSTouchIO::CON_NONE;

  if (detectHidrawFlag == 1) {
    // only detect hidraw

    LOG(INFO) << "Only detect hidraw";

    SisTouchFinder sisTouchFinder;
    if (sisTouchFinder.isSisTouchHid(device_name)) {
      LOG(INFO) << "valid input device name";
      // snprintf(device_name, DEVNAME_MAX_LEN, "/dev/%s", device_name);
    } else {
      if (std::string(device_name) == "")
        LOG(INFO) << "invalid input device name";
      device_name = sisTouchFinder.autoDetectDevicePath();
    }

    if (device_name != 0) {
      if (sisTouchFinder.getDeviceType() == SisTouchFinder::USB_817) {
        con = CON_AEGIS_MULTI_FOR_NEW_DRIVER;
      }

      snprintf(m_activeDeviceName, DEVNAME_MAX_LEN, "%s", device_name);
      // delete device_name;

      LOG(INFO) << "find activeDeviceName=" << m_activeDeviceName;
      return con;
    }
  }

  return con;
}
// 20130904 for multi device - end

void SiSTouchIO::dbg_print_buffer_hex(const unsigned char* data, int size) {
  LOG(INFO) << "size = " << size;

  int strSize = get_MAX_SIZE() * 5;

  char* str = new char[strSize];

  if (size > 0 && size <= static_cast<int>(get_MAX_SIZE())) {
    snprintf(str, strSize, "%02x", data[0]);

    for (int i = 1; i < size; i++) {
      snprintf(str, strSize, "%s %02x", str, data[i]);
    }

    LOG(INFO) << str;

  } else if (size > static_cast<int>(get_MAX_SIZE())) {
    snprintf(str, strSize, "%02x", data[0]);

    for (int i = 1; i < static_cast<int>(get_MAX_SIZE()); i++) {
      snprintf(str, strSize, "%s %02x", str, data[i]);
    }

    LOG(INFO) << "========================Too long return "
              << "packets=======================";
    LOG(INFO) << StringPrintf("The first %d bytes:", get_MAX_SIZE());
    LOG(INFO) << str;
  }

  delete[] str;
}

void SiSTouchIO::dbg_print_buffer_hex(int cmd, const unsigned char* data,
                                      int size) {
  LOG(INFO) << StringPrintf("CMD %02x: ", cmd);
  dbg_print_buffer_hex(data, size);
}

int SiSTouchIO::parse_syscall_return_value(int ret, int type) {
  int temp = 0;
  temp = errno;

  if (m_verbose) {
    LOG(ERROR) << "function type: " << type;
    LOG(ERROR) << "syscall error code: " << temp;
  }

  if ((type & NOT_SIS_IO_FLAG) != 0) {
    if (temp == ENOSYS) {
      return ERROR_SYSCALL;
    } else if (temp == EACCES) {
#ifdef VIA_IOCTL
      return ERROR_ACCESS;
    } else if (temp == ENOENT) {
      return ERROR_ENTRY_NOT_EXIST;
    } else if (temp == ENODEV) {
      return ERROR_DEVICE_NOT_EXIST;
#endif
    } else {
      return -temp;
    }
  } else {
    if (temp == ENOSYS) {
      return ERROR_SYSCALL;
    } else if ((type & ONLY_RETURN_ZERO_FLAG) != 0) {
      return ERROR_SYSCALL;
    } else {
      return -temp;
    }
  }
}

// [20150304]
void SiSTouchIO::setChipNum(int chipNum) { this->m_chipNum = chipNum; }

SiSTouchIO::~SiSTouchIO() {
#ifdef VIA_IOCTL
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = -1;
  }
#endif
}

OpenShortResult::OpenShortResult()
    : mWidth(0),
      mHeight(0),
      mPointResult(0),
      mLineResult(0),
      mPointNG(0),
      mSecondNG(0),
      mLineNG(0),
      mCycleTxDiff(0),
      mTXPointShortResult(0),
      mCycleRxDiff(0),
      mRXPointShortResult(0),
      mLineNearFarValue(0),
      mLineNearFarResult(0),
      lostSide(0) {
  mPointNG = 0;
  mLineNG = 0;
  mPointResult = 0;
  mLineResult = 0;

  setWH(DEFAULT_W, DEFAULT_H);
}

OpenShortResult::OpenShortResult(int w, int h)
    : mWidth(w),
      mHeight(h),
      mPointResult(0),
      mLineResult(0),
      mPointNG(0),
      mSecondNG(0),
      mLineNG(0),
      mCycleTxDiff(0),
      mTXPointShortResult(0),
      mCycleRxDiff(0),
      mRXPointShortResult(0),
      mLineNearFarValue(0),
      mLineNearFarResult(0),
      lostSide(0) {
  mPointNG = 0;
  mLineNG = 0;
  mPointResult = 0;
  mLineResult = 0;

  setWH(w, h);
}

OpenShortResult::~OpenShortResult() {
  delete[] mPointResult;
  delete[] mLineResult;

  delete[] mCycleTxDiff;         // MutualShortExtraCheck
  delete[] mTXPointShortResult;  // MutualShortExtraCheck
  delete[] mCycleRxDiff;         // MutualShortExtraCheck
  delete[] mRXPointShortResult;  // MutualShortExtraCheck

  delete[] mLineNearFarValue;   // MutualNearFarCheck
  delete[] mLineNearFarResult;  // MutualNearFarCheck
}

void OpenShortResult::setWH(int w, int h) {
  mPointNG = 0;
  mLineNG = 0;

  if (mLineResult != 0) {
    delete[] mPointResult;
    delete[] mLineResult;

    delete[] mCycleTxDiff;         // MutualShortExtraCheck
    delete[] mTXPointShortResult;  // MutualShortExtraCheck
    delete[] mCycleRxDiff;         // MutualShortExtraCheck
    delete[] mRXPointShortResult;  // MutualShortExtraCheck

    delete[] mLineNearFarValue;   // MutualNearFarCheck
    delete[] mLineNearFarResult;  // MutualNearFarCheck

    mPointResult = 0;
    mLineResult = 0;

    mCycleTxDiff = 0;         // MutualShortExtraCheck
    mTXPointShortResult = 0;  // MutualShortExtraCheck
    mCycleRxDiff = 0;         // MutualShortExtraCheck
    mRXPointShortResult = 0;  // MutualShortExtraCheck

    mLineNearFarValue = 0;   // MutualNearFarCheck
    mLineNearFarResult = 0;  // MutualNearFarCheck
  }

  mWidth = w;
  mHeight = h;

  mPointResult = new unsigned int[mWidth * mHeight];
  mLineResult = new unsigned int[mWidth + mHeight];

  memset(mPointResult, 0, sizeof(unsigned int) * mWidth * mHeight);
  memset(mLineResult, 0, sizeof(unsigned int) * (mWidth + mHeight));

  // MutualShortExtraCheck, begin
  mCycleTxDiff = new float[mWidth * mHeight];
  mTXPointShortResult = new unsigned int[mWidth * mHeight];
  mCycleRxDiff = new float[mWidth * mHeight];
  mRXPointShortResult = new unsigned int[mWidth * mHeight];
  memset(mCycleTxDiff, 0, sizeof(float) * mWidth * mHeight);
  memset(mTXPointShortResult, POINT_PASS,
         sizeof(unsigned int) * mWidth * mHeight);
  memset(mCycleRxDiff, 0, sizeof(float) * mWidth * mHeight);
  memset(mRXPointShortResult, POINT_PASS,
         sizeof(unsigned int) * mWidth * mHeight);
  // MutualShortExtraCheck, end

  // MutualNearFarCheck, begin
  mLineNearFarValue = new float[mWidth + mHeight];
  memset(mLineNearFarValue, 0, sizeof(float) * (mWidth + mHeight));
  mLineNearFarResult = new unsigned int[mWidth + mHeight];
  memset(mLineNearFarResult, 0, sizeof(unsigned int) * (mWidth + mHeight));
  // MutualNearFarCheck, end
}

float OpenShortResult::getCycleTxDiff(int x, int y) {
  return mCycleTxDiff[x * mHeight + y];
}

unsigned int OpenShortResult::getTXPointShortResult(int x, int y) {
  return mTXPointShortResult[x * mHeight + y];
}

float OpenShortResult::getCycleRxDiff(int x, int y) {
  return mCycleRxDiff[x * mHeight + y];
}

unsigned int OpenShortResult::getRXPointShortResult(int x, int y) {
  return mRXPointShortResult[x * mHeight + y];
}

void OpenShortResult::setPointResult(int x, int y, unsigned int result) {
  mPointResult[x * mHeight + y] = result;
}

void OpenShortResult::appendPointResult(int x, int y, unsigned int result) {
  mPointResult[x * mHeight + y] |= result;
}

int OpenShortResult::getSecondNG() { return mSecondNG; }

int OpenShortResult::getPointNG() { return mPointNG; }

int OpenShortResult::getLineNG() { return mLineNG; }

int OpenShortResult::getWidth() { return mWidth; }

int OpenShortResult::getHeight() { return mHeight; }

unsigned int OpenShortResult::getPointResult(int x, int y) {
  return mPointResult[x * mHeight + y];
}

unsigned int OpenShortResult::getXLineResult(int x) { return mLineResult[x]; }

unsigned int OpenShortResult::getYLineResult(int y) {
  return mLineResult[mWidth + y];
}

OpenShortData::OpenShortData()
    : mWidth(DEFAULT_W),
      mHeight(DEFAULT_H),
      mTargetVoltage(0),
      mBaseVoltage(0),
      mRawVoltage(0),
      mDiffVoltage(0),
      mCycle(0),
      mLoop(0),
      mReadBaseVoltage(1) {
  setWH(DEFAULT_W, DEFAULT_H);
}

OpenShortData::OpenShortData(int w, int h)
    : mWidth(w),
      mHeight(h),
      mTargetVoltage(0),
      mBaseVoltage(0),
      mRawVoltage(0),
      mDiffVoltage(0),
      mCycle(0),
      mLoop(0),
      mReadBaseVoltage(1) {
  setWH(w, h);
}

void OpenShortData::setWH(int w, int h) {
  if (mBaseVoltage != 0) {
    delete[] mBaseVoltage;
    delete[] mRawVoltage;
    delete[] mDiffVoltage;
    delete[] mCycle;
    delete[] mLoop;

    mBaseVoltage = 0;
    mRawVoltage = 0;
    mDiffVoltage = 0;
    mCycle = 0;
    mLoop = 0;
  }

  mWidth = w;
  mHeight = h;

  mBaseVoltage = new float[mWidth * mHeight];
  mRawVoltage = new float[mWidth * mHeight];
  mDiffVoltage = new float[mWidth * mHeight];
  mCycle = new float[mWidth * mHeight];
  mLoop = new float[mWidth * mHeight];

  memset(mBaseVoltage, 0, sizeof(float) * mWidth * mHeight);
  memset(mRawVoltage, 0, sizeof(float) * mWidth * mHeight);
  memset(mDiffVoltage, 0, sizeof(float) * mWidth * mHeight);
  memset(mCycle, 0, sizeof(float) * mWidth * mHeight);
  memset(mLoop, 0, sizeof(float) * mWidth * mHeight);
}

void OpenShortData::setTargetVoltage(float targetVoltage) {
  mTargetVoltage = targetVoltage;
}

void OpenShortData::setBaseVoltage(int x, int y, float baseVoltage) {
  mBaseVoltage[x * mHeight + y] = baseVoltage;
}

void OpenShortData::setRawVoltage(int x, int y, float rawVoltage) {
  mRawVoltage[x * mHeight + y] = rawVoltage;
}

void OpenShortData::setDiffVoltage(int x, int y, float diffVoltage) {
  mDiffVoltage[x * mHeight + y] = diffVoltage;
}

void OpenShortData::setCycle(int x, int y, float cycle) {
  mCycle[x * mHeight + y] = cycle;
}

void OpenShortData::setLoop(int x, int y, float loop) {
  mLoop[x * mHeight + y] = loop;
}

int OpenShortData::getWidth() { return mWidth; }

int OpenShortData::getHeight() { return mHeight; }

float OpenShortData::getBaseVoltage(int x, int y) {
  return mBaseVoltage[x * mHeight + y];
}

float OpenShortData::getTargetVoltage() { return mTargetVoltage; }

float OpenShortData::getRawVoltage(int x, int y) {
  return mRawVoltage[x * mHeight + y];
}

float OpenShortData::getDiffVoltage(int x, int y) {
  return mDiffVoltage[x * mHeight + y];
}

float OpenShortData::getCycle(int x, int y) { return mCycle[x * mHeight + y]; }

float OpenShortData::getLoop(int x, int y) { return mLoop[x * mHeight + y]; }

OpenShortData::~OpenShortData() {
  delete[] mBaseVoltage;
  delete[] mRawVoltage;
  delete[] mDiffVoltage;
  delete[] mCycle;
  delete[] mLoop;
}

int SiSTouch_Aegis_Multi::stop_driver() {
  int ret;

  if (m_verbose) {
    LOG(INFO) << "stop_driver()";
  }

  for (int i = 0; i < DEVICE_CLOSE_RETRY_TIMES; i++) {
    ret = sis_usb_stop();
    if (ret >= 0) {
      break;
    } else {
      if (errno == ENOSYS) {
        break;
      }
#ifdef VIA_IOCTL
      if (errno == EACCES || errno == ENOENT || errno == ENODEV) {
        break;
      }
#endif
    }
    usleep(DEVICE_CLOSE_RETRY_INTERVAL * 1000);
  }

  if (ret < 0) {
#ifdef VIA_IOCTL
    ret = parse_syscall_return_value(ret, NOT_SIS_IO_FLAG);
#else
    ret = parse_syscall_return_value(ret);
#endif
    if (m_verbose) {
      log_out_error_message(ret, -1);
    }
    return ret;
  }

#ifdef PWR_MODE
  if (m_ChangePWRMode) {
    for (int i = 0; i < DEVICE_CHANGEMODE_RETRY_TIMES; i++) {
      ret = call_85(PW_CMD_ACTIVE);

      if (ret >= 0) {
        break;
      }

      usleep(DEVICE_CHANGEMODE_RETRY_INTERVAL * 1000);
    }

    if (ret < 0) {
      // fail of stop, but device has been open, so we need to close device
      sis_usb_start();
      return ret;
    }

#ifdef PWR_MODE_QUERY
    for (int i = 0; i < DEVICE_CHANGEPWR_RETRY_TIMES; i++) {
      int addr[1] = {QUERY_STATUS_ADDRESS};
      int data[1] = {0};
      int mask[1] = {0};
      ret = call_88_master(addr, data, mask, 1);

      int pwCMDState = (data[0] >> 16) & 0xff;

      int active = PW_CMD_ACTIVE & 0xff;

      if (ret >= 0 && pwCMDState == active) {
        break;
      }

      usleep(DEVICE_CHANGEPWR_RETRY_INTERVAL * 1000);
    }
#else
    usleep(DEVICE_CHANGEPWR_RETRY_INTERVAL * 5 * 1000);
#endif
  }
#endif

#ifdef DIAGNOSIS_MODE
  if (m_ChangeDiagMode) {
    for (int i = 0; i < DEVICE_CHANGEMODE_RETRY_TIMES; i++) {
      ret = call_85(ENABLE_DIAGNOSIS_MODE);

      if (ret >= 0) {
        break;
      }

      usleep(DEVICE_CHANGEMODE_RETRY_INTERVAL * 1000);
    }

    if (ret < 0) {
      // fail of stop, but device has been open, so we need to close device
      sis_usb_start();
      return ret;
    }
  }

#endif

  return 0;
}

int SiSTouch_Aegis_Multi::start_driver() {
  int ret;

  if (m_verbose) {
    LOG(INFO) << "start_driver()";
  }

#ifdef PWR_MODE
  if (m_ChangePWRMode) {
    for (int i = 0; i < DEVICE_CHANGEMODE_RETRY_TIMES; i++) {
      ret = call_85(PW_CMD_FWCTRL);

      if (ret >= 0) {
        break;
      }

      usleep(DEVICE_CHANGEMODE_RETRY_INTERVAL * 1000);
    }
  }
#endif

#ifdef DIAGNOSIS_MODE
  if (m_ChangeDiagMode) {
    for (int i = 0; i < DEVICE_CHANGEMODE_RETRY_TIMES; i++) {
      ret = call_85(DISABLE_DIAGNOSIS_MODE);

      if (ret >= 0) {
        break;
      }

      usleep(DEVICE_CHANGEMODE_RETRY_INTERVAL * 1000);
    }
  }
#endif

  for (int i = 0; i < DEVICE_CLOSE_RETRY_TIMES; i++) {
    ret = sis_usb_start();
    if (ret >= 0) {
      break;
    } else {
      if (errno == ENOSYS) {
        break;
      }
#ifdef VIA_IOCTL
      if (errno == EACCES || errno == ENOENT || errno == ENODEV) {
        break;
      }
#endif
    }
    usleep(DEVICE_CLOSE_RETRY_INTERVAL * 1000);
  }

  if (ret < 0) {
#ifdef VIA_IOCTL
    ret = parse_syscall_return_value(ret, NOT_SIS_IO_FLAG);
#else
    ret = parse_syscall_return_value(ret);
#endif
    if (m_verbose) {
      log_out_error_message(ret, -1);
    }
    return ret;
  }

  return 0;
}

int SiSTouch_Aegis_Multi::call_04() {
  if (m_verbose) {
    LOG(INFO) << "call_04()";
  }

  int flag = 0;
  int ret = 0;
  for (int i = 0; i < m_retry; i++) {
    // 0x81 need delay between read and write

    ret = simple_io(0x81, DELAY_FOR_81);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) {
        break;
      }
    } else {
      ret = read_simple_ack(ret);

      if (ret == 0) flag = 1;

      if (ret != ERROR_NACK) break;
    }

    usleep(m_delay);
  }

  if (!flag) {
    if (ret == ERROR_NACK) ret = ERROR_TIMEOUT;

    if (m_verbose) LOG(ERROR) << "ERROR: 81 command not ACK, give up";
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x04);
    }
  }
  return ret;
}

int SiSTouch_Aegis_Multi::call_82() {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_82()";
  }

  for (int i = 0; i < m_retry; i++) {
    ret = simple_io_master(0x82, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) break;
    } else {
      ret = read_simple_ack_master(ret);

      if (ret == 0) break;
    }

    usleep(m_delay);
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x82);
    }
  } else {
#ifdef VIA_IOCTL

#ifdef CLOSE_AFTER_RESET
    sis_usb_start();
    usleep(WAIT_DEVICE_REMOVE * 1000);
#endif
    for (int i = 0; i < DEVICE_CLOSE_RETRY_TIMES; i++) {
      ret = stop_driver();
      // these three error not repeat retry in stop_driver()
      if (ret != ERROR_ACCESS && ret != ERROR_ENTRY_NOT_EXIST &&
          ret != ERROR_DEVICE_NOT_EXIST) {
        break;
      }
      usleep(DEVICE_CLOSE_RETRY_INTERVAL * 1000);
    }

#else
    ret = stop_driver();
#endif
  }

  return ret;
}

int SiSTouch_Aegis_Multi::make_83_buffer(int base, int pack_num) {
  if (m_SlaveAddr <= 0) {
    return make_83_buffer_master(base, pack_num);
  } else {
    return make_83_buffer_slave(base, pack_num);
  }
}

int SiSTouch_Aegis_Multi::make_83_buffer_master(int base, int pack_num) {
  int len = 10;

  make_common_header_master(0x83, len);

  memcpy(m_buffer + 4, &base, sizeof(int));
  memcpy(m_buffer + 8, &pack_num, sizeof(int) / 2);

  generate_output_crc_master();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::make_83_buffer_slave(int base, int pack_num) {
  int len = 14;

  make_common_header_slave(0x83, len);

  memcpy(m_buffer + 8, &base, sizeof(int));
  memcpy(m_buffer + 12, &pack_num, sizeof(int) / 2);

  generate_output_crc_slave();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::call_83(int base, int pack_num) {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_83()";
    LOG(INFO) << StringPrintf("base=%08x, pack_num=%d", base, pack_num);
  }

  for (int i = 0; i < m_retry; i++) {
    int len = make_83_buffer(base, pack_num);

    ret = io_command(m_buffer, len, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) break;
    } else {
      ret = read_simple_ack(ret);

      if (ret != ERROR_WRONG_FORMAT) break;
    }

    usleep(m_delay);
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x83);
    }
  }
  return ret;
}

int SiSTouch_Aegis_Multi::make_84_buffer(const int* data, int size) {
  if (m_SlaveAddr <= 0) {
    return make_84_buffer_master(data, size);
  } else {
    return make_84_buffer_slave(data, size);
  }
}

int SiSTouch_Aegis_Multi::make_84_buffer_master(const int* data, int size) {
  int len = size * 4 + 4;

  make_common_header_master(0x84, len);

  for (int j = 0; j < size; j++) {
    for (int k = 0; k < 4; k++) {
      ////////////////////////////// the endian is reverse with
      /// zeus///////////////////////
      char temp = (data[j] >> (8 * (3 - k))) & 0xff;
      //                char temp = ( data[j] >> (8*k) ) & 0xff;
      m_buffer[4 + 4 * j + k] = temp;
    }
  }

  generate_output_crc_master();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::make_84_buffer_slave(const int* data, int size) {
  int len = size * 4 + 8;

  make_common_header_slave(0x84, len);

  for (int j = 0; j < size; j++) {
    for (int k = 0; k < 4; k++) {
      ////////////////////////////// the endian is reverse with
      /// zeus///////////////////////
      char temp = (data[j] >> (8 * (3 - k))) & 0xff;
      //                char temp = ( data[j] >> (8*k) ) & 0xff;
      m_buffer[8 + 4 * j + k] = temp;
    }
  }

  generate_output_crc_slave();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::call_84(const int* data, int size) {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_84()";
    LOG(INFO) << StringPrintf("data=%08x, %08x, %08x,...,size=%d", data[0],
                              data[1], data[2], size);
  }

  for (int i = 0; i < m_retry; i++) {
    int len = make_84_buffer(data, size);

    ret = io_command(m_buffer, len, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) break;
    } else {
      ret = read_simple_ack(ret);

      if (ret != ERROR_NACK) break;
    }

    usleep(m_delay);
  }
  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x84);
    }
  }
  return ret;
}

int SiSTouch_Aegis_Multi::make_85_buffer(int data) {
  // note, 85 use master format

  int len = 6;

  make_common_header_master(0x85, len);

  m_buffer[4] = data & 0xff;
  m_buffer[5] = (data >> 8) & 0xff;

  generate_output_crc_master();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::call_85(int data) {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_85()";
    LOG(INFO) << StringPrintf(" data=%08x", data);
  }

  for (int i = 0; i < m_retry; i++) {
    int len = make_85_buffer(data);

    ret = io_command(m_buffer, len, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) break;
    } else {
      ret = read_simple_ack_master(ret);
      if (ret == 0) break;
    }

    usleep(m_delay);
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x85);
    }
  } else {
#ifdef DIAGNOSIS_MODE
    if (data == DISABLE_DIAGNOSIS_MODE || data == ENABLE_DIAGNOSIS_MODE) {
      usleep(DELAY_FOR_DIAGNOSIS_MODE_CHANGE * 1000);
    }
#endif
  }

  return ret;
}

int SiSTouch_Aegis_Multi::make_86_buffer(int addr) {
  if (m_SlaveAddr <= 0) {
    return make_86_buffer_master(addr);
  } else {
    return make_86_buffer_slave(addr);
  }
}

int SiSTouch_Aegis_Multi::make_86_buffer_master(int addr) {
  int len = 10;

  make_common_header_master(0x86, len);

  m_buffer[4] = addr & 0xff;
  m_buffer[5] = (addr >> 8) & 0xff;
  m_buffer[6] = (addr >> 16) & 0xff;
  m_buffer[7] = (addr >> 24) & 0xff;
  m_buffer[8] = DATA_UNIT_9257 & 0xff;
  m_buffer[9] = (DATA_UNIT_9257 >> 8) & 0xff;

  generate_output_crc_master();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::make_86_buffer_slave(int addr) {
  int len = 14;

  make_common_header_slave(0x86, len);

  m_buffer[8] = addr & 0xff;
  m_buffer[9] = (addr >> 8) & 0xff;
  m_buffer[10] = (addr >> 16) & 0xff;
  m_buffer[11] = (addr >> 24) & 0xff;
  m_buffer[12] = DATA_UNIT_9257 & 0xff;
  m_buffer[13] = (DATA_UNIT_9257 >> 8) & 0xff;

  generate_output_crc_slave();

  if (m_verbose) {
    dbg_print_buffer_hex(m_buffer, len);
  }

  return len;
}

int SiSTouch_Aegis_Multi::call_86(int address, int* data, int size) {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_86()";
    LOG(INFO) << StringPrintf(" address=%08x", address);
  }

  for (int i = 0; i < m_retry; i++) {
    int len = make_86_buffer(address);

    ret = io_command(m_buffer, len, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      break;
    } else {
      int dataStartIdx = get_data_start_idx();

      unsigned int read_size = ret - dataStartIdx;

      ret = read_simple_ack(ret);

      if (ret == 0) {
        read_size = (read_size > DATA_UNIT_9257) ? DATA_UNIT_9257 : read_size;

        ret = 0;
        for (unsigned int j = 0;
             j < read_size / 4 && j < static_cast<unsigned int>(size); j++) {
          data[j] = 0;
          for (int k = 0; k < 4; k++) {
            data[j] |= m_buffer[j * 4 + k + dataStartIdx] << 8 * k;
          }
          ret++;
        }
        break;
      }

      if (ret != ERROR_NACK) break;
    }

    usleep(m_delay);
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x86);
    }
  }
  return ret;
}

int SiSTouch_Aegis_Multi::get_data_start_idx() {
  if (m_SlaveAddr <= 0) {
    return get_data_start_idx_master();
  } else {
    return get_data_start_idx_slave();
  }
}

int SiSTouch_Aegis_Multi::get_data_start_idx_master() {
  return (8 - DUMMYBYTE);
}

int SiSTouch_Aegis_Multi::get_data_start_idx_slave() {
  return (12 - DUMMYBYTE);
}

int SiSTouch_Aegis_Multi::read_simple_ack(int size) {
  if (m_SlaveAddr <= 0) {
    return read_simple_ack_master(size);
  } else {
    return read_simple_ack_slave(size);
  }
}

int SiSTouch_Aegis_Multi::read_simple_ack_master(int size) {
  int length = size;

  m_crc = 0;

  for (int i = (4 - DUMMYBYTE);
       i < length && i < static_cast<int>(get_MAX_SIZE()); i++) {
    AppendCRC(m_buffer[i]);
  }

  if (m_buffer[3 - DUMMYBYTE] != (m_crc & 0xff)) {
    return ERROR_CRC_MISMATCH;
  }

  if (m_buffer[4 - DUMMYBYTE] == 0xEF && m_buffer[5 - DUMMYBYTE] == 0xBE) {
    return 0;
  } else if (m_buffer[4 - DUMMYBYTE] == 0xAD &&
             m_buffer[5 - DUMMYBYTE] == 0xDE) {
    return ERROR_NACK;
  }

  return ERROR_WRONG_FORMAT;
}

int SiSTouch_Aegis_Multi::read_simple_ack_slave(int size) {
  int master_len = size;

  m_crc = 0;

  for (int i = (4 - DUMMYBYTE);
       i < master_len && i < static_cast<int>(get_MAX_SIZE()); i++) {
    AppendCRC(m_buffer[i]);
  }

  if (m_buffer[3 - DUMMYBYTE] != (m_crc & 0xff)) {
    return ERROR_CRC_MISMATCH;
  }

  if (master_len < 12) {
    if (m_buffer[4 - DUMMYBYTE] == 0xEF && m_buffer[5 - DUMMYBYTE] == 0xBE) {
      return ERROR_WRONG_FORMAT;
    } else if (m_buffer[4 - DUMMYBYTE] == 0xAD &&
               m_buffer[5 - DUMMYBYTE] == 0xDE) {
      return ERROR_NACK;
    }
  } else {
    m_crc = 0;

    for (int i = (8 - DUMMYBYTE);
         i < master_len && i < static_cast<int>(get_MAX_SIZE()); i++) {
      AppendCRC(m_buffer[i]);
    }

    if (m_buffer[7 - DUMMYBYTE] != (m_crc & 0xff)) {
      return ERROR_CRC_MISMATCH;
    }

    if (m_buffer[8 - DUMMYBYTE] == 0xEF && m_buffer[9 - DUMMYBYTE] == 0xBE) {
      return 0;
    } else if (m_buffer[8 - DUMMYBYTE] == 0xAD &&
               m_buffer[9 - DUMMYBYTE] == 0xDE) {
      return ERROR_NACK;
    }
  }

  return ERROR_WRONG_FORMAT;
}

int SiSTouch_Aegis_Multi::is_noneed_retry_error(int ret) { return 1; }

int SiSTouch_Aegis_Multi::sis_usb_start(void) {
#ifdef VIA_IOCTL
  int ret = 0;
  if (m_fd >= 0) {
    ret = close(m_fd);
    m_fd = -1;
  }
  return ret;
#else
  return ERROR_NOT_SUPPORT;
#endif
}

int SiSTouch_Aegis_Multi::sis_usb_stop(void) {
#ifdef VIA_IOCTL
  const char* devName = SiSTouchIO::getActiveDeviceName();
  if (devName == NULL) devName = DEVICE_NAME;
  LOG(INFO) << StringPrintf("%s: devName = %s\n", __FUNCTION__, devName);

  if (m_fd >= 0) {
    // close(m_fd);
    m_fd = -1;
  }

  m_fd = open(devName, O_RDWR);
  return m_fd;
#else
  return ERROR_NOT_SUPPORT;
#endif
}

int SiSTouch_Aegis_Multi::make_simple_buffer(int cmd) {
  if (m_SlaveAddr <= 0) {
    return make_simple_buffer_master(cmd);
  } else {
    return make_simple_buffer_slave(cmd);
  }
}

int SiSTouch_Aegis_Multi::make_simple_buffer_master(int cmd) {
  int len = 4;
  make_common_header_master(cmd, len);
  generate_output_crc_master();

  if (m_verbose) dbg_print_buffer_hex(m_buffer, len);

  return len;
}

int SiSTouch_Aegis_Multi::make_simple_buffer_slave(int cmd) {
  int len = 8;
  make_common_header(cmd, len);
  generate_output_crc();

  if (m_verbose) dbg_print_buffer_hex(m_buffer, len);

  return len;
}

int SiSTouch_Aegis_Multi::simple_io(int cmd, int sleepms, int wTimeout,
                                    int rTimeout) {
  if (m_SlaveAddr <= 0) {
    return simple_io_master(cmd, sleepms, wTimeout, rTimeout);
  } else {
    return simple_io_slave(cmd, sleepms, wTimeout, rTimeout);
  }
}

int SiSTouch_Aegis_Multi::simple_io_master(int cmd, int sleepms, int wTimeout,
                                           int rTimeout) {
  int len = make_simple_buffer_master(cmd);

  int ret = sis_usb_write(m_buffer, len, wTimeout);

  if (ret < 0) {
    return ret;
  }

  if (sleepms >= 0) {
    usleep(sleepms * 1000);
  }

  ret = sis_usb_read(m_buffer, BUFFER_SIZE, rTimeout, INPUT_REPORT_ID);

  if (m_verbose) dbg_print_buffer_hex(cmd, m_buffer, ret);

  // usb return fixed 64 byte
  if (ret >= 0) {
    ret = m_buffer[1] + 1;
  }

  if (m_IODelayms >= 0) {
    usleep(m_IODelayms * 1000);
  }

  return ret;
}

int SiSTouch_Aegis_Multi::simple_io_slave(int cmd, int sleepms, int wTimeout,
                                          int rTimeout) {
  int len = make_simple_buffer(cmd);

  int ret = sis_usb_write(m_buffer, len, wTimeout);

  if (ret < 0) {
    return ret;
  }

  if (sleepms >= 0) {
    usleep(sleepms * 1000);
  }

  ret = sis_usb_read(m_buffer, BUFFER_SIZE, rTimeout, INPUT_REPORT_ID);

  if (m_verbose) dbg_print_buffer_hex(cmd, m_buffer, ret);

  // usb return fixed 64 byte
  if (ret >= 0) {
    ret = m_buffer[1] + 1;
  }

  if (m_IODelayms >= 0) {
    usleep(m_IODelayms * 1000);
  }

  return ret;
}

int SiSTouch_Aegis_Multi::io_command(unsigned char* buf, int size, int sleepms,
                                     int wTimeout, int rTimeout) {
  int ret = 0;
  unsigned char cmd = buf[2];

  ret = sis_usb_write(buf, size, wTimeout);

  if (ret < 0) {
    return ret;
  }

  if (sleepms >= 0) {
    usleep(sleepms * 1000);
  }

  ret = sis_usb_read(buf, BUFFER_SIZE, rTimeout, INPUT_REPORT_ID);

  if (m_verbose) dbg_print_buffer_hex(cmd, buf, ret);

  // usb return fixed 64 byte
  if (ret >= 0) {
    ret = m_buffer[1] + 1;
  }

  if (m_IODelayms >= 0) {
    usleep(m_IODelayms * 1000);
  }

  return ret;
}

void SiSTouch_Aegis_Multi::make_common_header(int cmd, int length) {
  if (m_SlaveAddr <= 0) {
    return make_common_header_master(cmd, length);
  } else {
    return make_common_header_slave(cmd, length);
  }
}

void SiSTouch_Aegis_Multi::make_common_header_master(int cmd, int length) {
  int byte_cnt = length - HEADER_LENGTH;

  m_buffer[0] = OUTPUT_REPORT_ID;
  m_buffer[1] = byte_cnt & 0xff;
  m_buffer[2] = cmd;
}

void SiSTouch_Aegis_Multi::make_common_header_slave(int cmd, int length) {
  int byte_cnt = length - HEADER_LENGTH;

  m_buffer[0] = OUTPUT_REPORT_ID;
  m_buffer[1] = byte_cnt & 0xff;
  m_buffer[2] = 0xb0;

  m_buffer[4] = OUTPUT_REPORT_ID;
  m_buffer[5] = m_SlaveAddr;
  m_buffer[6] = cmd;
}

void SiSTouch_Aegis_Multi::generate_output_crc() {
  if (m_SlaveAddr <= 0) {
    generate_output_crc_master();
  } else {
    generate_output_crc_slave();
  }
}

void SiSTouch_Aegis_Multi::generate_output_crc_master() {
  int byte_cnt = m_buffer[1];

  m_crc = 0;

  AppendCRC(m_buffer[2]);
  for (int i = 4;
       i < (byte_cnt + HEADER_LENGTH) && i < static_cast<int>(get_MAX_SIZE());
       i++) {
    AppendCRC(m_buffer[i]);
  }

  m_buffer[3] = m_crc & 0xff;
}

void SiSTouch_Aegis_Multi::generate_output_crc_slave() {
  int byte_cnt = m_buffer[1];

  m_crc = 0;

  AppendCRC(m_buffer[6]);
  for (int i = 8;
       i < (byte_cnt + HEADER_LENGTH) && i < static_cast<int>(get_MAX_SIZE());
       i++) {
    AppendCRC(m_buffer[i]);
  }

  m_buffer[7] = m_crc & 0xff;

  m_crc = 0;

  AppendCRC(m_buffer[2]);
  for (int i = 4;
       i < (byte_cnt + HEADER_LENGTH) && i < static_cast<int>(get_MAX_SIZE());
       i++) {
    AppendCRC(m_buffer[i]);
  }

  m_buffer[3] = m_crc & 0xff;
}

int SiSTouch_Aegis_Multi::sis_usb_write(void* data, unsigned int size,
                                        int timeout) {
  int ret = 0;
  unsigned char* buf = (unsigned char*)data;
#ifdef VIA_IOCTL

  unsigned char databuf[72] = {0};

  memcpy(databuf, buf, get_MAX_SIZE());

  databuf[get_MAX_SIZE()] = size >> 24 & 0xff;
  databuf[get_MAX_SIZE() + 1] = size >> 16 & 0xff;
  databuf[get_MAX_SIZE() + 2] = size >> 8 & 0xff;
  databuf[get_MAX_SIZE() + 3] = size & 0xff;

  databuf[get_MAX_SIZE() + 4] = timeout >> 24 & 0xff;
  databuf[get_MAX_SIZE() + 5] = timeout >> 16 & 0xff;
  databuf[get_MAX_SIZE() + 6] = timeout >> 8 & 0xff;
  databuf[get_MAX_SIZE() + 7] = timeout & 0xff;

  ret = write(m_fd, databuf, 72);

#else

  ret = ERROR_NOT_SUPPORT;

#endif
  return ret;
}

int SiSTouch_Aegis_Multi::sis_usb_read(void* data, unsigned int size,
                                       int timeout, int report_id) {
  int ret = 0;
  unsigned char* buf = (unsigned char*)data;
#ifdef VIA_IOCTL

  unsigned char databuf[72] = {0};

  memcpy(databuf, buf, get_MAX_SIZE());

  databuf[get_MAX_SIZE()] = size >> 24 & 0xff;
  databuf[get_MAX_SIZE() + 1] = size >> 16 & 0xff;
  databuf[get_MAX_SIZE() + 2] = size >> 8 & 0xff;
  databuf[get_MAX_SIZE() + 3] = size & 0xff;

  databuf[get_MAX_SIZE() + 4] = timeout >> 24 & 0xff;
  databuf[get_MAX_SIZE() + 5] = timeout >> 16 & 0xff;
  databuf[get_MAX_SIZE() + 6] = timeout >> 8 & 0xff;
  databuf[get_MAX_SIZE() + 7] = timeout & 0xff;

  do {
    ret = read(m_fd, databuf, 72);
    if (ret > 0 && (buf[0] != report_id))
      continue;
    break;
  } while(1);

  for (unsigned int i = 0; i < get_MAX_SIZE(); i++) {
    buf[i] = databuf[i];
  }

#else

  ret = ERROR_NOT_SUPPORT;

#endif

  return ret;
}

void SiSTouch_Aegis_Multi::log_out_error_message(int ret, int cmd) {
#ifdef VIA_IOCTL
  const char* devName = SiSTouchIO::getActiveDeviceName();
  if (devName == NULL) devName = DEVICE_NAME;
#endif

  if (cmd == -1) {
    LOG(ERROR) << StringPrintf("CMD error: %d", ret);
    switch (ret) {
      case SiSTouchIO::ERROR_SYSCALL:
        LOG(ERROR) << "called error system call";
        break;
#ifdef VIA_IOCTL
      case SiSTouchIO::ERROR_ACCESS:
        LOG(ERROR) << StringPrintf("cannot access device %s", devName);
        break;
      case SiSTouchIO::ERROR_ENTRY_NOT_EXIST:
        LOG(ERROR) << StringPrintf("entry %s not exist", devName);
        break;
      case SiSTouchIO::ERROR_DEVICE_NOT_EXIST:
        LOG(ERROR) << StringPrintf("device %s not exist", devName);
        break;
      default:
        LOG(ERROR) << StringPrintf("Got errno %d", ret);
        break;
#else
      default:
        LOG(ERROR) << StringPrintf("ASSERT: Got impossible error code %d", ret);
        break;
#endif
    }
  } else {
    LOG(ERROR) << StringPrintf("CMD %02x syscall error: %d", cmd, ret);

    switch (ret) {
      case SiSTouchIO::ERROR:
        LOG(ERROR) << StringPrintf("CMD %02x error", cmd);
        break;
      case SiSTouchIO::ERROR_NACK:
        LOG(ERROR) << StringPrintf("CMD %02x not receive ack", cmd);
        break;
      case SiSTouchIO::ERROR_DEAD:
        LOG(ERROR) << StringPrintf("CMD %02x dead", cmd);
        break;
      case SiSTouchIO::ERROR_SYSCALL:
        LOG(ERROR) << StringPrintf("CMD %02x invoke non exist system call",
                                   cmd);
        break;
      case SiSTouchIO::ERROR_TIMEOUT:
        LOG(ERROR) << StringPrintf("CMD %02x transmit timeout", cmd);
        break;
      case SiSTouchIO::ERROR_QUERY:
        LOG(ERROR) << "cannot query driver";
        break;
      case SiSTouchIO::ERROR_EACK:
        LOG(ERROR) << StringPrintf("CMD %02x get error ack format", cmd);
        break;
      /*
         case SiSTouchIO::ERROR_USBNACK:
             LOGE( "CMD %02x get USB nack", cmd );
             break;
      */
      case SiSTouchIO::ERROR_USBEACK:
        LOG(ERROR) << StringPrintf(" CMD %02x get error USB ack format", cmd);
        break;
      case SiSTouchIO::ERROR_ACCESS_USER_MEMORY:
        LOG(ERROR) << "Fail to access data from user memory";
        break;
      case SiSTouchIO::ERROR_ALLOCATE_KERNEL_MEMORY:
        LOG(ERROR) << "Fail to allocate kernel memory";
        break;
      case SiSTouchIO::ERROR_COPY_FROM_USER:
        LOG(ERROR) << "Fail to copy data from user memory to kernel memory";
        break;
      case SiSTouchIO::ERROR_COPY_FROM_KERNEL:
        LOG(ERROR) << "Fail to copy data from kernel memory to user memory";
        break;
      case SiSTouchIO::ERROR_USB_DEV_NULL:
        LOG(ERROR) << "SiS Touch device not exist";
        break;
      case ERROR_BROKEN_PIPE:
        LOG(ERROR) << "USB I/O broken pipe";
        break;
      case SiSTouchIO::ERROR_WRONG_FORMAT:
        LOG(ERROR) << StringPrintf("CMD %02x return not recognized format",
                                   cmd);
        break;
      case SiSTouchIO::ERROR_NOT_SUPPORT:
        LOG(ERROR) << StringPrintf("CMD %02x not supported by this interface",
                                   cmd);
        break;
      case SiSTouchIO::ERROR_CRC_MISMATCH:
        LOG(ERROR) << StringPrintf("CMD %02x crc error", cmd);
        break;
      case SiSTouchIO::ERROR_TRANSMIT_TIMOUT:
        LOG(ERROR) << StringPrintf("CMD %02x transmit timeout", cmd);
        break;
      default:
        LOG(ERROR) << StringPrintf("ASSERT: CMD %02x got impossible error code",
                                   cmd);
        break;
    }
  }
}

void SiSTouch_Aegis_Multi::setSlaveAddr(int slaveAddr) {
  m_SlaveAddr = slaveAddr;
}

int SiSTouch_Aegis_Multi::getSlaveAddr() { return m_SlaveAddr; }

const char* SiSTouch_Aegis_Multi::get_device_name() { return DEVICE_NAME; }

int SiSTouch_Aegis_Multi::device_Exist() {
#ifdef VIA_IOCTL
  const char* devName = SiSTouchIO::getActiveDeviceName();
  if (devName == NULL) devName = DEVICE_NAME;

  // check format
  if (strncmp(devName, DEVICE_NAME, strlen(DEVICE_NAME)) != 0) return 0;

  struct stat s;
  if (stat(devName, &s) == 0) {
    if (S_ISBLK(s.st_mode) || S_ISCHR(s.st_mode)) {
      return 1;
    }
  }
  return 0;
#else
  return 0;
#endif
}

MultiOpenShortData::MultiOpenShortData()
    : mChipNum(1), mDatas(0), mChipOrderIndexs(0), mReadBaseVoltage(1) {
  setChipNum(1);
}

MultiOpenShortData::MultiOpenShortData(int chipNum)
    : mChipNum(chipNum), mDatas(0), mChipOrderIndexs(0), mReadBaseVoltage(1) {
  if (chipNum < 1) {
    LOG(INFO) << "minimum chipNum is 1.";
    chipNum = 1;
  }

  setChipNum(chipNum);
}

MultiOpenShortData::~MultiOpenShortData() {
  delete[] mDatas;
  delete[] mChipOrderIndexs;
}

void MultiOpenShortData::setChipNum(int chipNum) {
  if (mDatas != 0) {
    delete[] mDatas;
    delete[] mChipOrderIndexs;
  }

  mChipNum = chipNum;
  mDatas = new OpenShortData[chipNum];
  mChipOrderIndexs = new int[chipNum];

  for (int i = 0; i < chipNum; i++) {
    mChipOrderIndexs[i] = i;
  }
}

void MultiOpenShortData::queryMappingSingle(int* devIdx, int* x, int* y) {
  if (mChipNum == 1) {
    *devIdx = 0;
  } else {
    *devIdx = 0;
    for (*devIdx = 0; *devIdx < mChipNum; (*devIdx)++) {
      int chipHeight = getHeight(*devIdx);
      if (*x >= chipHeight) {
        *x -= chipHeight;
      } else {
        break;
      }
    }
    // x, y switch;
    int temp = *x;
    *x = *y;
    *y = temp;
  }
}

int MultiOpenShortData::getWidth() {
  int width = 0;
  if (mChipNum == 1) {
    width += mDatas[0].getWidth();
  } else {
    for (int index = 0; index < mChipNum; index++) {
      width += mDatas[index].getHeight();
    }
  }
  return width;
}

int MultiOpenShortData::getWidth(int index) {
  if (index < mChipNum && index >= 0) {
    return mDatas[index].getWidth();
  } else {
    return 0;
  }
}

int MultiOpenShortData::getHeight() {
  int height = 0;
  if (mChipNum == 1) {
    height = mDatas[0].getHeight();
  } else if (mChipNum < 1) {
    height = 0;
  } else {
    // height =  mDatas[0].getWidth();
    for (int i = 0; i < mChipNum; i++) {
      if (mDatas[i].getWidth() > height) {  // get max
        height = mDatas[i].getWidth();
      }
    }
  }
  return height;
}

int MultiOpenShortData::getHeight(int index) {
  if (index < mChipNum && index >= 0) {
    return mDatas[index].getHeight();
  } else {
    return 0;
  }
}

float MultiOpenShortData::getCycle(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getCycle(x, y);
}

float MultiOpenShortData::getLoop(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getLoop(x, y);
}

float MultiOpenShortData::getBaseVoltage(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getBaseVoltage(x, y);
}

float MultiOpenShortData::getRawVoltage(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getRawVoltage(x, y);
}

float MultiOpenShortData::getDiffVoltage(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getDiffVoltage(x, y);
}

float MultiOpenShortData::getTargetVoltage(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mDatas[devIdx].getTargetVoltage();
}

void MultiOpenShortData::setCycle(int x, int y, float cycle) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setCycle(x, y, cycle);
}

void MultiOpenShortData::setLoop(int x, int y, float loop) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setLoop(x, y, loop);
}

void MultiOpenShortData::setBaseVoltage(int x, int y, float baseVoltage) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setBaseVoltage(x, y, baseVoltage);
}

void MultiOpenShortData::setRawVoltage(int x, int y, float rawVoltage) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setRawVoltage(x, y, rawVoltage);
}

void MultiOpenShortData::setDiffVoltage(int x, int y, float diffVoltage) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setDiffVoltage(x, y, diffVoltage);
}

void MultiOpenShortData::setTargetVoltage(int x, int y, float targetVoltage) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mDatas[devIdx].setTargetVoltage(targetVoltage);
}

MultiOpenShortResult::MultiOpenShortResult()
    : mChipNum(0),
      mResults(0),
      mChipOrderIndexs(0),
      mWidthTP(0),
      mHeightTP(0),
      mPointTpResult(0),
      mLineTpResult(0),
      mLineTpNG(0),
      mLineTpNearFarValue(0),
      mLineTpNearFarResult(0),
      // mPointTpNG(0),
      lostSide(0) {
  setChipNum(1);
}

MultiOpenShortResult::MultiOpenShortResult(int chipNum)
    : mChipNum(chipNum),
      mResults(0),
      mChipOrderIndexs(0),
      mWidthTP(0),
      mHeightTP(0),
      mPointTpResult(0),
      mLineTpResult(0),
      mLineTpNG(0),
      mLineTpNearFarValue(0),
      mLineTpNearFarResult(0),
      // mPointTpNG(0),
      lostSide(0) {
  if (chipNum < 1) {
    LOG(INFO) << "minimum chipNum is 1.";
    chipNum = 1;
  }
  setChipNum(chipNum);
}

MultiOpenShortResult::~MultiOpenShortResult() {
  delete[] mResults;
  delete[] mChipOrderIndexs;
  delete[] mLineTpNearFarValue;
  delete[] mLineTpNearFarResult;
}

void MultiOpenShortResult::setChipNum(int chipNum) {
  mChipNum = chipNum;

  if (mResults != 0) {
    delete[] mResults;
    delete[] mChipOrderIndexs;
  }
  mResults = new OpenShortResult[mChipNum];

  mChipOrderIndexs = new int[chipNum];

  for (int i = 0; i < chipNum; i++) {
    mChipOrderIndexs[i] = i;
  }
}

void MultiOpenShortResult::queryMappingSingle(int* devIdx, int* x, int* y) {
  if (mChipNum == 1) {
    *devIdx = 0;
  } else {
    *devIdx = 0;
    for (*devIdx = 0; *devIdx < mChipNum; (*devIdx)++) {
      int chipHeight = getHeight(*devIdx);
      if (*x >= chipHeight) {
        *x -= chipHeight;
      } else {
        break;
      }
    }
    // x, y switch;
    int temp = *x;
    *x = *y;
    *y = temp;
  }
}

int MultiOpenShortResult::getWidth() {
  int width = 0;
  if (mChipNum == 1) {
    width += mResults[0].getWidth();
  } else {
    for (int index = 0; index < mChipNum; index++) {
      width += mResults[index].getHeight();
    }
  }
  return width;
}

int MultiOpenShortResult::getWidth(int index) {
  if (index < mChipNum && index >= 0) {
    return mResults[index].getWidth();
  } else {
    return 0;
  }
}

int MultiOpenShortResult::getHeight() {
  int height = 0;
  if (mChipNum == 1) {
    height = mResults[0].getHeight();
  } else if (mChipNum < 1) {
    height = 0;
  } else {
    height = mResults[0].getWidth();
  }
  return height;
}

int MultiOpenShortResult::getHeight(int index) {
  if (index < mChipNum && index >= 0) {
    return mResults[index].getHeight();
  } else {
    return 0;
  }
}

int MultiOpenShortResult::getSecondNG() {
  int secondNG = 0;
  for (int index = 0; index < mChipNum; index++) {
    secondNG += mResults[index].getSecondNG();
  }
  return secondNG;
}

int MultiOpenShortResult::getPointNG() {
  int pointNG = 0;
  for (int index = 0; index < mChipNum; index++) {
    pointNG += mResults[index].getPointNG();
  }
  return pointNG;
}

int MultiOpenShortResult::getLineNG() {
  int lineNG = 0;
  for (int index = 0; index < mChipNum; index++) {
    lineNG += mResults[index].getLineNG();
  }
  return lineNG;
}

// MutualShortExtraCheck, begin
float MultiOpenShortResult::getCycleTxDiff(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  if (mChipNum == 1) {
    return mResults[devIdx].getCycleTxDiff(x, y);
  } else {
    // x/y switch
    return mResults[devIdx].getCycleRxDiff(x, y);
  }
}
unsigned int MultiOpenShortResult::getTXPointShortResult(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  if (mChipNum == 1) {
    return mResults[devIdx].getTXPointShortResult(x, y);
  } else {
    // x/y switch
    return mResults[devIdx].getRXPointShortResult(x, y);
  }
}

float MultiOpenShortResult::getCycleRxDiff(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  if (mChipNum == 1) {
    return mResults[devIdx].getCycleRxDiff(x, y);
  } else {
    // x/y switch
    return mResults[devIdx].getCycleTxDiff(x, y);
  }
}

unsigned int MultiOpenShortResult::getRXPointShortResult(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  if (mChipNum == 1) {
    return mResults[devIdx].getRXPointShortResult(x, y);
  } else {
    // x/y switch
    return mResults[devIdx].getTXPointShortResult(x, y);
  }
}
// MutualShortExtraCheck, end

unsigned int MultiOpenShortResult::getPointResult(int x, int y) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  return mResults[devIdx].getPointResult(x, y);
}

void MultiOpenShortResult::setPointResult(int x, int y, unsigned int result) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mResults[devIdx].setPointResult(x, y, result);
}

void MultiOpenShortResult::appendPointResult(int x, int y,
                                             unsigned int result) {
  int devIdx = 0;
  queryMappingSingle(&devIdx, &x, &y);
  mResults[devIdx].appendPointResult(x, y, result);
}

unsigned int MultiOpenShortResult::getXLineResult(int x) {
  unsigned result = 0;
  int devIdx = 0;
  int y = 0;
  queryMappingSingle(&devIdx, &x, &y);

  if (mChipNum == 1) {
    result = mResults[devIdx].getXLineResult(x);
  } else {
    // x/y switch
    result = mResults[devIdx].getYLineResult(y);
  }
  return result;
}

unsigned int MultiOpenShortResult::getYLineResult(int y) {
  unsigned result = 0;
  if (mChipNum == 1) {
    result = mResults[0].getYLineResult(y);
  } else {
    // x/y switch
    for (int devIdx = mChipNum - 1; devIdx >= 0; devIdx--) {
      result = result << MULTI_CHIP_MASK_LEN;
      unsigned lineResult = mResults[devIdx].getXLineResult(y);
      result |= lineResult;
    }
  }

  return result;
}

unsigned int MultiOpenShortResult::getYLineResult(int devIdx, int y) {
  unsigned result = 0;
  if (mChipNum == 1) {
    result = mResults[0].getYLineResult(y);
  } else {
    // x/y switch
    result = mResults[devIdx].getXLineResult(y);
  }

  return result;
}

SisTouchFinder::SisTouchFinder()
    : isRecoveryDevice(false), m_vId(0x0), m_deviceType(UNKNOW) {}

SisTouchFinder::~SisTouchFinder() {}

char* SisTouchFinder::autoDetectDevicePath() {
  // only detect hidraw
  /* Detect hidraw* */
  char* deviceName = autoDetectHidDevicePath();
  if (deviceName != 0) {
    return deviceName;
  }

  return 0;
}

SisTouchFinder::DeviceType SisTouchFinder::getDeviceType() {
  return m_deviceType;
}

/* Detect hidraw*
 * ==================================================================*/
char* SisTouchFinder::autoDetectHidDevicePath() {
  FILE* pipe = popen("ls /dev/hidraw*", "r");
  if (pipe) {
    const int L_devName = 128;
    char buffer[L_devName];
    char* deviceName = new char[L_devName];

    while (!feof(pipe)) {
      if (fgets(buffer, L_devName, pipe) != NULL) {
        snprintf(deviceName, L_devName, "%s", buffer);
        // strcpy(deviceName, buffer);

        if (deviceName[strlen(deviceName) - 1] == '\n') {
          deviceName[strlen(deviceName) - 1] = '\0';
        }

        /* check if deviceName is hid_Sis_touch */
        LOG(INFO) << "[check HID: " << deviceName << "]";
        if (isSisTouchHid(deviceName)) {
          pclose(pipe);
          return deviceName;  // search first
        }
      }
    }

    pclose(pipe);
  }

  return 0;
}

bool SisTouchFinder::isSisTouchHid(const char* deviceName) {
  STF_DEBUG LOG(INFO) << "[isSisTouchHid] deviceName=" << deviceName;

  bool ret = getHidInfo(deviceName);

  if (!ret) {
    return false;
  }

  if ((m_vId != 0x0457) && (m_vId != 0x266e)) {
    return false;
  }

  if (strstr(m_rawName, "HID") != NULL) {
    m_deviceType = USB_817;
  } else {
  }

  return true;
}

bool SisTouchFinder::getHidInfo(const char* deviceName) {
  int fd;
  int res;
  char buf[256];
  struct hidraw_report_descriptor rpt_desc;
  struct hidraw_devinfo info;

  STF_DEBUG LOG(INFO) << "getHidInfo: " << deviceName;

  /* Open the Device with non-blocking reads. In real life,
     don't use a hard coded path; use libudev instead. */
  fd = open(deviceName, O_RDWR | O_NONBLOCK);

  if (fd < 0) {
    STF_DEBUG LOG(WARNING) << "Unable to open device";
    return false;
  }

  memset(&rpt_desc, 0x0, sizeof(rpt_desc));
  memset(&info, 0x0, sizeof(info));
  memset(buf, 0x0, sizeof(buf));

  /* Get Raw Name */
  res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
  if (res < 0) {
    STF_DEBUG perror("HIDIOCGRAWNAME");
  } else {
    STF_DEBUG LOG(INFO) << "Raw Name: " << buf;

    // save raw name
    // strcpy(m_rawName, buf);
    snprintf(m_rawName, sizeof(m_rawName), "%s", buf);
    LOG(INFO) << "[rawName=" << m_rawName << "]";
  }

  /* Get Raw Info */
  res = ioctl(fd, HIDIOCGRAWINFO, &info);
  if (res < 0) {
    STF_DEBUG perror("HIDIOCGRAWINFO");
  } else {
    STF_DEBUG LOG(INFO) << "Raw Info:";
    STF_DEBUG LOG(INFO) << "bustype: " << info.bustype << " "
                        << bus_str(info.bustype);

    STF_DEBUG LOG(INFO) << StringPrintf("vendor: 0x%04hx",
                                        (uint16_t)info.vendor);

    // save vid
    m_vId = info.vendor;
    if (info.vendor == 0x0457) {
      isRecoveryDevice = true;
    }
    LOG(INFO) << StringPrintf("[vid=0x%04hx]", (uint16_t)m_vId);

    STF_DEBUG LOG(INFO) << StringPrintf("product: 0x%04hx",
                                        (uint16_t)info.product);
  }

  close(fd);
  return true;
}

const char* SisTouchFinder::bus_str(int bus) {
  switch (bus) {
    case BUS_USB:
      return "USB";
      break;
    case BUS_HIL:
      return "HIL";
      break;
    case BUS_BLUETOOTH:
      return "Bluetooth";
      break;
    /*
    case BUS_VIRTUAL:
        return "Virtual";
        break;
    */
    default:
      return "Other";
      break;
  }
}

int setLogInterface(int logInterface) {
  if (logInterface == ANDROID_LOG_FLAG) {
    m_LogInterface = NULL_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoNull;
    m_vLOGEpointer = &vLOGEtoNull;
  } else if (logInterface == FILE_LOG_FLAG) {
    m_LogInterface = NULL_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoNull;
    m_vLOGEpointer = &vLOGEtoNull;
  } else if (logInterface == ANDROID_AND_FILE_LOG_FLAG) {
#if defined(FOR_ANDROID_LOG) && defined(SDCARD_FILE_LOG)
    m_LogInterface = ANDROID_AND_FILE_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoLogcatAndFile;
    m_vLOGEpointer = &vLOGEtoLogcatAndFile;
#elif !defined(FOR_ANDROID_LOG) && defined(SDCARD_FILE_LOG)
    m_LogInterface = FILE_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoFile;
    m_vLOGEpointer = &vLOGEtoFile;
#elif defined(FOR_ANDROID_LOG) && !defined(SDCARD_FILE_LOG)
    m_LogInterface = ANDROID_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoLogcat;
    m_vLOGEpointer = &vLOGEtoLogcat;
#else
    m_LogInterface = NULL_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoNull;
    m_vLOGEpointer = &vLOGEtoNull;
#endif
  } else {
    m_LogInterface = NULL_LOG_FLAG;
    m_vLOGIpointer = &vLOGItoNull;
    m_vLOGEpointer = &vLOGEtoNull;
  }

  LOG(INFO) << StringPrintf("LogInterface set to %d", m_LogInterface);
  return m_LogInterface;
}

int LOGI(const char* format, ...) {
  if (m_vLOGIpointer) {
    va_list vl;
    va_start(vl, format);
    (*m_vLOGIpointer)(format, vl);
    va_end(vl);
    return 1;
  } else {
    return 0;
  }
}

int LOGE(const char* format, ...) {
  if (m_vLOGEpointer) {
    va_list vl;
    va_start(vl, format);
    (*m_vLOGEpointer)(format, vl);
    va_end(vl);
    return 1;
  } else {
    return 0;
  }
}

#ifdef LOG_TIME

int getCurrentTimeString(char* buf) {
  timeval tv;
  tm stamp;
  gettimeofday(&tv, NULL);
  localtime_r(&(tv.tv_sec), &stamp);
  int max_time_string_length = 1000;
  return snprintf(buf, max_time_string_length, "%02d-%d %d:%d:%d - %ld",
                  stamp.tm_mon, stamp.tm_mday, stamp.tm_hour, stamp.tm_min,
                  stamp.tm_sec, tv.tv_usec / 1000);
}

#endif

int vLOGItoNull(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

int vLOGItoFile(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

int vLOGItoLogcatAndFile(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

int vLOGEtoNull(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

int vLOGEtoFile(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

int vLOGEtoLogcatAndFile(const char* format, va_list args) {
  int ret = 0;

#ifdef LOG_TIME
  char str[256] = {0};
  getCurrentTimeString(str);
  printf("%s ", str);
#endif

  ret = vprintf(format, args);
  printf("\n");
  fflush(stdout);
  return ret;
}

void initialLogger() {
#if defined(FOR_ANDROID_LOG) && defined(SDCARD_FILE_LOG)
  setLogInterface(ANDROID_AND_FILE_LOG_FLAG);
#elif !defined(FOR_ANDROID_LOG) && defined(SDCARD_FILE_LOG)
  setLogInterface(NULL_LOG_FLAG);
#elif defined(FOR_ANDROID_LOG) && !defined(SDCARD_FILE_LOG)
  setLogInterface(ANDROID_LOG_FLAG);
#else
  setLogInterface(NULL_LOG_FLAG);
#endif
}

void releaseLogger() {
  m_LogInterface = 0;
  m_vLOGIpointer = 0;
  m_vLOGEpointer = 0;
}

const char* SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::get_device_name() {
  return getActiveDeviceName();
}

int SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::sis_usb_write(void* data,
                                                       unsigned int size,
                                                       int timeout) {
  int ret = 0;
  unsigned char* buf = (unsigned char*)data;

#ifdef VIA_IOCTL

  ret = write(m_fd, buf, size);

#else

  ret = ERROR_NOT_SUPPORT;

#endif
  return ret;
}

int SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::sis_usb_read(void* data,
                                                      unsigned int size,
                                                      int timeout,
                                                      int report_id) {
  int ret = 0;
  unsigned char* buf = (unsigned char*)data;

#ifdef VIA_IOCTL

  do {
    fd_set fd_read;
    FD_ZERO(&fd_read);
    FD_SET(m_fd, &fd_read);

    struct timeval one_second_timeout;
    one_second_timeout.tv_sec = 1;
    one_second_timeout.tv_usec = 0;

    if (select(m_fd + 1, &fd_read, NULL, NULL, &one_second_timeout) != 1) {
      // The methods calling this do not propagate the return value properly and
      // I see no way of recovering from this error.
      close(m_fd);
      LOG(ERROR) << "SiSTouch did not respond. Terminating firmware update.";
      exit(1);
    }
    ret = read(m_fd, buf, size);
    if (ret > 0 && (buf[0] != report_id))
      continue;
    break;
  } while (1);
#else

  ret = ERROR_NOT_SUPPORT;

#endif

  return ret;
}

int SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::simple_io_master(int cmd, int sleepms,
                                                          int wTimeout,
                                                          int rTimeout) {
  int len = make_simple_buffer_master(cmd);

  int ret = sis_usb_write(m_buffer, len, wTimeout);

  if (ret < 0) {
    return ret;
  }

  if (sleepms >= 0) {
    usleep(sleepms * 1000);
  }

  ret = sis_usb_read(m_buffer, BUFFER_SIZE, rTimeout, INPUT_REPORT_ID);

  /* close m_fd for remove /dev/hidraw =======================================*/
  if (cmd == 0x82 || cmd == 0x87) {
    int ret = sis_usb_start();
    if (ret != 0) {
      LOG(WARNING) << StringPrintf("close m_fd (%s): fail ret=%d",
                                   get_device_name(), ret);
    }
  }
  /* =============================================================== */

  if (m_verbose) dbg_print_buffer_hex(cmd, m_buffer, ret);

  // usb return fixed 64 byte
  if (ret >= 0) {
    ret = m_buffer[1] + 1;
  }

  if (m_IODelayms >= 0) {
    usleep(m_IODelayms * 1000);
  }

  return ret;
}

bool SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::reFindSisTouchName() {
  SisTouchFinder sisTouchFinder;
  const char* deviceName = sisTouchFinder.autoDetectDevicePath();

  if (deviceName != 0) {
    // strcpy(m_activeDeviceName, deviceName);
    snprintf(m_activeDeviceName, DEVNAME_MAX_LEN, "%s", deviceName);
    delete deviceName;

    LOG(INFO) << "re-find activeDeviceName=" << m_activeDeviceName;
  } else {
    return false;
  }

  return true;
}

int SiSTouch_Aegis_Multi_FOR_NEW_DRIVER::call_82() {
  int ret = 0;

  if (m_verbose) {
    LOG(INFO) << "call_82()";
  }

  for (int i = 0; i < m_retry; i++) {
    ret = simple_io_master(0x82, DELAY_FOR_GENEARL);

    if (ret < 0) {
      ret = parse_syscall_return_value(ret, GENERAL_TYPE_FLAG);
      if (is_noneed_retry_error(ret)) break;
    } else {
      ret = read_simple_ack_master(ret);

      if (ret == 0) break;
    }

    usleep(m_delay);
  }

  if (ret < 0) {
    if (m_verbose) {
      log_out_error_message(ret, 0x82);
    }
  } else {
#ifdef VIA_IOCTL

#ifdef CLOSE_AFTER_RESET
    sis_usb_start();
    usleep(WAIT_DEVICE_REMOVE * 1000);
#endif
    for (int i = 0; i < DEVICE_CLOSE_RETRY_TIMES; i++) {
      ret = stop_driver();
      // these three error not repeat retry in stop_driver()
      if (ret != ERROR_ACCESS && ret != ERROR_ENTRY_NOT_EXIST &&
          ret != ERROR_DEVICE_NOT_EXIST) {
        break;
      }

      /* re-find /dev/hidraw* if hidrawCnt changed ====================*/
      // if (reFindSisTouchName() == false) {
      // }
      /* ===========================================*/

      usleep(DEVICE_CLOSE_RETRY_INTERVAL * 1000);
    }

#else
    ret = stop_driver();
#endif
  }

  return ret;
}
