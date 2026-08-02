/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <vector>
#include <memory>
#include <string.h>
#include "ceilogwrite.h"
#include "vssdk.h"
#include "ipsdk.h"
#include "command.h"
#include "sdk_image_util.h"
#include "sdk_information_util.h"
#include "sense2vs_error.h"
#include "message_queue_interface.h"
#include "end_sequence_interface.h"
#include "settings_framework.h"
#include "global_apis.h"
void shading_front(ICeiImage* pin);
void shading_back(ICeiImage* pin);
void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window);
void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window);
void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window);
void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window);
long get_scanner_specific_offset(long dpi)
{
	return 8670 * dpi / 24500;
}
namespace {
	class CImageWrapper : public ICeiImage
	{
	public:
		CImageWrapper(ICeiImage* p) : m_p(p), m_w(0), m_h(0), m_sync(0)
		{
			m_w = p->width();
			m_h = p->height();
			m_sync = p->sync();
		}
		long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut) { return -1; }
		unsigned long STDMETHODCALLTYPE AddRef() { return 1; }
		unsigned long STDMETHODCALLTYPE Release() {	return 1;}
		char* img() { return m_p->img(); }
		long width() { return m_w; }
		long height() { return m_h; }
		long xdpi() { return m_p->xdpi(); }
		long ydpi() { return m_p->ydpi(); }
		long spp() { return m_p->spp(); }
		long bps() { return m_p->bps(); }
		long sync() { return m_sync; }
		long size() { return sync()*height(); }
		long comptype() { return m_p->comptype(); }
		long compinfo() { return m_p->compinfo(); }
		long rgb_order() { return 1; }
		void width(long w) { m_w = w; }
		void height(long h) { m_h = h; }
		void sync(long s) { m_sync = s; }
	private:
		ICeiImage* m_p;
		long m_w, m_h, m_sync;
	};
	inline long range255to7(long v0_255)
	{
		long out = 3;
		long th = 36;
		if (v0_255 <= th) {
			out = 1;
		}
		else if (v0_255 <= th * 2) {
			out = 2;
		}
		else if (v0_255 <= th * 3) {
			out = 3;
		}
		else if (v0_255 <= th * 4) {
			out = 4;
		}
		else if (v0_255 <= th * 5) {
			out = 5;
		}
		else if (v0_255 <= th * 6) {
			out = 6;
		}
		else if (v0_255 < 256) {
			out = 7;
		}
		//WriteLog("%d -> %d", v0_255, out);
		return out;
	}
	void clear(std::vector<CStreamCmd*>& stream_list)
	{
		std::vector<CStreamCmd*>::iterator itr = stream_list.begin();
		for (; itr != stream_list.end(); itr++) {
			if (*itr) delete (*itr);
		}
		stream_list.clear();
	}
	inline void WriteLog_image(ICeiImage* pimg, long bback)
	{
		WriteLog("//////////%s////////////", bback ? "back" : "front");
		WriteLog("width:%d", pimg->width());
		WriteLog("length:%d", pimg->height());
		WriteLog("spp:%d", pimg->spp());
		WriteLog("bps:%d", pimg->bps());
		WriteLog("xdpi:%d", pimg->xdpi());
		WriteLog("ydpi:%d", pimg->ydpi());
		WriteLog("sync:%d", pimg->sync());
		WriteLog("comptype:%d", pimg->comptype());
		WriteLog("compinfo:%d", pimg->compinfo());
		WriteLog("");
	}
}
class CFrameworkEndSequence : public IEndSequence
{
public:
	CFrameworkEndSequence(IMessageQueue *q, CSettingsFramework *p);
	virtual ~CFrameworkEndSequence();
	void on_batch_start(ICeiMessage *pmsg);
	void on_page_start(ICeiMessage *pmsg);
	void on_image_start(ICeiMessage *pmsg);
	void on_image(ICeiMessage *pmsg);
	void on_image_end(ICeiMessage *pmsg);
	void on_info_start(ICeiMessage *pmsg);
	void on_info(ICeiMessage *pmsg);
	void on_info_end(ICeiMessage *pmsg);
	void on_page_end(ICeiMessage *pmsg);
		void on_page_end_simplex();
		void on_page_end_duplex();
			void on_page_end_duplex_main();
			void on_page_end_duplex_end();
	void on_error(ICeiMessage *pmsg);
	void on_batch_end(ICeiMessage *pmsg);
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *pout);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();	
private:
	void cutoffset(ICeiImage** ppInOut);
	void detect_4points(ICeiImage* pIn, CEISDK_POINT* pos, long back);
	void autosize_deskew(ICeiImage** pInOut, CEISDK_POINT* pos);
	void cutout(ICeiImage** ppOut);
	void gamma(ICeiImage* pIn);
	void gray2binary(ICeiImage** ppInOut);
	void resolution_convert(ICeiImage **ppInOut);
	void image_process(ICeiImage** pimg, long back);
private:
	enum {
		FRONT=0,
		BACK
	};
	long m_ref;
	bool m_isFront;
	IMessageQueue *m_pprev;
	long m_error;
	CSettingsFramework *m_psettings;
private:
	typedef std::vector<CStreamCmd*> STREAMLIST;
	STREAMLIST m_imgPieces;
	XInterface<ICeiImage>m_image[2];
	std::unique_ptr<CCeiImageInformationCmd>m_information[2];
};
CFrameworkEndSequence::CFrameworkEndSequence(IMessageQueue *q, CSettingsFramework *p):m_ref(1), m_isFront(true), m_pprev(q), m_error(VS3_OK), m_psettings(p)
{
}
CFrameworkEndSequence::~CFrameworkEndSequence()
{
	clear(m_imgPieces);
}
long CFrameworkEndSequence::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CFrameworkEndSequence::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CFrameworkEndSequence::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CFrameworkEndSequence::on_batch_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_batch_start()");
}
void CFrameworkEndSequence::on_page_start(ICeiMessage* pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_page_start()");
	long back = 0;
	pmsg->get((void**)&back);
	if (back) {

	}
	else {
		clear(m_imgPieces);
		m_information[FRONT].reset(create_vscsdsdk_information_cmd());
		m_information[BACK].reset(create_vscsdsdk_information_cmd());
	}
}
void CFrameworkEndSequence::on_image_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_image_start()");
}
void CFrameworkEndSequence::on_image(ICeiMessage *pmsg)
{
	CStreamCmd *p = NULL;
	pmsg->get((void**)&p, true);
	if ( p != NULL) { 
		m_imgPieces.push_back(p); 
	}
}
void CFrameworkEndSequence::on_image_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_image_end()");
}
void CFrameworkEndSequence::on_info_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_info_start()");
}
void CFrameworkEndSequence::on_info(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_info()");
	CStreamCmd *p=NULL;
	pmsg->get((void**)&p, true);
	m_information[FRONT]->set((CStreamCmd*)p->clone());
	m_information[BACK]->set(p);
}
void CFrameworkEndSequence::on_info_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_info_end()");
}
void CFrameworkEndSequence::gray2binary(ICeiImage** ppInOut)
{
	CWindow w;
	w = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	if (w.bps() == 1) {
		gray2binary_internal2(ppInOut, w.brightness());
	}
}
void CFrameworkEndSequence::resolution_convert(ICeiImage **ppInOut)
{
	ICeiImage* pimg = *ppInOut;
	CWindow w = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	long dst_w = pimg->width() * w.xdpi() / pimg->xdpi();
	long dst_h = pimg->height() * w.ydpi() / pimg->ydpi();
	resolution_convert_internal(ppInOut, dst_w, dst_h, w.xdpi());
}
void CFrameworkEndSequence::gamma(ICeiImage* pIn)
{	
	CWindow w;
	w = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	ICeiImage* img = pIn;
	if (w.spp()==3 && img->spp() == 3) {
		ceisdk_gamma(img, ceisdk_get_gamma_table_color, w.brightness(), range255to7(w.contrast()));
	}
	else if (w.bps()==8 && img->bps() == 8) {
		ceisdk_gamma(img, ceisdk_get_gamma_table_gray, w.brightness(), range255to7(w.contrast()));
	}
	else if (w.bps() == 1 && img->bps() == 8) {
		ceisdk_gamma(img, ceisdk_get_gamma_table_bw, w.brightness(), range255to7(w.contrast()));
	}	
}
void CFrameworkEndSequence::cutout(ICeiImage** ppOut)
{
	CScanParam sp = m_psettings->scanParam_bothr();
	if (sp.autosize()) return;
	if (sp.deskew()) return;
	CWindow window = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	CMode mode = m_psettings->mode();
	ceisdk_cutout_simple(ppOut,
		window.xoffset() * window.xdpi() / mode.mud(),
		window.yoffset() * window.ydpi() / mode.mud(),
		window.width() * window.xdpi() / mode.mud(),
		window.length() * window.ydpi() / mode.mud());
}
void CFrameworkEndSequence::cutoffset(ICeiImage** ppInOut)
{
	CScanParam sp = m_psettings->scanParam_bothr();
	if (sp.autosize() || sp.deskew()) {

	}
	else {
		ICeiImage* pimg = *ppInOut;
		ceisdk_cut_offset_simple(ppInOut, get_scanner_specific_offset(pimg->ydpi()));
	}
}
void CFrameworkEndSequence::detect_4points(ICeiImage* pIn, CEISDK_POINT* pos, long back)
{
	CScanParam sp = m_psettings->scanParam_bothr();
	if (sp.autosize() || sp.deskew()) {
		if (pIn->spp() == 3) {
			CImageWrapper iw(pIn);
			iw.sync(pIn->sync() * 3);
			if (back) ceisdk_detect_4points_simple_back(&iw, pos);
			else      ceisdk_detect_4points_simple_front(&iw, pos);
		}
		else {
			if (back) ceisdk_detect_4points_simple_back(pIn, pos);
			else      ceisdk_detect_4points_simple_front(pIn, pos);
		}
	}
	else {
		memset(pos, 0, sizeof(CEISDK_POINT*) * 4);
	}
}
void CFrameworkEndSequence::autosize_deskew(ICeiImage** ppOut, CEISDK_POINT* pos)
{
	CScanParam sp = m_psettings->scanParam_bothr();
	if (sp.autosize() && sp.deskew()) {
		ceisdk_autosize_deskew_simple(ppOut, pos);
	}
	else if (sp.autosize()) {
		ceisdk_autosize_simple(ppOut, pos);
	}
	else if (sp.deskew()) {
		ceisdk_deskew_simple(*ppOut, pos);
	}
}
void CFrameworkEndSequence::image_process(ICeiImage** ppInOut, long back)
{
	resolution_convert(ppInOut);
	cutout(ppInOut);
	gamma(*ppInOut);
	gray2binary(ppInOut);
}
void CFrameworkEndSequence::on_page_end_simplex()
{
	CEISDK_POINT pos[4] = { 0 };
	CWindow window;
	CMode mode;
	CScanParam scanparamr = m_psettings->scanParam_bothr();
	window = m_psettings->window(CSettingsFramework::TO_SCANNER);
	CWindow window_from_client = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	window.length(window_from_client.length());
	mode = m_psettings->mode();
	ICeiImage* pimg = NULL;
	if (scanparamr.autosize()) toImage_autosize(m_imgPieces, &pimg, window);
	else                       toImage_fixedsize(m_imgPieces, &pimg, window);
	detect_4points(pimg, pos, FRONT);
	shading_front(pimg);
	ceisdk_to_pixelorder_simple(&pimg);
	autosize_deskew(&pimg, pos);
	image_process(&pimg, FRONT);
	m_image[FRONT].reset(pimg);
}
void CFrameworkEndSequence::on_page_end_duplex()
{
	on_page_end_duplex_main();
	on_page_end_duplex_end();
}
void CFrameworkEndSequence::on_page_end_duplex_main()
{
	CEISDK_POINT pos[4 + 4] = { 0 };
	CWindow window[2];
	CScanParam scanparamr = m_psettings->scanParam_bothr();
	window[0] = m_psettings->window(CSettingsFramework::TO_SCANNER);
	window[1] = m_psettings->window(CSettingsFramework::FROM_CLIENT);
	window[0].length(window[1].length() + get_scanner_specific_offset(1200));
	ICeiImage* front = NULL;
	ICeiImage* back = NULL;
	if (scanparamr.autosize()) toImage_autosize(m_imgPieces, &front, &back, window[0]);
	else                       toImage_fixedsize(m_imgPieces, &front, &back, window[0]);
	cutoffset(&front);
	detect_4points(front, pos, FRONT);
	shading_front(front);	
	ceisdk_mirror_lineorder(back);
	detect_4points(back, pos+4, BACK);
	shading_back(back);
	ceisdk_to_pixelorder_simple(&front);
	ceisdk_to_pixelorder_simple(&back);
	autosize_deskew(&front, pos);
	autosize_deskew(&back, pos+4);
	image_process(&front, FRONT);
	image_process(&back, BACK);
	m_image[FRONT].reset(front);
	m_image[BACK].reset(back);
}
void CFrameworkEndSequence::on_page_end_duplex_end()
{
	CScanCmd scancmd_client = m_psettings->scancmd(CSettingsFramework::FROM_CLIENT);
	if (scancmd_client.duplex()) {

	}
	else {
		if (scancmd_client.data()[0]) {
			//simplex(back)
			m_image[FRONT] = NULL;
			m_information[FRONT] = NULL;
		}
		else {
			//simplex(front)
			m_image[BACK] = NULL;
			m_information[BACK] = NULL;
		}
	}
}
void CFrameworkEndSequence::on_page_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_page_end() start");
	long back = 0;
	pmsg->get((void**)&back);
	if (back) return;
	try {
		CScanCmd scancmd_to_scanner = m_psettings->scancmd(CSettingsFramework::TO_SCANNER);
		if (scancmd_to_scanner.duplex()) {
			on_page_end_duplex();
		}
		else {
			on_page_end_simplex();
		}

	}
	catch (std::bad_alloc&) {
		m_error = VS3_NOMEM;
		WriteLog((char*)"CFrameworkEndSequence::on_page_end() end (mem error)");
		return;
	}
	catch(...)
	{
		m_error = 100;
		WriteLog((char*)"CFrameworkEndSequence::on_page_end() end (error)");
		return;
	}
	WriteLog((char*)"CFrameworkEndSequence::on_page_end() end");
	return;
}
void CFrameworkEndSequence::on_error(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_error()");
	CSenseCmd *psns = NULL;
	pmsg->get((void**)&psns);
	if (psns) {
		m_error = sense2vs3_error(*psns);
		WriteLog((char*)"m_error %d", m_error);
	}
}
void CFrameworkEndSequence::on_batch_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"CFrameworkEndSequence::on_batch_end()");
}
long CFrameworkEndSequence::get_image(ICeiImage **ppOut)
{
	WriteLog("CFrameworkEndSequence::get_image() start");
	long ret = VS3_OK;
	if (m_image[FRONT].get()) {
		*ppOut = m_image[FRONT].Detach();
	}
	else if (m_image[BACK].get()) {
		*ppOut = m_image[BACK].Detach();
	}
	else {
		ret = m_error;
	}
	WriteLog((char*)"CFrameworkEndSequence::get_image() end %d", ret);
	return ret;
}
long CFrameworkEndSequence::get_information(long , void *pout)
{
	WriteLog((char*)"CFrameworkEndSequence::get_information() start");
	ICeiImageInformation ** ppOut = (ICeiImageInformation **)pout;
	long ret = VS3_OK;///m_error

	if (m_information[FRONT].get()) {
		*ppOut = m_information[FRONT].release();
	}
	else if (m_information[BACK].get()) {
		*ppOut = m_information[BACK].release();
	}
	else {
		//ret = m_error;
	}
	WriteLog((char*)"CFrameworkEndSequence::get_information() end");
	return ret;
}
IEndSequence *end_sequence(IMessageQueue *q, IUnknown *handle)
{
	return new CFrameworkEndSequence(q, (CSettingsFramework *)handle);
}
