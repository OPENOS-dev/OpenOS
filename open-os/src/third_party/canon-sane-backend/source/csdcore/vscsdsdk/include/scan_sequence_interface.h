/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SCAN_SEQUENCE_INTERFACE_HEADER_DEFINE__
#define __SCAN_SEQUENCE_INTERFACE_HEADER_DEFINE__

#include "unknown.h"
#include "message_queue_interface.h"

class IScanSequenceCallback : public IUnknown
{
public:
	virtual bool stop_request()=0;
	virtual void batch_start() = 0;
	virtual void page_start() = 0;
	virtual void page_end(long number_of_scanned_image=1) = 0;//if a scanner gives front and back image at the same time, number_of_image should be 2.
	virtual void batch_end(long error_happened=0) = 0;//no paper is not included in error_happened.
};
class IScanSequenceCallback2 : public IScanSequenceCallback
{
public:
	virtual void stop_request(bool b)=0;
};


class IScanSequence : public IUnknown
{
public:
	virtual void main(IScanSequenceCallback *pcallback)=0;
};


/*
in case of vs
s is IScannerConnector
opt1/opt2 are not used
in case of csd
opt1 is ICsdTags
opt2 is IScannedImageCtrl
s is IVirtualScanner.
*/
IScanSequence *scan_sequence(IMessageQueue *q, IUnknown *scanner, IUnknown *handle, IUnknown *opt1, IUnknown *opt2);
IScanSequenceCallback2 *scan_sequence_callback(IUnknown *opt);
#endif