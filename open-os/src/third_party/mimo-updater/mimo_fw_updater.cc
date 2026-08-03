//
// Copyright 2017 Mimo monitors. All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause
//
#include <libusb-1.0/libusb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <base/logging.h>
#include <brillo/syslog_logging.h>

#include <memory>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// File Description:
// The codes are used to read a binary file, the firmware used by
// the Displaylink ASIC (inside the MIMO touch screen), and write into the
// EEPROM inside the ASIC.
// The following things will be checked:
// 1. The updater will check if the firmware file exists and if the firmware
//    file name and size are valid.
// 2. The firmware version in the device and the version from the CrOS release.
//    If the 2 numbers are inconsistent, the updater will perform the firmware
//    update. Note that the firmware version is not provided in the firmware
//    file itself. The version is coded in the firmware file name.
// Note
//    The actual firmware contents are opaque to the updater, and it is only
//    known to the vendor, Displaylink. In addition to that, the sequence of
//    updating the firmware is also provided by Displaylink and MIMO monitors.
////////////////////////////////////////////////////////////////////////////////

namespace std {

// This class is used for auto file close.
template<>
struct default_delete<FILE> {
  void operator()(FILE* file_ptr) const {
    fclose(file_ptr);
  }
};
}  // namespace std

namespace {

// Mode config
const int kNrParamPollMode = 1;
const int kNrParamUpdateMode = 2;

// device control commands and addresses
const int kDlAddr = 0xa2;
const int kSizeOfRebootPayload = 4;
const int kVersionNumberAddr = 0xaa;
const int kNrI2cFlag16bitAddr = 0x01;
const int kNrI2cFlagInhabit9Clock = 0x04;
const int kNrI2cFlagWaitForWrite = 0x02;
const int kNrUsbReqExec = 0x15;
const int kNrUsbReqStatusDw = 0x06;
const int kRebootAddr = 0x0381;
const int kWriteCmdAddr = 2;
const int kWriteCmdSeqA = 0xc212;
const int kWriteCmdSeqB = 0xc213;
const int kNrUsbReqTimeoutSec = 10000;
const int kI2cFlag =
    (kDlAddr |
        ((kNrI2cFlag16bitAddr |
          kNrI2cFlagInhabit9Clock |
          kNrI2cFlagWaitForWrite) << 8));

// version numbers
const int kFwMajorVerLen = 4;
const int kFwMinorVerLen = 4;
const int kFwVersionLen = kFwMajorVerLen + kFwMinorVerLen;
const int kFwVersionLenByte = kFwVersionLen/2;
const uint8_t kFwInvalidVersion[kFwVersionLenByte] = {0xff, 0xff, 0xff, 0xff};
const int kFwNameLen = 28;
const int kFwNameVersionOffset = 12;
const int kFwSizeLimit = 16 * 1024;

// USB ids and target dependent consts
const uint16_t kMimoVendorId = 0x17e9;
const uint16_t kMimoProductIdA = 0x416d;
const uint16_t kMimoProductIdB = 0x016b;
const uint16_t kMimoProductIdRecovery = 0x8060;
const uint8_t kMimoFwHeader = 0xa2;
const int kReservedChunkNumber = 2;
const int kSizeIOChunk = 64;
const int kUsbTimeOutMs = 1000;

// vendor-specific IDs
const int kVendorReqPoke = 0x03;
const int kVendorI2cRead = 0x19;
const int kVendorI2cWrite = 0x18;

const int kReqTypeVendorIn =
    (int(LIBUSB_REQUEST_TYPE_VENDOR) | int(LIBUSB_ENDPOINT_IN));
const int kReqTypeVendorOut =
    (int(LIBUSB_REQUEST_TYPE_VENDOR) | int(LIBUSB_ENDPOINT_OUT));

enum TestPointLocation {
  E_TP_INVALID,
  E_TP_BEFORE_INIT_FW_VERSION,
  E_TP_WRITE_FW
};

enum StatusCode {
  E_OK = 0,
  E_OK_NO_UPDATE,
  E_FAILED_USB_OPERATIONS,
  E_INVALID_FW_FILE_NAME,
  E_FAILED_FW_FILE_NOT_ACCESSIBLE,
  E_FAILED_INCORRECT_PARAM_LIST,
  E_FAILED_TO_RESET_FW_VERSION,
  E_FAILED_TO_GET_DEVICE_LIST,
  E_FAILED_TO_SEND_USB_CONTROL,
  E_FAILED_TO_FSEEK,
  E_FAILED_TO_GET_FILE_SIZE,
  E_FAILED_TO_OPEN_FW_FILE,
  E_FAILED_TO_READ_FW_FILE,
  E_INVALID_FW_FILE_SIZE
};

enum OperationMode {
  E_POLL_VERSION_MODE,
  E_FW_UPDATE_MODE
};

TestPointLocation test_point_position = E_TP_INVALID;
uint32_t test_point_wait_time = 0;

// This is a testing function
// This function can do 2 things.
// 1. mimic the power cut.
//    When you put the function in a certain place and setup
//    the test_point_position as the test_point_number parameter is,
//    the program will die if the 'cond' is true. By using this
//    method, you can test if the power-cut mechanism works.
// 2. delay the processing
//    When you want to test the complicated USB behaviors, you can
//    set the test_point_wait_time to a non-zero value. The program
//    will stop and wait for you. Then, you can do other USB commands.
static void TestPoint(const TestPointLocation test_point, bool cond = true) {
  if (test_point_position == test_point && cond) {
    if (test_point_wait_time == 0)
      abort();
    else
      sleep(10);
  }
}

// Description
//    Compare 2 version arrays
// Param:
//    v1 and v2 are uint8_t array, length of kFwVersionLenByte
// Return
//    true:   if they are identical
//    fasle:  if they are not identical.
static bool CompareVersionNumber(const uint8_t* v1, const uint8_t* v2) {
  return (memcmp(v1, v2, kFwVersionLenByte) == 0);
}

// Description
//    The function check the return code of the libusb func.
//    if it's not a good expected value, log the error.
// Param
//    usb_ret_code: actual return code
//    expected_ret: what we're expecting, generally 0
// Return
//    StatusCode.
static StatusCode LibUsbHelperError(
    const int usb_ret_code,
    const int expected_ret = 0) {
  if (usb_ret_code == expected_ret)
    return E_OK;

  LOG(ERROR)
      << "Usb error: "
      << libusb_error_name(usb_ret_code)
      << "(" << usb_ret_code << "," << expected_ret << ")";

  return E_FAILED_USB_OPERATIONS;
}

// Description
//    The function check the return code of the libusb func.
//    if it's not a good expected value, log the warning.
// Param
//    usb_ret_code: actual return code
//    expected_ret: what we're expecting, generally 0
// Return
//    StatusCode.
static StatusCode LibUsbHelperWarning(
    const int usb_ret_code,
    const int expected_ret = 0) {
  if (usb_ret_code == expected_ret)
    return E_OK;

  LOG(WARNING)
      << "Possible usb error: "
      << libusb_error_name(usb_ret_code)
      << "(" << usb_ret_code << "," << expected_ret << ")";

  return E_FAILED_USB_OPERATIONS;
}

// Description
//    The function sends a 'POKE' command to the ASIC.
//    Upon receiving this vendor command,
//    the ASIC will update the internal status according to the
//    addr and value. The payload of the control transfer is
//    the data provided.
// Param
//    dev_handle: the usb device we want to write to
//    value/addr: used by the control transfer
//    data/data_len: payload and its size
// return
//    Status code
static StatusCode Poke(
    libusb_device_handle *dev_handle,
    int value,
    int addr,
    uint8_t *data,
    int data_len) {
  int written_length = libusb_control_transfer(
      dev_handle,
      kReqTypeVendorOut,
      kVendorReqPoke,
      value,
      addr,
      data,
      data_len,
      kUsbTimeOutMs);

  if (LibUsbHelperError(written_length, data_len))
    return E_FAILED_TO_SEND_USB_CONTROL;

  return E_OK;
}

// Description
//    Request the device to do a I2C read access.
// return
//    Status code
static StatusCode I2cRead(
    libusb_device_handle *dev_handle,
    int value,
    int addr,
    uint8_t *data,
    int data_len) {
  int written_len = libusb_control_transfer(
      dev_handle,
      kReqTypeVendorIn,
      kVendorI2cRead,
      value,
      addr,
      data,
      data_len,
      kUsbTimeOutMs);

  if (LibUsbHelperError(written_len, data_len))
    return E_FAILED_TO_SEND_USB_CONTROL;

  return E_OK;
}

// Description
//    Request the device to do a I2C write access.
// return
//    Status code
static StatusCode I2cWrite(
    libusb_device_handle *dev_handle,
    int value,
    int addr,
    uint8_t *data,
    int data_len) {
  int written_len = libusb_control_transfer(
      dev_handle,
      kReqTypeVendorOut,
      kVendorI2cWrite,
      value,
      addr,
      data,
      data_len,
      kUsbTimeOutMs);

  if (LibUsbHelperError(written_len, data_len))
    return E_FAILED_TO_SEND_USB_CONTROL;

  return E_OK;
}

// Description
//    Reboot the device.
// return
//    Status code
static StatusCode RebootMimo(libusb_device_handle *dev_handle) {
  uint32_t len = 4;
  uint8_t payload[kSizeOfRebootPayload] = {0, 0, 0, 0};

  int ret = libusb_control_transfer(
      dev_handle,
      kReqTypeVendorOut,
      kNrUsbReqExec,
      0,
      kRebootAddr,
      payload,
      kSizeOfRebootPayload,
      kNrUsbReqTimeoutSec);
  LibUsbHelperWarning(ret, 4);

  ret = libusb_control_transfer(
      dev_handle,
      kReqTypeVendorOut,
      kNrUsbReqStatusDw,
      0,
      0,
      reinterpret_cast<uint8_t*>(&len),
      sizeof(len),
      kUsbTimeOutMs);

  LibUsbHelperWarning(ret, sizeof(len));
  return E_OK;
}

static StatusCode WriteFwVersion(
    libusb_device *dev,
    libusb_device_handle *dev_handle,
    int prog_if,
    const uint8_t* version) {
  StatusCode return_code;
  uint8_t payload;
  uint8_t version_payload[kFwVersionLenByte];

  TestPoint(E_TP_BEFORE_INIT_FW_VERSION);

  payload = 9;
  const int payload_size = sizeof(payload);
  if (Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqA, &payload, payload_size) ||
      Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqB, &payload, payload_size)) {
    return E_FAILED_USB_OPERATIONS;
  }

  // copy the version number(const type conversion for libusb)
  memcpy(version_payload, version, kFwVersionLenByte);

  return_code = I2cWrite(
      dev_handle,
      kVersionNumberAddr,
      kI2cFlag,
      version_payload,
      kFwVersionLenByte);

  if (return_code)
    return return_code;

  return E_OK;
}

static StatusCode CalcFileSize(
    const std::string& file_name,
    size_t *fw_file_size) {
  struct stat fw_file_info;

  if (stat(file_name.c_str(), &fw_file_info))
    return E_FAILED_TO_GET_FILE_SIZE;

  *fw_file_size = fw_file_info.st_size;
  return E_OK;
}

static StatusCode WriteFw(
    libusb_device *dev,
    libusb_device_handle *dev_handle,
    int prog_if,
    bool reset,
    const std::string& file_name) {
  uint8_t payload[kSizeIOChunk];
  StatusCode status;
  size_t fw_file_size;
  int n_chunks;
  int c_pos;
  int current_chunk;
  size_t rc;

  // check filesize
  status = CalcFileSize(file_name, &fw_file_size);
  if (status) {
    return status;
  } else {
    if (fw_file_size <= 0 || fw_file_size > kFwSizeLimit) {
      return E_INVALID_FW_FILE_SIZE;
    }
  }

  // Note: we use 'e' flag here. glibc extension
  std::unique_ptr<FILE> fw_file(fopen(file_name.c_str(), "rbe"));

  if (fw_file.get() == nullptr)
    return E_FAILED_TO_OPEN_FW_FILE;

  // magic codes needed by the device
  payload[0] = 9;

  if (Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqA, payload, 1) ||
      Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqB, payload, 1)) {
    return E_FAILED_USB_OPERATIONS;
  }

  n_chunks = (fw_file_size + (kSizeIOChunk - 1)) / kSizeIOChunk;
  c_pos = 0;

  for (current_chunk = 0; current_chunk < n_chunks; current_chunk++) {
    // Test function: Simulate power-cut
    TestPoint(E_TP_WRITE_FW, (current_chunk == 5));

    rc = fread(payload, sizeof(uint8_t), kSizeIOChunk, fw_file.get());

    if (ferror(fw_file.get()) != 0 || rc == 0) {
      return E_FAILED_TO_READ_FW_FILE;
    }

    payload[0] *= static_cast<uint8_t>(current_chunk != 0);

    // avoid overwrite serial number
    if (current_chunk == kReservedChunkNumber) {
      // skipping the reserved parts.
      status = I2cWrite(dev_handle, 0xa0, kI2cFlag, &payload[16], 10);
      if (status)
        return status;

      // skipping the reserved parts again.
      status = I2cWrite(dev_handle, 0xc0, kI2cFlag, &payload[32], 16);
      if (status)
        return status;

    } else {
      // for normal blocks
      status = I2cWrite(dev_handle, c_pos, kI2cFlag, payload, rc);
    }

    if (status)
      return status;

    // increment the current position
    c_pos += kSizeIOChunk;
  }

  // make sure to write back the altered byte
  payload[0] = kMimoFwHeader;
  status = I2cWrite(dev_handle, 0, kI2cFlag, payload, sizeof(kMimoFwHeader));
  return status;
}

// read the firmware version from the device.
// The firmware version is located in the very last position of the firmware
static StatusCode ReadDeviceFwVersion(
    libusb_device *device,
    libusb_device_handle *dev_handle,
    const int prog_if,
    uint8_t *version) {
  StatusCode status;
  uint8_t data = 9;

  // Step 2. check if the USB kernel driver is active.
  // if active, we need to detach the driver first
  if (libusb_kernel_driver_active(dev_handle, prog_if) == 1) {
    if (LibUsbHelperError(libusb_detach_kernel_driver(dev_handle, prog_if))) {
      return E_FAILED_USB_OPERATIONS;
    }
  }

  // Step 3. set USB configurations
  LibUsbHelperWarning(libusb_set_configuration(dev_handle, 1));

  // Step 4. claim USB interface
  if (LibUsbHelperError(libusb_claim_interface(dev_handle, prog_if)))
    return E_FAILED_USB_OPERATIONS;

  // secret source: write 9 to 0xc212 and 0xc213.(using req type 2)
  if (Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqA, &data, 1) ||
      Poke(dev_handle, kWriteCmdAddr, kWriteCmdSeqB, &data, 1))
    return E_FAILED_USB_OPERATIONS;

  status = I2cRead(
      dev_handle,
      kVersionNumberAddr,
      kI2cFlag,
      version,
      kFwVersionLenByte);

  if (status)
    return status;

  return E_OK;
}

static void PrintFwVersion(uint8_t* version) {
  uint32_t version_number = 0;

  for (int i = 0; i < 4; ++i) {
    uint32_t current_digit = version[i];
    version_number = version_number + (current_digit << (8 * i));
  }

  LOG(INFO)
      << "Firmware version: "
      << "0x" << std::hex << version_number
      << std::dec;
}

// Actually update the firmware.
static StatusCode UpdateFw(
    libusb_device *dev,
    libusb_device_handle *device_handle,
    libusb_device_descriptor *desc,
    const int prog_if,
    const std::string& fw_file_name,
    const uint8_t* version) {
  // store the firmware version in the device.
  uint8_t curr_version[kFwVersionLenByte];
  StatusCode status;

  // Step 0. read the current version number
  status = ReadDeviceFwVersion(dev, device_handle, prog_if, curr_version);
  if (status != E_OK)
    return status;

  PrintFwVersion(curr_version);

  // the version is the same, no need to update
  if (CompareVersionNumber(version, curr_version))
    return E_OK_NO_UPDATE;

  // Step 1. reset the version number in firmware
  status = WriteFwVersion(dev, device_handle, prog_if, kFwInvalidVersion);
  if (status)
    return E_FAILED_TO_RESET_FW_VERSION;

  // Step 2. flash the firmware
  status = WriteFw(dev, device_handle, prog_if, false, fw_file_name);
  if (status)
    return status;

  // Step 3. update the correct firmware version
  status = WriteFwVersion(dev, device_handle, prog_if, version);
  if (status)
    return status;

  return E_OK;
}

static void Usage() {
  std::cout << "[Usage]" << std::endl;
  std::cout << "  mimo_fw_updater [fw binary file]: update the firmware";
  std::cout << std::endl;
  std::cout << "  mimo_fw_updater: to poll the version information";
  std::cout << std::endl;
}

static StatusCode ReadFwVersionFromFwFileName(
    const std::string& fw_file_name,
    uint8_t *version) {

  // reset the version
  memcpy(version, kFwInvalidVersion, kFwVersionLenByte);

  // firmware name format: mimofw_0x016B_0x00000001.bin
  if (fw_file_name.length() < kFwNameLen)
    return E_INVALID_FW_FILE_NAME;

  const int starting_position = fw_file_name.length() - kFwNameVersionOffset;
  std::string version_string(
      fw_file_name.substr(starting_position, kFwVersionLen));

  // validate the version string.
  for (char& c : version_string) {
    if (isxdigit(c) == 0)
      return E_INVALID_FW_FILE_NAME;
  }

  // construct the value
  uint32_t version_from_file;
  std::stringstream ss;
  ss << std::hex << version_string;
  ss >> version_from_file;

  // the endianness here would be correct
  // since stringstream should handle that
  memcpy(version, &version_from_file, kFwVersionLenByte);
  return E_OK;
}

static StatusCode CheckFwFileValidityAndGetVersion(
    const std::string& fw_file_name,
    uint8_t* fw_version) {
  std::ifstream fw_file_stream(
      fw_file_name.c_str(),
      std::ios_base::in | std::ios_base::binary);

  // verify the file existence and access priviledge
  if (!fw_file_stream.good())
    return E_FAILED_FW_FILE_NOT_ACCESSIBLE;

  // Retrieve the version number in the file name
  if (ReadFwVersionFromFwFileName(fw_file_name, fw_version))
    return E_INVALID_FW_FILE_NAME;

  return E_OK;
}

static StatusCode FwUpdateMainOrPollVersion(
    const std::string& file_name,
    const OperationMode operation_mode) {
  libusb_device **devs;
  libusb_context *ctx;
  libusb_device_descriptor device_descriptor;
  StatusCode status;
  uint8_t version[kFwVersionLenByte];

  // check the validity of the FW file name
  if (operation_mode == E_FW_UPDATE_MODE) {
    status = CheckFwFileValidityAndGetVersion(file_name, version);

    if (status)
      return status;
  }

  // Step 1. init USB lib
  if (LibUsbHelperError(libusb_init(&ctx)))
    return E_FAILED_USB_OPERATIONS;

#if defined(LIBUSB_API_VERSION) && (LIBUSB_API_VERSION >= 0x01000106)
  libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
#else
  libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_INFO);
#endif

  // Step 2. get all USB devices
  const ssize_t dev_cnt = libusb_get_device_list(ctx, &devs);

  if (dev_cnt < 0) {
    libusb_exit(ctx);
    return E_FAILED_TO_GET_DEVICE_LIST;
  }

  // Step 3. Iterator through the device list
  for (ssize_t i = 0; i < dev_cnt; i++) {
    // current device pointer
    libusb_device *current_device = devs[i];

    // get the device descriptor, if failed, we just keep going.
    if (libusb_get_device_descriptor(current_device, &device_descriptor))
      continue;

    // check ID
    if (device_descriptor.idVendor != kMimoVendorId)
      continue;

    const int pid = device_descriptor.idProduct;

    if ((pid == kMimoProductIdA) ||
        (pid == kMimoProductIdB) ||
        (pid == kMimoProductIdRecovery)) {
      const int prog_if = (pid == kMimoProductIdA) ? 1 : 0;
      libusb_device_handle *device_handle;

      if (LibUsbHelperError(libusb_open(current_device, &device_handle)))
        return E_FAILED_USB_OPERATIONS;

      switch (operation_mode) {
        case E_POLL_VERSION_MODE: {
          uint8_t fw_version[kFwVersionLenByte];
          status = ReadDeviceFwVersion(
              current_device,
              device_handle,
              prog_if,
              fw_version);

          if (status == E_OK) {
            PrintFwVersion(fw_version);
          } else {
            LOG(ERROR)
                << "Failed to poll firmware: " << status
                << ", device: " << i;
          }
          break;
        }
        case E_FW_UPDATE_MODE: {
          StatusCode status;
          status = UpdateFw(
              current_device,
              device_handle,
              &device_descriptor,
              prog_if,
              file_name,
              version);

          if (status != E_OK && status != E_OK_NO_UPDATE) {
            LOG(ERROR)
                << "Failed to update firmware: " << status
                << ", device: " << i;
          }

          // The reason that it is rebooted even if the update fails is that
          // the device will be in the recovery mode after reboot. the device
          // to let the device go into the recovery mode.
          // If it's in the recovery mode, the system will have a chance to
          // redo it again to fix the problem.
          if (status != E_OK_NO_UPDATE)
            RebootMimo(device_handle);

          break;
        }
        default: {
          LOG(ERROR)
              << "Unknown operation mode("
              << operation_mode << ")";
          break;
        }
      }

      LibUsbHelperWarning(libusb_release_interface(device_handle, prog_if));
      LibUsbHelperWarning(libusb_attach_kernel_driver(device_handle, prog_if));
      libusb_close(device_handle);
    }
  }

  // clean up libusb devices
  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  return E_OK;
}
}  // namespace

int main(int argc, char **argv) {
  // Initialize the logging system.
  brillo::InitLog(brillo::InitFlags::kLogToSyslog);

  OperationMode operation_mode;
  std::string fw_file_name;

  // check the input
  if (argc == kNrParamPollMode) {
    LOG(INFO) << "Querying device firmware version info:";
    operation_mode = E_POLL_VERSION_MODE;
  } else if (argc == kNrParamUpdateMode) {
    operation_mode = E_FW_UPDATE_MODE;
    fw_file_name = argv[1];
  } else {
    Usage();
    return E_FAILED_INCORRECT_PARAM_LIST;
  }

  StatusCode status;

  // executing the update procedure
  status = FwUpdateMainOrPollVersion(fw_file_name, operation_mode);

  if (status != E_OK) {
    LOG(ERROR)
        << "Failed to update firmware: "
        << "status: " << status;
    return status;
  }

  return E_OK;
}
