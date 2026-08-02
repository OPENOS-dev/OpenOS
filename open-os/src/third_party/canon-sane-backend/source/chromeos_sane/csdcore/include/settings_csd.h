/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SETTINGS_VSCORE_CLASS_HEADER_DEFINED__
#define __SETTINGS_VSCORE_CLASS_HEADER_DEFINED__

#include <memory>
#include "command.h"
#include "unknown.h"
#include "sdk_def.h"
#include "tags_interface.h"

class CSettingsCsd : public IUnknown
{ 
public:
	CSettingsCsd(LPVSCSD_SDK_INIT_INFORMATION pinfo);
	virtual ~CSettingsCsd();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void ** ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	void tags(ICsdTags *t);
	ICsdTags *tags();
	VSCSD_SDK_INIT_INFORMATION &info();
private:
	long m_ref;
	ICsdTags *m_tags;
	VSCSD_SDK_INIT_INFORMATION m_info;
};

#endif