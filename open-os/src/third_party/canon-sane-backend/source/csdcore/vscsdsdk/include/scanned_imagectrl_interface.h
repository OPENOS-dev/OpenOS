/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SDK_SCANNED_IMAGE_CONTROL_INTERFACE_DEFINED_HEADER__
#define __SDK_SCANNED_IMAGE_CONTROL_INTERFACE_DEFINED_HEADER__

#include "unknown.h"

class IScannedImageCtrl : public IUnknown
{
public:
	virtual void scan_start() = 0;
	virtual void increment()=0;
	virtual void decrement()=0;
	virtual void scan_end(long err=0) = 0;
};

#endif