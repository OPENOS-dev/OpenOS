/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __VIRTUAL_SCANNER_INTERFACE_HEADER_DEFINED__
#define __VIRTUAL_SCANNER_INTERFACE_HEADER_DEFINED__

#include "image_interface.h"
#include "image_info_interface.h"
#include "scanner_connector_interface.h"
#include "scanned_imagectrl_interface.h"

class IVirtualScanner : public IUnknown 
{
public:
	virtual long scan_start(IScannedImageCtrl *psic)=0;
	virtual long image(ICeiImage **ppOut, ICeiImageInformation **ppiOut)=0;
	virtual long scanning()=0;/*returned value TRUE or FALSE*/
	virtual long abort()=0;
	virtual long stop()=0;
	virtual long scan_end()=0;

	virtual long exec_write(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_read(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_none(char *cdb, long cdb_size)=0;
};
class IVirtualScanner2 : public IVirtualScanner
{
public:
    virtual long set(long type, void *v)=0;
    virtual long get(long type, void *p)=0;
};
// {7A047D06-1476-4E13-ACE6-1CB18F54BB91} or 100001
extern REFIID IID_IVirtualScanner2;
class IInternalVirtualScanner : public IUnknown 
{
public:
	virtual long set_vsvalue(long type, void* v) = 0;
	virtual long get_vsvalue(long type, void* p) = 0;
};
// {EEF3118C-D16D-48D4-A8EE-A00EEFF49E8B} or 100002
extern REFIID IID_IInternalVirtualScanner;


IVirtualScanner *create_virtual_scanner(IScannerConnector *pscanner, IUnknown *handle);

#endif
