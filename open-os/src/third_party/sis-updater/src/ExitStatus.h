// Copyright (c) 2017, Mimo Display LLC d/b/a Mimo Monitors
// Copyright (c) 2017, Silicon Integrated Systems Corporation
// All rights reserved

#ifndef SRC_EXITSTATUS_H_
#define SRC_EXITSTATUS_H_

#define EXIT_OK 0       /* Test pass or compare result is same */
#define EXIT_ERR 32     /* Error occurs */
#define EXIT_FAIL 33    /* Test fail or compare result is different */
#define EXIT_BADARGU 34 /* Error input argument */
#define EXIT_NODEV 35   /* Device not found */

#endif  // SRC_EXITSTATUS_H_
