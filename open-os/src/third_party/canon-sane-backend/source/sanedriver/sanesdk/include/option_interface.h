/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_OPTION_INTERFACE_DEFINE_HEADER__
#define __SANE_OPTION_INTERFACE_DEFINE_HEADER__

#include "sane/sane.h"
#include "unknown.h"

class ISaneOption : public IUnknown
{
public:
	virtual SANE_Status control(SANE_Action action, void *value, SANE_Int *info)=0;
	virtual SANE_Option_Descriptor *descriptor()=0;	
};


#endif