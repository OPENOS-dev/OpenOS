/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include <algorithm>
#include <time.h>
#include "vssdk.h"
#include "ceilogwrite.h"
#include "sdk_command_util.h"
#include "sdk_message.h"
#include "message_queue_interface.h"
#include "scan_sequence_interface.h"
#include "settings_framework.h"
long adjust_scanner(CScsiCommand& scanner, CSenseCmd& sense, long sensor);
void send_shading_data_front(CScsiCommand& scanner);
void send_shading_data_back(CScsiCommand& scanner);
namespace {
	inline CScanMode::COLOR_TYPE to_coltype(long id)
	{
		CScanMode::COLOR_TYPE coltype = CScanMode::NONE;
		switch (id)
		{
		case 1:
			coltype = CScanMode::RED;
			break;
		case 2:
			coltype = CScanMode::GREEN;
			break;
		case 3:
			coltype = CScanMode::BLUE;
			break;
		}
		return coltype;
	}
}
class CFrameworkScanSequence : public IScanSequence
{
public:
	CFrameworkScanSequence(IMessageQueue *q, IScannerConnector *s, CSettingsFramework *h);
	virtual ~CFrameworkScanSequence();
	void main(IScanSequenceCallback *pcallback);
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
private:
	void to_scanner();
	bool adjust();
	bool scan();
	bool read_image();
	bool read_information();
private:
	long m_ref;
	std::unique_ptr<CScsiCommand>m_pscanner;
	IMessageQueue *m_pnext;
	CSettingsFramework *m_psettings;
};
CFrameworkScanSequence::CFrameworkScanSequence(IMessageQueue *q, IScannerConnector *s, CSettingsFramework *h):
	m_ref(1), 
	m_pnext(q), 
	m_psettings(h)
{
	m_pscanner.reset(new CScsiCommand(s));
}
CFrameworkScanSequence::~CFrameworkScanSequence()
{
}
long CFrameworkScanSequence::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CFrameworkScanSequence::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CFrameworkScanSequence::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
void CFrameworkScanSequence::to_scanner()
{
	WriteLog((char*)"CFrameworkScanSequence::to_scanner() start");
	CScanCmd &scancmd_to_scanner  = m_psettings->scancmd(CSettingsFramework::TO_SCANNER);
	CScanCmd &scancmd_from_client = m_psettings->scancmd(CSettingsFramework::FROM_CLIENT);
	{
		bool card = false;
		if (m_psettings->has_card()) {
			CStreamCmd adf(CStreamCmd::PAPER, 0);
			adf.transfer_length(1);
			m_pscanner->read(adf);
			if (adf.data()[0] & 8) {
				card = true;
			}
		}
		CScanMode sm;
		sm.page_code(CScanMode::PAGE_CODE_SCAN);
		m_pscanner->read(sm);
		{
			CScanParam scanparam = m_psettings->scanParam_both();
			long num = scanparam.maximum_number_of_documents_in_batch_mode();
			switch (num) {
			case 0:sm.batch(true);
				sm.maximum_number_of_documents_in_batch_mode(0);
				break;
			case 1:
				sm.batch(false);
				sm.maximum_number_of_documents_in_batch_mode(1);
				break;
			default:
				sm.batch(true);
				sm.maximum_number_of_documents_in_batch_mode(num);
				break;
			}
		}
		{
			if (card) {
				sm.data()[6+4] |= 8;
				sm.batch(false);
				sm.maximum_number_of_documents_in_batch_mode(1);
				m_psettings->card_scan(true);
			}
			else {
				sm.data()[6+4] &= ~8;
				m_psettings->card_scan(false);
			}
		}
		{
			CScanParam scanparam = m_psettings->scanParam_bothr();
			if (scancmd_from_client.duplex()) {
				sm.duplex(true);
				scancmd_to_scanner.duplex(true);
				scancmd_to_scanner.data()[0] = 0;
			}
			else {
				//simplex  --> scanner
				sm.duplex(false);
				scancmd_to_scanner.duplex(false);
				scancmd_to_scanner.data()[0] = 0;
			}
		}
		sm.autosize(true);
		m_pscanner->write(sm);
	}
	{
		CScanMode sm;
		sm.page_code(CScanMode::PAGE_CODE_OPTION);
		m_pscanner->read(sm);
		CScanParam param = m_psettings->scanParam_option();
		sm.dfd_uss(param.double_feed_detection_ultrasonic());
		sm.dfd_length(param.double_feed_detection_length());
		m_pscanner->write(sm);
	}
	{
		CScanMode sm;
		sm.page_code(CScanMode::PAGE_CODE_FILTER);
		m_pscanner->read(sm);
		CScanParam param[2] = { m_psettings->scanParam_sep_rescan(0), m_psettings->scanParam_sep_rescan(1) };
		sm.drop_out(CScanMode::FRONT, to_coltype(param[0].drop_out()));
		sm.drop_out(CScanMode::BACK, to_coltype(param[1].drop_out()));
		m_pscanner->write(sm);
	}
	{
		CScanParam scanparam = m_psettings->scanParam_both();
		CStreamCmd userdata(CStreamCmd::USERDATA, 1);
		m_pscanner->read(userdata);
		if (scanparam.maximum_paper_length() < 1000000) {
			userdata.maximum_paper_length(355600);
		}
		else if (scanparam.maximum_paper_length() < 3000000) {
			userdata.maximum_paper_length(1000000);
		}
        else {
            userdata.maximum_paper_length(3000000);
		}
		m_pscanner->write(userdata);
		CInquiryCmd inqex = m_psettings->inquiry_ex(CSettingsFramework::TO_SCANNER);
		CMode mode = m_psettings->mode();
		CScanParam scanparamr = m_psettings->scanParam_bothr();
		CWindow& window_to_scanner = m_psettings->window(CSettingsFramework::TO_SCANNER);
		window_to_scanner.xoffset(0);
		if (scanparamr.autosize() || scanparamr.deskew()) {
			window_to_scanner.yoffset(-m_psettings->margin_1200dpi());
		}
		else {
			window_to_scanner.yoffset(0);
		}
		long bxdpi = inqex.basic_xdpi();
		if (!bxdpi) bxdpi = 600;
		long bydpi = inqex.basic_ydpi();
		if (!bydpi) bydpi = 600;
		window_to_scanner.width(inqex.window_width() * mode.mud() / bxdpi);		
    	    window_to_scanner.length(inqex.window_length() * mode.mud() / bydpi);
		window_to_scanner.bps(8);
		if (window_to_scanner.xdpi()>300) {
			window_to_scanner.xdpi(600);
			window_to_scanner.ydpi(600);
		} else {
			window_to_scanner.xdpi(300);
			window_to_scanner.ydpi(300);
		}
		window_to_scanner.side(false);
		m_pscanner->write(window_to_scanner);
		window_to_scanner.side(true);
		m_pscanner->write(window_to_scanner);
	}
    WriteLog((char*)"CFrameworkScanSequence::to_scanner() end");
}
bool CFrameworkScanSequence::adjust()
{
	CSenseCmd* psense = new CSenseCmd;
	long ret = adjust_scanner(*m_pscanner, *psense, m_psettings->sensor_version());
	if (ret != 0) {
		m_pnext->push(create_message(ICeiMessage::MID_ERROR, psense));
		WriteLog((char*)"CFrameworkScanSequence::ERROR:adjust_scanner");
		return false;
	}
	delete psense; 
	if (m_psettings->scanner_needs_shading_data()) {
		CScanCmd scancmd = m_psettings->scancmd(CSettingsFramework::TO_SCANNER);
		send_shading_data_front(*m_pscanner); 
		if (scancmd.duplex()) {
			send_shading_data_back(*m_pscanner);
		}	
	}
	return true;
}
bool CFrameworkScanSequence::scan()
{
	CScanCmd& scn_to_scanner = m_psettings->scancmd(0);
	long ret = m_pscanner->write(scn_to_scanner);
	if (ret) {
		CSenseCmd* psense = new CSenseCmd;
		m_pscanner->read(*psense);
		m_pnext->push(create_message(ICeiMessage::MID_ERROR, psense));
		WriteLog((char*)"CFrameworkScanSequence::scan ERROR");
		return false;
	}
	return true;
}
bool CFrameworkScanSequence::read_image()
{
	WriteLog((char*)"CFrameworkScanSequence::read_image start");
	CScanCmd scn = m_psettings->scancmd(0);
	CMode mode = m_psettings->mode();
	CWindow w = m_psettings->window(CSettingsFramework::TO_SCANNER);
	long line_length = (w.width() * w.xdpi() / mode.mud()) * (scn.duplex()?2:1) * w.spp();
	const long MAX_READSIZE = 1024 * 1024;
	long read_size = MAX_READSIZE / line_length * line_length;
 	long ret = 0;
	bool bloop = true;
	//read 1st
	std::unique_ptr<CStreamCmd>pstream(new CStreamCmd(line_length/2));
	ret = m_pscanner->read(*pstream);
	if (ret == VS3_OK) {
		m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (void*)pstream.release()));
	}
	else {
		std::unique_ptr<CSenseCmd>psense(new CSenseCmd);
		m_pscanner->read(*psense);
		m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense.release()));
		WriteLog((char*)"CFrameworkScanSequence::read_image ERROR:%d", __LINE__);
		return false;
	}
	//read 2nd
	while (bloop) {
		std::unique_ptr<CStreamCmd>pstream(new CStreamCmd(read_size));
		ret = m_pscanner->read(*pstream);
		if (ret == VS3_OK) {
			m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (void*)pstream.release()));
		}
		else {
			std::unique_ptr<CSenseCmd>psense(new CSenseCmd);
			m_pscanner->read(*psense);
			if (psense->ILI())
			{
				pstream->transfer_length(pstream->transfer_length() - psense->information_bytes());
				m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (void*)pstream.release()));
				bloop = false;
			}
			else {
				m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense.release()));
				WriteLog((char*)"CFrameworkScanSequence::read_image ERROR:%d", __LINE__);
				return false;
			}
		}
	}
	return true;
}
bool CFrameworkScanSequence::read_information()
{
	return true;
}
void CFrameworkScanSequence::main(IScanSequenceCallback* pcallback)
{
	WriteLog((char*)"CFrameworkScanSequence::main() start\r\n");
	to_scanner();
	if (!adjust()) {
		return;
	}
	if (!scan()) {
		return;
	}
	CScanParam scanparam = m_psettings->scanParam_both();
	long maxnum = scanparam.maximum_number_of_documents_in_batch_mode();
	long num = 0;
	CScanCmd scancmd_from_client = m_psettings->scancmd(CSettingsFramework::FROM_CLIENT);
	pcallback->batch_start();//this api must be called before batch scanning
	while (1) {
		if (pcallback->stop_request()) {//this api must be called to stop scanning urgently.
			CStopBatchCmd stop;
			m_pscanner->none(stop);
		}
		{
			CStreamCmd button(CStreamCmd::PANEL, 0);
			m_pscanner->read(button);
			if (button.stop_key()) {
				CStopBatchCmd stop;
				m_pscanner->none(stop);
			}
		}
		{
			CObjectPositionCmd adf(CObjectPositionCmd::MediumPosition);
			if (m_pscanner->none(adf)) {
				CSenseCmd * psense = new CSenseCmd;
				m_pscanner->read(*psense);
				m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense));
				break;
			}
		}
		pcallback->page_start();//this api must be called before MID_PAGE_START
		m_pnext->push(create_message(ICeiMessage::MID_PAGE_START, (long long)0));
		m_pnext->push(create_message(ICeiMessage::MID_IMAGE_START, (long long)0));
		if (!read_image()) {
			break;
		}
		m_pnext->push(create_message(ICeiMessage::MID_IMAGE_END, (long long)0));
		m_pnext->push(create_message(ICeiMessage::MID_INFO_START, (long long)0));
		if (!read_information()) {
			break;
		}
		m_pnext->push(create_message(ICeiMessage::MID_INFO_END, (long long)0));
		m_pnext->push(create_message(ICeiMessage::MID_PAGE_END, (long long)0));
		pcallback->page_end(scancmd_from_client.duplex()?2:1);//this api must be called after MID_PAGE_END
		if (scancmd_from_client.duplex()) {
			m_pnext->push(create_message(ICeiMessage::MID_PAGE_START, (long long)1));
			m_pnext->push(create_message(ICeiMessage::MID_PAGE_END, (long long)1));
		}
		num++;
		if (maxnum && num == maxnum) {
			CSenseCmd *psense = new CSenseCmd;
			psense->nopaper();
			m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense));
			break;
		}

	}
	pcallback->batch_end();//this api must be called after batch scanning

	CAbortCmd abt;
	m_pscanner->none(abt);
	WriteLog((char*)"CFrameworkScanSequence::main() end\r\n");
}
IScanSequence *scan_sequence(IMessageQueue *q, IUnknown *scanner, IUnknown *handle, IUnknown *, IUnknown *)
{
	return new CFrameworkScanSequence(q, (IScannerConnector*)scanner, (CSettingsFramework*)handle);
}
