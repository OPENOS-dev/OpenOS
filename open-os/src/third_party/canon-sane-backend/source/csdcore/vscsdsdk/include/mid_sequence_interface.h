/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __MID_SEQUENE_INTERFACE_DEFINE_HEADER__
#define __MID_SEQUENE_INTERFACE_DEFINE_HEADER__

#include "unknown.h"
#include "message_queue_interface.h"
#include "image_interface.h"

class IMidSequence : public IUnknown
{
public:
	virtual void proc()=0;
};

IMidSequence *ip_sequence(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *handle);
IMidSequence *comp_sequence(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *handle);
IMidSequence *backup_sequence(IMessageQueue *prevq, IMessageQueue *nextq, IUnknown *handle);

#endif