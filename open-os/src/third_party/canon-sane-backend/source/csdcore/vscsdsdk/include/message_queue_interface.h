/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __QUEUE_INTERFACE_HEADER_VS_HEADER__
#define __QUEUE_INTERFACE_HEADER_VS_HEADER__

#include "message_interface.h"

class IMessageQueue : public IUnknown
{
public:
	virtual long push(ICeiMessage *pin)=0;
	virtual long pop(ICeiMessage **ppout)=0;
	virtual long peek(ICeiMessage **ppout, long order/*from 1*/)=0;
	virtual long count()=0;
};

#endif