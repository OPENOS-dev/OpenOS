/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __END_SEQUENCE_INTERFACE_HEADER_DEFINE__
#define __END_SEQUENCE_INTERFACE_HEADER_DEFINE__


#include "unknown.h"
#include "scanner_connector_interface.h"
#include "message_queue_interface.h"
#include "image_interface.h"

class IEndSequence : public IUnknown
{
public:
	virtual void on_batch_start(ICeiMessage *pmsg)=0;
	virtual void on_page_start(ICeiMessage *pmsg)=0;
	virtual void on_image_start(ICeiMessage *pmsg)=0;
	virtual void on_image(ICeiMessage *pmsg)=0;
	virtual void on_image_end(ICeiMessage *pmsg)=0;
	virtual void on_info_start(ICeiMessage *pmsg)=0;
	virtual void on_info(ICeiMessage *pmsg)=0;
	virtual void on_info_end(ICeiMessage *pmsg)=0;
	virtual void on_page_end(ICeiMessage *pmsg)=0;
	virtual void on_error(ICeiMessage *pmsg)=0;
	virtual void on_batch_end(ICeiMessage *pmsg)=0;
	virtual long get_image(ICeiImage **ppOut)=0;
	virtual long get_information(long id, void *pout)=0;
};

/*long get_information(long id, void *pout)

id > 0: id is csdtag.
see csdtags.h

id < 0: id is CStreamCmd etc.
see SDK_INFO_ID_XXXX , which is defined below.

*/
#define SDK_INFO_ID_STREAM -1

IEndSequence *end_sequence(IMessageQueue *q, IUnknown *handle);

#endif