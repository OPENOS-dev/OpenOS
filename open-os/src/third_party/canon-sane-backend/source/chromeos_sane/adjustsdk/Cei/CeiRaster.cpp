/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#include "CeiRaster.h"
#include <new>

CCeiRaster::CCeiRaster(void)
: m_Buffer(0), m_ulSize(0)
{
}

CCeiRaster::~CCeiRaster(void)
{
	Clear();
}

void CCeiRaster::Clear(void)
{
	if (m_Buffer) {
		delete[] m_Buffer;
		m_ulSize = 0;
	}
}

bool CCeiRaster::SetSize(unsigned long dwNewLen)
{
	if (dwNewLen == 0) {
		return false;
	}
	Clear();
	m_Buffer = new(std::nothrow) unsigned char[dwNewLen];
	if (!m_Buffer) {
		return false;
	}
	m_ulSize = dwNewLen;
	return true;
}

unsigned long CCeiRaster::GetSize(void)
{
	return m_ulSize;
}

unsigned char* CCeiRaster::GetPtr(void)
{
	return m_Buffer;
}


