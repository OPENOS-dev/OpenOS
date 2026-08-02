/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SCANCONTROL_VIRTUAL_SCANNER_HEADER__
#define __SCANCONTROL_VIRTUAL_SCANNER_HEADER__

#include "image_interface.h"
#include "scanner_connector_interface.h"
#include "virtual_scanner_interface.h"

class IScanCtrl : public IUnknown
{
public:
	virtual long scan_start()=0;
	virtual long get_image(ICeiImage **ppOut)=0;
	virtual long get_information(long id, void *)=0;
	virtual long scan_end()=0;
	virtual long scanning()=0;//1:scanning 0:not scanning
	virtual long abort()=0;
	virtual long stop()=0;
};

/*
in case of vs
pscanner is IScannerConnector
option is IScannedImageCtrl
in case of csd
pscanner is IVirtualScanner
option is ICsdTags
*/
IScanCtrl *scan_control(IUnknown *pscanner, IUnknown *option, IUnknown *handle, IScanCtrl *prescan);
IScanCtrl* prescan_control(IUnknown* pscanner, IUnknown* option, IUnknown* handle);

#endif