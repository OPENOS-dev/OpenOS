/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __IMAGEINFOMATION_INTERFACE_HEADER_DEFINE_HEADER_DEFINED__
#define __IMAGEINFOMATION_INTERFACE_HEADER_DEFINE_HEADER_DEFINED__

#include "unknown.h"

class ICeiImageInformation : public IUnknown
{
public:
	virtual int information(long id, void *)=0; 
};

#endif