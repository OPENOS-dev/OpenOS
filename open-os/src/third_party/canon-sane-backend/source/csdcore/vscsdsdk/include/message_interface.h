/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __MESSAGE_INTERFACE_HEADER_VS_HEADER__
#define __MESSAGE_INTERFACE_HEADER_VS_HEADER__

#include "unknown.h"

class ICeiMessage : public IUnknown
{
public:
	virtual long type()=0;
	virtual long get(void **ppout, bool brelease=false)=0;
	typedef enum tagMESSAGE_TYPE {
		MID_BATCH_START=0,
		MID_ERROR,
		MID_PAGE_START,
		MID_IMAGE_START,
		MID_IMAGE,
		MID_IMAGE_END,
		MID_INFO_START,
		MID_INFO,
		MID_INFO_END,
		MID_PAGE_END,
		MID_BATCH_END,
		MID_COUNT
	}MESSAGE_TYPE;
};

#endif