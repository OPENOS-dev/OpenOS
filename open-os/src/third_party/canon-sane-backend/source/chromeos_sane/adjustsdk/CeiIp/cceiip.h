/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/


#ifndef _CCEIIP
#define _CCEIIP

#include <ceiip.h>

class CCeiIP {
public :
	CCeiIP() {};
	virtual ~CCeiIP() {};
	virtual BOOL Start(CEIIMAGEINFO *pVD,CEIIMAGEINFO *pVS,void *pInfo) { return TRUE; };
	virtual void Line() {};
	virtual void Rect() {};
	virtual void Finish() {};
};

#endif	//_CCEIIP
