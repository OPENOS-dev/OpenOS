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
namespace {
	void Set2BYTE(unsigned char* pData, int nIndex, short wData)
	{
		pData[nIndex] = (char)((wData) >> 8);
		pData[nIndex + 1] = (char)((wData));
	}
	long vs_invert_black_and_white(ICeiImage* pin)
	{
		if (pin->bps() == 1 && pin->spp() == 1) {
			char* p = pin->img();
			for (long h = 0; h < pin->height(); h++) {
				for (long w = 0; w < pin->sync(); w++) {
					p[w] = ~p[w];
				}
				p += pin->sync();
			}
		}
		return 0;
	}
}
class CFrameworkEndSequence : public IEndSequence
{
public:
	CFrameworkEndSequence(IMessageQueue *q, CSettingsFramework *p);
	virtual ~CFrameworkEndSequence();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();

	void on_batch_start(ICeiMessage *pmsg);
	void on_page_start(ICeiMessage *pmsg);
	void on_image_start(ICeiMessage *pmsg);
	void on_image(ICeiMessage *pmsg);
	void on_image_end(ICeiMessage *pmsg);
	void on_info_start(ICeiMessage *pmsg);
	void on_info(ICeiMessage *pmsg);
	void on_info_end(ICeiMessage *pmsg);
	void on_page_end(ICeiMessage *pmsg);
	void on_error(ICeiMessage *pmsg);
	void on_batch_end(ICeiMessage *pmsg);
	long get_image(ICeiImage **ppOut);
	long get_information(long id, void *pout);
private:
	void make_image(ICeiImage** ppOut, long back);
	void jpeg_decomp(ICeiImage** ppInOut);
	void invert(ICeiImage* pIn);
	void change_sync(ICeiImage** ppfront, ICeiImage** ppback);
	long total_size(long back);
private:
	long m_ref;
	IMessageQueue *m_pprev;
	long m_error;
	CSettingsFramework *m_psettings;
private:
	long m_back;
	enum {
		FRONT = 0,
		BACK,
		FRONT_BACK
	};
	std::vector<CStreamCmd*> m_array[FRONT_BACK];
	XInterface<ICeiImage>m_image[FRONT_BACK];
	std::unique_ptr<CCeiImageInformationCmd>m_information[FRONT_BACK];
};
CFrameworkEndSequence::CFrameworkEndSequence(IMessageQueue *q, CSettingsFramework *p):m_ref(1), m_pprev(q), m_error(VS3_OK), m_psettings(p), m_back(0)
{
}
CFrameworkEndSequence::~CFrameworkEndSequence()
{
	for (long i = 0; i < FRONT_BACK; i++) {
		std::vector<CStreamCmd*>::iterator itr = m_array[i].begin();
		for (; itr != m_array[i].end(); itr++) {
			delete* itr;
		}
		m_array[i].clear();
	}
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
	WriteLog((char*)"on_batch_start() start");
	WriteLog((char*)"on_batch_start() end");
}
void CFrameworkEndSequence::on_page_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_page_start() start");
    pmsg->get((void**)&m_back);
	WriteLog((char*)"on_page_start() end");
}
void CFrameworkEndSequence::on_image_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_image_start() start");
	pmsg->get((void**)&m_back);
	WriteLog((char*)"on_image_start() end");
}
void CFrameworkEndSequence::on_image(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_image() start");
	CStreamCmd *p=NULL;
	pmsg->get((void**)&p, true);
	m_array[m_back].push_back(p);
	WriteLog((char*)"on_image() end");
}
void CFrameworkEndSequence::on_image_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_image_end() start");
	WriteLog((char*)"on_image_end() end");
}
void CFrameworkEndSequence::on_info_start(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_info_start() start");
	pmsg->get((void**)&m_back);
	m_information[m_back].reset(create_vscsdsdk_information_cmd());
	WriteLog((char*)"on_info_start() end");
}
void CFrameworkEndSequence::on_info(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_info() start");
	CStreamCmd *p=NULL;
	pmsg->get((void**)&p, true);
	m_information[m_back]->set(p);
	WriteLog((char*)"on_info() end");
}
void CFrameworkEndSequence::on_info_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_info_end() start");
	WriteLog((char*)"on_info_end() end");
}
void CFrameworkEndSequence::on_page_end(ICeiMessage* pmsg)
{
	WriteLog((char*)"on_page_end() start");
	try {
		ICeiImage* pimg = NULL;
		make_image(&pimg, m_back);
		jpeg_decomp(&pimg);
		ceisdk_change_sync((ICeiImage**)&pimg, 1);
		invert(pimg);
		m_image[m_back].reset(pimg);
	}
	catch (std::bad_alloc&) {
		m_error = VS3_NOMEM;
		WriteLog((char*)"CFrameworkEndSequence::on_page_end() end (mem error)");
		return;
	}
	catch (...)
	{
		m_error = 100;
		WriteLog((char*)"CFrameworkEndSequence::on_page_end() end (error)");
		return;
	}
	WriteLog((char*)"on_page_end() end");
}
long CFrameworkEndSequence::total_size(long back)
{
	long total = 0;
	for (const auto& ele : m_array[back]) {
		total += ele->transfer_length();
	}
	return total;
}
void CFrameworkEndSequence::make_image(ICeiImage** ppOut, long back)
{
	WriteLog("CFrameworkEndSequecne::make_image() start");
	
	CWindow& w_to_scanner = m_psettings->window_to_scanner(back);
	CMode& mode = m_psettings->mode();
	CStreamCmd s(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_PAPERAREA);
	m_information[back]->information(0, &s);
	long w = s.autosize_width() * w_to_scanner.dpi() / mode.mud();
	long h = s.autosize_length() * w_to_scanner.dpi() / mode.mud();
	
	CVSCSDSDKImage* pimg = create_vscsdsdk_image();
	pimg->width(w);
	pimg->height(h);
	pimg->spp(w_to_scanner.spp());
	pimg->bps(w_to_scanner.bps());
	pimg->xdpi(w_to_scanner.dpi());
	pimg->ydpi(w_to_scanner.dpi());
	switch (pimg->spp() * pimg->bps()) {
	case 1: {
		long sync = (pimg->width() + 7) / 8;
		sync = ((sync + 31) / 32) * 32;
		pimg->sync(sync);
		break;
	}
	case 8:pimg->sync(pimg->width()); break;
	default:
	case 24:pimg->sync(pimg->width() * 3); break;
	}
	if (pimg->spp() * pimg->bps() != 1) {
		pimg->comptype(1);
	}
	int total = 0;
	unsigned char app0[] = { 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x01/*dpi*/, 0x01/*x res*/, 0x2c/*x res*/, 0x00/*y res*/, 0x01/*y res*/, 0x00, 0x00 };
	unsigned char eof_ffd9[2] = { 0xff, 0xd9 }; 
	if (w_to_scanner.compression_type() == 0) {
		total = total_size(back);
	}
	else {
		Set2BYTE(app0, 12, w_to_scanner.xdpi());
		Set2BYTE(app0, 14, w_to_scanner.ydpi());
		total = total_size(back) + sizeof(app0) + sizeof(eof_ffd9);
	}
	pimg->size(total);
	if (w_to_scanner.compression_type() == 0) {
		int index = 0;
		for (const auto& ele : m_array[back]) {
			memcpy(&(pimg->img()[index]), ele->data(), ele->transfer_length());
			index += ele->transfer_length();
			delete ele;
		}
		m_array[back].clear();
	}
	else {
		int index = 0;
		memcpy(&(pimg->img()[index]), m_array[back][0]->data(), 2);
		index += 2;
		memcpy(&(pimg->img()[index]), app0, sizeof(app0));
		index += sizeof(app0);
		memcpy(&(pimg->img()[index]), m_array[back][0]->data() + 2, m_array[back][0]->transfer_length() - 2);
		index += m_array[back][0]->transfer_length() - 2;
		delete m_array[back][0];
		std::vector<CStreamCmd*>::iterator itr = m_array[back].begin();
		for (itr++; itr != m_array[back].end(); itr++) {
			memcpy(&(pimg->img()[index]), (*itr)->data(), (*itr)->transfer_length());
			index += (*itr)->transfer_length();
			delete* itr;
		}
		m_array[back].clear();
		memcpy(&(pimg->img()[index]), eof_ffd9, sizeof(eof_ffd9));
	}
	*ppOut = pimg;
	WriteLog("CFramework::make_image() end");
}
void CFrameworkEndSequence::jpeg_decomp(ICeiImage** ppInOut)
{
	ICeiImage* pimg = *ppInOut;
	if (pimg->comptype()) {
		ceisdk_jpeg_decomp(ppInOut);
	}
}
void CFrameworkEndSequence::invert(ICeiImage* pimg)
{
	if (pimg->spp() * pimg->bps() == 1) {
		vs_invert_black_and_white((ICeiImage*)pimg);
	}
}
void CFrameworkEndSequence::change_sync(ICeiImage** ppfront, ICeiImage** ppback)
{
	ceisdk_change_sync(ppfront, 1);
	ceisdk_change_sync(ppback, 1);
}
void CFrameworkEndSequence::on_error(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_error() start");
	CSenseCmd *psns = NULL;
	pmsg->get((void**)&psns);
	if (psns) {
		m_error = sense2vs3_error(*psns);
		WriteLog((char*)"m_error %d", m_error);
	}
	WriteLog((char*)"on_error() end");
}
void CFrameworkEndSequence::on_batch_end(ICeiMessage *pmsg)
{
	WriteLog((char*)"on_batch_end() start");
	WriteLog((char*)"on_batch_end() end");
}
long CFrameworkEndSequence::get_image(ICeiImage **ppOut)
{
	WriteLog((char*)"get_image() start");
	long ret = m_error;
	if (m_error) {
		//WriteLog((char*)"m_error is %d", m_error);
		ret = m_error;
	} else if (m_image[FRONT].get()) {
		*ppOut = m_image[FRONT].Detach();
	}
	else if (m_image[BACK].get()) {
		*ppOut = m_image[BACK].Detach();
	}
	else {
		WriteLog((char*)"m_image[FRONT and BACK].get() is NULL");
		ret = VS3_NOPAGE;
	}
	WriteLog((char*)"get_image() end %d", ret);
	return ret;
}
long CFrameworkEndSequence::get_information(long , void *pout)
{
	WriteLog((char*)"get_information() start");
	long ret = VS3_OK;
	ICeiImageInformation ** ppOut = (ICeiImageInformation **)pout;
	if (m_information[FRONT].get()) {
		*ppOut = m_information[FRONT].release();
	}
	else if (m_information[BACK].get()) {
		*ppOut = m_information[BACK].release();
	}
	else {
		WriteLog((char*)"m_information[FRONT and BACK].get() is NULL");
		ret = VS3_NOPAGE;
	}
	WriteLog((char*)"get_information() end %ld", ret);
	return ret;
}
IEndSequence *end_sequence(IMessageQueue *q, IUnknown *handle)
{
	return new CFrameworkEndSequence(q, (CSettingsFramework *)handle);
}
