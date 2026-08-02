/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include "ceilogwrite.h"
#include "sdk_pagesize_table.h"
#include "csdtags.h"
#include "sdk_tag_basic.h"
CYDpi::CYDpi(ICsdTags2 *parent):CCsdTagBase(CSDP_YRESOLUTION, parent), m_pxdpi(NULL)
{
	m_pxdpi=parent->get(CSDP_XRESOLUTION);
}
CYDpi::~CYDpi()
{
}
int CYDpi::get(void *lpParam)
{
	//WriteLog((char*)"CYDpi::get() start");
	int ret = m_pxdpi->get(lpParam);
	//WriteLog((char*)"CYDpi::get() end");
	return ret;
}
int CYDpi::get_default(void *lpParam)
{
	return m_pxdpi->get_default(lpParam);
}
int CYDpi::set(long long lParam)
{
	return m_pxdpi->set(lParam);
}
int CYDpi::set_default()
{
	return m_pxdpi->set_default();
}
int CYDpi::choice_count(long *lpCount)
{
	return m_pxdpi->choice_count(lpCount);
}
int CYDpi::choice(int index, void *lpParam)
{
	return m_pxdpi->choice(index, lpParam);
}
ICsdTag::CSDTAG_CHOICE_FLAG CYDpi::choice_flag()
{
	return m_pxdpi->choice_flag();
}
void CYDpi::save()
{
	m_pxdpi->save();
}
void CYDpi::restore()
{
	m_pxdpi->restore();
}
void CYDpi::flush()
{
	m_pxdpi->flush();
}
CScanSide::CScanSide(ICsdTags2 *parent):CCsdTagLong(CSDP_FEEDER, parent)
{
	m_def=m_choice[0]=0;//simplex
	m_choice[1]=1;//duplex
	m_pchoice=m_choice;
	m_choice_size = (long)(sizeof(m_choice)/sizeof(m_choice[0]));
}
CScanSide::~CScanSide()
{}
char *CScanSide::id_name()
{
	return (char*)"CSDP_FEEDER";
}


