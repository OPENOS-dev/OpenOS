/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <memory.h>
#include <vector>
#include <memory>
#include "sane_ctrl_interface.h"
#include "csdcore_interface.h"
#include "options_interface.h"
#include "log.h"
#include "sane_global_apis.h"

namespace {
	const char *sanecap2str(SANE_Int cap, char *s)
	{
		*s = 0;
		if (cap&SANE_CAP_SOFT_SELECT) strcat(s, "SANE_CAP_SOFT_SELECT ");
		if (cap&SANE_CAP_HARD_SELECT) strcat(s, "SANE_CAP_HARD_SELECT ");
		if (cap&SANE_CAP_SOFT_DETECT) strcat(s, "SANE_CAP_SOFT_DETECT ");
		if (cap&SANE_CAP_EMULATED) strcat(s, "SANE_CAP_EMULATED ");
		if (cap&SANE_CAP_AUTOMATIC) strcat(s, "SANE_CAP_AUTOMATIC ");
		if (cap&SANE_CAP_INACTIVE) strcat(s, "SANE_CAP_INACTIVE ");
		if (cap&SANE_CAP_ADVANCED) strcat(s, "SANE_CAP_ADVANCED ");
		return s;
	}
	const char *saneunite2str(SANE_Unit type)
	{
		switch (type) {
	    case SANE_UNIT_NONE:return "SANE_UNIT_NONE";		/* the value is unit-less (e.g., # of scans) */
		case SANE_UNIT_PIXEL:return "SANE_UNIT_PIXEL";		/* value is number of pixels */
		case SANE_UNIT_BIT:return "SANE_UNIT_BIT";		/* value is number of bits */
		case SANE_UNIT_MM:return "SANE_UNIT_MM";		/* value is millimeters */
		case SANE_UNIT_DPI:return "SANE_UNIT_DPI";		/* value is resolution in dots/inch */
		case SANE_UNIT_PERCENT:return "SANE_UNIT_PERCENT";		/* value is a percentage */
		case SANE_UNIT_MICROSECOND:return "SANE_UNIT_MICROSECOND";	/* value is micro seconds */
		}
		return "SANE_UNIT_UNKNOWN";
   }
	const char *sanetype2str(SANE_Value_Type type)
	{
		switch (type) {
			case SANE_TYPE_BOOL:return "SANE_TYPE_BOOL";
			case SANE_TYPE_INT:return "SANE_TYPE_INT";
			case SANE_TYPE_FIXED:return "SANE_TYPE_FIXED";
			case SANE_TYPE_STRING:return "SANE_TYPE_STRING";
			case SANE_TYPE_BUTTON:return "SANE_TYPE_BUTTON";
			case SANE_TYPE_GROUP:return "SANE_TYPE_GROUP";
		}
	    return "SANE_TYPE_UNKNOWN";
	}
	const char *saneconstraint_type2str(SANE_Constraint_Type type)
	{
		switch (type) {
    	case SANE_CONSTRAINT_NONE:return "SANE_CONSTRAINT_NONE";
    	case SANE_CONSTRAINT_RANGE:return "SANE_CONSTRAINT_RANGE";
    	case SANE_CONSTRAINT_WORD_LIST:return "SANE_CONSTRAINT_WORD_LIST";
    	case SANE_CONSTRAINT_STRING_LIST:return "SANE_CONSTRAINT_STRING_LIST";
	    }
	    return "SANE_CONSTRAINT_UNKNOWN";
	}
	void SaneWriteLog_descripter(const SANE_Option_Descriptor *out)
	{
		if (!IsSaneWriteLog()) return;
		if (out==NULL) return;
		if (1) {
			SaneWriteLog("%s", out->title);
		} else {
			SaneWriteLog("name:%s", out->name);
			SaneWriteLog("title:%s", out->title);
			SaneWriteLog("desc:%s", out->desc);
			SaneWriteLog("type:%s", sanetype2str(out->type));
			SaneWriteLog("unit:%s", saneunite2str(out->unit));
			SaneWriteLog("size:%d", out->size);
			char s[256];
			SaneWriteLog("cap:%s", sanecap2str(out->cap, s));
			SaneWriteLog("constraint_type:%s", saneconstraint_type2str(out->constraint_type));
		}
	}
	void SaneWriteLog_Params(SANE_Parameters * params)
	{
		if (params==NULL) return;
		SaneWriteLog(("format %s"), params->format==SANE_FRAME_GRAY?"SANE_FRAME_GRAY":"SANE_FRAME_RGB");
		SaneWriteLog(("last_frame %d"), params->last_frame);
		SaneWriteLog(("bytes_per_line %d"), params->bytes_per_line);
		SaneWriteLog(("pixels_per_line %d"), params->pixels_per_line);
		SaneWriteLog(("lines %d"), params->lines);
		SaneWriteLog(("depth %d"), params->depth);	
	}
    SANE_Status _csderror2saneerror(INT32 ret)
    {
        struct {
            INT32 csd;
            SANE_Status sn;
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
        for (long i=0; tbl[i].csd>0; i++) {
            if (tbl[i].csd==ret) return tbl[i].sn;
        }
        return SANE_STATUS_INVAL;
    }
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
    class CScan
    {
    public:
    	CScan(ISaneCsdCore *csdcore, ISaneOptions *options);
    	~CScan();
    	SANE_Status start();
    	SANE_Status get_parameters(SANE_Parameters * params);
    	SANE_Status read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
    private:
    	ISaneCsdCore *m_csdcore;
    	ISaneOptions *m_options;
    	CEIIMAGEINFO2 m_img;
    	unsigned char *m_cur;
    	SANE_Int  m_last;
    	bool m_first;
    	bool m_outofdocuments;
    	SANE_Status csderror2saneerror(INT32 ret);
    };
	CScan::CScan(ISaneCsdCore *csdcore, ISaneOptions *options) :  m_csdcore(csdcore), m_options(options), m_cur(NULL), m_last(0), m_first(true), m_outofdocuments(false)
	{
		memset(&m_img, 0, sizeof(m_img));
		m_img.cbSize = sizeof(m_img);
	}
	CScan::~CScan()
	{
		if (m_img.lpImage) {
			m_csdcore->CsdReleaseImage(&m_img);
			m_img.lpImage=NULL;
		}		
		m_csdcore->CsdStopScan();
		m_csdcore->CsdFlashScannedImage();
		m_options->scan_end();
	}
	SANE_Status CScan::csderror2saneerror(INT32 ret)
	{
		long err = m_options->csderror2saneerror((long)ret);
		if (err>0) return (SANE_Status)err;
		return _csderror2saneerror(ret);
	}
   	SANE_Status CScan::start()
   	{
   		//SaneWriteLog(("CScan::start() start"));
		INT32 ret = CSD_OK;
		if (m_first) {
			m_options->scan_start();
			ret = m_csdcore->CsdStartScan();
			if (ret) {
				SaneWriteLog(("ERROR:m_csdcore->CsdStartScan() error %ld"), ret);
				return csderror2saneerror(ret);
			}
			m_first=false;
		}

		while (1) {
			if (m_img.lpImage) {
				m_csdcore->CsdReleaseImage(&m_img);
				m_img.lpImage=NULL;
			}
			
			ret = m_csdcore->CsdReadPage(&m_img);
			if (ret) {
				if (ret==CSD_NOPAGE || ret==CSD_NOPAPER) {
					m_outofdocuments=true;
					SaneWriteLog(("m_csdcore->CsdReadPgae return no paper"));
					return SANE_STATUS_NO_DOCS;
				}
				SaneWriteLog(("m_csdcore->CsdReadPgae error 0x%ld"), ret);
				m_options->on_error(ret);
				return csderror2saneerror(ret);
			}

			SaneWriteLog(("width:%d"), m_img.lWidth);
			SaneWriteLog(("height:%d"), m_img.lHeight);
			SaneWriteLog(("sync:%d"), m_img.lSync);
			SaneWriteLog(("bps:%d"), m_img.lBps);
			SaneWriteLog(("spp:%d"), m_img.lSpp);
			SaneWriteLog(("xdpi:%d"), m_img.lXResolution);
			SaneWriteLog(("ydpi:%d"), m_img.lYResolution);
			SaneWriteLog(("size:%d"), m_img.tImageSize);

			m_options->image_process(&m_img);

			if (m_img.lpImage) break;
		}

		m_last = (SANE_Int)m_img.tImageSize;
		m_cur  = m_img.lpImage;

		//SaneWriteLog(("CScan::start() end"));
		return SANE_STATUS_GOOD;
   	}
   	SANE_Status CScan::get_parameters(SANE_Parameters * params)
   	{
		//SaneWriteLog(("CScan::sane_get_parameters() start"));

		if (m_img.lpImage) {
			params->format=mode2frame(m_img.lSpp, m_img.lBps);
			params->last_frame=1;
			params->bytes_per_line=m_img.lSync;
			params->pixels_per_line=m_img.lWidth;
			params->lines=m_img.lHeight;
			params->depth=m_img.lBps;
		} else {
			SaneWriteLog("m_img.lpImage is NULL");
			INT32 width=2480, length=3508, spp=3, bps=8, sync=width;
			m_csdcore->CsdParGet(CSDP_IMAGEWIDTH, &width);
			m_csdcore->CsdParGet(CSDP_IMAGELENGTH, &length);
			m_csdcore->CsdParGet(CSDP_SAMPLESPERPIXEL, &spp);
			m_csdcore->CsdParGet(CSDP_BITSPERSAMPLE, &bps);
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
		
		//SaneWriteLog(("CScan::sane_get_parameters() end"));
		return SANE_STATUS_GOOD;
   	}
	SANE_Status CScan::read(SANE_Byte * data, SANE_Int max_length, SANE_Int *length)
	{
		SaneWriteLog(("CScan::sane_read(%x, %d, %x) start"), data, max_length, length);
		if (m_outofdocuments) return SANE_STATUS_NO_DOCS;
		if (m_cur) {
			if (m_last<=0) {
				SaneWriteLog(("EOF"));
				m_cur=NULL;
				m_last=0;
				*length=0;
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
		SaneWriteLog(("CScan::sane_read() end: *length is %d"), length?*length:-1);
		return SANE_STATUS_GOOD;
	}
}
class CSaneCtrl : public ICeiSane
{
public:
	CSaneCtrl();
	virtual ~CSaneCtrl();
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
public:
	long init(const char *devicename, const char *scanner_name/*DR-M260*/, const char *lib_path/*/opt/Canon/drm260*/);
	void uninit();	
private:
	long m_ref;
	XInterface<ISaneCsdCore>m_csdcore;
	XInterface<ISaneOptions> m_options;
	std::unique_ptr<CScan> m_scan;
};
CSaneCtrl::CSaneCtrl():m_ref(1)
{
}
CSaneCtrl::~CSaneCtrl()
{
	uninit();
}
long CSaneCtrl::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CSaneCtrl::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CSaneCtrl::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return  0;
	}
	return m_ref;
}
const SANE_Option_Descriptor *CSaneCtrl::get_option_descriptor(SANE_Int option)
{
	const SANE_Option_Descriptor *out = m_options->get_option_descriptor(option);
	SaneWriteLog_descripter(out);
	return out;
}
SANE_Status CSaneCtrl::control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int *info)
{
	return m_options->control_option(option, action, value, info);
}
SANE_Status CSaneCtrl::get_parameters(SANE_Parameters * params)
{
	//SaneWriteLog(("sane_get_parameters() start"));
	if (m_scan.get()) {
		m_scan->get_parameters(params);
	} else {
		//SaneWriteLog(("sane_get_parameters(normal) start"));
		INT32 width=2480, length=3508, spp=3, bps=8, sync=width;
		m_csdcore->CsdParGet(CSDP_IMAGEWIDTH, &width);
		m_csdcore->CsdParGet(CSDP_IMAGELENGTH, &length);
		m_csdcore->CsdParGet(CSDP_SAMPLESPERPIXEL, &spp);
		m_csdcore->CsdParGet(CSDP_BITSPERSAMPLE, &bps);
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
	SaneWriteLog_Params(params);
	//SaneWriteLog(("sane_get_parameters() end"));
	return SANE_STATUS_GOOD;
}
SANE_Status CSaneCtrl::start()
{
	//SaneWriteLog(("CSaneCtrl::sane_start() start"));
	if (m_scan.get()==NULL) { 
		m_scan.reset(new CScan(m_csdcore.get(), m_options.get()));
		if (m_scan.get()==NULL) {
			SaneWriteLog("ERROR:m_scan is NULL");
			return SANE_STATUS_NO_MEM;
		}
	}
	SANE_Status ret = m_scan->start();
	//SaneWriteLog(("CSaneCtrl::sane_start() end %d"), ret);
	return ret;
}
SANE_Status CSaneCtrl::read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length)
{
	SaneWriteLog(("CSaneCtrl::sane_read() start"));
	if (m_scan.get()==NULL) return SANE_STATUS_NO_MEM;
	SANE_Status status = m_scan->read(data, max_length, length);
	SaneWriteLog(("CSaneCtrl::sane_read() end"));
	return status;
}
void CSaneCtrl::cancel()
{
	SaneWriteLog(("CSaneCtrl::sane_cancel() start"));
	m_scan.reset(NULL);
	SaneWriteLog(("CSaneCtrl::sane_cancel() end"));	
}
SANE_Status CSaneCtrl::set_io_mode(SANE_Bool non_blocking)
{
    SaneWriteLog("CSaneCtrl::set_io_mode(%s) start", non_blocking?"true":"false");
    if (non_blocking != SANE_FALSE) {
    	 SaneWriteLog("CSaneCtrl::set_io_mode() end SANE_STATUS_UNSUPPORTED");
        return SANE_STATUS_UNSUPPORTED;
    }   
    SaneWriteLog("CSaneCtrl::set_io_mode() end SANE_STATUS_GOOD");
    return SANE_STATUS_GOOD;
}
SANE_Status CSaneCtrl::get_select_fd(SANE_Int *)/* (SANE_Int *f)d */
{
    SaneWriteLog("CSaneCtrl::get_select_fd() start");
    SaneWriteLog("CSaneCtrl::get_select_fd() end");
    return SANE_STATUS_UNSUPPORTED;
}
long CSaneCtrl::init(const char *dev, const char *scanner_name/*DR-M260*/, const char *lib_path/*/opt/Canon/drm260*/)
{
	SaneWriteLog("CSaneCtrl::init(%s, %s ,%s) start", dev, scanner_name, lib_path);
	INIT_INFORMATION iinfo={sizeof(iinfo)};
	iinfo.szProductName = scanner_name;
	iinfo.szLibraryFilePath = lib_path;
	PROBE_INFORMATION pbinfo={sizeof(pbinfo)};
	pbinfo.szProductName = iinfo.szProductName;
	m_csdcore.reset(create_csdcore_for_sane(dev, &iinfo, &pbinfo));
	if (m_csdcore.get()==NULL) {
		SaneWriteLog("m_csdcore is NULL");
		return -1;
	}
	m_options.reset(create_options(m_csdcore.get()));
	if (m_options.get()==NULL) {
		SaneWriteLog("m_options is NULL");
		return -1;
	}
	SaneWriteLog("CSaneCtrl::init() end");
	return 0;
}
void CSaneCtrl::uninit()
{
	SaneWriteLog("CSaneCtrl::uninit() start");
	m_scan.reset(NULL);
	m_csdcore.reset(NULL);
	SaneWriteLog("CSaneCtrl::uninit() end");
}

/////////////////////////////////////////////////////////////////////////
//
ICeiSane *create_sane_ctrl(const char *devicename, const char *scanner_name/*DR-M260*/, const char *lib_path/*/opt/Canon/drm260*/)
{
	ICeiSane *out = NULL;
	if (strcmp(devicename, "simulation")==0) {
		out = (ICeiSane*)create_simulation_sane_ctrl();
	} else {
		CSaneCtrl *p = new CSaneCtrl;
		if (p==NULL) return NULL;
		sanesdk::ceisdk_set_scanner_name((char*)scanner_name);
    	sanesdk::ceisdk_set_library_path((char*)lib_path);
		long ret = p->init(devicename, scanner_name, lib_path);
		if (ret) {
			delete p;
			return NULL;
		}
		out = (ICeiSane*)p;
	}
	return out;
}