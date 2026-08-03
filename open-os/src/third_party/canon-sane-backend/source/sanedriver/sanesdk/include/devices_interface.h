/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_SDK_DEVICES_INTERFACE_DEFINE_HEADER__
#define __SANE_SDK_DEVICES_INTERFACE_DEFINE_HEADER__

#include "sane/sane.h"
#include "unknown.h"

class ISaneDevices : public IUnknown
{
public:
	virtual  SANE_Device **get_devices(SANE_Bool local_only=SANE_FALSE) = 0;
};

ISaneDevices *create_devices(const char *vendor_name, long vendor_id, const char *product_name, long product_id, const char *type_desc);
#endif