/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SEQUENCE_THREAD_INTERFACE_HEADER__
#define __SEQUENCE_THREAD_INTERFACE_HEADER__

#include "scanned_imagectrl_interface.h"
#include "message_queue_interface.h"
#include "image_interface.h"

class IStartSequenceThread : public IUnknown
{
public:
	virtual void proc() = 0;
	virtual long abort()=0;
	virtual long stop()=0;
};

class IMidSequenceThread : public IUnknown
{
public:
	virtual void proc()=0;
};

class IEndSequenceThread : public IUnknown
{
public:
	virtual long get_image(ICeiImage **ppOut)=0;
	virtual long get_information(long id, void *pout)=0;
};
/*
in case of vs
pscanner is IScannerConnector
option is not used
in case of csd
pscanner is IVirtualScanner
option is ICsdTags
*/
IStartSequenceThread *scan_sequence_thread(IUnknown *pscanner, IUnknown *option, IScannedImageCtrl *sic, IMessageQueue *q, IUnknown *handle);
IMidSequenceThread *ip_sequence_thread(IMessageQueue *prevq, IMessageQueue *nextq, IScannedImageCtrl *sic, IUnknown *handle);
IMidSequenceThread *comp_sequence_thread(IMessageQueue *prevq, IMessageQueue *nextq, IScannedImageCtrl *sic, IUnknown *tags);
IMidSequenceThread *backup_sequence_thread(IMessageQueue *prevq, IMessageQueue *nextq, IScannedImageCtrl *sic, IUnknown *tags);
IEndSequenceThread *end_sequence_thread(IMessageQueue *q, IScannedImageCtrl *sic, IUnknown *handle);

#endif