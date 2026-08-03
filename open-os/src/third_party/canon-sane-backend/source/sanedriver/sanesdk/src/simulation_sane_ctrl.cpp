/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include <vector>
#include <memory>
#include <string>
#include "log.h"
#include "option_util.h"
#include "sane_ctrl_interface.h"
#include "options_interface.h"

#ifndef STRING_LENGTH_MAX
#define STRING_LENGTH_MAX 256
#endif
#ifndef SANE_VALUE_SCAN_MODE_BLACK_WHITE
#define SANE_VALUE_SCAN_MODE_BLACK_WHITE SANE_I18N("Black & White")
#endif
#ifndef SANE_VALUE_SCAN_MODE_ATEII
#define SANE_VALUE_SCAN_MODE_ATEII SANE_I18N("ATEII")
#endif
#ifndef SANE_VALUE_SCAN_MODE_ERROR_DIFFUSION
#define SANE_VALUE_SCAN_MODE_ERROR_DIFFUSION SANE_I18N("Error Diffusion")
#endif
namespace {
	SANE_Frame mode2frame(long spp, long bps)
	{
		SANE_Frame out;
		if (spp==3) {
			out = SANE_FRAME_RGB;
		} else {
			out = SANE_FRAME_GRAY;
		}
		return out;
	}
	class CNumOfOptions : public CSaneOptionBase
	{
	public:
		CNumOfOptions(long num);
		virtual ~CNumOfOptions();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;	
		long m_num;
	};
	SANE_Option_Descriptor CNumOfOptions::m_desc = {
		SANE_NAME_NUM_OPTIONS,
		SANE_TITLE_NUM_OPTIONS,
		SANE_DESC_NUM_OPTIONS,
		SANE_TYPE_INT,
		SANE_UNIT_NONE,
		sizeof(SANE_Word),
		SANE_CAP_SOFT_DETECT,
		SANE_CONSTRAINT_NONE,
		{NULL}
	};
	CNumOfOptions::CNumOfOptions(long num):CSaneOptionBase(), m_num(num)
	{
	}
	CNumOfOptions::~CNumOfOptions()
	{}
	SANE_Status CNumOfOptions::get(void *value, SANE_Int *info)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		*(SANE_Word*)value = m_num;
		return SANE_STATUS_GOOD;
	}
#if 0
	class CPreview : public CSaneOptionBase
	{
	public:
		CPreview();
		virtual ~CPreview();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;	
		SANE_Bool m_value;
	};	
	SANE_Option_Descriptor CPreview::m_desc = {
		SANE_NAME_PREVIEW,
		SANE_TITLE_PREVIEW,
		SANE_DESC_PREVIEW,
		SANE_TYPE_BOOL,
		SANE_UNIT_NONE,
		sizeof(SANE_Bool),
		SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
		SANE_CONSTRAINT_NONE,
		{NULL}
	};	
	CPreview::CPreview():CSaneOptionBase(), m_value(SANE_TRUE)
	{
	}
	CPreview::~CPreview()
	{
	}
	SANE_Status CPreview::get(void *value, SANE_Int *)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		*(SANE_Bool*)value = m_value;
		return SANE_STATUS_GOOD;
	}
	SANE_Status CPreview::set(void *value, SANE_Int *, bool)
	{
		if (value==NULL) return SANE_STATUS_INVAL;	
		m_value=*(SANE_Bool*)value;
		return SANE_STATUS_GOOD;
	}
#endif
	class CScanModeTitle : public CSaneOptionBase
	{
	public:
		CScanModeTitle();
		virtual ~CScanModeTitle();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;	
		SANE_Bool m_value;
	};	
	SANE_Option_Descriptor CScanModeTitle::m_desc = {
		"Scan Mode",
		"Scan Mode",
		"",
		SANE_TYPE_GROUP,
		SANE_UNIT_NONE,
		sizeof(SANE_Bool),
		SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
		SANE_CONSTRAINT_NONE,
		{NULL}
	};	
	CScanModeTitle::CScanModeTitle():CSaneOptionBase(), m_value(SANE_TRUE)
	{
	}
	CScanModeTitle::~CScanModeTitle()
	{
	}
	SANE_Status CScanModeTitle::get(void *value, SANE_Int *)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		*(SANE_Bool*)value = m_value;
		return SANE_STATUS_GOOD;
	}
	SANE_Status CScanModeTitle::set(void *value, SANE_Int *, bool)
	{
		if (value==NULL) return SANE_STATUS_INVAL;	
		m_value=*(SANE_Bool*)value;
		return SANE_STATUS_GOOD;
	}	
	class CScanMode : public CSaneOptionBase
	{
	public:
		CScanMode();
		virtual ~CScanMode();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;
		static SANE_String_Const m_list[];
		std::string m_s;
	};	
	SANE_String_Const CScanMode::m_list[6] = {
		SANE_VALUE_SCAN_MODE_LINEART,
		SANE_VALUE_SCAN_MODE_GRAY,
		SANE_I18N("Color"),
		SANE_I18N("Auto Color Detection"),
		0
	};
	SANE_Option_Descriptor CScanMode::m_desc = {
		SANE_NAME_SCAN_MODE,
		SANE_TITLE_SCAN_MODE,
		"Scan Mode",
		SANE_TYPE_STRING,
		SANE_UNIT_NONE,
		STRING_LENGTH_MAX,
		SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
		SANE_CONSTRAINT_STRING_LIST,
		{CScanMode::m_list}
	};
	CScanMode::CScanMode():CSaneOptionBase()
	{
		m_s = "Color";
	}
	CScanMode::~CScanMode()
	{
	}
	SANE_Status CScanMode::get(void *value, SANE_Int *info)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		strcpy((char*)value, m_s.c_str());
		return SANE_STATUS_GOOD;
	}
	SANE_Status CScanMode::set(void *value, SANE_Int *info, bool bauto)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		m_s = (char *)value;
		return SANE_STATUS_GOOD;
	}
#if 0
	class CResolution : public CSaneOptionBase
	{
	public:
		CResolution();
		virtual ~CResolution();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
	public:
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;
		static SANE_Word m_list[];
		SANE_Word m_v;

	};
	SANE_Word CResolution::m_list[8] = {
		sizeof(CResolution::m_list)/sizeof(CResolution::m_list[0]) - 1,
		100,
		150,
		200,
		240,
		300,
		400,
		600
	};
	SANE_Option_Descriptor CResolution::m_desc = {
		SANE_NAME_SCAN_RESOLUTION,
		"Scan resolution",
		"Scan resolution",
		SANE_TYPE_INT,
		SANE_UNIT_DPI,
		sizeof(SANE_Int),
		SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
		SANE_CONSTRAINT_WORD_LIST,
		/* Gcc warns "dereferencing type-punned
		pointer will break strict-aliasing rules" at here.
		But this code is safe and we can't suppress it.
		Please ignore the warning. */
		{(const SANE_Char* const*)CResolution::m_list}
	};		
	CResolution::CResolution():CSaneOptionBase(), m_v(CResolution::m_list[0])
	{
	}
	CResolution::~CResolution()
	{
	}
	SANE_Status CResolution::get(void *value, SANE_Int *)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		*(SANE_Word*)value=m_v;
		return SANE_STATUS_GOOD;
	}
	SANE_Status CResolution::set(void *value, SANE_Int *, bool)
	{
		if (value==NULL) return SANE_STATUS_INVAL;	
		m_v = *(SANE_Word *)value;
		return SANE_STATUS_GOOD;
	}
	class CScanSide : public CSaneOptionBase
	{
	public:
		CScanSide();
		virtual ~CScanSide();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
	public:
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;
		static SANE_String_Const m_list[];
		std::string m_s;
	};	
	SANE_String_Const CScanSide::m_list[] = {"Simplex", "Duplex", "Skip blank page", NULL};
	SANE_Option_Descriptor CScanSide::m_desc = {
		"ScanMode",
		"ScanMode",
		"scanmode,choose simplex, duplex or skip blank page scan",
		SANE_TYPE_STRING,
		SANE_UNIT_NONE,
		STRING_LENGTH_MAX,
		SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
		SANE_CONSTRAINT_STRING_LIST,
		{CScanSide::m_list}
	};
	CScanSide::CScanSide():CSaneOptionBase()
	{
	}
	CScanSide::~CScanSide()
	{
	}
	SANE_Status CScanSide::get(void *value, SANE_Int *)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		strcpy((char*)value, m_s.c_str());		
		return SANE_STATUS_GOOD;
	}
	SANE_Status CScanSide::set(void *value, SANE_Int *, bool)
	{
		if (value==NULL) return SANE_STATUS_INVAL;	
		m_s = (char *)value;
		return SANE_STATUS_GOOD;
	}
	class CDeskew : public CSaneOptionBase
	{
	public:
		CDeskew();
		virtual ~CDeskew();
		SANE_Status get(void *value, SANE_Int *info=NULL);
		SANE_Status set(void *value, SANE_Int *info=NULL, bool bauto=false);
	public:
		SANE_Option_Descriptor*descriptor(){return &m_desc;}
	private:	
		static SANE_Option_Descriptor m_desc;
		SANE_Bool m_value;
	};		
	SANE_Option_Descriptor CDeskew::m_desc = {
	             "Deskew",
	            "Deskew",
	            "Image skew collection",
	            SANE_TYPE_BOOL,
	            SANE_UNIT_NONE,
	            sizeof(SANE_Bool),
	            SANE_CAP_SOFT_DETECT | SANE_CAP_SOFT_SELECT,
	            SANE_CONSTRAINT_NONE,
	            {NULL}
	};
	CDeskew::CDeskew():CSaneOptionBase(), m_value(SANE_TRUE)
	{
	}
	CDeskew::~CDeskew()
	{
	}
	SANE_Status CDeskew::get(void *value, SANE_Int *)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		*(SANE_Bool*)value = m_value;	
		return SANE_STATUS_GOOD;
	}
	SANE_Status CDeskew::set(void *value, SANE_Int *, bool)
	{
		if (value==NULL) return SANE_STATUS_INVAL;
		m_value = *(SANE_Word *)value;
		return SANE_STATUS_GOOD;
	}
#endif
	class CSimulationOptions : public ISaneOptions
	{
	public:
		CSimulationOptions();
		virtual ~CSimulationOptions();
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
	};
	CSimulationOptions::CSimulationOptions():m_ref(1)
	{
		m_options.push_back(NULL);
		m_options.push_back(new CScanModeTitle);
		m_options.push_back(new CScanMode);
		//m_options.push_back(new CScanSide);
		//m_options.push_back(new CResolution);
		//m_options.push_back(new CDeskew);
		m_options[0] = new CNumOfOptions((long)m_options.size());
	}
	CSimulationOptions::~CSimulationOptions()
	{
		OPTIONLIST::iterator itr = m_options.begin();
		for (; itr!=m_options.end(); itr++) {
			(*itr)->Release();
		}
		m_options.clear();
	}
	long CSimulationOptions::QueryInterface(REFIID id, void **ppOut)
	{
		return -1;
	}
	unsigned long CSimulationOptions::AddRef()
	{
		m_ref++;
		return m_ref;
	}
	unsigned long CSimulationOptions::Release()
	{
		m_ref--;
		if (m_ref<=0) {
			delete this;
			return  0;
		}
		return m_ref;
	}
	void CSimulationOptions::scan_start()
	{

	}
	void CSimulationOptions::image_process(LPCEIIMAGEINFO2)
	{
		
	}
	void CSimulationOptions::on_error(long errorcode)
	{
		
	}
	long CSimulationOptions::csderror2saneerror(long)
	{
		return -1;
	}
	void CSimulationOptions::scan_end()
	{

	}
	const SANE_Option_Descriptor *CSimulationOptions::get_option_descriptor(SANE_Int option)
	{
		if (option<0) return NULL;
		if (option>=(SANE_Int)m_options.size()) return NULL;
		return m_options[option]->descriptor();
	}
	SANE_Status CSimulationOptions::control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)
	{
		if (option<0) return SANE_STATUS_INVAL;
		if (option>=(SANE_Int)m_options.size()) return SANE_STATUS_INVAL;
		return m_options[option]->control(action, value, info);
	}
    class CSimulationScan
    {
    public:
    	CSimulationScan();
    	~CSimulationScan();
    	SANE_Status start();
    	SANE_Status get_parameters(SANE_Parameters * params);
    	SANE_Status read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
    private:
    	unsigned char *m_cur;
    	SANE_Int  m_last;
    	bool m_first;
    	bool m_outofdocuments;
    	struct {
    		unsigned char *lpImage;
    		long lWidth, lHeight, lSync, lBps, lSpp, lXResolution, lYResolution, tImageSize;
    	}m_img;
    	long m_cnt;
    };
	CSimulationScan::CSimulationScan() :  m_cur(NULL), m_last(0), m_first(true), m_outofdocuments(false), m_cnt(4)
	{
		memset(&m_img, 0, sizeof(m_img));
	}
	CSimulationScan::~CSimulationScan()
	{
	}
	SANE_Status CSimulationScan::start()
	{
		SaneWriteLog(("CSimulationScan::start() start"));
		if (m_first) {
			m_img.lWidth = 2480;
			m_img.lHeight = 3507;
			m_img.lSpp = 3;
			m_img.lBps = 8;
			m_img.lSync = m_img.lWidth * m_img.lSpp;
			m_img.lXResolution = m_img.lYResolution = 300;
			m_img.tImageSize = m_img.lSync * m_img.lHeight;
			m_img.lpImage = new unsigned char [m_img.tImageSize];
			m_first=false;
		} else {
			m_cnt--;
		}
		
		if (m_cnt<=0) {
			m_outofdocuments=true;
			return SANE_STATUS_GOOD;
		}

		SaneWriteLog(("width:%d"), m_img.lWidth);
		SaneWriteLog(("height:%d"), m_img.lHeight);
		SaneWriteLog(("sync:%d"), m_img.lSync);
		SaneWriteLog(("bps:%d"), m_img.lBps);
		SaneWriteLog(("spp:%d"), m_img.lSpp);
		SaneWriteLog(("xdpi:%d"), m_img.lXResolution);
		SaneWriteLog(("ydpi:%d"), m_img.lYResolution);
		SaneWriteLog(("size:%d"), m_img.tImageSize);

		m_last = m_img.tImageSize;
		m_cur  = m_img.lpImage;

		SaneWriteLog(("CSimulationScan::start() end"));
		return SANE_STATUS_GOOD;
	}
	SANE_Status CSimulationScan::get_parameters(SANE_Parameters * params)
	{
		SaneWriteLog(("CSimulationScan::sane_get_parameters() start"));

		params->format=mode2frame(m_img.lSpp, m_img.lBps);
		params->last_frame=1;
		params->bytes_per_line=m_img.lSync;
		params->pixels_per_line=m_img.lWidth;
		params->lines=m_img.lHeight;
		params->depth=m_img.lBps;

		SaneWriteLog(("format %s"), params->format==SANE_FRAME_GRAY?"SANE_FRAME_GRAY":"SANE_FRAME_RGB");
		SaneWriteLog(("last_frame %d"), params->last_frame);
		SaneWriteLog(("bytes_per_line %d"), params->bytes_per_line);
		SaneWriteLog(("pixels_per_line %d"), params->pixels_per_line);
		SaneWriteLog(("lines %d"), params->lines);
		SaneWriteLog(("depth %d"), params->depth);

		SaneWriteLog(("CSimulationScan::sane_get_parameters() end"));
		return SANE_STATUS_GOOD;
	}
	SANE_Status CSimulationScan::read(SANE_Byte * data, SANE_Int max_length, SANE_Int *length)
	{
		SaneWriteLog(("CSimulationScan::sane_read() start"));
		if (m_outofdocuments) return SANE_STATUS_NO_DOCS;
		if (m_cur) {
			if (m_last<=0) {
				SaneWriteLog(("EOF"));
				m_cur=NULL;
				m_last=0;
				return SANE_STATUS_EOF;
			}
			SANE_Int s=max_length;
			if (s>(SANE_Int)m_last) s=(SANE_Int)m_last;
			memcpy(data, m_cur, s);
			*length=s;
			m_cur+=s;
			m_last-=s;
		} else {
			SaneWriteLog(("EOF(2)"));
			return SANE_STATUS_EOF;
		}
		SaneWriteLog(("CSimulationScan::sane_read() end"));
		return SANE_STATUS_GOOD;
	}
}
class CSimulationSaneCtrl : public ICeiSane
{
public:
	CSimulationSaneCtrl();
	virtual ~CSimulationSaneCtrl();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
	const SANE_Option_Descriptor *get_option_descriptor(SANE_Int option);
	SANE_Status control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info);
	SANE_Status get_parameters(SANE_Parameters * params);
	SANE_Status start();
	SANE_Status read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
	void cancel();
	SANE_Status set_io_mode(SANE_Bool non_blocking);
	SANE_Status get_select_fd(SANE_Int *fd);
private:
	long m_ref;
	XInterface<ISaneOptions> m_options;
	std::unique_ptr<CSimulationScan> m_scan;
};
CSimulationSaneCtrl::CSimulationSaneCtrl():m_ref(1)
{
	m_options.reset(new CSimulationOptions);
}
CSimulationSaneCtrl::~CSimulationSaneCtrl()
{
}
long CSimulationSaneCtrl::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSimulationSaneCtrl::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSimulationSaneCtrl::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return  0;
	}
	return m_ref;
}
const SANE_Option_Descriptor *CSimulationSaneCtrl::get_option_descriptor(SANE_Int option)
{
	return m_options->get_option_descriptor(option);
}
SANE_Status CSimulationSaneCtrl::control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)
{
	return m_options->control_option(option, action, value, info);
}
SANE_Status CSimulationSaneCtrl::get_parameters(SANE_Parameters * params)
{
	SaneWriteLog(("sane_get_parameters() start"));
	if (m_scan.get()) {
		m_scan->get_parameters(params);
	} else {
		SaneWriteLog(("sane_get_parameters(normal) start"));
		long width=2480, length=3508, spp=3, bps=8, sync=width;
		params->format=spp==3?SANE_FRAME_RGB:SANE_FRAME_GRAY;
		params->last_frame=1;
		if (spp==3) {
			sync=3*width;
		} else if (bps==8) {
			sync = width;
		} else {
			sync = (width+7)/8;
		}
		params->bytes_per_line=sync;
		params->pixels_per_line=width;
		params->lines=length;
		params->depth=bps;
	}
	SaneWriteLog(("format %s"), params->format==SANE_FRAME_GRAY?"SANE_FRAME_GRAY":"SANE_FRAME_RGB");
	SaneWriteLog(("last_frame %d"), params->last_frame);
	SaneWriteLog(("bytes_per_line %d"), params->bytes_per_line);
	SaneWriteLog(("pixels_per_line %d"), params->pixels_per_line);
	SaneWriteLog(("lines %d"), params->lines);
	SaneWriteLog(("depth %d"), params->depth);
	SaneWriteLog(("sane_get_parameters(normal) end"));
	SaneWriteLog(("sane_get_parameters() end"));
	return SANE_STATUS_GOOD;
}
SANE_Status CSimulationSaneCtrl::start()
{
	SaneWriteLog(("CSimulationSaneCtrl::sane_read() start"));
	if (m_scan.get()==NULL) {
		m_scan.reset(new CSimulationScan());
		if (m_scan.get()==NULL) return SANE_STATUS_NO_MEM;
	}
	SANE_Status ret = m_scan->start();
	SaneWriteLog(("CSimulationSaneCtrl::sane_read() end"));
	return ret;
}
SANE_Status CSimulationSaneCtrl::read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length)
{
	SaneWriteLog(("CSimulationSaneCtrl::sane_read() start"));
	if (m_scan.get()==NULL) return SANE_STATUS_NO_MEM;
	SANE_Status status = m_scan->read(data, max_length, length);
	SaneWriteLog(("CSimulationSaneCtrl::sane_read() end"));
	return status;
}
void CSimulationSaneCtrl::cancel()
{
	SaneWriteLog(("CSimulationSaneCtrl::sane_cancel() start"));
	m_scan.reset(NULL);
	SaneWriteLog(("CSimulationSaneCtrl::sane_cancel() end"));	
}
SANE_Status CSimulationSaneCtrl::set_io_mode(SANE_Bool /*non_blocking*/)
{
    SaneWriteLog("CSimulationSaneCtrl::set_io_mode() start");
    SaneWriteLog("CSimulationSaneCtrl::set_io_mode() end");
    return SANE_STATUS_UNSUPPORTED;
}
SANE_Status CSimulationSaneCtrl::get_select_fd(SANE_Int *)/*(SANE_Int * fd)*/
{
   SaneWriteLog("CSimulationSaneCtrl::get_select_fd() start");
    SaneWriteLog("CSimulationSaneCtrl::get_select_fd() end");
    return SANE_STATUS_UNSUPPORTED;
}
/////////////////////////////////////////////////////////////////////////
//
ICeiSane *create_simulation_sane_ctrl()
{
	return new CSimulationSaneCtrl;
}