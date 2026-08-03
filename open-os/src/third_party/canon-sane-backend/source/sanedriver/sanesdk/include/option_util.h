/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _SANE_TAGS_DEFINE_
#define _SANE_TAGS_DEFINE_

#include "sane/sane.h"
#include "sane/saneopts.h"
#include "csdcore_interface.h"
#include "option_interface.h"

class CSaneOptionBase : public ISaneOption
{
public:
	CSaneOptionBase();
	virtual ~CSaneOptionBase();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	virtual SANE_Status control(SANE_Action action, void *value, SANE_Int *info);
	virtual SANE_Status get(void *value, SANE_Int *info=NULL);
	virtual SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
private:
	long m_ref;
};
class CSaneOptionCsdCore : public CSaneOptionBase
{
public:
	CSaneOptionCsdCore(ISaneCsdCore *pcsdcore);
	virtual ~CSaneOptionCsdCore();
protected:
	ISaneCsdCore *m_csdcore;
};

#endif
