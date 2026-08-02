/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include "ceilogwrite.h"
#include "sdk_def.h"
#include "settings_csd.h"

CSettingsCsd::CSettingsCsd(LPVSCSD_SDK_INIT_INFORMATION pinfo):m_ref(1), m_tags(NULL)
{
	//WriteLog((char*)"SettingsCsd::CSettingsCsd() this is 0x%x", this);
	if (pinfo) {
		long sz = (long)sizeof(m_info);
		if (sz>pinfo->dwSize) sz=pinfo->dwSize;
		memcpy(&m_info, pinfo, sz);
	}
	m_info.dwSize = sizeof(m_info);
}
CSettingsCsd::~CSettingsCsd()
{
	//WriteLog((char*)"SettingsCsd::~CSettingsCsd()");
}
long CSettingsCsd::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSettingsCsd::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSettingsCsd::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CSettingsCsd::tags(ICsdTags *t)
{
	m_tags = t;
}
ICsdTags *CSettingsCsd::tags()
{
	return m_tags;
}
VSCSD_SDK_INIT_INFORMATION &CSettingsCsd::info()
{
	return m_info;
}
IUnknown *CreateCsdCoreDependentClass(LPVSCSD_SDK_INIT_INFORMATION pinfo)
{
	CSettingsCsd *p = new CSettingsCsd(pinfo);
	return (IUnknown*)p;
}