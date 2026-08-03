/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include "option_util.h"
#include "log.h"

CSaneOptionBase::CSaneOptionBase():m_ref(1)
{
}
CSaneOptionBase::~CSaneOptionBase()
{
}
long CSaneOptionBase:: QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSaneOptionBase:: AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSaneOptionBase:: Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
SANE_Status CSaneOptionBase::control(SANE_Action action, void *value, SANE_Int *info)
{
    SANE_Status status = SANE_STATUS_GOOD;
    
    SANE_Option_Descriptor *desc = descriptor();
    SaneWriteLog("%s", desc->title);
    switch (action) {
    case SANE_ACTION_GET_VALUE:
    status=get(value, info);
    break;
    case SANE_ACTION_SET_VALUE:
    if (info) *info = 0;
    status=set(value, info, false);
    break;
    case SANE_ACTION_SET_AUTO:
    status=set(value, info, true);
    break;
    default:return SANE_STATUS_INVAL;
    }
    return status;
}
SANE_Status CSaneOptionBase::get(void *value, SANE_Int *info)
{
	return SANE_STATUS_GOOD;
}
SANE_Status CSaneOptionBase::set(void *value, SANE_Int *info, bool bauto)
{
	return SANE_STATUS_GOOD;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
CSaneOptionCsdCore::CSaneOptionCsdCore(ISaneCsdCore *pcsdcore):m_csdcore(pcsdcore)
{
}
CSaneOptionCsdCore::~CSaneOptionCsdCore()
{
}


