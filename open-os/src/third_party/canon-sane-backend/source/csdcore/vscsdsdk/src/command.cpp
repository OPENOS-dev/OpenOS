/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <algorithm>
#include "command.h"
namespace command {
	unsigned char GetBYTE(unsigned char * pData, int nIndex)
	{ 
		return pData[nIndex]; 
	}
	void SetBYTE(unsigned char * pData, int nIndex, char data)
	{ 
		pData[nIndex] = data; 
	}
	int Get4BYTE(unsigned char * pData, int nIndex)
	{
		return 	((int)((pData[nIndex] << 24) | (pData[nIndex+1] << 16) | (pData[nIndex+2] << 8) | pData[nIndex+3]));
	}
	void Set4BYTE(unsigned char * pData, int nIndex,int dwData)
	{
		pData[nIndex]		= (char)((dwData) >> 24);
		pData[nIndex + 1]	= (char)((dwData) >> 16);
		pData[nIndex + 2]	= (char)((dwData) >> 8);
		pData[nIndex + 3]	= (char)(dwData);
	}
	short Get2BYTE(unsigned char * pData, int nIndex)
	{
		return (short)((pData[nIndex] << 8) | pData[nIndex+1]);
	}
	void Set2BYTE(unsigned char * pData, int nIndex, short wData)
	{
		pData[nIndex]   = (char)((wData) >> 8);
		pData[nIndex+1] = (char)((wData) );
	}
	int  Get3BYTE(unsigned char * pData, int nIndex)
	{
		return (int)((pData[nIndex]  << 16) | (pData[nIndex+1]  << 8) | pData[nIndex+2] );
	}
	void  Set3BYTE(unsigned char * pData, int nIndex,int dwData)
	{
		pData[nIndex]	  = (char)((dwData) >> 16);	
		pData[nIndex+1] = (char)((dwData) >> 8);	
		pData[nIndex+2] = (char)(dwData);
	}
	//////////////////////////////////////////
	// |**|**|**|**|**|**|**|**|
	//   7  6  5  4  3  2  1  0
	//////////////////////////////////////////
	const unsigned char byBitAccess[]={
		(unsigned char)0x01,(unsigned char)0x02,(unsigned char)0x04,(unsigned char)0x8,(unsigned char)0x10,(unsigned char)0x20,(unsigned char)0x40,(unsigned char)0x80
	};
	#if 0
	const unsigned char byBitAccessr[]={
		(unsigned char)0xfe,(unsigned char)0xfd,(unsigned char)0xfb,(unsigned char)0xf7,(unsigned char)0xef,(unsigned char)0xdf,(unsigned char)0xbf,(unsigned char)0x7f
	};
	#endif
	int firstBitForAccess(const unsigned char mask)
	{
		int result =0;
		for (result=0; result < 8; result++) {
			if ((mask & byBitAccess[result]) != 0x00) break;
		}
		return result;
	}
	unsigned char GetBit(unsigned char *pData, int nIndex, unsigned char byMask)
	{
		if (byMask == 0x00) return 0;
		unsigned char answer = (pData[nIndex] & byMask);
		answer >>= firstBitForAccess(byMask);
		return answer;
	}
	void SetBit(unsigned char *pData, int nIndex, char by , unsigned char byMask)
	{
		if (byMask == 0x00) return;
		pData[nIndex] = pData[nIndex]&~byMask;	
		pData[nIndex] |= (pData[nIndex] & byMask) | ((by << firstBitForAccess(byMask)) & byMask);
	}
	void SetString(char *pData, int nIndex, char *str)
	{
		while (*str) {
			pData[nIndex] = *str;
			str++;
			nIndex++;
		}
	}
	void SetNString(char * pData, int nIndex, int n, char *str)
	{
		while (*str && n--) {
			pData[nIndex] = *str;
			str++;
			nIndex++;
		}
	}
	char *GetNString(char *pData, int nIndex, int n, char *out)
	{
		char *ret = out;
		while (n--) {
			*out = pData[nIndex];
			out++;
			nIndex++;
		}
		*out=0;
		return ret;
	}
}
//////////////////////////////////////////////////////////////////
CCommand::CCommand()
{
	m_pdata=NULL;
	m_cdb_size=sizeof(m_cdb);
	memset(m_cdb, 0, sizeof(m_cdb));
}
CCommand::~CCommand()
{}
CCommand* CCommand::clone()
{
	return NULL; 
}
void CCommand::I_am_in(CCommand::EXEC_TYPE type)
{}
char *CCommand::cdb()
{
	return (char*)m_cdb;
}
long CCommand::cdb_size()
{
	return m_cdb_size;
}
void CCommand::transfer_identification(short id)
{
	command::Set2BYTE(m_cdb, 4, id);
}
short CCommand::transfer_identification()
{
	return command::Get2BYTE(m_cdb, 4);
}
long CCommand::transfer_length()
{
	return command::Get3BYTE(m_cdb, 6);
}
void CCommand::transfer_length(long s)
{
	command::Set3BYTE(m_cdb, 6, (int)s);
}
unsigned char CCommand::transfer_data_type()
{
	return (unsigned char)m_cdb[2];
}
unsigned char CCommand::allocation_length()
{
	return m_cdb[4];
}
void CCommand::allocation_length(unsigned char s)
{
	m_cdb[4] = s;
}
void CCommand::transfer_data_type(unsigned char s)
{
	m_cdb[2] = s;
}
char *CCommand::data()
{
	return (char*)m_pdata;
}
long CCommand::data_size()
{
	long out = cdb()[4];
	if (cdb_size() > 6) out = transfer_length();
	return out;
}
void CCommand::serialize(FILE *fp)
{
	char exist=0;
	if (cdb_size()&&data_size()) {
		exist=1;
		fwrite(&exist, 1, sizeof(exist), fp);
		fwrite(cdb(), 1, cdb_size(), fp);
		fwrite(data(), 1, data_size(), fp);
	} else {
		exist=0;
		fwrite(&exist, 1, sizeof(exist), fp);
	}
}
void CCommand::deserialize(FILE *fp)
{
	char exist=0;
	size_t ret = 0;
	ret = fread(&exist, 1, sizeof(exist), fp);
	if (exist&&ret==sizeof(exist)) {
		ret = fread(cdb(), 1, cdb_size(), fp);
		ret = fread(data(), 1, data_size(), fp);
	}
}
void CCommand::copy(CCommand &src)
{
	//cdb
	long sz = cdb_size();
	if (sz > src.cdb_size()) sz = src.cdb_size();
	memcpy(m_cdb, src.cdb(), sz);
	//data
	if (data_size() && src.data_size()) {		
		if (m_pdata && src.m_pdata) {
			sz = data_size();
			if (sz > src.data_size()) sz = src.data_size();
			memcpy(m_pdata, src.data(), sz);
		}
	}
}
bool CCommand::operator==(CCommand &src)
{
	return m_cdb[0]==src.cdb()[0] &&
		   transfer_data_type()==src.transfer_data_type();
}
void CCommand::input(char * cdb, long cdb_size, char * data, long data_size)
{
	if (cdb_size > (long)sizeof(m_cdb)) cdb_size = sizeof(m_cdb);
	memcpy(m_cdb, cdb, cdb_size);
	m_cdb_size = cdb_size;
	m_pdata=(unsigned char*)data;
}
////////////////////////////////////////////////////////////////////////////
CTestUnitReadyCmd::CTestUnitReadyCmd()
{
	command::SetBYTE(m_cdb, 0, 0);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size = 6;
}
////////////////////////////////////////////////////////////////////////////////////////////////
CInquiryCmd::CInquiryCmd()
{
	memset(m_data, 0, sizeof(m_data));
	memset(m_out, 0, sizeof(m_out));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=6;
	command::SetBYTE(m_cdb, 0, (char)0x12);
	command::SetBYTE(m_cdb, 1, (char)0x0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, (char)sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
}
CInquiryCmd::CInquiryCmd(CInquiryCmd& in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
	memset(m_out, 0, sizeof(m_out));
}
CInquiryCmd::CInquiryCmd(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	for (long i = 0; i < (long)sizeof(m_data); i++) m_data[i] = (unsigned char)i;
	memset(m_out, 0, sizeof(m_out));
}
CInquiryCmd::~CInquiryCmd()
{
}
CInquiryCmd& CInquiryCmd::operator = (CInquiryCmd& in)
{
	copy(in);
	return *this;
}
void CInquiryCmd::allocation_length(long v)
{
	m_cdb[4]=(unsigned char)v;
}
long CInquiryCmd::allocation_length()
{
	return m_cdb[4];
}
void CInquiryCmd::scanner_name(char *in)
{
	long len = (long)strlen(in);
	long n = 16;
	if (len > n) return;
	if (len < n) n = len;
	command::SetNString((char*)m_pdata, 16, (int)n, in);
}
char *CInquiryCmd::scanner_name()
{
	m_out[0] = 0;
	command::GetNString((char*)m_pdata, 16, 16, m_out);
	return m_out;
}
void CInquiryCmd::product_revision_level(char *in)
{
	long len = (long)strlen(in);
	long n = 4;
	if (len > n) return;
	if (len < n) n = len;
	command::SetNString((char*)m_pdata, 32, (int)n, in);
}
char *CInquiryCmd::product_revision_level()
{
	m_out[0] = 0;
	command::GetNString((char*)m_pdata, 32, 4, m_out);
	return m_out;	
}
bool CInquiryCmd::has_flatbed()
{
	return command::GetBit(m_pdata, 31, (char)0x80)>0;
}
void CInquiryCmd::evpd(bool sw)
{
	command::SetBit(m_cdb, 1, sw, (char)0x1);
	if (sw) page_code((char)0xF0);
	else    page_code(0);	
}
bool CInquiryCmd::evpd()
{
	return command::GetBit(m_cdb, 1, (char)0x1)>0?true:false;
}
long CInquiryCmd::basic_xdpi()
{
	return command::Get2BYTE(m_pdata, 5);
}
void CInquiryCmd::basic_xdpi(long v)
{
	command::Set2BYTE(m_pdata, 5, (short)v);
}
long CInquiryCmd::basic_ydpi()
{
	return command::Get2BYTE(m_pdata, 7);
}
void CInquiryCmd::basic_ydpi(long v)
{
	command::Set2BYTE(m_pdata, 7, (short)v);
}
void CInquiryCmd::page_code(char code)
{
	command::SetBYTE(m_cdb, 2, code);
}
bool CInquiryCmd::wireless()
{
	return command::GetBit(m_pdata, 32, (char)0x10)>0;
}
void CInquiryCmd::wireless(bool sw)
{
	command::SetBit(m_pdata, 32, sw?1:0, (char)0x10);
}
long CInquiryCmd::window_width()
{
	return command::Get4BYTE(m_pdata, 20);
}
long CInquiryCmd::window_length()
{
	return command::Get4BYTE(m_pdata, 24);
}
void CInquiryCmd::window_width(long v)
{
	command::Set4BYTE(m_pdata, 20, (int)v);
}
void CInquiryCmd::window_length(long v)
{
	command::Set4BYTE(m_pdata, 24, (int)v);
}
long CInquiryCmd::real_window_width()
{
	return command::Get4BYTE(m_pdata, 48);
}
void CInquiryCmd::real_window_width(long v)
{
	command::Set4BYTE(m_pdata, 48, (int)v);
}
////////////////////////////////////////////////////////////////////////////
CReserveUnitCmd::CReserveUnitCmd()
{
	command::SetBYTE(m_cdb, 0, opReserveUnit);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
}
////////////////////////////////////////////////////////////////////////////
CReleaseUnitCmd::CReleaseUnitCmd()
{
	command::SetBYTE(m_cdb, 0, opReleaseUnit);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
}
////////////////////////////////////////////////////////////////////////////
CObjectPositionCmd::CObjectPositionCmd(long type)
{
	command::SetBYTE(m_cdb, 0, (char)0x31);
	command::SetBit(m_cdb, 1, (char)type, (char)0x7);
	command::Set3BYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 5, 0);
	command::SetBYTE(m_cdb, 6, 0);
	command::SetBYTE(m_cdb, 7, 0);
	command::SetBYTE(m_cdb, 8, 0);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size=10;
}
CObjectPositionCmd::CObjectPositionCmd(char * cdb, long cdb_size)
{
	memcpy(m_cdb, cdb, cdb_size);
	m_cdb_size=10;
}
CObjectPositionCmd::~CObjectPositionCmd()
{
}
CObjectPositionCmd::TYPE CObjectPositionCmd::position_type()
{
	return (CObjectPositionCmd::TYPE)command::GetBit(m_cdb, 1, (char)0x7);
}
////////////////////////////////////////////////////////////////////////////
CAbortCmd::CAbortCmd():CObjectPositionCmd(CObjectPositionCmd::AbortScan)
{
}
CAbortCmd::CAbortCmd(CAbortCmd &in) : CObjectPositionCmd((CObjectPositionCmd&)in)
{
}
////////////////////////////////////////////////////////////////////////////
CScanCmd::CScanCmd()
{
	command::SetBYTE(m_cdb, 0, (char)0x1B);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
	m_pdata=(unsigned char*)m_data;
	memset(m_data, 0, sizeof(m_data));
}
CScanCmd::CScanCmd(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CScanCmd::CScanCmd(CScanCmd &in)
{
	command::SetBYTE(m_cdb, 0, (char)0x1B);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size = 6;
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
	command::SetBYTE(m_cdb, 4, (char)sz);
}
CScanCmd::~CScanCmd()
{
}
CScanCmd& CScanCmd::operator=(CScanCmd& in)
{
	copy(in);
	return *this;
}
void CScanCmd::duplex(bool on)
{
	if (on) {
		command::SetBYTE(m_cdb, 4, 2);
		command::SetBYTE(m_pdata, 1, 1);
	} else {
		command::SetBYTE(m_cdb, 4, 1);
		command::SetBYTE(m_pdata, 1, 0);
	}
}
bool CScanCmd::duplex()
{
	return command::GetBYTE(m_cdb, 4)==2;
}
void CScanCmd::window_identifier(char v)// == main window id
{
	command::SetBit(m_pdata, 0, v?1:0, 0xf0);
}
char CScanCmd::window_identifier()//==main window id
{
	return command::GetBit(m_pdata, 0, 0xf0);
}
void CScanCmd::extention(char number, bool v)
{
	command::SetBit(m_pdata, 0, v ? 1 : 0, 1 << number);
}
bool CScanCmd::extention(char number)
{
	return command::GetBit(m_pdata, 0, 1 << number);
}
void CScanCmd::side(bool v)
{
	extention(0, v);
}
bool CScanCmd::side()
{
	return extention(0);
}
char CScanCmd::main_window()
{
	return command::GetBYTE(m_pdata, 0);
}
char CScanCmd::sub_window()
{
	if (duplex()) return command::GetBYTE(m_pdata, 1);
	return 0;
}
void CScanCmd::main_window(char id)
{
	command::SetBYTE(m_pdata, 0, id);
}
void CScanCmd::sub_window(char id)
{
	command::SetBYTE(m_pdata, 1, id);
}
#if 0
////////////////////////////////////////////////////////////////////////////
CScanCmd2::CScanCmd2()
{
	command::SetBYTE(m_cdb, 0, (char)0x1B);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, (char)sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size = 6;
	m_pdata = (unsigned char*)m_data;
	memset(m_data, 0, sizeof(m_data));
}
CScanCmd2::CScanCmd2(char* cdb, long cdb_size, char* data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CScanCmd2::CScanCmd2(CScanCmd2& w):CCommand((CCommand&)w)
{
}
CScanCmd2::~CScanCmd2()
{
}
void CScanCmd2::window_identifier(int index, char v)
{
	command::SetBit(m_pdata, index, v, 0xf0);
}
char CScanCmd2::window_identifier(int index)
{
	return command::GetBit(m_pdata, index, 0xf0) > 0;
}
void CScanCmd2::extention(int index, int number, bool v)
{
	command::SetBit(m_pdata, index, v ? 1 : 0, 1 << number);
}
bool CScanCmd2::extention(int index, int number)
{
	return command::GetBit(m_pdata, index, 1 << number)>0;
}
void CScanCmd2::side(int index, bool v)
{
	extention(index, 0, v);
}
bool CScanCmd2::side(int index)
{
	return extention(index, 0);
}
void CScanCmd2::set_scanner_key(char* v)
{
	memcpy(m_pdata + 64, v, SCANNER_KEY_SIZE);
}
void CScanCmd2::get_scanner_key(char* v)
{
	memcpy(v, m_pdata + 64, SCANNER_KEY_SIZE);
}
#endif
//////////////////////////////////////////////////////////////////////////
CStopBatchCmd::CStopBatchCmd()
{
	command::SetBYTE(m_cdb, 0, (char)0xD8);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
}
//////////////////////////////////////////////////////////////////////////
CDiscardCmd::CDiscardCmd()
{
	command::SetBYTE(m_cdb, 0, (char)0xC4);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
	memset(m_data, 0, sizeof(m_data));
	m_pdata = m_data;
}
CDiscardCmd::CDiscardCmd(CDiscardCmd &in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CDiscardCmd::CDiscardCmd(char* cdb, long cdb_size, char* data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	for (long i = 0; i < (long)sizeof(m_data); i++) m_data[i] = (unsigned char)i;
}
CDiscardCmd::~CDiscardCmd()
{}
CDiscardCmd& CDiscardCmd::operator=(CDiscardCmd& in)
{
	copy(in);
	return *this;
}
void CDiscardCmd::main_window(char v)
{
	command::SetBYTE(m_pdata, 0, v);
}
void CDiscardCmd::window_identifier(char v)
{
	command::SetBit(m_pdata, 0, v, 0xf0);
}
char CDiscardCmd::window_identifier()
{
	return command::GetBit(m_pdata, 0, 0xf0);
}
void CDiscardCmd::resampling_after_scan(bool v)
{
	command::SetBit(m_pdata, 0, v ? 1 : 0, 0x4);
}
bool CDiscardCmd::resampling_after_scan()
{
	return command::GetBit(m_pdata, 0, 0x4) > 0;
}
void CDiscardCmd::resampling(bool v)
{
	command::SetBit(m_pdata, 0, v ? 1 : 0, 0x2);
}
bool CDiscardCmd::resampling()
{
	return command::GetBit(m_pdata, 0, 0x2)>0;
}
void CDiscardCmd::side(bool v)//false:front, true:back
{
	command::SetBit(m_pdata, 0, v ? 1 : 0, 0x1);
}
bool CDiscardCmd::side()
{
	return command::GetBit(m_pdata, 0, 0x1) > 0;
}
//////////////////////////////////////////////////////////////////////////
CSenseCmd::CSenseCmd()
{
	command::SetBYTE(m_cdb, 0, (char)0x3);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 14);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
	m_pdata=(unsigned char*)m_data;
	memset(m_data, 0, sizeof(m_data));
	command::SetBYTE(m_data, 0, (char)0xf0);
	command::SetBYTE(m_data, 7, 6);
}
CSenseCmd::CSenseCmd(CSenseCmd& in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CSenseCmd::CSenseCmd(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CSenseCmd::~CSenseCmd()
{
}
CSenseCmd& CSenseCmd::operator=(CSenseCmd& in)
{
	copy(in);
	return *this;
}
bool CSenseCmd::has_error()
{
	if (ILI()) return true;
	if (sense_key()) {
		if (additional_sense_code()||
		    additional_sense_code_qualifier()) {
			return true;
		}
	}
	return false;
}
void CSenseCmd::clear()
{
	set_error(0, 0, 0);
}
bool CSenseCmd::is_no_paper()
{
	if (!has_error()) return false;
	return (additional_sense_code()==(char)0x3A)&&(additional_sense_code_qualifier()==(char)0x00);
}
bool CSenseCmd::is_cover_open()
{
	if (!has_error()) return false;
	bool out = (additional_sense_code()==(char)0x80)&&(additional_sense_code_qualifier()==(char)0x01);
	return out;	
}
bool CSenseCmd::is_double_feed_error()
{
	if (!has_error()) return false;
	bool out = (additional_sense_code()==(char)0x81)&&(additional_sense_code_qualifier()==(char)0x01);
	return out;
}
bool CSenseCmd::is_bad_sequence_error()
{
	if (!has_error()) return false;
	bool out = (additional_sense_code()==(char)0x2C)&&(additional_sense_code_qualifier()==(char)0x00);
	return out;
}
bool CSenseCmd::is_bad_cdb_error()
{
    if (!has_error()) return false;
    bool out = (additional_sense_code()==(char)0x24)&&(additional_sense_code_qualifier()==(char)0x00);
    return out;
}
bool CSenseCmd::is_power_on_reset_error()
{
	if (!has_error()) return false;
	bool out = (additional_sense_code()==(char)0x29)&&(additional_sense_code_qualifier()==(char)0x00);
	return out;
}
bool CSenseCmd::is_jam_error()
{
	if (!has_error()) return false;
	bool out = (additional_sense_code() == (char)0x80) && (additional_sense_code_qualifier() == (char)0x00);
	return out;
}
int CSenseCmd::bad_sequence()
{
	return set_error((char)0x5, (char)0x2c, (char)0x00);
}
int CSenseCmd::nomemory()
{
	return set_error(0x5, (char)0x55, (char)0x00);
}
int CSenseCmd::invalid_param()
{
	return set_error((char)0x5, (char)0x26, (char)0x00);
}
int CSenseCmd::nopaper()
{
	return set_error((char)0x5, (char)0x3A, (char)0x00);
}
int CSenseCmd::cover_open()
{
	return set_error((char)0x3, (char)0x80, (char)0x01);
}
int CSenseCmd::jam()
{
	return set_error((char)0x3, (char)0x80, (char)0x00);
}
int CSenseCmd::doublefeed()
{
	return set_error((char)0x3, (char)0x81, (char)0x01);
}
int CSenseCmd::cancel()
{
    return set_error((char)0x7, (char)0x09, (char)0x01);
}
int CSenseCmd::set_error(char key, char code, char qual)
{
	command::SetBYTE(m_pdata, 0, (char)0xf0);
	command::SetBYTE(m_pdata, 1, 0);
	command::SetBYTE(m_pdata, 2, 0);
	command::SetBit(m_pdata, 2, key, (char)0xf);
	command::Set4BYTE(m_pdata, 3, 0);
	command::SetBYTE(m_pdata, 7, 6);
	command::Set4BYTE(m_pdata, 8, 0);
	command::SetBYTE(m_pdata, 12, code);
	command::SetBYTE(m_pdata, 13, qual);
	return CCommand::CMD_CHECKCONDITION;
}
bool CSenseCmd::ILI()
{
	return command::GetBit(m_pdata, 2, (char)0x20)>0;
}
char CSenseCmd::sense_key()
{
	return command::GetBit(m_pdata, 2, (char)0xf);
}
void  CSenseCmd::sense_key(char v)
{
	command::SetBit(m_pdata, 2, v, (char)0xf);
}
long CSenseCmd::information_bytes()
{
	return command::Get4BYTE(m_pdata, 3);
}
int CSenseCmd::information_bytes(long size)
{
	clear();
	ILI(true);
	command::Set4BYTE(m_pdata, 3, (int)size);
	return CCommand::CMD_CHECKCONDITION;
}
void CSenseCmd::ILI(bool sw)
{
	command::SetBit(m_pdata, 2, sw?1:0, (char)0x20);
}
void CSenseCmd::valid(char v)
{
	command::SetBit(m_pdata, 0, v, (char)0x80);
}
char CSenseCmd::valid()
{
	return command::GetBit(m_pdata, 0, (char)0x80);
}
void CSenseCmd::error_code(char v)
{
	command::SetBit(m_pdata, 0, v, (char)0x7f);
}
char CSenseCmd::error_code()
{
	return command::GetBit(m_pdata, 0, (char)0x7f);
}
void CSenseCmd::additional_sense_length(char v)
{
	command::SetBYTE(m_pdata, 7, v);
}
char CSenseCmd::additional_sense_length()
{
	return command::GetBYTE(m_pdata, 7);
}
short CSenseCmd::code()
{
	return command::Get2BYTE(m_data, 12);
}
char CSenseCmd::additional_sense_code()
{
	return command::GetBYTE(m_data, 12);
}
char CSenseCmd::additional_sense_code_qualifier()
{
	return command::GetBYTE(m_data, 13);
}
//////////////////////////////////////////////////////////////
CStreamCmd::CStreamCmd():m_buffer(NULL)
{
	command::SetBYTE(m_cdb, 0, (char)0x28);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	command::Set3BYTE(m_cdb, 6, 16);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size=10;
	m_buffer = new unsigned char[transfer_length()];
	if (m_buffer) {
		memset(m_buffer, 0, transfer_length());
		m_pdata = (unsigned char*)m_buffer;
	}
}
CStreamCmd::CStreamCmd(CStreamCmd& in):CCommand((CCommand&)in), m_buffer(NULL)
{
	command::SetBYTE(m_cdb, 0, (char)0x28);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	command::Set3BYTE(m_cdb, 6, 0);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size = 10;
	if (in.transfer_length()) {
		m_buffer = new unsigned char[in.transfer_length()];
		if (m_buffer) {
			m_pdata = m_buffer;
			transfer_length(in.transfer_length());
			copy(in);
		}
	}
}
CStreamCmd::CStreamCmd(char* cdb, long cdb_size, char* data, long data_size) :m_buffer(NULL)
{
	input(cdb, cdb_size, data, data_size);

}
CStreamCmd::CStreamCmd(char* in_data, long in_data_size): m_buffer(NULL)
{
	command::SetBYTE(m_cdb, 0, (char)0x28);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, (char)0);//transfer data type
	command::SetBYTE(m_cdb, 3, 0);
	command::Set2BYTE(m_cdb, 4, (char)0);//transfer identification
	command::Set3BYTE(m_cdb, 6, (int)in_data_size);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size = 10;
	m_pdata = (unsigned char*)in_data;
}
CStreamCmd::CStreamCmd(long len)
{
	command::SetBYTE(m_cdb, 0, (char)0x28);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, (char)0);//transfer data type
	command::SetBYTE(m_cdb, 3, 0);
	command::Set2BYTE(m_cdb, 4, (char)0);//transfer identification
	command::Set3BYTE(m_cdb, 6, (int)len);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size = 10;
	m_buffer = new unsigned char[transfer_length()];
	if (m_buffer) {
		memset(m_buffer, 0, transfer_length());
		m_pdata = (unsigned char*)m_buffer;
	}
}
CStreamCmd::CStreamCmd(long transfer_data_type, long transfer_identification):m_buffer(NULL)
{
	command::SetBYTE(m_cdb, 0, (char)0x28);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, (char)transfer_data_type);//transfer data type
	command::SetBYTE(m_cdb, 3, 0);
	command::Set2BYTE(m_cdb, 4, (char)transfer_identification);//transfer identification
	command::Set3BYTE(m_cdb, 6, (int)16);
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size=10;
	switch (transfer_data_type) {
	case FEEDING_OPTION:
	case BLANKPAGE_DETECTION:
		transfer_length(16);
		break;
	case MICR:break;
	case IMAGE:break;
	case AREAINFO:
		switch (transfer_identification) {
		case IMAGEAREA:
		case MARGIN:
		case PAPERINFO:
			transfer_length(16);
			break;
		case AREAINFO_4POINTS_BEFORE:
		case AREAINFO_4POINTS_AFTER:
		case AREAINFO_4POINTS2_AFTER:
        case AREAINFO_4POINTS2_BEFORE:
		default:
			transfer_length(32);
			break;
		}
		break;
	case EJECT:
		transfer_length(2);
		break;
	case SHADING:
		transfer_length(1024);
		break;
	case PANEL:
		switch (transfer_identification) {
		case 102:
			transfer_length(0x80);
			break;
        case 104:
        case 105:
        case 106:
			transfer_length(1024);
            break;
		default:
			transfer_length(8);
			break;
		}
		break;
	case USERDATA://SERVICEDATA
		transfer_length(128);
		break;
	case PAPER:
	case PATCHCODE:
		transfer_length(2);
		break;
	case IMPRINTER:
		transfer_length(98);
		break;	
	case COLOR_DETECTION:
		transfer_length(8);
		break;	
	default:break;
	}
	m_buffer = new unsigned char[transfer_length()];
	if (m_buffer) {
		memset(m_buffer, 0, transfer_length());
		m_pdata = (unsigned char*)m_buffer;
	}
}
CStreamCmd::~CStreamCmd()
{
	if (m_buffer) {
		delete [] m_buffer;
		m_buffer=NULL;
	}
}
void CStreamCmd::I_am_in(CCommand::EXEC_TYPE type)
{
	if (type == CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0x28);
	}
	else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0x2A);
	}
}
CStreamCmd& CStreamCmd::operator = (CStreamCmd& src)
{
	if (this!=&src) {
		if (transfer_length() != src.transfer_length()) {
			if (m_buffer) delete[] m_buffer;
			m_pdata = NULL;
			m_buffer = NULL;
			m_buffer = new unsigned char[src.transfer_length()];
			if (m_buffer) {
				m_pdata = m_buffer;
				memset(m_pdata, 0, src.transfer_length());
				transfer_length(src.transfer_length());
			}
		}
		copy((CCommand&)src);
	}
	return *this;
}
bool CStreamCmd::gamma_download()
{
	return command::GetBit(m_cdb, 4, (char)0x80)>0;
}
bool CStreamCmd::gamma_back()
{
	return command::GetBit(m_cdb, 4, (char)0x40)>0;
}
char CStreamCmd::gamma_colortype()
{
	return command::GetBit(m_cdb, 4, (char)0x3f);
}
bool CStreamCmd::patchcode()
{
	return command::GetBit(m_pdata, 0, (char)0x80)>0;
}
void CStreamCmd::patchcode(bool v)
{
	command::SetBit(m_pdata, 0, v?1:0, (char)0x80);
}
long CStreamCmd::patchcode_type()
{
	return command::GetBit(m_pdata, 0, (char)0xf);
}
void CStreamCmd::patchcode_type(long type)
{
	command::SetBit(m_pdata, 0, (char)type, (char)0xf);
}
//autosize info
long CStreamCmd::areainfo_upperleftx()
{
	return command::Get4BYTE(m_pdata, 0);
}
long CStreamCmd::areainfo_upperlefty()
{
	return command::Get4BYTE(m_pdata, 4);
}
long CStreamCmd::areainfo_width()
{
	return command::Get4BYTE(m_pdata, 8);
}
long CStreamCmd::areainfo_length()
{
	return command::Get4BYTE(m_pdata, 12);
}
void CStreamCmd::areainfo_upperleftx(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
void CStreamCmd::areainfo_upperlefty(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
void CStreamCmd::areainfo_width(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
void CStreamCmd::areainfo_length(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
//autosize info
long CStreamCmd::autosize_upperleftx()
{
	return command::Get4BYTE(m_pdata, 0);
}
long CStreamCmd::autosize_upperlefty()
{
	return command::Get4BYTE(m_pdata, 4);
}
long CStreamCmd::autosize_width()
{
	return command::Get4BYTE(m_pdata, 8);
}
long CStreamCmd::autosize_length()
{
	return command::Get4BYTE(m_pdata, 12);
}
void CStreamCmd::autosize_upperleftx(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
void CStreamCmd::autosize_upperlefty(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
void CStreamCmd::autosize_width(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
void CStreamCmd::autosize_length(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
long CStreamCmd::maximum_paper_length()
{
	return command::Get4BYTE(m_pdata, 16);
}
void CStreamCmd::maximum_paper_length(long v)
{
	command::Set4BYTE(m_pdata, 16, (int)v);
}
long CStreamCmd::vertical_scaling()
{
	return command::Get2BYTE(m_pdata, 44);
}
void CStreamCmd::vertical_scaling(long v)
{
	command::Set2BYTE(m_pdata, 44, (short)v);
}
long CStreamCmd::paper_counter()
{
	return command::Get4BYTE(m_pdata, 4);
}
void CStreamCmd::paper_counter(long v)
{
	return command::Set4BYTE(m_pdata, 4, (int)v);
}
long  CStreamCmd::parts1_counter()
{
	return command::Get4BYTE(m_pdata, 68);
}
void  CStreamCmd::parts1_counter(long v)
{
	command::Set4BYTE(m_pdata, 68, (int)v);
}
void CStreamCmd::parts1_time(long v)
{
	command::Set4BYTE(m_pdata, 72, (int)v);
}
long CStreamCmd::parts1_time()
{
	return command::Get4BYTE(m_pdata, 72);
}
long CStreamCmd::poweroff_time()
{
	return command::Get4BYTE(m_pdata, 56) & 0x7fffffff;
}
void CStreamCmd::poweroff_time(long time)
{
	long v = 0;
	if (m_pdata[56] & 0x80) {
		v = 0x80000000 | time;
	}
	else {
		v = time;
	}

	command::Set4BYTE(m_pdata, 56, (int)v);
}
long CStreamCmd::sleep_time()
{
	return command::Get4BYTE(m_pdata, 12) & 0x7fffffff;
}
void CStreamCmd::sleep_time(long time)
{
	long v = 0;
	if (m_pdata[12] & 0x80) {
		v = 0x80000000 | time;
	}
	else {
		v = time;
	}

	command::Set4BYTE(m_pdata, 12, (int)v);
}
long CStreamCmd::parts1_counter_limit()
{
	return command::Get4BYTE(m_pdata, 70);
}
void CStreamCmd::parts1_counter_limit(long v)
{
	command::Set4BYTE(m_pdata, 70, (int)v);
}
long CStreamCmd::enable_parts1_warning()
{
	return command::GetBit(m_pdata, 92, 0xf);
}
void CStreamCmd::enable_parts1_warning(long v)
{
	command::SetBit(m_pdata, 92, (char)v, 0xf);
}
//panel
bool CStreamCmd::non_sep_key()
{
    return command::GetBit(m_pdata, 1, (char)0x4)>0;
}
bool CStreamCmd::dfr_key()
{
    return command::GetBit(m_pdata, 1, (char)0x10)>0;
}
bool CStreamCmd::start_key()
{
	return command::GetBit(m_pdata, 0, (char)0x80)>0;
}
bool CStreamCmd::stop_key()
{
	return command::GetBit(m_pdata, 0, (char)0x40)>0;
}
bool CStreamCmd::up_key()
{
	return command::GetBit(m_pdata, 0, (char)0x8)>0;
}
bool CStreamCmd::down_key()
{
	return command::GetBit(m_pdata, 0, (char)0x4)>0;
}
bool CStreamCmd::left_key()
{
	return command::GetBit(m_pdata, 0, (char)0x2)>0;
}
bool CStreamCmd::right_key()
{
	return command::GetBit(m_pdata, 0, (char)0x1)>0;
}
void CStreamCmd::enable_stop_key(bool v)
{
	command::SetBit(m_pdata, 2, v, 2);
}
bool CStreamCmd::enalbe_stop_key()
{
	return command::GetBit(m_pdata, 2, 2)>0;
}
void CStreamCmd::set_or_clear(bool v)
{
	command::SetBit(m_pdata, 0, v, 1);
}
bool CStreamCmd::set_or_clear()
{
	return command::GetBit(m_pdata, 0, 1)>0;
}
void CStreamCmd::mode(char v)
{
	m_pdata[1]=v;
}
char CStreamCmd::mode()
{
	return m_pdata[1];
}
void CStreamCmd::title(wchar_t  *s)
{
	unsigned char *dst = (unsigned char *)m_pdata + 4;
	for (long i=0; s[i]; i++) {
		unsigned char *src = (unsigned char*)&s[i];
		dst[i*2] = src[0];
		dst[i*2+1] = src[1];
	}
}
void CStreamCmd::panel_json(wchar_t  *sjson)
{
    unsigned char *dst = (unsigned char *)m_pdata + 2;
    for (long i=0; sjson[i]; i++) {
        unsigned char *src = (unsigned char*)&sjson[i];
        dst[i*2] = src[0];
        dst[i*2+1] = src[1];
    }
}
char CStreamCmd::feeding_option()
{
	return command::GetBYTE(m_pdata, 1);
}
void CStreamCmd::feeding_option(char v)
{
	command::SetBYTE(m_pdata, 1, v);
}
char CStreamCmd::prescan()
{
	return command::GetBYTE(m_pdata, 2);
}
void CStreamCmd::prescan(char v)
{
	command::SetBYTE(m_pdata, 2, v);
}
//margin
long CStreamCmd::margin_left()
{
	return command::Get4BYTE(m_pdata, 0);
}
long CStreamCmd::margin_top()
{
	return command::Get4BYTE(m_pdata, 4);
}
long CStreamCmd::margin_right()
{
	return command::Get4BYTE(m_pdata, 8);
}
long CStreamCmd::margin_bottom()
{
	return command::Get4BYTE(m_pdata, 12);
}
void CStreamCmd::margin_left(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
void CStreamCmd::margin_top(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
void CStreamCmd::margin_right(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
void CStreamCmd::margin_bottom(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
//4points
long CStreamCmd::p4_upperleftx()
{
	return command::Get4BYTE(m_pdata, 0);
}
long CStreamCmd::p4_upperlefty()
{
	return command::Get4BYTE(m_pdata, 4);
}
long CStreamCmd::p4_upperrightx()
{
	return command::Get4BYTE(m_pdata, 8);
}
long CStreamCmd::p4_upperrighty()
{
	return command::Get4BYTE(m_pdata, 12);
}
long CStreamCmd::p4_lowerleftx()
{
	return command::Get4BYTE(m_pdata, 16);
}
long CStreamCmd::p4_lowerlefty()
{
	return command::Get4BYTE(m_pdata, 20);
}
long CStreamCmd::p4_lowerrightx()
{
	return command::Get4BYTE(m_pdata, 24);
}
long CStreamCmd::p4_lowerrighty()
{
	return command::Get4BYTE(m_pdata, 28);
}
void CStreamCmd::p4_upperleftx(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
void CStreamCmd::p4_upperlefty(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
void CStreamCmd::p4_upperrightx(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
void CStreamCmd::p4_upperrighty(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
void CStreamCmd::p4_lowerleftx(long v)
{
	command::Set4BYTE(m_pdata, 16, (int)v);
}
void CStreamCmd::p4_lowerlefty(long v)
{
	command::Set4BYTE(m_pdata, 20, (int)v);
}
void CStreamCmd::p4_lowerrightx(long v)
{
	command::Set4BYTE(m_pdata, 24, (int)v);
}
void CStreamCmd::p4_lowerrighty(long v)
{
	command::Set4BYTE(m_pdata, 28, (int)v);
}
	//color detection
char CStreamCmd::front_result()
{
	return command::GetBYTE(m_pdata, 0);
}
char CStreamCmd::front_color_pixels()
{
	return command::GetBYTE(m_pdata, 1);
}
long CStreamCmd::front_color_lines()
{
	return command::Get2BYTE(m_pdata, 2);
}
char CStreamCmd::back_result()
{
	return command::GetBYTE(m_pdata, 4);
}
char CStreamCmd::back_color_pixels()
{
	return command::GetBYTE(m_pdata, 5);
}
long CStreamCmd::back_color_lines()
{
	return command::Get2BYTE(m_pdata, 6);
}
void CStreamCmd::front_result(char v)
{
	command::SetBYTE(m_pdata, 0, v);
}
void CStreamCmd::front_color_pixels(char v)
{
	command::SetBYTE(m_pdata, 1, v);
}
void CStreamCmd::front_color_lines(long v)
{
	command::Set2BYTE(m_pdata, 2, (short)v);
}
void CStreamCmd::back_result(char v)
{
	command::SetBYTE(m_pdata, 4, v);
}
void CStreamCmd::back_color_pixels(char v)
{
	command::SetBYTE(m_pdata, 5, v);
}
void CStreamCmd::back_color_lines(long v)
{
	command::Set2BYTE(m_pdata, 6, (short)v);
}
//detect blank paper
long CStreamCmd::number_of_change_points_x_front()
{
	return command::Get4BYTE(m_pdata, 0);
}
long CStreamCmd::number_of_change_points_y_front()
{
	return command::Get4BYTE(m_pdata, 4);
}
long CStreamCmd::number_of_change_points_x_back()
{
	return command::Get4BYTE(m_pdata, 8);
}
long CStreamCmd::number_of_change_points_y_back()
{
	return command::Get4BYTE(m_pdata, 12);
}
long CStreamCmd::skipped_paper_counter()//vs original command.
{
	return command::Get4BYTE(m_pdata, 0);
}
void CStreamCmd::skipped_paper_counter(long v)//vs original command.
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
long CStreamCmd::paper_counter2()//vs original command.
{
	return command::Get4BYTE(m_pdata, 4);
}
void CStreamCmd::paper_counter2(long v)//vs original command.
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
void CStreamCmd::status_is(long status)//vs original command.
{
	command::SetBYTE(m_pdata, 8, (char)status);
}
long CStreamCmd::status_is()//vs original command.
{
	return command::GetBYTE(m_pdata, 8);
}
void CStreamCmd::is_scan_done(bool v)//vs original command.
{
	command::SetBit(m_pdata, 9, v, (char)0x1);
}
bool CStreamCmd::is_scan_done()//vs original command.
{
	return command::GetBit(m_pdata, 9, (char)0x1);		
}
void CStreamCmd::image_is_blankpage_front(long v)//vs original command.
{
	command::SetBit(m_pdata, 0, v?1:0, (char)0x1);
}
long CStreamCmd::image_is_blankpage_front()//vs original command.
{
	return command::GetBit(m_pdata, 0, (char)0x1);
}
void CStreamCmd::image_is_blankpage_back(long v)//vs original command.
{
	command::SetBit(m_pdata, 0, v?1:0, (char)0x2);
}
long CStreamCmd::image_is_blankpage_back()//vs original command.
{
	return command::GetBit(m_pdata, 0, (char)0x2);
}
void CStreamCmd::image_is(long front)//vs original command.
{
	command::SetBit(m_pdata, 0, (char)front, (char)0x4);
}
long CStreamCmd::image_is()//vs original command.
{
	return command::GetBit(m_pdata, 0, (char)0x4);
}
void CStreamCmd::angle_of_rotation_is(long angle)//vs original command.
{
	command::SetBit(m_pdata, 0, (char)angle, (char)0xf0);
}
long CStreamCmd::angle_of_rotation_is()//vs original command.
{
	return command::GetBit(m_pdata, 0, (char)0xf0);
}
void CStreamCmd::eject(bool eject)
{
	command::SetBit(m_pdata, 0, eject?1:0, (char)0x1);
}
bool CStreamCmd::eject()
{
	return command::GetBit(m_pdata, 0, (char)0x1)>0;
}
void CStreamCmd::doublefeed(bool v)
{
	command::SetBit(m_pdata, 0, v?1:0, (char)0x80);
}
bool CStreamCmd::doublefeed()
{
	return command::GetBit(m_pdata, 0, (char)0x80)>0;
}
long CStreamCmd::paper_length()
{
	return (long)command::Get4BYTE(m_pdata, 12);
}
void CStreamCmd::paper_length(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
void CStreamCmd::black_or_white(bool black)
{
	command::SetBit(m_pdata, 0, black?1:0, (char)0x40);
}
void CStreamCmd::black()
{
	black_or_white(true);
}
void CStreamCmd::white()
{
	black_or_white(false);
}
void CStreamCmd::rgb(CStreamCmd::RGB_TYPE rgb/*0:r, 1:g, 2:b*/)
{
	command::SetBit(m_pdata, 0, rgb, (char)0xc);
}
void CStreamCmd::side(bool front)
{
	command::SetBit(m_pdata, 0, front?0:1, (char)0x1);
}
void CStreamCmd::shading(char * pdata, long size)
{
	long s=size;
	if (s>10240) s=10240;
	memcpy(m_pdata+4, pdata, s);
}
//
char *CStreamCmd::micr_text()
{
	return (char*)data();
}
void CStreamCmd::micr_text(char *text)
{
	if (m_buffer) delete [] m_buffer;
	m_buffer=new unsigned char [strlen(text)+2];
	if (m_buffer==NULL) return;

	strcpy((char*)m_buffer, (char*)text);

	m_pdata=(unsigned char*)m_buffer;
}

void CStreamCmd::paper_is(bool bpaper)
{
	command::SetBit(m_pdata, 0, bpaper, (char)0x1);
}
bool CStreamCmd::paper_is_detected()
{
	return  command::GetBit(m_pdata, 0, (char)0x1)>0;
}

void CStreamCmd::attach_buffer(char *buffer, long size)
{
	////WriteLog(((char*)"attach_buffer(%d)"), size);
	m_buffer = (unsigned char*)buffer;
	m_pdata = (unsigned char*)m_buffer;
	//m_max_data_size=m_data_size=size;
	command::Set3BYTE(m_cdb, 6, (int)transfer_length());
}

char *CStreamCmd::serial_number()
{
	return (char*)(m_pdata+108);
}
void CStreamCmd::serial_number(char *s)
{
	char *d = (char*)(m_pdata + 108);
	while (*s) {
		*d = *s;
		d++;
		s++;
	}
}
char *CStreamCmd::data_of_imprint(char *work)
{
	return command::GetNString((char*)m_pdata, 2, 95, work);
}
CCommand *CStreamCmd::clone()
{
	if (data_size()==0) {
		return NULL;
	}
	CStreamCmd *p = new CStreamCmd(transfer_length());
	if (p==NULL) {
		return NULL;
	}
	*p=*this;
	return p;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMode::CMode()
{
	command::SetBYTE(m_cdb, 0, (char)0x1A);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBit(m_cdb, 2, 3/*value*/, (char)0x3F);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 12);
	command::SetBYTE(m_cdb, 5, 0);
	m_cdb_size=6;
	m_pdata=(unsigned char*)m_data;
	memset(m_data, 0, sizeof(m_data));	
}
CMode::CMode(CMode& in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) {
		sz = sizeof(m_data);
		allocation_length((unsigned char)sz);
	}
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CMode::CMode(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CMode::~CMode()
{
}
CMode& CMode::operator = (CMode& in)
{
	copy(in);
	return *this;
}
void CMode::I_am_in(CCommand::EXEC_TYPE type)
{
	if (type==CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0x1A);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBit(m_cdb, 2, 3/*value*/, (char)0x3F);
		command::SetBYTE(m_cdb, 3, 0);
		command::SetBYTE(m_cdb, 4, 12);
		command::SetBYTE(m_cdb, 5, 0);
	
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0x15);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBit(m_cdb, 1, 1/*value*/, (char)0x10);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		command::SetBYTE(m_cdb, 4, 12);
		command::SetBYTE(m_cdb, 5, 0);
	}
}
void CMode::mud(long v)
{
	command::Set2BYTE(m_pdata, 8, (short)v);
}
long CMode::mud()
{
	long out = command::Get2BYTE(m_pdata, 8);
	if (!out) {
		out=1200;
	}
	return out;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
CWindow::CWindow()
{
	command::SetBYTE(m_cdb, 0, (char)0x25);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBit(m_cdb, 1, 1/*value*/, (char)0x1);	
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);//0:front / 1:back
	command::Set3BYTE(m_cdb, 6, sizeof(m_data));
	command::SetBYTE(m_cdb, 9, 0);
	m_cdb_size=10;
	m_pdata=(unsigned char*)m_data;
	memset(m_data, 0, sizeof(m_data));
}
CWindow::CWindow(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CWindow::CWindow(CWindow &in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CWindow::~CWindow()
{
}
void CWindow::I_am_in(CCommand::EXEC_TYPE type)
{
	char id = line_5_and_8();
	if (type==CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0x25);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBit(m_cdb, 1, 1/*value*/, (char)0x1);	
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		command::SetBYTE(m_cdb, 4, 0);
		command::SetBYTE(m_cdb, 5, id);//0:front / 1:back
		command::Set3BYTE(m_cdb, 6, sizeof(m_data));
		command::SetBYTE(m_cdb, 9, 0);
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0x24);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		command::SetBYTE(m_cdb, 4, 0);
		command::SetBYTE(m_cdb, 5, 0);
		command::Set3BYTE(m_cdb, 6, sizeof(m_data));
		command::SetBYTE(m_cdb, 9, 0);
		command::SetBYTE(m_pdata, 8, id);
	}
}
CWindow  &CWindow::operator = (CWindow &src)
{
	copy((CCommand&)src);
	return *this;
}
bool CWindow::rif()
{
	return command::GetBit(m_pdata, 37, (char)0x80);
}
void CWindow::rif(bool v)
{
	command::SetBit(m_pdata, 37, v?true:false, (char)0x80);
}
char CWindow::threshold()
{
	return command::GetBYTE(m_pdata, 31);
}
void CWindow::threshold(char v)
{
	command::SetBYTE(m_pdata, 31, v);
}
bool CWindow::high_speed()
{
	return command::GetBit(m_pdata, 50, (char)0x80);
}
void CWindow::high_speed(bool v)
{
	command::SetBit(m_pdata, 50, v, (char)0x80);
}
long CWindow::compression_type()
{
	return command::GetBYTE(m_pdata, 40);
}
void CWindow::compression_type(long v)
{
	command::SetBYTE(m_pdata, 40, (char)v);
}
long CWindow::rotation()
{
	return command::Get2BYTE(m_pdata, 48);
}
void CWindow::rotation(long v)
{
	command::Set2BYTE(m_pdata, 48, (short)v);
}
long CWindow::compression_argument()
{
	return command::GetBYTE(m_pdata, 41);
}
void CWindow::compression_argument(long v)
{
	command::SetBYTE(m_pdata, 41, (char)v);
}
void CWindow::brightness(long v)
{
	if (v>255) v=255;
	else if (v<0) v=0;	
	command::SetBYTE(m_pdata, 30, (char)v);
}
void CWindow::contrast(long v)
{
	if (v>255) v=255;
	else if (v<0) v=0;
	command::SetBYTE(m_pdata, 32, (char)v);
}
long CWindow::brightness()
{
	return command::GetBYTE(m_pdata, 30);
}
long CWindow::contrast()
{
	return command::GetBYTE(m_pdata, 32);
}
void CWindow::AEmode(char v)
{
	command::SetBit(m_pdata, 50, v, (char)0x3);
}
char CWindow::AEmode()
{
	return command::GetBit(m_pdata, 50, (char)0x3);
}
bool CWindow::error_diffusion()
{
	return spp()==1&&bps()==1&&image_composition()==1;
}
void CWindow::error_diffusion(bool sw)
{
	if (spp()==1&&bps()==1) {
		if (sw) image_composition(1);
		else    image_composition(0);
	}
}
bool CWindow::ateii()
{
	return spp()==1&&bps()==1&&image_composition()==0&&AEmode()==3;
}
void CWindow::ateii(bool sw)
{
	if (spp()==1&&bps()==1) {
		
		if (sw) {
			image_composition(0);
			AEmode(3);
		} else {
			AEmode(0);
		}
	}
}
void CWindow::grc(bool v)
{
	command::SetBit(m_pdata, 50, v, (char)0x8);
}
bool CWindow::grc()
{
	return command::GetBit(m_pdata, 50, (char)0x8)>0;
}
bool CWindow::through_grc()
{
	return command::GetBit(m_pdata, 50, (char)0x4)>0;	
}
void CWindow::through_grc(bool v)
{
	command::SetBit(m_pdata, 50, v?1:0, (char)0x4);	
}
void CWindow::width(long w)
{
	command::Set4BYTE(m_pdata, 22, (int)w);
}
void CWindow::length(long l)
{
	command::Set4BYTE(m_pdata, 26, (int)l);
}
void CWindow::bpp(char bpp)
{
	command::SetBYTE(m_pdata, 34, bpp);
}
char  CWindow::bpp()
{
	return command::GetBYTE(m_pdata, 34);
}
void CWindow::spp(char v)
{
	if (v==3) {
		image_composition(5);//color
	} else if (v==1) {
		image_composition(2);//gray
	}
}
void CWindow::bps(char v)
{
	if (v==8) {
		if (spp()==3) {
			/*color*/
			image_composition(5);
			bpp(8);
		} else {
			/*gray*/
			image_composition(2);
			bpp(8);
		}
	} else if (v==1) {
		/*binary*/
		image_composition(0);
		bpp(1);
	}
}
long CWindow::width()
{
	return command::Get4BYTE(m_pdata, 22);
}
long CWindow::length()
{
	return command::Get4BYTE(m_pdata, 26);
}
char CWindow::image_composition()
{
	return command::GetBYTE(m_pdata, 33);
}
void CWindow::image_composition(char ic)
{
	command::SetBYTE(m_pdata, 33, ic);
}
long CWindow::spp()
{
	if (image_composition()==5) {
		return 3;
	} 
	return 1;
}
long CWindow::bps()
{
	if (image_composition()<2) {
		return 1;
	}
	return 8;
}
long CWindow::image_processing_progress()
{
	return command::Get4BYTE(m_pdata, 42);
}
void CWindow::image_processing_progress(long v)
{
	command::Set4BYTE(m_pdata, (int)v, 42);
}
char CWindow::line_5_and_8()
{
	char out=0;
	if (m_cdb[0]==0x25) {
		out= command::GetBYTE(m_cdb, 5);
	} else if (m_cdb[0]==0x24) {
		out= command::GetBYTE(m_pdata, 8);
	}
	return out;	
}
void CWindow::window_identifier(char v)
{
	if (m_cdb[0]==0x25) {
		command::SetBit(m_cdb, 5, v, 0xf0);	
	} else if (m_cdb[0]==0x24) {
		command::SetBit(m_pdata, 8, v, 0xf0);
	}
}
char CWindow::window_identifier()
{
	char out=0;
	if (m_cdb[0]==0x25) {
		out= command::GetBit(m_cdb, 5, 0xf0);
	} else if (m_cdb[0]==0x24) {
		out= command::GetBit(m_pdata, 8, 0xf0);
	}
	return out;
}
void CWindow::side(bool v)
{
	if (m_cdb[0]==0x25) {
		command::SetBit(m_cdb, 5, v?1:0, 0x1);
	} else if (m_cdb[0]==0x24) {
		command::SetBit(m_pdata, 8, v?1:0, 0x1);
	}
}
bool CWindow::side()
{
	char out=0;
	if (m_cdb[0]==0x25) {
		out= command::GetBit(m_cdb, 5, 0x1);
	} else if (m_cdb[0]==0x24) {
		out= command::GetBit(m_pdata, 8, 0x1);
	}
	return out;	
}
void CWindow::resampling(bool v)
{
	if (m_cdb[0]==0x25) {
		command::SetBit(m_cdb, 5, v?1:0, 0x2);	
	} else if (m_cdb[0]==0x24) {
		command::SetBit(m_pdata, 8, v?1:0, 0x2);
	}
}
bool CWindow::resampling()
{
	char out=0;
	if (m_cdb[0]==0x25) {
		out= command::GetBit(m_cdb, 5, 0x2);
	} else if (m_cdb[0]==0x24) {
		out= command::GetBit(m_pdata, 8, 0x2);
	}
	return out;	
}
void CWindow::resampling_after_scan(bool v)
{
	if (m_cdb[0]==0x25) {
		command::SetBit(m_cdb, 5, v?1:0, 0x4);
	} else if (m_cdb[0]==0x24) {
		command::SetBit(m_pdata, 8, v?1:0, 0x4);
	}
}
bool CWindow::resampling_after_scan()
{
	char out=0;
	if (m_cdb[0]==0x25) {
		out= command::GetBit(m_cdb, 5, 0x4);
	} else if (m_cdb[0]==0x24) {
		out= command::GetBit(m_pdata, 8, 0x4);
	}
	return out;	
}
bool CWindow::I_am_front_window()
{
	return !side();//true is back, false is front
}
bool CWindow::I_am_back_window()
{
	return !I_am_front_window();
}
short CWindow::xdpi()
{
	return command::Get2BYTE(m_pdata, 10);
}
void CWindow::xdpi(short dpi)
{
	command::Set2BYTE(m_pdata, 10, dpi);
}
short CWindow::ydpi()
{	
	return command::Get2BYTE(m_pdata, 12);
}
void CWindow::ydpi(short dpi)
{
	command::Set2BYTE(m_pdata, 12, dpi);
}
short CWindow::dpi()
{
	return xdpi();
}
void CWindow::dpi(short dpi)
{
	xdpi(dpi);
	ydpi(dpi);
}
void CWindow::xoffset(long x)
{
	command::Set4BYTE(m_pdata, 14, (int)x);
}
void CWindow::yoffset(long y)
{
	command::Set4BYTE(m_pdata, 18, (int)y);
}
long CWindow::xoffset()
{
	return (long)command::Get4BYTE(m_pdata, 14);
}
long CWindow::yoffset()
{
	return (long)command::Get4BYTE(m_pdata, 18);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
CScanMode::CScanMode(CScanMode::PAGE_CODE_TYPE page_code_value)
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=6;
	command::SetBYTE(m_cdb, 0, (char)0xD6);
	command::SetBYTE(m_cdb, 1, (char)0x10);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, (char)20);//ParameterListLength
	command::SetBYTE(m_cdb, 5, 0);
	page_code((char)page_code_value);
}
CScanMode::CScanMode(CScanMode& in)
{
	copy(in);
	m_cdb_size = 6;
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CScanMode::CScanMode()
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=6;
	command::SetBYTE(m_cdb, 0, (char)0xD6);
	command::SetBYTE(m_cdb, 1, (char)0x10);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, (char)20);//ParameterListLength
	command::SetBYTE(m_cdb, 5, 0);
	page_code((char)CScanMode::PAGE_CODE_OPTION);
}
CScanMode::CScanMode(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	for (unsigned long i=0; i<sizeof(m_data); i++) m_data[i]=(unsigned char)i;
}
CScanMode::~CScanMode()
{
}
CScanMode& CScanMode::operator=(CScanMode& in)
{
	copy(in);
	return *this;
}
char CScanMode::length(char page_code)
{
	struct {
		char page_code;
		char length;
	}tbl[] = {
		{CScanMode::PAGE_CODE_OPTION, 20},//scan
		{CScanMode::PAGE_CODE_SCAN, 20},
		{CScanMode::PAGE_CODE_SCAN2, 20},
		{CScanMode::PAGE_CODE_FILTER, 20},
		{CScanMode::PAGE_CODE_DATE, 20},
		{CScanMode::PAGE_CODE_MICR, 24},//micr
		{CScanMode::PAGE_CODE_OCR, 24},//ocr
		{CScanMode::PAGE_CODE_FILTER2, 24},//filter2
		{0,0}
	};
	for (long i=0; tbl[i].page_code; i++) {
		if (tbl[i].page_code==page_code) return tbl[i].length;
	}
	return 20;
}
bool CScanMode::dfd_uss()
{
    return command::GetBit(m_pdata, 3+DATA_BLOCK_OFFSET, (char)0x4);
}
void CScanMode::dfd_uss(bool v)
{
	command::SetBit(m_pdata, 3+DATA_BLOCK_OFFSET, v?1:0, (char)0x4);
}
bool CScanMode::dfd_length()
{
    return command::GetBit(m_pdata, 3+DATA_BLOCK_OFFSET, (char)0x1);
}
void CScanMode::dfd_length(bool v)
{
	command::SetBit(m_pdata, 3+DATA_BLOCK_OFFSET, v?1:0, (char)0x1);
}
bool CScanMode::platen()
{
	return command::GetBit(m_pdata, 6+DATA_BLOCK_OFFSET, (char)0x10);
}
void CScanMode::platen(bool p)
{
	command::SetBit(m_pdata, 6+DATA_BLOCK_OFFSET, p?1:0, (char)0x10);
}
long CScanMode::maxdocument()
{
	return command::Get2BYTE(m_pdata, 8+DATA_BLOCK_OFFSET);
}
void CScanMode::maxdocument(long count)
{
	command::Set2BYTE(m_pdata, 8+DATA_BLOCK_OFFSET, (short)count);
}
long CScanMode::maximum_number_of_documents_in_batch_mode()
{
	return maxdocument();
}
void CScanMode::maximum_number_of_documents_in_batch_mode(long v)
{
	maxdocument(v);
}
bool CScanMode::micr()
{
	return command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0x4)>0;
}
void CScanMode::micr(bool v)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, v?1:0, (char)0x4);
}
bool CScanMode::imprinter()
{
	return command::GetBit(m_pdata, 2+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
void CScanMode::imprinter(bool v)
{
	command::SetBit(m_pdata, 2+DATA_BLOCK_OFFSET, v?1:0, (char)0x1);
}
void CScanMode::page_code(char code)
{
	if (m_cdb[0]==0xD5) {
		//GetScanMode
		command::SetBit(m_cdb, 2, code, (char)0x3f);//page code
		command::SetBYTE(m_cdb, 4, (char)transfer_length());//allocation length
	} else {
		//DefineScanMode
		command::SetBit(m_pdata, 0+DATA_BLOCK_OFFSET, code, (char)0x3f);//page code
		command::SetBYTE(m_pdata, 1+DATA_BLOCK_OFFSET, (char)transfer_length());//parameter list length
	}
}
char CScanMode::page_code()
{
	char out=0x30;
	if (m_cdb[0]==0xD5) {
		//GetScanMode
		out = command::GetBit(m_cdb, 2, (char)0x3f);
	} else {
		//DefineScanMode
		out = command::GetBit(m_pdata, 0+DATA_BLOCK_OFFSET, (char)0x3f);
	}
	return out;
}
void CScanMode::I_am_in(CCommand::EXEC_TYPE type)
{
	if (type==CCommand::EXEC_READ) {
		//read
		char code = page_code();
		command::SetBYTE(m_cdb, 0, (char)0xD5);
		command::SetBYTE(m_cdb, 1, 0);	
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBit(m_cdb, 2, code, (char)0x3f);
		command::SetBYTE(m_cdb, 3, 0);
		//command::SetBYTE(m_cdb, 4, (char)m_data_size);//AllocationLength
		command::SetBYTE(m_cdb, 5, 0);
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0xD6);
		command::SetBYTE(m_cdb, 1, (char)0x10);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		//command::SetBYTE(m_cdb, 4, (char)m_data_size);//ParameterListLength
		command::SetBYTE(m_cdb, 5, 0);
	}
}
bool CScanMode::deskew()
{
	return command::GetBit(m_pdata, 3+DATA_BLOCK_OFFSET, (char)0x20)>0;
}
void CScanMode::deskew(bool sw)
{
	command::SetBit(m_pdata, 3+DATA_BLOCK_OFFSET, sw?1:0, (char)0x20);
}
bool CScanMode::feeding_direction()
{
	return command::GetBit(m_pdata, 10+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
void CScanMode::feeding_direction(bool sw)
{
	command::SetBit(m_pdata, 10+DATA_BLOCK_OFFSET, sw?1:0, (char)0x1);
}
char CScanMode::autosize_option()
{
	return command::GetBit(m_pdata, 7+DATA_BLOCK_OFFSET, (char)0xf0);
}
void CScanMode::autosize_option(char v)
{
	command::SetBit(m_pdata, 7+DATA_BLOCK_OFFSET, v, (char)0xf0);
}
char CScanMode::detect_slant_option()
{
	return command::GetBit(m_pdata, 7+DATA_BLOCK_OFFSET, (char)0xf);
}
void CScanMode::detect_slant_option(char v)
{
	command::SetBit(m_pdata, 7+DATA_BLOCK_OFFSET, v, (char)0xf);
}
char CScanMode::bothscanmode()
{
	return command::GetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET);
}
void CScanMode::bothscanmode(char v)
{
	command::SetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET, v);
}
char CScanMode::scanside()
{
	return command::GetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET);
}
void CScanMode::scanside(char v)
{
	command::SetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET, v);
}
bool CScanMode::duplex()
{
	return scanside()>=2;
}
void CScanMode::duplex(bool on)
{
	scanside(on?2:0);
}
bool CScanMode::batch()
{
	return command::GetBit(m_pdata, 6+DATA_BLOCK_OFFSET, (char)0x40)>0;
}
void CScanMode::batch(bool sw)
{
	command::SetBit(m_pdata, 6+DATA_BLOCK_OFFSET, sw?1:0, (char)0x40);
}
bool CScanMode::autosize()
{
	return command::GetBit(m_pdata, 6+DATA_BLOCK_OFFSET, (char)0x20)>0;
}
void CScanMode::autosize(bool a)
{
	command::SetBit(m_pdata, 6+DATA_BLOCK_OFFSET, a?1:0, (char)0x20);
}
char CScanMode::deskew_option()
{
	return command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0xf0);
}
void CScanMode::deskew_option(char v)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, v, (char)0xf0);
}
bool CScanMode::folio()
{
	return command::GetBit(m_pdata, 10+DATA_BLOCK_OFFSET, (char)0x2)>0;
}
void CScanMode::folio(bool v)
{
	command::SetBit(m_pdata, 10+DATA_BLOCK_OFFSET, v?1:0, (char)0x2);
}
bool CScanMode::skip_blank_page()
{
	return command::GetBit(m_pdata, 10+DATA_BLOCK_OFFSET, (char)0x4)>0;
}
void CScanMode::skip_blank_page(bool v)
{
	command::SetBit(m_pdata, 10+DATA_BLOCK_OFFSET, v?1:0, (char)0x4);
}
long CScanMode::blank_page_param()
{
	return command::GetBYTE(m_pdata, 15+DATA_BLOCK_OFFSET);
}
void CScanMode::blank_page_param(long v)
{
	command::SetBYTE(m_pdata, 15+DATA_BLOCK_OFFSET, (char)v);
}
//PAGE_CODE_FILTER
bool CScanMode::edgeemphasis()
{
	return command::GetBit(m_pdata, 0+DATA_BLOCK_OFFSET, (char)0x40);
}
void CScanMode::edgeemphasis(bool v)
{
	command::SetBit(m_pdata, 0+DATA_BLOCK_OFFSET, v, (char)0x40);
}
long CScanMode::intensity_of_edgeemphasis()
{
	return command::GetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET);
}
void CScanMode::intensity_of_edgeemphasis(long v)
{
	command::SetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET, (char)v);
}
CScanMode::COLOR_TYPE CScanMode::drop_out(CScanMode::SIDE_TYPE type)
{
	long index=7;
	if (type==CScanMode::BACK) index=8;
	return (CScanMode::COLOR_TYPE)command::GetBYTE(m_pdata, (int)index+DATA_BLOCK_OFFSET);
}
void CScanMode::drop_out(CScanMode::SIDE_TYPE type, CScanMode::COLOR_TYPE col)
{
	long index=7;
	if (type==CScanMode::BACK) index=8;
	command::SetBYTE(m_pdata, (int)index+DATA_BLOCK_OFFSET, col);
}
CScanMode::COLOR_TYPE CScanMode::emphasis(CScanMode::SIDE_TYPE type)
{
	long index=9;
	if (type==CScanMode::BACK) index=10;
	return (CScanMode::COLOR_TYPE)command::GetBYTE(m_pdata, (int)index+DATA_BLOCK_OFFSET);
}
void CScanMode::emphasis(CScanMode::SIDE_TYPE type, CScanMode::COLOR_TYPE col)
{
	long index=9;
	if (type==CScanMode::BACK) index=10;
	command::SetBYTE(m_pdata, (int)index+DATA_BLOCK_OFFSET, col);
}
bool CScanMode::notch_erasure()
{
	return command::GetBit(m_pdata, 13+DATA_BLOCK_OFFSET, (char)0x4);
}
void CScanMode::notch_erasure(bool v)
{
	command::SetBit(m_pdata, 13+DATA_BLOCK_OFFSET, v, (char)0x4);
}
bool CScanMode::dot_erasure()
{
	return command::GetBit(m_pdata, 13+DATA_BLOCK_OFFSET, (char)0x2);
}
void CScanMode::dot_erasure(bool v)
{
	command::SetBit(m_pdata, 13+DATA_BLOCK_OFFSET, v, (char)0x2);
}
//PAGE_CODE_FILTER2
char CScanMode::autocolor()
{
	return command::GetBit(m_pdata, 1+DATA_BLOCK_OFFSET, (char)0x1);
}
void CScanMode::autocolor(char v)
{
	command::SetBit(m_pdata, 1+DATA_BLOCK_OFFSET, v?1:0, (char)0x1);
}
char CScanMode::sensitivity_of_colorbinary()
{
	return command::GetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET);
}
void CScanMode::sensitivity_of_colorbinary(char v)
{
	command::SetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET, v);
}
char CScanMode::intensity_of_colorbinary()
{
	return command::GetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET);
}
void CScanMode::intensity_of_colorbinary(char v)
{
	command::SetBYTE(m_pdata, 3+DATA_BLOCK_OFFSET, v);
}
bool CScanMode::patch()
{
	return command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
void CScanMode::patch(bool sw)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, sw, (char)0x1);
}
long CScanMode::patch_orientation()//0:0, 1:90, 2:180, 3:270
{
	return command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0x6);
}
void CScanMode::patch_orientation(long ori)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)ori, (char)0x6);
}
char CScanMode::erase_bleedthrough()
{
	return command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0x8);
}
void CScanMode::erase_bleedthrough(char v)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, v, (char)0x8);
}
char CScanMode::erase_bleedthrough_level()
{
	return  command::GetBit(m_pdata, 4+DATA_BLOCK_OFFSET, (char)0xf0);
}
void CScanMode::erase_bleedthrough_level(char v)
{
	command::SetBit(m_pdata, 4+DATA_BLOCK_OFFSET, v, (char)0xf0);
}
void CScanMode::background_color_equalization(bool v)
{
	command::SetBit(m_pdata, 5+DATA_BLOCK_OFFSET, v, (char)0x1);
}
bool CScanMode::background_color_equalization()
{
	return command::GetBit(m_pdata, 5+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
bool CScanMode::auto_rotation()
{
	return command::GetBit(m_pdata, 5+DATA_BLOCK_OFFSET, (char)0x2);
}
void CScanMode::auto_rotation(bool on)
{
	command::SetBit(m_pdata, 5+DATA_BLOCK_OFFSET, on?1:0, (char)0x2);
}
char CScanMode::autocolor_type()
{
	return command::GetBYTE(m_pdata, 12+DATA_BLOCK_OFFSET);
}
void CScanMode::autocolor_type(char type)
{
	command::SetBYTE(m_pdata, 12+DATA_BLOCK_OFFSET, type);
}
char CScanMode::autocolor_binary_type()
{
	return command::GetBYTE(m_pdata, 13+DATA_BLOCK_OFFSET);
}
void CScanMode::autocolor_binary_type(char type)
{
	command::SetBYTE(m_pdata, 13+DATA_BLOCK_OFFSET, type);
}
char CScanMode::sensitivity_of_colorgray()
{
	return command::GetBYTE(m_pdata, 14+DATA_BLOCK_OFFSET);
}
void CScanMode::sensitivity_of_colorgray(char v)
{
	command::SetBYTE(m_pdata, 14+DATA_BLOCK_OFFSET, v);
}
char CScanMode::intensity_of_colorgray()
{
	return command::GetBYTE(m_pdata, 15+DATA_BLOCK_OFFSET);
}
void CScanMode::intensity_of_colorgray(char v)
{
	command::SetBYTE(m_pdata, 15+DATA_BLOCK_OFFSET, v);
}
long CScanMode::ftf()
{
	return command::GetBYTE(m_pdata, 16+DATA_BLOCK_OFFSET);
}
void CScanMode::ftf(long v)
{
	command::SetBYTE(m_pdata, 16+DATA_BLOCK_OFFSET, (char)v);
}
void CScanMode::max_ahead_pages(long v)
{
	command::SetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET, (char)v);
}
long CScanMode::max_ahead_pages()
{
	return command::GetBYTE(m_pdata, 2+DATA_BLOCK_OFFSET);
}
void CScanMode::disable_error_recovery_ex(bool v)
{
	command::SetBit(m_pdata, 3+DATA_BLOCK_OFFSET, v?1:0, (char)0x80);
}
bool CScanMode::disable_error_recovery_ex()
{
	return command::GetBit(m_pdata, 3+DATA_BLOCK_OFFSET, (char)0x80)?true:false;
}
//PAGE_CODE_MICR
bool CScanMode::micr_wave()
{
	return command::GetBit(m_pdata, 2+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
void CScanMode::micr_wave(bool v)
{
	command::SetBit(m_pdata, 2+DATA_BLOCK_OFFSET, v?1:0, (char)0x1);
}
//PAGE_CODE_OCR
bool CScanMode::ocr()
{
	return command::GetBit(m_pdata, 2+DATA_BLOCK_OFFSET, (char)0x1)>0;
}
void CScanMode::ocr(bool v)
{
	command::SetBit(m_pdata, 2+DATA_BLOCK_OFFSET, v?1:0, (char)0x1);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
CScanParam::CScanParam(CScanParam::PAGE_CODE_TYPE page_code_value, short id)
{
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0xE5);//operation code
	command::SetBYTE(m_cdb, 1, 0);//lun reserved
	command::SetBYTE(m_cdb, 2, page_code_value);//page code
	command::SetBYTE(m_cdb, 3, 0);//reserved
	command::Set2BYTE(m_cdb, 4, id);//identification
	command::Set3BYTE(m_cdb, 6, (int)16);//length
	command::SetBYTE(m_cdb, 9, 0);
	memset(m_data, 0, sizeof(m_data));
	m_pdata = (unsigned char*)m_data;
	m_cdb_size = 10;
}
CScanParam::CScanParam()
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=10;
	command::SetBYTE(m_cdb, 0, (char)0xE5);//operation code
	command::SetBYTE(m_cdb, 1, 0);//lun reserved
	command::SetBYTE(m_cdb, 2, PAGE_CODE_SCAN_BOTH);
	command::SetBYTE(m_cdb, 3, 0);//reserved
	command::Set2BYTE(m_cdb, 4, 0);//identification
	command::Set3BYTE(m_cdb, 6, (int)16);//length
	command::SetBYTE(m_cdb, 9, 0);
}
CScanParam::CScanParam(CScanParam &in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CScanParam::CScanParam(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	for (unsigned long i=0; i<sizeof(m_data); i++) m_data[i]=(unsigned char)i;
}
CScanParam::~CScanParam()
{
}
CScanParam& CScanParam::operator=(CScanParam& in)
{
	copy(in);
	return *this;
}
long CScanParam::maximum_paper_length()
{
	long out = command::Get4BYTE(m_pdata, 12);
	out = out&0x7fffffff;
	return out;
}
void CScanParam::maximum_paper_length(long v)
{
	long s = command::GetBit(m_pdata, 12, (char)0x80);
	command::Set4BYTE(m_pdata, 12, (int)v);
	command::SetBit(m_pdata, 12, s?1:0, (char)0x80);
}
bool CScanParam::side()
{
	return command::GetBit(m_cdb, 5, 1);
}
void CScanParam::side(bool v)
{
	command::SetBit(m_cdb, 5, v?true:false, 1);
}
long CScanParam::window_identifier()
{
    return command::GetBit(m_cdb, 5, 0xf0);
}
void CScanParam::window_identifier(long v)
{
	command::SetBit(m_cdb, 5, (char)v, 0xf0);
}
long CScanParam::ulx_of_paper()
{
	return command::Get4BYTE(m_pdata, 0);
}
void CScanParam::ulx_of_paper(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
long CScanParam::uly_of_paper()
{
	return command::Get4BYTE(m_pdata, 4);
}
void CScanParam::uly_of_paper(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
long CScanParam::width_of_paper()
{
	return command::Get4BYTE(m_pdata, 8);
}
void CScanParam::width_of_paper(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
long CScanParam::length_of_paper()
{
	return command::Get4BYTE(m_pdata, 12);
}
void CScanParam::length_of_paper(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}	
long CScanParam::left_margin()
{
	return command::Get4BYTE(m_pdata, 0);
}
void CScanParam::left_margin(long v)
{
	command::Set4BYTE(m_pdata, 0, (int)v);
}
long CScanParam::top_margin()
{
	return command::Get4BYTE(m_pdata, 4);
}
void CScanParam::top_margin(long v)
{
	command::Set4BYTE(m_pdata, 4, (int)v);
}
long CScanParam::right_margin()
{
	return command::Get4BYTE(m_pdata, 8);
}
void CScanParam::right_margin(long v)
{
	command::Set4BYTE(m_pdata, 8, (int)v);
}
long CScanParam::bottom_margin()
{
	return command::Get4BYTE(m_pdata, 12);
}
void CScanParam::bottom_margin(long v)
{
	command::Set4BYTE(m_pdata, 12, (int)v);
}
void CScanParam::detect_blank_paper(bool v)//sep resampling reading option
{
	command::SetBit(m_pdata, 7, v?1:0, (char)0x1);
}
bool CScanParam::detect_blank_paper()//sep resampling reading option
{
	return command::GetBit(m_pdata, 7, (char)0x1)>0;
}
bool CScanParam::patch()
{
	return command::GetBit(m_pdata, 4, (char)0x2);
}
void CScanParam::patch(bool sw)
{
	command::SetBit(m_pdata, 4, sw?1:0, (char)0x2);
}
void CScanParam::year(short v)
{
	command::Set2BYTE(m_pdata, 2, v);
}
void CScanParam::month(char v)
{
	command::SetBYTE(m_pdata, 4, v);
}
void CScanParam::day(char v)
{
	command::SetBYTE(m_pdata, 5, v);
}
void CScanParam::hour(char v)
{
	command::SetBYTE(m_pdata, 6, v);
}
void CScanParam::minutes(char v)
{
	command::SetBYTE(m_pdata, 7, v);
}
void CScanParam::second(char v)
{
	command::SetBYTE(m_pdata, 8, v);
}
short CScanParam::year()
{
	return command::Get2BYTE(m_pdata, 2);
}
char CScanParam::month()
{
	return command::GetBYTE(m_pdata, 4);
}
char CScanParam::day()
{
	return command::GetBYTE(m_pdata, 5);
}
char CScanParam::hour()
{
	return command::GetBYTE(m_pdata, 6);
}
char CScanParam::minutes()
{
	return command::GetBYTE(m_pdata, 7);
}
char CScanParam::second()
{
	return command::GetBYTE(m_pdata, 8);
}	
long CScanParam::disable_dfd_starty()
{
	return command::Get2BYTE(m_pdata, 7);
}
void CScanParam::disable_dfd_starty(long v)
{
	command::Set2BYTE(m_pdata, 7, (short)v);
}
long CScanParam::disable_dfd_endy()
{
	return command::Get2BYTE(m_pdata, 9);
}
void CScanParam::disable_dfd_endy(long v)
{
	command::Set2BYTE(m_pdata, 9, (short)v);
}
char CScanParam::gamma_mode()
{
	return command::GetBYTE(m_pdata, 11);
}
void CScanParam::gamma_mode(char v)
{
	command::SetBYTE(m_pdata, 11, v);
}
char CScanParam::color_gamma_mode()
{
	return command::GetBYTE(m_pdata, 12);
}
void CScanParam::color_gamma_mode(char v)
{
	command::SetBYTE(m_pdata, 12, v);
}
bool CScanParam::imprinter()//PAGE_CODE_OPTION
{
	return command::GetBit(m_pdata, 2, 1);
}
void CScanParam::imprinter(bool v)//PAGE_CODE_OPTION
{
	command::SetBit(m_pdata, 2, v?1:0, 1);
}
bool CScanParam::skew_detection()
{
	return command::GetBit(m_pdata, 3, (char)0x40)>0;
}
void CScanParam::skew_detection(bool v)
{
	command::SetBit(m_pdata, 3, v?1:0, (char)0x40);
}
bool CScanParam::double_feed_detection_length()
{
	return command::GetBit(m_pdata, 3, (char)0x1)>0;
}
void CScanParam::double_feed_detection_length(bool v)
{
	command::SetBit(m_pdata, 3, v?1:0, (char)0x1);
}
bool CScanParam::double_feed_detection_ultrasonic()
{
	return command::GetBit(m_pdata, 3, (char)0x4)>0;
}
void CScanParam::double_feed_detection_ultrasonic(bool v)
{
	command::SetBit(m_pdata, 3, v?1:0, (char)0x4);
}
char CScanParam::erase_bleedthrough()
{
	return command::GetBit(m_pdata, 1, 1)>0;
}
void CScanParam::erase_bleedthrough(char v)
{
	command::SetBit(m_pdata, 1, v?1:0, 1);
}
char CScanParam::erase_bleedthrough_level()
{
	return intensity_of_bleed_through();
}
void CScanParam::erase_bleedthrough_level(char v)
{
	intensity_of_bleed_through(v);
}
long CScanParam::intensity_of_bleed_through()
{
	return command::GetBYTE(m_pdata, 2);
}
void CScanParam::intensity_of_bleed_through(long v)
{
	command::SetBYTE(m_pdata, 2, (char)v);
}
bool CScanParam::platen()
{
	return command::GetBit(m_pdata, 6, (char)0x10)>0;
}
void CScanParam::platen(bool v)
{
	command::SetBit(m_pdata, 6, v?1:0, (char)0x10);
}
void CScanParam::identification(short id)
{
	command::Set2BYTE(m_cdb, 4, id);
}
short CScanParam::identification()
{
	return command::Get2BYTE(m_cdb, 4);
}
void CScanParam::page_code(char code)
{
	command::SetBYTE(m_cdb, 2, code);
}
char CScanParam::page_code()
{
	return command::GetBYTE(m_cdb, 2);
}
void CScanParam::I_am_in(CCommand::EXEC_TYPE type)
{	
	if (type==CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0xE4);
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0xE5);
	}
}
//imprinter
void CScanParam::headid(char id)//PAGE_CODE_IMPRINTER_HEAD
{
	command::SetBit(m_cdb, 4, id, (char)0xf);
}
char CScanParam::headid()//PAGE_CODE_IMPRINTER_HEAD
{
	return command::GetBit(m_cdb, 4, (char)0xf);
}
char CScanParam::print_timing()//PAGE_CODE_IMPRINTER_HEAD
{
	return command::GetBit(m_pdata, 9, (char)0xf);
}
void CScanParam::print_timing(char v)//PAGE_CODE_IMPRINTER_HEAD
{
	command::SetBit(m_pdata, 9, v, (char)0xf);
}
char *CScanParam::strings(char *work, long work_size)//PAGE_CODE_IMPRINTER_TEXT
{
	command::GetNString((char*)m_pdata, 45	, 95, work);
	return work;
}
void CScanParam::strings(char *str)//PAGE_CODE_IMPRINTER_TEXT
{
	memset(m_pdata+45, 0, 95);
	command::SetString((char*)m_pdata, 45, str);
}
void CScanParam::xdpi(long v)//PAGE_CODE_IMPRINTER_TEXT
{
	command::Set2BYTE(m_pdata, 1, (short)v);
}
long CScanParam::xdpi()//PAGE_CODE_IMPRINTER_TEXT
{
	return command::Get2BYTE(m_pdata, 1);
}
void CScanParam::ydpi(long v)//PAGE_CODE_IMPRINTER_TEXT
{
	command::Set2BYTE(m_pdata, 3, (short)v);
}
long CScanParam::ydpi()//PAGE_CODE_IMPRINTER_TEXT
{
	return command::Get2BYTE(m_pdata, 3);
}
void CScanParam::format(char v)//PAGE_CODE_IMPRINTER_TEXT
{
	command::SetBit(m_pdata, 9, v, (char)0x30);
}
char CScanParam::format()//PAGE_CODE_IMPRINTER_TEXT
{
	return command::GetBit(m_pdata, 9, (char)0x30);
}
char CScanParam::fontsize()
{
	return command::GetBYTE(m_pdata, 10);
}
void CScanParam::fontsize(char v)
{
	command::SetBYTE(m_pdata, 10, v);
}
void CScanParam::countup_timing1(char v)//PAGE_CODE_IMPRINTER_TEXT
{
    command::SetBYTE(m_pdata, 12, v);
}
char CScanParam::countup_timing1()//PAGE_CODE_IMPRINTER_TEXT
{
    return command::GetBYTE(m_pdata, 12);
}
void CScanParam::countup_amount1(long v)//PAGE_CODE_IMPRINTER_TEXT
{
	command::Set4BYTE(m_pdata, 13, (int)v);
}
long CScanParam::countup_amount1()//PAGE_CODE_IMPRINTER_TEXT
{
    return command::Get4BYTE(m_pdata, 13);
}
void CScanParam::reset_timing1(char v)//PAGE_CODE_IMPRINTER_TEXT
{
    command::SetBYTE(m_pdata, 17, v);
}
char CScanParam::reset_timing1()//PAGE_CODE_IMPRINTER_TEXT
{
    return command::GetBYTE(m_pdata, 17);
}
void CScanParam::reset_value1(long  v)//PAGE_CODE_IMPRINTER_TEXT
{
	command::Set4BYTE(m_pdata, 18, (int)v);
}
long CScanParam::reset_value1()//PAGE_CODE_IMPRINTER_TEXT
{
    return command::Get4BYTE(m_pdata, 18);
}
void CScanParam::reset_timing2(char v)//PAGE_CODE_IMPRINTER_TEXT
{
    command::SetBYTE(m_pdata, 28, v);
}
char CScanParam::reset_timing2()//PAGE_CODE_IMPRINTER_TEXT
{
    return command::GetBYTE(m_pdata, 28);
}
bool CScanParam::imp_enable()//PAGE_CODE_IMPRINTER_TEXT and AGE_CODE_IMPRINTER_HEAD
{
	return command::GetBit(m_data, 0, (char)0x1);
}
void CScanParam::imp_enable(bool b)//PAGE_CODE_IMPRINTER_TEXT and AGE_CODE_IMPRINTER_HEAD
{
	command::SetBit(m_data, 0, b?1:0, (char)0x1);
}
//PAGE_CODE_DOUBLEFEED
void CScanParam::indifferent_start_position_of_Y_axis(unsigned short v)
{
	command::Set2BYTE(m_pdata, 7, v);
}
unsigned short CScanParam::indifferent_start_position_of_Y_axis()
{
	return command::Get2BYTE(m_pdata, 7);
}
void CScanParam::indifferent_end_position_of_Y_axis(unsigned short v)
{
	command::Set2BYTE(m_pdata, 9, v);
}
unsigned short CScanParam::indifferent_end_position_of_Y_axis()
{
	return command::Get2BYTE(m_pdata, 9);
}
void CScanParam::indifferent_start_position_of_X_axis(unsigned short v)
{
	command::Set2BYTE(m_pdata, 11, v);
}
unsigned short CScanParam::indifferent_start_position_of_X_axis()
{
	return command::Get2BYTE(m_pdata, 11);
}
void CScanParam::indifferent_end_position_of_X_axis(unsigned short v)
{
	command::Set2BYTE(m_pdata, 13, v);
}
unsigned short CScanParam::indifferent_end_position_of_X_axis()
{
	return command::Get2BYTE(m_pdata, 13);
}
//scan sep
char CScanParam::autocolor()
{
	return command::GetBit(m_pdata, 1, (char)0x1);
}
void CScanParam::autocolor(char v)
{
	command::SetBit(m_pdata, 1, v?1:0, (char)0x1);
}
char CScanParam::sensitivity_of_autocolor()
{
	return command::GetBYTE(m_pdata, 2);
}
void CScanParam::sensitivity_of_autocolor(char v)
{
	command::SetBYTE(m_pdata, 2, v);
}
char CScanParam::intensity_of_autocolor()
{
	return command::GetBYTE(m_pdata, 3);
}
void CScanParam::intensity_of_autocolor(char v)
{
	command::SetBYTE(m_pdata, 3, v);
}
char CScanParam::sensitivity_of_colorbinary()
{
	return sensitivity_of_autocolor();
}
void CScanParam::sensitivity_of_colorbinary(char v)
{
	sensitivity_of_autocolor(v);
}
char CScanParam::intensity_of_colorbinary()
{
	return intensity_of_autocolor();
}
void CScanParam::intensity_of_colorbinary(char v)
{
	intensity_of_autocolor(v);
}
bool CScanParam::noise_remove()
{
	return command::GetBit(m_pdata, 10, 0x1);
}
void CScanParam::noise_remove(bool v)
{
	command::SetBit(m_pdata, 10, v ? 1 : 0, 0x1);
}
char CScanParam::noise_remove_level()
{
	return command::GetBYTE(m_pdata, 11);
}
void CScanParam::noise_remove_level(char v)
{
	command::SetBYTE(m_pdata, 11, v);
}
bool CScanParam::background()
{
	return command::GetBit(m_pdata, 8, 1);
}
void CScanParam::background(bool v)
{
	command::SetBit(m_pdata, 8, v ? 1 : 0, 1);
}
bool CScanParam::deskew()
{
	return command::GetBit(m_pdata, 6, (char)0x8);
}
void CScanParam::deskew(bool sw)
{
	command::SetBit(m_pdata, 6, sw?1:0, (char)0x8);
}
bool CScanParam::batch()
{
	return command::GetBit(m_pdata, 6, (char)0x40)>0;
}
void CScanParam::batch(bool sw)
{
	command::SetBit(m_pdata, 6, sw?1:0, (char)0x40);
}
long CScanParam::maximum_number_of_documents_in_batch_mode()
{
	return command::Get2BYTE(m_pdata, 8);
}
void CScanParam::maximum_number_of_documents_in_batch_mode(long v)
{
	command::Set2BYTE(m_pdata, 8, (short)v);
}
bool CScanParam::passport_carriersheet()
{
	return command::GetBit(m_pdata, 6, 0x02) > 0;
}
void CScanParam::passport_carriersheet(bool v)
{
	command::SetBit(m_pdata, 6, v ? 1 : 0, 0x02);
}
bool CScanParam::standard_carriersheet()
{
	return command::GetBit(m_pdata, 7, 0x01) > 0;
}
void CScanParam::standard_carriersheet(bool v)
{
	command::SetBit(m_pdata, 7, v ? 1 : 0, 0x01);
}
char CScanParam::autosize()
{
	return command::GetBit(m_pdata, 6, (char)0x30);
}
void CScanParam::autosize(char a)
{
	command::SetBit(m_pdata, 6, a, (char)0x30);
}
void CScanParam::black_dot_erasure(bool v)
{
	command::SetBit(m_pdata, 13, v?1:0, (char)0x1);
}
bool CScanParam::black_dot_erasure()
{
	return command::GetBit(m_pdata, 13, (char)0x1);
}
void CScanParam::white_dot_erasure(bool v)
{
	command::SetBit(m_pdata, 13, v?1:0, (char)0x2);
}
bool CScanParam::white_dot_erasure()
{
	return command::GetBit(m_pdata, 13, (char)0x2);
}
void CScanParam::notch_erasure(bool v)
{
	command::SetBit(m_pdata, 13, v?1:0, (char)0x4);
}
bool CScanParam::notch_erasure()
{
	return command::GetBit(m_pdata, 13, (char)0x4);
}
void CScanParam::moire_reduction(bool b)
{
	command::SetBit(m_pdata, 5, b?1:0, (char)0x1);
}
bool CScanParam::moire_reduction()
{
	return command::GetBit(m_pdata, 5, (char)0x1);
}
char CScanParam::deskew_option()
{
	return command::GetBYTE(m_pdata, 8);
}
void CScanParam::deskew_option(char v)
{
	command::SetBYTE(m_pdata, 8, v);
}
char CScanParam::drop_out()
{
	return command::GetBYTE(m_pdata, 9);
}
void CScanParam::drop_out(char v)
{
	command::SetBYTE(m_pdata, 9, v);
}
char CScanParam::emphasis()
{
	return command::GetBYTE(m_pdata, 10);
}
void CScanParam::emphasis(char v)
{
	command::SetBYTE(m_pdata, 10, v);
}
char CScanParam::emphasis_color()
{
	return emphasis();
}
void CScanParam::emphasis_color(char v)
{
	emphasis(v);
}
long CScanParam::intensity_of_edgeemphasis()
{
	return command::GetBYTE(m_pdata, 4);
}
void CScanParam::intensity_of_edgeemphasis(long v)
{
	command::SetBYTE(m_pdata, 4, (char)v);
}
bool CScanParam::edgeemphasis()
{
	return command::GetBit(m_pdata, 3, (char)0x1)>0;
}
void CScanParam::edgeemphasis(bool v)
{
	command::SetBit(m_pdata, 3, v?1:0, (char)0x1);
}
bool CScanParam::nsf()
{
	return command::GetBit(m_pdata, 6, (char)0x80)>0;
}
void CScanParam::nsf(bool v)
{
	command::SetBit(m_pdata, 6, v?1:0, (char)0x80);
}
bool CScanParam::thinpaper()
{
	return command::GetBit(m_pdata, 6, (char)0x4)>0;
}
void CScanParam::thinpaper(bool v)
{
	command::SetBit(m_pdata, 6, v?1:0, (char)0x4);
}
bool CScanParam::thickpaper()
{
	return command::GetBit(m_pdata, 6, (char)0x2)>0;
}
void CScanParam::thickpaper(bool v)
{
	command::SetBit(m_pdata, 6, v?1:0, (char)0x2);
}
bool CScanParam::error_recovery()
{
	return command::GetBit(m_pdata, 5, (char)0x20)>0;
}
void CScanParam::error_recovery(bool v)
{
	command::SetBit(m_pdata, 5, v?1:0, (char)0x20);
}
bool CScanParam::error_recovery_ex()
{
	return command::GetBit(m_pdata, 5, (char)0x40)>0;
}
void CScanParam::error_recovery_ex(bool v)
{
	command::SetBit(m_pdata, 5, v?1:0, (char)0x40);
}
bool CScanParam::continue_scan()
{
	return command::GetBit(m_pdata, 5, (char)0x10)>0;
}
void CScanParam::continue_scan(bool v)
{
	command::SetBit(m_pdata, 5, v?1:0, (char)0x10);
}
void CScanParam::dfd_retry(long v)
{
	command::SetBit(m_pdata, 3, v?1:0, (char)0x08);
}
long CScanParam::dfd_retry()
{
	return command::GetBit(m_pdata, 3, (char)0x08);
}
////////////////////////
//
CAdjustCmd::CAdjustCmd(long length, long id)
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=10;

	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0xE0);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	m_id = id;
	command::Set2BYTE(m_cdb, 4, (short)id);
	command::Set3BYTE(m_cdb, 6, (int)length);
	command::SetBYTE(m_cdb, 9, 0);	
}
CAdjustCmd::CAdjustCmd(CAdjustCmd& in):m_id(0)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CAdjustCmd::~CAdjustCmd()
{
}
CAdjustCmd& CAdjustCmd::operator=(CAdjustCmd& in)
{
	copy(in);
	return *this;
}
void CAdjustCmd::transfer_identification(long v)
{
	command::Set2BYTE(m_cdb, 4, (short)v);
}
void CAdjustCmd::parameter_list_length(long v)
{
    command::Set3BYTE(m_cdb, 6, (int)v);
}
void CAdjustCmd::I_am_in(CCommand::EXEC_TYPE type)
{
	if (type==CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0xE0);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		command::Set2BYTE(m_cdb, 4, (short)m_id);
		//command::Set3BYTE(m_cdb, 6, (int)m_data_size);
		command::SetBYTE(m_cdb, 9, 0);
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0xE1);
		command::SetBYTE(m_cdb, 1, 0);
		command::SetBYTE(m_cdb, 2, 0);
		command::SetBYTE(m_cdb, 3, 0);
		command::Set2BYTE(m_cdb, 4, (short)m_id);
		//command::Set3BYTE(m_cdb, 6, (int)m_data_size);
		command::SetBYTE(m_cdb, 9, 0);
	}
}
char CAdjustCmd::gain1_f()
{
	return command::GetBYTE(m_pdata, 0);
}
char CAdjustCmd::gain2_f()
{
	return command::GetBYTE(m_pdata, 1);
}
char CAdjustCmd::gain3_f()
{
	return command::GetBYTE(m_pdata, 2);
}
char CAdjustCmd::gain4_f()
{
	return command::GetBYTE(m_pdata, 3);
}
char CAdjustCmd::offset1_f()
{
	return command::GetBYTE(m_pdata, 4);
}
char CAdjustCmd::offset2_f()
{
	return command::GetBYTE(m_pdata, 5);
}
char CAdjustCmd::offset3_f()
{
	return command::GetBYTE(m_pdata, 6);
}
char CAdjustCmd::offset4_f()
{
	return command::GetBYTE(m_pdata, 7);
}
short CAdjustCmd::red_led_f()
{
	return command::Get2BYTE(m_pdata, 8);
}
short CAdjustCmd::green_led_f()
{
	return command::Get2BYTE(m_pdata, 10);
}
short CAdjustCmd::blue_led_f()
{
	return command::Get2BYTE(m_pdata, 12);
}
short CAdjustCmd::led_f1()
{
	return command::Get2BYTE(m_pdata, 14);
}
short CAdjustCmd::led_f2()
{
	return command::Get2BYTE(m_pdata, 16);
}
short CAdjustCmd::led_f3()
{
	return command::Get2BYTE(m_pdata, 18);
}
void CAdjustCmd::offset1_b2(char v)
{
	command::SetBYTE(m_pdata, 64, v);
}
void CAdjustCmd::offset2_b2(char v)
{
	command::SetBYTE(m_pdata, 65, v);
}
void CAdjustCmd::offset3_b2(char v)
{
	command::SetBYTE(m_pdata, 66, v);
}
void CAdjustCmd::offset4_b2(char v)
{
	command::SetBYTE(m_pdata, 67, v);
}
void CAdjustCmd::offset5_b2(char v)
{
	command::SetBYTE(m_pdata, 68, v);
}
void CAdjustCmd::offset6_b2(char v)
{
	command::SetBYTE(m_pdata, 69, v);
}
void CAdjustCmd::red_led_b2(short v)
{
	command::Set2BYTE(m_pdata, 80, v);
}
void CAdjustCmd::green_led_b2(short v)
{
	command::Set2BYTE(m_pdata, 82, v);
}
void CAdjustCmd::blue_led_b2(short v)
{
	command::Set2BYTE(m_pdata, 84, v);
}
void CAdjustCmd::current_led_b(short v)
{
	command::Set2BYTE(m_pdata, 86, v);
}
void CAdjustCmd::red_led_f2(short v)
{
	command::Set2BYTE(m_pdata, 32, v);
}
void CAdjustCmd::green_led_f2(short v)
{
	command::Set2BYTE(m_pdata, 34, v);
}
void CAdjustCmd::blue_led_f2(short v)
{
	command::Set2BYTE(m_pdata, 36, v);
}
void CAdjustCmd::current_led_f(short v)
{
	command::Set2BYTE(m_pdata, 38, v);
}
void CAdjustCmd::offset1_f2(char v)
{
	command::SetBYTE(m_pdata, 16, v);
}
void CAdjustCmd::offset2_f2(char v)
{
	command::SetBYTE(m_pdata, 17, v);
}
void CAdjustCmd::offset3_f2(char v)
{
	command::SetBYTE(m_pdata, 18, v);
}
void CAdjustCmd::offset4_f2(char v)
{
	command::SetBYTE(m_pdata, 19, v);
}
void CAdjustCmd::offset5_f2(char v)
{
	command::SetBYTE(m_pdata, 20, v);
}
void CAdjustCmd::offset6_f2(char v)
{
	command::SetBYTE(m_pdata, 21, v);
}
void CAdjustCmd::gain1_f(char v)
{
	command::SetBYTE(m_pdata, 0, v);
}
void CAdjustCmd::gain2_f(char v)
{
	command::SetBYTE(m_pdata, 1, v);
}
void CAdjustCmd::gain3_f(char v)
{
	command::SetBYTE(m_pdata, 2, v);
}
void CAdjustCmd::gain4_f(char v)
{
	command::SetBYTE(m_pdata, 3, v);
}
void CAdjustCmd::offset1_f(char v)
{
	command::SetBYTE(m_pdata, 4, v);
}
void CAdjustCmd::offset2_f(char v)
{
	command::SetBYTE(m_pdata, 5, v);
}
void CAdjustCmd::offset3_f(char v)
{
	command::SetBYTE(m_pdata, 6, v);
}
void CAdjustCmd::offset4_f(char v)
{
	command::SetBYTE(m_pdata, 7, v);
}
void CAdjustCmd::red_led_f(short v)
{
	command::Set2BYTE(m_pdata, 8, v);
}
void CAdjustCmd::green_led_f(short v)
{
	command::Set2BYTE(m_pdata, 10, v);
}
void CAdjustCmd::blue_led_f(short v)
{
	command::Set2BYTE(m_pdata, 12, v);
}
void CAdjustCmd::led_f1(short v)
{
	command::Set2BYTE(m_pdata, 14, v);
}
void CAdjustCmd::led_f2(short v)
{
	command::Set2BYTE(m_pdata, 16, v);
}
void CAdjustCmd::led_f3(short v)
{
	command::Set2BYTE(m_pdata, 18, v);
}
char CAdjustCmd::gain1_b()
{
	return command::GetBYTE(m_pdata, 20);
}
char CAdjustCmd::gain2_b()
{
	return command::GetBYTE(m_pdata, 21);
}
char CAdjustCmd::gain3_b()
{
	return command::GetBYTE(m_pdata, 22);
}
char CAdjustCmd::gain4_b()
{
	return command::GetBYTE(m_pdata, 23);
}
char CAdjustCmd::offset1_b()
{
	return command::GetBYTE(m_pdata, 24);
}
char CAdjustCmd::offset2_b()
{
	return command::GetBYTE(m_pdata, 25);
}
char CAdjustCmd::offset3_b()
{
	return command::GetBYTE(m_pdata, 26);
}
char CAdjustCmd::offset4_b()
{
	return command::GetBYTE(m_pdata, 27);
}
short CAdjustCmd::red_led_b()
{
	return command::Get2BYTE(m_pdata, 28);
}
short CAdjustCmd::green_led_b()
{
	return command::Get2BYTE(m_pdata, 30);
}
short CAdjustCmd::blue_led_b()
{
	return command::Get2BYTE(m_pdata, 32);
}
short CAdjustCmd::led_b1()
{
	return command::Get2BYTE(m_pdata, 34);
}
short CAdjustCmd::led_b2()
{
	return command::Get2BYTE(m_pdata, 36);
}
short CAdjustCmd::led_b3()
{
	return command::Get2BYTE(m_pdata, 38);
}
void CAdjustCmd::gain1_b(char v)
{
	command::SetBYTE(m_pdata, 20, v);
}
void CAdjustCmd::gain2_b(char v)
{
	command::SetBYTE(m_pdata, 21, v);
}
void CAdjustCmd::gain3_b(char v)
{
	command::SetBYTE(m_pdata, 22, v);
}
void CAdjustCmd::gain4_b(char v)
{
	command::SetBYTE(m_pdata, 23, v);
}
void CAdjustCmd::offset1_b(char v)
{
	command::SetBYTE(m_pdata, 24, v);
}
void CAdjustCmd::offset2_b(char v)
{
	command::SetBYTE(m_pdata, 25, v);
}
void CAdjustCmd::offset3_b(char v)
{
	command::SetBYTE(m_pdata, 26, v);
}
void CAdjustCmd::offset4_b(char v)
{
	command::SetBYTE(m_pdata, 27, v);
}
void CAdjustCmd::red_led_b(short v)
{
	command::Set2BYTE(m_pdata, 28, v);
}
void CAdjustCmd::green_led_b(short v)
{
	command::Set2BYTE(m_pdata, 30, v);
}
void CAdjustCmd::blue_led_b(short v)
{
	command::Set2BYTE(m_pdata, 32, v);
}
void CAdjustCmd::led_b1(short v)
{
	command::Set2BYTE(m_pdata, 34, v);
}
void CAdjustCmd::led_b2(short v)
{
	command::Set2BYTE(m_pdata, 36, v);
}
void CAdjustCmd::led_b3(short v)
{
	command::Set2BYTE(m_pdata, 38, v);
}
////////////////////////
//
CGetScannerStatusCmd::CGetScannerStatusCmd()
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=6;
	command::SetBYTE(m_cdb, 0, (char)0xC5);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::Set2BYTE(m_cdb, 4, 3);
	command::Set2BYTE(m_cdb, 6, (short)sizeof(m_data));
	command::SetBYTE(m_cdb, 9, 0);
}
CGetScannerStatusCmd::CGetScannerStatusCmd(CGetScannerStatusCmd& in)
{
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CGetScannerStatusCmd::CGetScannerStatusCmd(char * cdb, long cdb_size, char * data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	memset(m_data, 0, sizeof(m_data));
}
CGetScannerStatusCmd::~CGetScannerStatusCmd()
{
}
long CGetScannerStatusCmd::bufferred_image_count()
{
	return command::GetBYTE(m_pdata, 1);
}
void CGetScannerStatusCmd::bufferred_image_count(long v)
{
	command::SetBYTE(m_pdata, 1, (char)v);
}
bool CGetScannerStatusCmd::error()
{
	return command::GetBit(m_pdata, 0, (char)0x40);
}
void CGetScannerStatusCmd::error(bool err)
{
	command::SetBit(m_pdata, 0, err?1:0, (char)0x40);
}
////////////////////////
//
CBufferCmd::CBufferCmd()
{
	m_cdb_size=10;
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0x3B);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::Set2BYTE(m_cdb, 4, 0);
	command::Set2BYTE(m_cdb, 6, 0);
	command::SetBYTE(m_cdb, 9, 0);
}
CBufferCmd::CBufferCmd(char * data, long in_data_size)
{
	m_pdata=(unsigned char*)data;
	m_cdb_size=10;
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0x3B);
	command::SetBYTE(m_cdb, 1, 0);
	command::Set4BYTE(m_cdb, 2, (char)0x10080000);
	command::Set3BYTE(m_cdb, 6, (int)in_data_size);
	command::SetBYTE(m_cdb, 9, 0);
}
CBufferCmd::~CBufferCmd()
{
	if (m_pdata) delete [] m_pdata;
	m_pdata=NULL;
}
////////////////////////
CBufferCmd2::CBufferCmd2(char * data, long data_size)
{
	m_pdata=(unsigned char*)data;
	m_pdata2=data;
	m_cur=data;
	m_total_size=data_size;
	m_cdb_size=10;
	m_offset=0;
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0x3B);
	command::SetBYTE(m_cdb, 1, 0);
	command::Set4BYTE(m_cdb, 2, 0x10080000);
	command::Set3BYTE(m_cdb, 6, (int)0x2000);
	command::SetBYTE(m_cdb, 9, 0);
}
CBufferCmd2::CBufferCmd2(CBufferCmd2& in):
m_pdata2(NULL),
m_cur(NULL),
m_total_size(0),
m_offset(0)
{
	copy(in);
}
CBufferCmd2::~CBufferCmd2()
{
	if (m_pdata2) delete [] m_pdata2;
	m_pdata=NULL;
	m_pdata2=NULL;
}
CBufferCmd2& CBufferCmd2::operator=(CBufferCmd2& in)
{
	copy(in);
	return *this;
}
bool CBufferCmd2::end(){
	return m_total_size>0;
}
void CBufferCmd2::next()
{
	if (transfer_length()<m_total_size) {
		m_total_size-= transfer_length();
	} else {
		transfer_length(m_total_size);
		m_total_size=0;
	}
	m_pdata+= transfer_length();
	m_offset+= transfer_length();
	command::Set4BYTE(m_cdb, 2, (int)(0x10080000 + m_offset));
}
////////////////////////
//
CServiceCmd::CServiceCmd(char mode, long submode, char *data, long size) 
{
	m_pdata=(unsigned char *)data;
	m_cdb[0] = 0xff;
	m_cdb[2] = mode;
	command::Set3BYTE(m_cdb, 3, (int)submode);
	command::Set3BYTE(m_cdb, 6, (int)size);
}
CServiceCmd::~CServiceCmd()
{}
char *CServiceCmd::firm_name(char *wk)
{
	return command::GetNString((char*)m_pdata, 0, 16, wk);
}
char *CServiceCmd::firm_version(char *wk)
{
	return command::GetNString((char*)m_pdata, 16, 8, wk);
}
////////////////////////////////////////////////////
CShadingDataCmd::CShadingDataCmd():m_cur(NULL)
{
	m_buffer.reserve(0x10000);
	if (m_buffer.capacity()) {
		m_buffer.assign(m_buffer.capacity(), 0);
	}
	m_pdata=(unsigned char *)&m_buffer[0];
	m_cdb_size=sizeof(m_cdb);
	//m_max_data_size=(long)m_buffer.size();
	memset(m_cdb, 0, sizeof(m_cdb));

	m_cdb[0] = 0x3b;
	command::Set4BYTE(m_cdb, 2, 0x80000);
	command::Set3BYTE(m_cdb, 6, (int)m_buffer.size());		
}
CShadingDataCmd::~CShadingDataCmd()
{
}
void  CShadingDataCmd::first()
{
	char *timestamp = data();
	char *version = timestamp + 4;
    m_cur = version + 4;
}
bool CShadingDataCmd::eof()
{
	return *((int*)m_cur)==0;
}
char *CShadingDataCmd::next()
{
    ////WriteLog(((char*)"CShadingDataCmd::next() start"));
    
    char *out = m_cur;
    
    int size = 0;
    //int offset = 0;
	char *s = m_cur;
   // //WriteLog(((char*)"dpi %d"), *((short*)s));
	s+=2;//resolution
   // //WriteLog(((char*)"mode %d"), *((short*)s));
	s+=2;//mode
    //offset = *((int*)s);
   // //WriteLog(((char*)"offset %d"), offset);
    s+=4;//offset;
    s+=32;//adjust data
    size=*((int*)s);
   // //WriteLog(((char*)"front blak data size %d"), size);
    s += 4;//front black data size;
    s+=size;//front black data
    size=*((int*)s);
   // //WriteLog(((char*)"front white data size %d"), size);
    s += 4;//front white data size
    s += size;//front white data
    size=*((int*)s);
   // //WriteLog(((char*)"back black data size %d"), size);
    s += 4;//back black data size
    s += size;//back black data
    size=*((int*)s);
   // //WriteLog(((char*)"back white data size %d"), size);
    s += 4;//back white data size
    s += size; //back white data
    
    m_cur = s;
    
    ////WriteLog(((char*)"CShadingDataCmd::next() end"));
    return out;
}
char *CShadingDataCmd::search(KEYINFO &key)
{
    char *item = NULL;
    
    first();

	while (!eof()) {

		item = next();
		if (key.dpi==*((short*)item) && key.mode==*((short*)(item+2))) {
			return item;
		}

	}

	return NULL;
}
char *CShadingDataCmd::adjust_data(KEYINFO &key)
{
	char *adj = search(key);
	adj += 2;//resolution;
	adj += 2;//mode
	adj += 4;//offset
	return adj;
}
namespace {
	char* front_black(char* item)
	{
		//int size = 0;
		//int offset = 0;
		char* s = item;
		// //WriteLog(((char*)"dpi %d"), *((short*)s));
		s += 2;//resolution
		// //WriteLog(((char*)"mode %d"), *((short*)s));
		s += 2;//mode
		//offset = *((int*)s);
		// //WriteLog(((char*)"offset %d"), offset);
		s += 4;//offset;
		s += 32;//adjust data
		//size=*((int*)s);
		// //WriteLog(((char*)"front blak data size %d"), size);
		s += 4;//front black data size;
		return s;
	}
	long front_black_size(char* item)
	{
		int size = 0;
		//int offset = 0;
		char* s = item;
		// //WriteLog(((char*)"dpi %d"), *((short*)s));
		s += 2;//resolution
		// //WriteLog(((char*)"mode %d"), *((short*)s));
		s += 2;//mode
		//offset = *((int*)s);
		// //WriteLog(((char*)"offset %d"), offset);
		s += 4;//offset;
		s += 32;//adjust data
		size = *((int*)s);
		// //WriteLog(((char*)"front blak data size %d"), size);
		return size;
	}
	char* front_white(char* item)
	{
		int size = 0;
		//int offset = 0;
		char* s = item;
		// //WriteLog(((char*)"dpi %d"), *((short*)s));
		s += 2;//resolution
		// //WriteLog(((char*)"mode %d"), *((short*)s));
		s += 2;//mode
		//offset = *((int*)s);
		// //WriteLog(((char*)"offset %d"), offset);
		s += 4;//offset;
		s += 32;//adjust data
		size = *((int*)s);
		// //WriteLog(((char*)"front blak data size %d"), size);
		s += 4;//front black data size;
		s += size;//front black data
		size = *((int*)s);
		// //WriteLog(((char*)"front white data size %d"), size);
		s += 4;//front white data size
		return s;
	}
	long front_white_size(char* item)
	{
		int size = 0;
		//int offset = 0;
		char* s = item;
		// //WriteLog(((char*)"dpi %d"), *((short*)s));
		s += 2;//resolution
		// //WriteLog(((char*)"mode %d"), *((short*)s));
		s += 2;//mode
		//offset = *((int*)s);
		// //WriteLog(((char*)"offset %d"), offset);
		s += 4;//offset;
		s += 32;//adjust data
		size = *((int*)s);
		// //WriteLog(((char*)"front blak data size %d"), size);
		s += 4;//front black data size;
		s += size;//front black data
		size = *((int*)s);
		// //WriteLog(((char*)"front white data size %d"), size);
		return size;
	}
}
void CShadingDataCmd::read(KEYINFO &key, char *ptr, long size)
{
	char *item = search(key);
	if (key.front) {
		if (key.white) {

            memcpy(ptr, front_white(item), std::min(front_white_size(item), size));

		} else {

            memcpy(ptr, front_black(item), std::min(front_black_size(item), size));

		}
	} else {
		if (key.white) {

            memcpy(ptr, front_white(item), std::min(front_white_size(item), size));

		} else {

            memcpy(ptr, front_black(item), std::min(front_black_size(item), size));

		}
	}
}
///////////////////////////////////
CErrorHistoryCmd::CErrorHistoryCmd()
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=10;
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0xfd);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, (char)0x80);
	command::SetBYTE(m_cdb, 3, (char)0x22);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	command::SetBYTE(m_cdb, 6, 0);
	command::SetBYTE(m_cdb, 7, 0);
	command::SetBYTE(m_cdb, 8, (char)0xC0);
	command::SetBYTE(m_cdb, 9, 0);	
}
CErrorHistoryCmd::CErrorHistoryCmd(CErrorHistoryCmd& in)
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CErrorHistoryCmd::CErrorHistoryCmd(char *pdata/*size must be 192*/)
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)pdata;
	m_cdb_size=10;
	//cdb must be decided initially.
	command::SetBYTE(m_cdb, 0, (char)0xfd);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, (char)0x80);
	command::SetBYTE(m_cdb, 3, (char)0x22);
	command::SetBYTE(m_cdb, 4, 0);
	command::SetBYTE(m_cdb, 5, 0);
	command::SetBYTE(m_cdb, 6, 0);
	command::SetBYTE(m_cdb, 7, 0);
	command::SetBYTE(m_cdb, 8, (char)0xC0);
	command::SetBYTE(m_cdb, 9, 0);
}
CErrorHistoryCmd::~CErrorHistoryCmd()
{
}
CErrorHistoryCmd& CErrorHistoryCmd::operator=(CErrorHistoryCmd& in)
{
	copy(in);
	return *this;
}
void CErrorHistoryCmd::I_am_in(CCommand::EXEC_TYPE type)
{
	if (type==CCommand::EXEC_READ) {
		//read
		command::SetBYTE(m_cdb, 0, (char)0xfd);
	} else {
		//write,
		command::SetBYTE(m_cdb, 0, (char)0xfe);
		command::SetBYTE(m_cdb, 8, (char)0x00);
	}
}
///////////////////////////////////
CCheckScanSize::CCheckScanSize()
{
	memset(m_data, 0, sizeof(m_data));
	m_pdata=(unsigned char*)m_data;
	m_cdb_size=6;
	command::SetBYTE(m_cdb, 0, (char)0xC6);
	command::SetBYTE(m_cdb, 1, 0);
	command::SetBYTE(m_cdb, 2, 0);
	command::SetBYTE(m_cdb, 3, 0);
	command::SetBYTE(m_cdb, 4, (char)sizeof(m_data));
	command::SetBYTE(m_cdb, 5, 0);
	m_data[0] = 0x0;
	m_data[1] = 0x0;
}
CCheckScanSize::CCheckScanSize(CCheckScanSize& in) 
{
	copy(in);
	long sz = in.data_size();
	if (sz > (long)sizeof(m_data)) sz = sizeof(m_data);
	memcpy(m_data, in.data(), sz);
	m_pdata = m_data;
}
CCheckScanSize::CCheckScanSize(char* cdb, long cdb_size, char* data, long data_size)
{
	input(cdb, cdb_size, data, data_size);
	for (long i = 0; i < (long)sizeof(m_data); i++) m_data[i] = (unsigned char)i;
}
CCheckScanSize::~CCheckScanSize() 
{}
CCheckScanSize& CCheckScanSize::operator=(CCheckScanSize& in)
{
	copy(in);
	return *this;
}
bool CCheckScanSize::duplex()
{
	return data_size()>1;
}
void CCheckScanSize::duplex(bool v)
{
	if (v) {
		main_windowid(0);
		sub_windowid(1);
		command::SetBYTE(m_cdb, 4, 2);
	} else {
		main_windowid(0);
		sub_windowid(0);
		command::SetBYTE(m_cdb, 4, (char)1);
	}
}
void CCheckScanSize::main_windowid(unsigned char v)
{
	m_data[0] = v;
}
unsigned char CCheckScanSize::main_windowid()
{
	return m_data[0];
}
void CCheckScanSize::sub_windowid(unsigned char v)
{
	m_data[1] = v;
}
unsigned char CCheckScanSize::sub_windowid()
{
	return m_data[1];
}
CErrorClear::CErrorClear()
{
    command::SetBYTE(m_cdb, 0, (char)0xC7);
    command::SetBYTE(m_cdb, 1, 0);
    command::SetBYTE(m_cdb, 2, 0);
    command::SetBYTE(m_cdb, 3, 0);
    command::SetBYTE(m_cdb, 4, 0);
    command::SetBYTE(m_cdb, 5, 0);
    m_cdb_size=6;
}
CErrorClear::CErrorClear(CErrorClear& in) :CCommand((CCommand&)in)
{
}
CErrorClear::~CErrorClear()
{
}
