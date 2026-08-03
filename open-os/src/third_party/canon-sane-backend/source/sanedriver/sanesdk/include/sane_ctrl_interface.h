/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_SANE_CONTROL_INTERFACE_HEADER_DEFINED__
#define __SANE_SANE_CONTROL_INTERFACE_HEADER_DEFINED__

#include "sane/sane.h"
#include "unknown.h"
class ICeiSane : public IUnknown
{
public:
	virtual const SANE_Option_Descriptor *get_option_descriptor(SANE_Int option)=0;
	virtual SANE_Status control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)=0;
	virtual SANE_Status get_parameters(SANE_Parameters * params)=0;
	virtual SANE_Status start()=0;
	virtual SANE_Status read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length)=0;
	virtual void cancel()=0;
	virtual SANE_Status set_io_mode(SANE_Bool non_blocking)=0;
	virtual SANE_Status get_select_fd(SANE_Int *fd)=0;
};

ICeiSane *create_sane_ctrl(const char *devicename, const char *scanner_name/*DR-M260*/, const char *lib_path/*/opt/Canon/drm260*/);
ICeiSane *create_simulation_sane_ctrl();
#endif