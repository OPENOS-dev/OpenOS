// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved.

#ifndef SRC_UPDATEFWPARAMETER_H_
#define SRC_UPDATEFWPARAMETER_H_

#include <cstring>
#include <string>
#include <vector>
#include "ApplicationParameter.h"
#include "Parameter.h"

/*===========================================================================*/
class UpdateFWParameter : public ApplicationParameter {
 public:
  UpdateFWParameter();
  ~UpdateFWParameter();
  int parse(int argc, char** argv);
  bool parseArgument(char* arg);
  int check();
  void print_usage();

 public:
  std::vector<std::string> filenames;
  bool update_bootloader;
  bool update_bootloader_auto;
  bool reserve_RODATA;
  bool update_parameter;
  bool force_update;
  bool jump_check;
  bool single_device;
  int wait_time;
  // This parameter is for test use only.
  int interrupt_point;
}; /* end of class UpdateFWParameter */
/*===========================================================================*/

#endif  // SRC_UPDATEFWPARAMETER_H_
