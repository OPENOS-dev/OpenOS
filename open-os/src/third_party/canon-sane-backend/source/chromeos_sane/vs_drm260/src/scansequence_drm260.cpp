/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <memory>
#include "vssdk.h"
#include "ceilogwrite.h"
#include "sdk_command_util.h"
#include "sdk_message.h"
#include "message_queue_interface.h"
#include "scan_sequence_interface.h"
#include "settings_framework.h"
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
    bool read_image();
    void read_information();
        void senser_pos2image_pos(CStreamCmd *pstream);
    void eject_check(CStreamCmd& stream_eject, bool bduplex, bool bfront);
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
  {
        CScanParam param =  m_psettings->scanparam_sep(0);
        param.data()[10] = 0;
        m_pscanner->write(param);
  }
  {
        CScanParam param =  m_psettings->scanparam_sep(1);
        param.data()[10] = 0;
        m_pscanner->write(param);	
  }
  {
      CWindow w = m_psettings->window_from_client(0);
      CScanParam &param =  m_psettings->scanparam_sepr_to_scanner(0);
      m_pscanner->write(param);
  }
  {
      CWindow w = m_psettings->window_from_client(0);
      CScanParam &param = m_psettings->scanparam_sepr_to_scanner(1);
      m_pscanner->write(param);	
  }
  {
      CScanParam param = m_psettings->scanparam_both();
      m_pscanner->write(param);
  }
  {
      CScanParam param = m_psettings->scanparam_bothr();
      m_pscanner->write(param);	
  }
  {
      CScanParam param = m_psettings->scanparam_option();
      m_pscanner->write(param);
  }
  {
      //CScanParam param = m_psettings->scanparam_margin();
      //param.side(false);
      //m_pscanner->write(param);
      //param.side(true);
      //m_pscanner->write(param);
  }
  {
      CWindow& w = m_psettings->window_to_scanner(0);
      if (w.bps()>1) {
        w.compression_type(0x80);
      } else {
        w.compression_type(0);
      }
      w.side(false);
      m_pscanner->write(w);
  }
  {
    CWindow& w = m_psettings->window_to_scanner(1);
    if (w.bps()>1) {
    w.compression_type(0x80);
    } else {
    w.compression_type(0);
    }
    w.side(true);
    m_pscanner->write(w);
  }
  {
    CWindow& w = m_psettings->window_to_scanner(0);
    CScanParam sp(CScanParam::PAGE_CODE_PAPER_SIZE);
    sp.ulx_of_paper(w.xoffset());
	sp.uly_of_paper(w.yoffset());
	sp.width_of_paper(w.width());
	sp.length_of_paper(w.length());
    m_pscanner->write(sp);
  }
}
bool CFrameworkScanSequence::read_image()
{
    static const long READ_BUFFER_SIZE = 1024 * 1024;
    bool bloop = true;
    bool bOK = false;
    while (bloop) {
        std::unique_ptr<CStreamCmd>pstream(new CStreamCmd(READ_BUFFER_SIZE));
        if (pstream.get() && pstream->data()) {
            long ret = m_pscanner->read(*pstream);
            if (ret) {
                std::unique_ptr<CSenseCmd>psense(new CSenseCmd);
                if (psense.get()) {
                    m_pscanner->read(*psense);
                    if (psense->ILI()) {
                        long s = pstream->data_size();
                        pstream->transfer_length(s - psense->information_bytes());
                        m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (void*)pstream.release()));
                        bOK = true;
                    }
                    else {
                        m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense.release()));
                        bOK = false;
                    }
                }
                else {
                    m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)0));
                }
                bloop = false;
            }
            else {
                m_pnext->push(create_message(ICeiMessage::MID_IMAGE, (void*)pstream.release()));
            }
        }
        else {
            m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)0));//out of memory error
            bloop = false;
            bOK = false;
        }
    }
    return bOK;
}
void CFrameworkScanSequence::senser_pos2image_pos(CStreamCmd* pstream)
{
    CWindow w = m_psettings->window_from_client(0);
    CScanParam sp = m_psettings->scanparam_bothr();
    if (sp.autosize()) {

    }
    else {
        pstream->p4_upperleftx(pstream->p4_upperleftx() - w.xoffset());
        pstream->p4_upperrightx(pstream->p4_upperrightx() - w.xoffset());
        pstream->p4_lowerleftx(pstream->p4_lowerleftx() - w.xoffset());
        pstream->p4_lowerrightx(pstream->p4_lowerrightx() - w.xoffset());
    }
}
void CFrameworkScanSequence::read_information()
{
    CStreamCmd* pstream = NULL;

    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::IMAGEAREA);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::MARGIN);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_MARGIN2_AFTER);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_PAPERAREA);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_4POINTS2_AFTER);
    m_pscanner->read(*pstream);
    senser_pos2image_pos(pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_VALIDAREA);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::COLOR_DETECTION, 0);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::BLANKPAGE_DETECTION, 0);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::AREAINFO, CStreamCmd::AREAINFO_4POINTS_AFTER);
    m_pscanner->read(*pstream);
    senser_pos2image_pos(pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
    //
    pstream = new CStreamCmd(CStreamCmd::EJECT, 0);
    m_pscanner->read(*pstream);
    m_pnext->push(create_message(ICeiMessage::MID_INFO, (void*)pstream));
}
void CFrameworkScanSequence::eject_check(CStreamCmd &stream_eject, bool bduplex, bool bfront)
{
    m_pscanner->read(stream_eject);
    if (stream_eject.doublefeed()) {
        if (bduplex) {
            if (bfront) {
                stream_eject.doublefeed(false);
            }
            else {
                std::unique_ptr<CSenseCmd>psense(new CSenseCmd);
                psense->doublefeed();
                m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense.release()));
            }
        }
        else {
            std::unique_ptr<CSenseCmd>psense(new CSenseCmd);
            psense->doublefeed();
            m_pnext->push(create_message(ICeiMessage::MID_ERROR, (void*)psense.release()));
        }
    }
    else if (stream_eject.eject()) {
        WriteLog((char*)"stream_eject.eject()");
    }
    else {
        WriteLog((char*)"Exception: CStreamCmd::EJECT have no data");
    }
}
void CFrameworkScanSequence::main(IScanSequenceCallback *pcallback)
{
	WriteLog((char*)"CFrameworkScanSequence::main() start");

	to_scanner();

    CScanCmd &scn_from_client = m_psettings->scancmd_from_client();
    CScanCmd &scn_to_scanner  = m_psettings->scancmd_to_scanner();
    if ((!scn_from_client.duplex() && (scn_from_client.data()[0] == 1))) {
        scn_to_scanner.duplex(true);
    }
	long ret = m_pscanner->write(scn_to_scanner);
    if (ret) {
        CSenseCmd* psense = new CSenseCmd;
        m_pscanner->read(*psense);
        m_pnext->push(create_message(ICeiMessage::MID_ERROR, psense));
        return;
    }
	bool bfront=true;
	bool bduplex = scn_to_scanner.duplex();
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
        //check paper
        CObjectPositionCmd medium(CObjectPositionCmd::MediumPosition);
        ret = m_pscanner->none(medium);
        if (ret) {
            CSenseCmd *psense = new CSenseCmd;
            m_pscanner->read(*psense);
            m_pnext->push(create_message(ICeiMessage::MID_ERROR, psense));
            break;
        }
        
        if (scn_from_client.duplex() || (scn_from_client.data()[0] == 0) || (!bfront)) {
            pcallback->page_start();//this api must be called before MID_PAGE_START
            m_pnext->push(create_message(ICeiMessage::MID_PAGE_START, (long long)(bfront ? 0 : 1)));
            m_pnext->push(create_message(ICeiMessage::MID_IMAGE_START, (long long)(bfront ? 0 : 1)));
            read_image();
            m_pnext->push(create_message(ICeiMessage::MID_IMAGE_END, (long long)(bfront ? 0 : 1)));
            m_pnext->push(create_message(ICeiMessage::MID_INFO_START, (long long)(bfront ? 0 : 1)));
            read_information();
            m_pnext->push(create_message(ICeiMessage::MID_INFO_END, (long long)(bfront ? 0 : 1)));
            m_pnext->push(create_message(ICeiMessage::MID_PAGE_END, (long long)(bfront ? 0 : 1)));
            pcallback->page_end(1);//this api must be called after MID_PAGE_END
        }
        CStreamCmd stream_eject(CStreamCmd::EJECT, 0x0000);
        eject_check(stream_eject, bduplex, bfront);
        CDiscardCmd discard;
        m_pscanner->write(discard);
        if (stream_eject.doublefeed()) {
            break;
        }
        if (bduplex) bfront = bfront ? false : true;
        
	}
	pcallback->batch_end();//this api must be called after batch scanning
	CAbortCmd abt;
	m_pscanner->none(abt);
	WriteLog((char*)"CFrameworkScanSequence::main() end");
}
IScanSequence *scan_sequence(IMessageQueue *q, IUnknown *scanner, IUnknown *handle, IUnknown *, IUnknown *)
{
	return new CFrameworkScanSequence(q, (IScannerConnector*)scanner, (CSettingsFramework*)handle);
}

