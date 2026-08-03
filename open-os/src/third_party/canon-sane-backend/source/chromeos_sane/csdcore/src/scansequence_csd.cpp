/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <string>
#include "ceilogwrite.h"
#include "sdk_message.h"
#include "sdk_command_util.h"
#include "csderr.h"
#include "vserr.h"
#include "message_queue_interface.h"
#include "scan_sequence_interface.h"
#include "tags_interface.h"
#include "settings_csd.h"
#include "global_apis.h"
#include "csdtags.h"
#include "ipsdk.h"
#include "vs_error2csd_error.h"
#include "csdtags.org.h"
namespace {
	ICeiMessage::MESSAGE_TYPE print_msg(ICeiMessage::MESSAGE_TYPE type)
	{
		switch (type) {
		case ICeiMessage::MID_BATCH_START:WriteLog((char*)"MID_BATCH_START");break;
		case ICeiMessage::MID_ERROR:WriteLog((char*)"MID_ERROR");break;
		case ICeiMessage::MID_PAGE_START:WriteLog((char*)"MID_PAGE_START");break;
		case ICeiMessage::MID_IMAGE_START:WriteLog((char*)"MID_IMAGE_START");break;
		case ICeiMessage::MID_IMAGE:WriteLog((char*)"MID_IMAGE");break;
		case ICeiMessage::MID_IMAGE_END:WriteLog((char*)"MID_IMAGE_END");break;
		case ICeiMessage::MID_INFO_START:WriteLog((char*)"MID_INFO_START");break;
		case ICeiMessage::MID_INFO:WriteLog((char*)"MID_INFO");break;
		case ICeiMessage::MID_INFO_END:WriteLog((char*)"MID_INFO_END");break;
		case ICeiMessage::MID_PAGE_END:WriteLog((char*)"MID_PAGE_END");break;
		case ICeiMessage::MID_BATCH_END:WriteLog((char*)"MID_BATCH_END");break;
		default:WriteLog((char*)"MID_UNKNOWN");break;
		}
		return type;
	}
	IUnknown * print_img(ICeiImage *pimg)
	{	
		WriteLog("width:%ld", pimg->width());
		WriteLog("height:%ld", pimg->height());
		WriteLog("xdpi:%ld", pimg->xdpi());
		WriteLog("ydpi:%ld", pimg->ydpi());
		WriteLog("spp:%ld", pimg->spp());
		WriteLog("size:%ld", pimg->size());
		if (pimg->comptype()) {
			WriteLog("comp:jpeg");
		} else {
			WriteLog("comp:none");
			WriteLog("sync:%ld", pimg->sync());
		}
		return (IUnknown*)pimg;
	}
}
class CScanSequenceCsd : public IScanSequence
{
public:
	CScanSequenceCsd(IMessageQueue *q, ICsdTags *t, IScannedImageCtrl *psic, IVirtualScanner *s, CSettingsCsd *h);
	virtual ~CScanSequenceCsd();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	void main(IScanSequenceCallback *pcallback);
private:
	void simplex_loop(IScanSequenceCallback* pcallback, long bfront);
	void duplex_loop(IScanSequenceCallback* pcallback);
private:
	void to_scanner();
	void to_scanner_vstag();
	void to_scanner_scsi();
	void make_scan_cmd(CScanCmd& cmd, long long& bfront);
	long getlong(int id, long def = 0);
	std::string getascii(int id, const char *def="");
	long getlongmax(int id, long def=0);
	long setlong(int id, long v);
private:
	bool duplex();
	bool simplex_front();
	bool simplex_back();
	long xoffset_1200dpi();
	long yoffset_1200dpi();
	long width_1200dpi();
	long length_1200dpi();
	long quantize_dpi(long dpi);
	long xdpi();
	long ydpi();
	long spp();
	long bps();
	long brightness();
	long contrast();
	long maximum_paper_length();
private:
	long m_ref;
	IMessageQueue *m_pnext;
	IScanSequenceCallback *m_pcallback;
	CSettingsCsd *m_psettings;
	IVirtualScanner *m_pscanner;
	ICsdTags *m_ptags;
	IScannedImageCtrl*m_psic;
private:
	CInquiryCmd m_inqex;
	CMode m_mode;
};
CScanSequenceCsd::CScanSequenceCsd(IMessageQueue *q, ICsdTags *t, IScannedImageCtrl *psic, IVirtualScanner *s, CSettingsCsd *h):
m_ref(1), 
m_pnext(q), 
m_pcallback(NULL), 
m_psettings(h), 
m_pscanner(s),
m_ptags(t),
m_psic(psic)
{
	CScsiVSCommand scanner(m_pscanner);
	m_inqex.evpd(true);
	scanner.read(m_inqex);
	scanner.read(m_mode);
}
CScanSequenceCsd::~CScanSequenceCsd()
{
}
long CScanSequenceCsd::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CScanSequenceCsd::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CScanSequenceCsd::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
std::string CScanSequenceCsd::getascii(int id, const char *def)
{
	char v[256];
	strcpy(v, def);
	m_ptags->get(id, &v);
	return v;	
}
long CScanSequenceCsd::getlongmax(int id, long def)
{
	long v=def;
	m_ptags->choice(id, ICsdTag::RANGE_HIGH, &v);
	return v;
}
long CScanSequenceCsd::getlong(int id, long def)
{
	long v = def;
	m_ptags->get(id, &v);
	return v;
}
long CScanSequenceCsd::setlong(int id, long v)
{
	m_ptags->set(id, v);
	return 0;
}
bool CScanSequenceCsd::duplex()
{
	return getlong(CSDP_FEEDER)?true:false;
}
bool CScanSequenceCsd::simplex_front()
{
  if (duplex()) {
    return false;
  }

	return getlong(CSDP_FEEDER_OPTION)?false:true;
}
bool CScanSequenceCsd::simplex_back()
{
  if (duplex()) {
    return false;
  }
  return getlong(CSDP_FEEDER_OPTION)?true:false;
}
long CScanSequenceCsd::maximum_paper_length()
{
	long out = 355600;
	switch (getlong(CSDP_NORMAL_LONG_PAPER)) {
	default:
	case 0:
	out = ceisdk_get_private_profile_int("SCANNER", "maximum_paper_length(normal)", 355600);
	break;
	case  1:
	out = 1000000; 
	break;
	case 3:
	out = 3000000;
	break;
	case 5:
	out = 5588000;
	break;
	}
	return out;
}
long CScanSequenceCsd::xoffset_1200dpi()
{
	long out_1200dpi=0;
    long paper_width_1200dpi = getlong(CSDP_IMAGEWIDTH1200DPI) ;
	long basic_dpi = m_inqex.basic_xdpi();
	if (!basic_dpi) basic_dpi = 600;
    long sensor_max_width_1200dpi = m_inqex.window_width() * 1200 / basic_dpi;
    out_1200dpi = (sensor_max_width_1200dpi - paper_width_1200dpi) / 2 + getlong(CSDP_XOFFSET1200DPI);
	return out_1200dpi;
}
long CScanSequenceCsd::yoffset_1200dpi()
{
	return getlong(CSDP_YOFFSET1200DPI);
}
long CScanSequenceCsd::width_1200dpi()
{
	return getlong(CSDP_IMAGEWIDTH1200DPI);
}
long CScanSequenceCsd::length_1200dpi()
{
	return getlong(CSDP_IMAGELENGTH1200DPI);
}
long CScanSequenceCsd::quantize_dpi(long dpi)
{
  static const int DPI[] = {100, 150, 200, 240, 300, 400};
  for (unsigned int i = 0; i < sizeof(DPI) / sizeof(int); i++) {
    if (dpi <= DPI[i]) {
      return DPI[i];
    }
  }
  return 600;
}
long CScanSequenceCsd::xdpi()
{
	return getlong(CSDP_XRESOLUTION);
}
long CScanSequenceCsd::ydpi()
{
	return xdpi();
}
long CScanSequenceCsd::spp()
{
	return getlong(CSDP_SAMPLESPERPIXEL);
}
long CScanSequenceCsd::bps()
{
	return getlong(CSDP_BITSPERSAMPLE);
}
long CScanSequenceCsd::brightness()
{
	return getlong(CSDP_BRIGHTNESS);
}
long CScanSequenceCsd::contrast()
{
	return getlong(CSDP_CONTRAST);
}
void CScanSequenceCsd::to_scanner_vstag()
{
}
void CScanSequenceCsd::to_scanner_scsi()
{
	CScsiVSCommand scanner(m_pscanner);
	{
		CScanParam param(CScanParam::PAGE_CODE_SCAN_BOTH);
		scanner.read(param);
		param.maximum_number_of_documents_in_batch_mode(0);
		scanner.write(param);
	}
	{
		CScanParam param(CScanParam::PAGE_CODE_SCAN_BOTH_RESCAN);
		scanner.read(param);
		param.autosize(getlong(CSDP_AUTOSIZE) == 0 ? 0 : 2);
		param.deskew(getlong(CSDP_DESKEW) == 0 ? false : true);
		param.side(false);
		scanner.write(param);
	}
	{
		CScanParam param(CScanParam::PAGE_CODE_OPTION);
		scanner.read(param);
		param.double_feed_detection_length(getlong(CSDP_DBLFEEDLENGTH));
		param.double_feed_detection_ultrasonic(getlong(CSDP_DBLFEEDUSS));
		if (getlong(CSDP_DESKEW)||getlong(CSDP_AUTOSIZE)) {
			param.skew_detection(false);
		} else {
			param.skew_detection(true);
		}
		scanner.write(param);
	}
	{
		CScanParam param(CScanParam::PAGE_CODE_SCAN_SEP_RESCAN);
		scanner.read(param);
		param.side(0);
		scanner.write(param);
		param.side(1);
		scanner.write(param);
	}
	{
		CWindow window;
		scanner.read(window);
		window.yoffset(yoffset_1200dpi());
		window.xoffset(xoffset_1200dpi());
		window.width(width_1200dpi());
		window.length(length_1200dpi());
		window.xdpi((short)quantize_dpi(xdpi()));
		window.ydpi((short)quantize_dpi(ydpi()));
		window.spp((char)spp());
		window.bps((char)bps());
		window.brightness(brightness());
		window.threshold((char)window.brightness());
		window.contrast(contrast());
		window.compression_type(0x0);
		window.compression_argument(90);
		scanner.write(window);
		window.side(true);
		scanner.write(window);
	}
}
void CScanSequenceCsd::to_scanner()
{
	WriteLog("to_scanner() start");
	to_scanner_vstag();
	to_scanner_scsi();
	WriteLog("to_scanner() end");
}
void CScanSequenceCsd::make_scan_cmd(CScanCmd& scn, long long &bfront)
{
	if (duplex()) {
		scn.duplex(true);
		scn.data()[0] = 0;
		scn.data()[1] = 1;
	}
	else {
		scn.duplex(false);
		if (simplex_front()) {
			scn.data()[0] = 0;
			bfront = 1;
		}
		else {
			scn.data()[0] = 1;
			bfront = 0;
		}
	}
}
void CScanSequenceCsd::simplex_loop(IScanSequenceCallback* pcallback, long bfront)
{
	long ret = VS3_OK;
	ICeiImage* pimg = NULL;
	ICeiImageInformation* pimginfo = NULL;
	while (1) {
		if (m_pcallback->stop_request()) {
			WriteLog((char*)"stop");
			m_pscanner->stop();
			break;
		}
		ret = m_pscanner->image(&pimg, &pimginfo);
		if (ret) {
			//WriteLog((char*)"m_pscanner->image() error %ld", ret);
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_ERROR), (long long)vserror2csderr(ret)));
			break;
		}
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_PAGE_START), bfront));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_START), bfront));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE), print_img(pimg)));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_END), bfront));
		if (pimginfo) {
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_START), bfront));
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO), (IUnknown*)pimginfo));
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_END), bfront));
		}
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_PAGE_END), bfront));
	}
}
void CScanSequenceCsd::duplex_loop(IScanSequenceCallback* pcallback)
{
	long ret = VS3_OK;
	ICeiImage* pimg = NULL;
	ICeiImageInformation* pimginfo = NULL;
	while (1) {
		if (m_pcallback->stop_request()) {
			WriteLog((char*)"stop");
			m_pscanner->stop();
			break;
		}
		ret = m_pscanner->image(&pimg, &pimginfo);
		if (ret) {
			//WriteLog((char*)"m_pscanner->image() error %ld", ret);
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_ERROR), (long long)vserror2csderr(ret)));
			break;
		}
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_PAGE_START),(long long)1/*no meaning*/));

		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_START), (long long)1/*front*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE), print_img(pimg)));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_END), (long long)1/*front*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_START), (long long)1/*front*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO), (IUnknown*)pimginfo));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_END), (long long)1/*front*/));
		ret = m_pscanner->image(&pimg, &pimginfo);
		if (ret) {
			//WriteLog((char*)"m_pscanner->image() error %ld", ret);
			m_pnext->push(create_message(print_msg(ICeiMessage::MID_ERROR), (long long)vserror2csderr(ret)));
			break;
		}
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_START), (long long)0/*back*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE), print_img(pimg)));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_IMAGE_END), (long long)0/*back*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_START), (long long)0/*back*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO), (IUnknown*)pimginfo/*back*/));
		m_pnext->push(create_message(print_msg(ICeiMessage::MID_INFO_END), (long long)0/*back*/));

		m_pnext->push(create_message(print_msg(ICeiMessage::MID_PAGE_END), (long long)1/*no meaning*/));
	}
}
void CScanSequenceCsd::main(IScanSequenceCallback *pcallback)
{
	WriteLog((char*)"main() start");
	m_pcallback =  pcallback;
	CScsiVSCommand scanner(m_pscanner);
	to_scanner();
	long long bfront = 1;
	CScanCmd scn;
	make_scan_cmd(scn, bfront);
	scanner.write(scn);
	m_pscanner->scan_start(m_psic);
	if (scn.duplex()) {
		duplex_loop(pcallback);
	}
	else {
		simplex_loop(pcallback, (long)bfront);
	}
	m_pscanner->scan_end();
	WriteLog((char*)"main() end");
	WriteLog((char*)"");
}
IScanSequence *scan_sequence(IMessageQueue *q, IUnknown *scanner, IUnknown *handle, IUnknown *ptags, IUnknown *psic)
{
	CSettingsCsd*s = (CSettingsCsd*)handle;
	s->tags((ICsdTags*)ptags);
	return new CScanSequenceCsd(q, (ICsdTags*)ptags, (IScannedImageCtrl*)psic, (IVirtualScanner*)scanner, s);
}
