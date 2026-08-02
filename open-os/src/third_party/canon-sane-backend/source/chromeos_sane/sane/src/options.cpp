/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <vector>
#include "log.h"
#include "option.h"
#include "options_interface.h"
#include "sane_global_apis.h"
class CSaneOptions : public ISaneOptions
{
public:
	CSaneOptions(ISaneCsdCore *p);
	virtual ~CSaneOptions();
	long init();
	void uninit();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	const SANE_Option_Descriptor *get_option_descriptor(SANE_Int option);
	SANE_Status control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info);
	void scan_start();
	void image_process(LPCEIIMAGEINFO2);
	void on_error(long errorcode);
	long csderror2saneerror(long errorcode);
	void scan_end();
private:
	long m_ref;
	typedef std::vector<ISaneOption *> OPTIONLIST;
	OPTIONLIST m_options;
	ISaneCsdCore *m_csdcore;
};
CSaneOptions::CSaneOptions(ISaneCsdCore *p):m_ref(1), m_csdcore(p)
{
}
CSaneOptions::~CSaneOptions()
{
	uninit();
}
long CSaneOptions::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSaneOptions::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSaneOptions::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return  0;
	}
	return m_ref;
}
long CSaneOptions::init()
{
	SaneWriteLog(" CSaneOptions::init() start");
	m_options.push_back(NULL); //for CNumOfOptions
	m_options.push_back(new CScanMode(m_csdcore));
	m_options.push_back(new CResolution(m_csdcore));
	m_options.push_back(new CScanSide(m_csdcore));
	m_options.push_back(new CAutoSizeDeskew(m_csdcore));
	if (sanesdk::ceisdk_get_private_profile_int("SCANNER", "DoubleFeedDetection")) {
		m_options.push_back(new CDoubleFeedDetection(m_csdcore, CSDP_DBLFEEDUSS));
	} else if (sanesdk::ceisdk_get_private_profile_int("SCANNER", "DoubleFeedDetection(length)"))
	{
		m_options.push_back(new CDoubleFeedDetection(m_csdcore, CSDP_DBLFEEDLENGTH));
	}
	m_options.push_back(new CBrightness(m_csdcore));
	m_options.push_back(new CContrast(m_csdcore));

	m_options.push_back(new CSerialNumber(m_csdcore));
	m_options.push_back(new CScanCount(m_csdcore));
	m_options.push_back(new CAdfStatus(m_csdcore));
	if (sanesdk::ceisdk_get_private_profile_int("SCANNER", "button", 1)) {
		m_options.push_back(new CButtonStatus(m_csdcore));
	}
	m_options[0] = new CNumOfOptions(m_options.size());
	m_csdcore->CsdParSet(CSDP_PAGESIZE, (LPARAM)"Maximum");
	SaneWriteLog(" CSaneOptions::init() end");
	return 0;
}
void CSaneOptions::uninit()
{
	OPTIONLIST::iterator itr = m_options.begin();
	for (; itr!=m_options.end(); itr++) {
		(*itr)->Release();
	}
	m_options.clear();
}
const SANE_Option_Descriptor *CSaneOptions::get_option_descriptor(SANE_Int option)
{
	SaneWriteLog("get option desc start");
	if (option<0) return NULL;
	if (option>=(SANE_Int)m_options.size()) {
		SaneWriteLog("get option desc end NULL");
		return NULL;
	}
	SaneWriteLog("get option desc end");
	return m_options[option]->descriptor();
}
SANE_Status CSaneOptions::control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)
{
	if (option<0) return SANE_STATUS_INVAL;
	if (option>=(SANE_Int)m_options.size()) return SANE_STATUS_INVAL;
	return m_options[option]->control(action, value, info);
}
void CSaneOptions::scan_start()
{
	// this api is called before CsdStartScan() is called.
}
void CSaneOptions::image_process(LPCEIIMAGEINFO2 pimg)
{
	//SaneWriteLog("CSaneOptions::image_process() start");
	/*
		if you want to release image here, do the following steps.
		1:m_csdcore->CsdReleaseImage(pimg);
		2:pimg->lpImage = NULL;
	*/
	//SaneWriteLog("CSaneOptions::image_process end");	
}
void CSaneOptions::on_error(long errorcode)
{
}
long CSaneOptions::csderror2saneerror(long errorcode)
{
	struct {
		INT32 csd;
		long sn;
	}tbl[] = {
		{CSD_NOPAGE, SANE_STATUS_NO_DOCS},
		{CSD_NOPAPER, SANE_STATUS_NO_DOCS},
		{CSD_COVEROPEN, SANE_STATUS_COVER_OPEN},
		{CSD_JAM, SANE_STATUS_JAMMED},
		{CSD_SKEWFEED, SANE_STATUS_JAMMED},
		{CSD_DOUBLEFEED, SANE_STATUS_JAMMED},
		{CSD_DRIVERBUSY, SANE_STATUS_DEVICE_BUSY},
		{CSD_CANCEL, SANE_STATUS_CANCELLED},
		{CSD_NOMEM, SANE_STATUS_NO_MEM},

		{-1, SANE_STATUS_INVAL}
	};
	for (long i = 0; tbl[i].csd > 0; i++) {
		if (tbl[i].csd == errorcode) return tbl[i].sn;
	}
	return SANE_STATUS_INVAL;
}
void CSaneOptions::scan_end()
{
	// this api is called after CsdFlashScannedImage() is called.
}
//////////////////////////////////////////////////////////////////////////////////////
ISaneOptions *create_options(ISaneCsdCore *pcsdcore)
{
	CSaneOptions *p = new CSaneOptions(pcsdcore);
	long ret = p->init();
	if (ret) {
		delete p;
		return NULL;
	}
	return (ISaneOptions *)p;
}
