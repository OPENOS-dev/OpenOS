/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef ___IMAGE_INTERFACE_HEADER__20211014__
#define ___IMAGE_INTERFACE_HEADER__20211014__


#ifndef ___IMAGE_INTERFACE2021__
#define ___IMAGE_INTERFACE2021__

#include "unknown.h"

class ICeiImage : public IUnknown
{
public:
	virtual char *img()=0;
	virtual long width()=0;
	virtual long height()=0;
	virtual long xdpi()=0;
	virtual long ydpi()=0;
	virtual long spp()=0;
	virtual long bps()=0;
	virtual long sync()=0;
	virtual long size()=0;
	virtual long comptype()=0;//0:none, 1:jpeg
	virtual long compinfo()=0;//comptype is none:not used, comptype is jpeg:quality 
};
#endif


#endif