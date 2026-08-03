/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_SDK_OPTIONS_INTERFACE_DEFINED__
#define __SANE_SDK_OPTIONS_INTERFACE_DEFINED__

#include "sane/sane.h"
#include "option_interface.h"
#include "csdcore_interface.h"

class ISaneOptions : public IUnknown
{
public:
	virtual const SANE_Option_Descriptor *get_option_descriptor(SANE_Int option)=0;
	virtual SANE_Status control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)=0;
	virtual void scan_start()=0;
	virtual void image_process(LPCEIIMAGEINFO2 pimg)=0;
	virtual void on_error(long errorcode)=0;
	virtual long csderror2saneerror(long errorcode)=0;//if this api returns -1, default processing will work.
	virtual void scan_end()=0;
};

ISaneOptions *create_options(ISaneCsdCore *pcsdcore);

#endif