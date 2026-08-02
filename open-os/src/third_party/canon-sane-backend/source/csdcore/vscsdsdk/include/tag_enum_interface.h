/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_TAG_ENUM_INTERFACE_HEADER_DEFINED__
#define __CSD_TAG_ENUM_INTERFACE_HEADER_DEFINED__

#include "tag_interface.h"
#include "tags_interface.h"
#include "virtual_scanner_interface.h"
#include "scanctrl_interface.h"

long enum_csdtags(ICsdTags2 *parent, IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *handle);

#endif