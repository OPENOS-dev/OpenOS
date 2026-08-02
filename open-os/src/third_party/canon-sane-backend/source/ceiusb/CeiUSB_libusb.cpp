/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "stdafx.h"
#include "CeiUSB.h"
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <vector>
#include <stdlib.h>
#include <ctype.h>
#include <mutex>
namespace ceiusb {
	char g_name[32]={0};
	bool FileExists(TCHAR *path)
	{
		FILE* fp;
		bool out=false;

		fp = fopen(path, "r" );
		if( fp == NULL ){
			out = false;
		}
		else{
			out=true;
			fclose( fp );
		}
		return out;
	}
	bool g_logflag=false;
	bool IsLogMode()
	{
		static bool c_once=true;
		if (c_once) {
			g_logflag=FileExists((TCHAR*)"/tmp/ceiusb.log");
			c_once=false;
		}
		return g_logflag;
	}
	int WriteLogToFile(char *str)
	{
		int out = 0;
		FILE *fp = fopen("/tmp/ceiusb.log", "a");
		if (fp==NULL) return 0;
		fseek(fp, 0, SEEK_END);
		time_t timer;
		time(&timer);
		char strtm[32];
		sprintf(strtm, "%s", ctime(&timer));
		strtm[strlen(strtm)-1]=0;
		fprintf(fp, _T("[ceiusb]%s %s\r\n"), strtm, str);
		fclose(fp);
		return out;
	}
	int WriteLog(const char *fmt, ...)
	{
		if (!IsLogMode()) return 0;
		char *buf=new char[1024];
		if (buf==NULL) return 0;
		va_list app;
		va_start(app, fmt);
		vsprintf(buf, fmt, app);
		va_end(app);
		int out = WriteLogToFile(buf);
		delete []buf;
		return out;
	}
	BYTE GetBYTE(LPBYTE pData, int nIndex)
	{ 
		return pData[nIndex]; 
	}
	void SetBYTE(LPBYTE pData, int nIndex, BYTE data)
	{ 
		pData[nIndex] = data; 
	}
	DWORD GetDWORD(LPBYTE pData, int nIndex)//nIndex：先頭からのバイト数
	{
		return 	((DWORD)((pData[nIndex] << 24) | (pData[nIndex+1] << 16) | (pData[nIndex+2] << 8) | pData[nIndex+3]));
	}
	void SetDWORD(LPBYTE pData, int nIndex,DWORD dwData)
	{
		pData[nIndex]		= (BYTE)((dwData) >> 24);
		pData[nIndex + 1]	= (BYTE)((dwData) >> 16);
		pData[nIndex + 2]	= (BYTE)((dwData) >> 8);
		pData[nIndex + 3]	= (BYTE)(dwData);
	}
	WORD GetWORD(LPBYTE pData, int nIndex)
	{
		return (WORD)((pData[nIndex] << 8) | pData[nIndex+1]);
	}
	void SetWORD(LPBYTE pData, int nIndex, WORD wData)
	{
		pData[nIndex]   = (BYTE)((wData) >> 8);
		pData[nIndex+1] = (BYTE)((wData) );
	}
	void   SetTriBYTE(LPBYTE pData, int nIndex,DWORD dwData)
	{
		pData[nIndex]	  = (BYTE)((dwData) >> 16);	
		pData[nIndex+1] = (BYTE)((dwData) >> 8);	
		pData[nIndex+2] = (BYTE)(dwData);
	}
} 
class CCeiUSB : public ICeiUSBLinux2
{
public:
	CCeiUSB();
	~CCeiUSB();
	long AddRef();
	long Release();
	long init(char *dev);
	void uninit();
	long exec_write(char *cdb, long cdb_size, char *data, long data_size);
	long exec_read(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none(char *cdb, long cdb_size);
	long request_sense(char *sense);
	long lock(long timeout){return 0;}
	long  unlock(){return 0;}
private:
	long m_ref;
	unsigned char * m_pbuf;
private:
	libusb_context *m_ctx;
	libusb_device_handle *m_hd;
public:
	static long s_timeout;
private:
	long bulk_write(char *data, long data_size);
	long bulk_read(char *data, long data_size);
	void clear_stall(bool bwrite);
		void clear_stall_1(bool);
private:
	long exec_write1(char *cdb, long cdb_size, char *data, long data_size);
	long exec_read1(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none1(char *cdb, long cdb_size);
	long exec_write2(char *cdb, long cdb_size, char *data, long data_size);
	long exec_read2(char *cdb, long cdb_size, char *data, long data_size);
	long exec_none2(char *cdb, long cdb_size);	
private:
	long m_ProtocolVersion;//0,1
	enum {
		ENDPOINT_READ=0,
		ENDPOINT_WRITE,
		ENDPOINTS_SIZE
	};
	long m_endpoint[ENDPOINTS_SIZE];//index0:read, index1:write
    std::mutex m_mutex;
	long read_information(long cdb_size, char *data, long data_size);
};
long CCeiUSB::s_timeout=1*1000*60*2;//2 min
const long max_write_buffer_size = 10244 + 12;
CCeiUSB::CCeiUSB():m_ref(1), m_pbuf(NULL), m_ProtocolVersion(0)
{
	ceiusb::WriteLog(_T("CCeiUSB::CCeiUSB()"));
	m_hd = NULL;
	m_ctx = NULL;
	m_endpoint[ENDPOINT_READ]=1 | LIBUSB_ENDPOINT_IN;
	m_endpoint[ENDPOINT_WRITE]=2 | LIBUSB_ENDPOINT_OUT;
}
CCeiUSB::~CCeiUSB()
{
	uninit();
	ceiusb::WriteLog(_T("CCeiUSB::~CCeiUSB()"));
}
long CCeiUSB::AddRef()
{	
	m_ref++;
	return m_ref++;
}
long CCeiUSB::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
namespace {
    char *remove_ch(char *s, char c)
    {
        char *src = s;
        char *dst = s;
        while (*src) {
            if (*src==c) {
                src++;
            } else {
                *dst = *src;
                src++;
                dst++;
            }
        }
        *dst=0;
        return s;
    }	
	char *tolower_str(char *s)
	{
	    for (char *p = s; *p; p++) *p = tolower(*p);
	    return (s);
	}
	struct {
		const char *name;
		long id;
		long protcol;
	}tbl[] = {
		{_T("P-208II"), 0x165f},
		{_T("DR-P208II"), 0x165d},
		{_T("P-215II"), 0x165b},
		{_T("DR-P215II"), 0x1659},
		{_T("DR-C225II"), 0x1658},
		{_T("DR-C230"), 0x1670},
		{_T("DR-C240"), 0x1661},
		{_T("R40"), 0x1679},
		{_T("DR-M260"), 0x166D, 1},
		{_T("R50"), 0x167e, 1},
		{NULL, 0}
		};	
	long devname2vendorid(char *)
	{
		return 0x1083;
	}
	long devname2productid(char *devname)
	{
		if (devname==NULL) return 0;
	    if (devname[0]==0) return 0;
		char c1[16];
		char c2[16];
		for (long i=0; tbl[i].name; i++) {
			strcpy(c1, devname);
			strcpy(c2, tbl[i].name);
			tolower_str(c1);
			tolower_str(c2);
			remove_ch(c1, '-');
			remove_ch(c2, '-');
			if (strcmp(c1, c2)==0) {
				return tbl[i].id;
			}		
		}
		long out = atoi(devname);
		if (out<0) out=0;
		return out;
	}
	long ProtocolVersion(long id)
	{
		for (long i=0; tbl[i].name; i++) {
			if (tbl[i].id==id) return tbl[i].protcol;
		}
		return 0;
	}		
}
long CCeiUSB::init(char *devname)
{
	ceiusb::WriteLog(_T("CCeiUSB::init(%s) start"), devname);
    std::lock_guard<std::mutex> lg(m_mutex);
    int ret = 0;
	ret = libusb_init(&m_ctx);
	if (ret < 0) {
		ceiusb::WriteLog(_T("libusb_init() error %d"), ret);
		return CEIUSB_DEVICE_NOT_FOUND;
	}
	//libusb_set_debug(m_ctx, 3);
	long VendorID = devname2vendorid(devname);
	long ProductID = devname2productid(devname);
	m_hd = libusb_open_device_with_vid_pid(m_ctx, VendorID, ProductID);
	if (m_hd == NULL) {
		ceiusb::WriteLog(_T("libusb_open_device_with_vid_pid(m_ctx, 0x%x, 0x%x) error"), VendorID, ProductID);
		return CEIUSB_DEVICE_NOT_FOUND;
	}
	ceiusb::WriteLog("handle is 0x%x, product id is 0x%x", m_hd, ProductID);
	if (libusb_kernel_driver_active(m_hd, 0) == 1) { //find out if kernel driver is attached
		ceiusb::WriteLog(_T("libusb_detach_kernel_driver() start"));
		if (libusb_detach_kernel_driver(m_hd, 0) == 0) //detach it
		{
			ceiusb::WriteLog("libusb_detach_kernel_driver() return 0");
		}
		ceiusb::WriteLog(_T("libusb_detach_kernel_driver() end"));
	}

	ret = libusb_claim_interface(m_hd, 0);
	if (ret<0) {
		ceiusb::WriteLog(_T("libusb_claim_interface() errror %d (%s)"), ret, libusb_error_name(ret));
	} 
	
	m_pbuf = new unsigned char [max_write_buffer_size];
	if (m_pbuf == NULL) {
		ceiusb::WriteLog(_T("m_pbuf allocate error"));
		return CEIUSB_CANNOT_OPEN_USB;
	}
	m_ProtocolVersion = ProtocolVersion(ProductID);;
	ceiusb::WriteLog(_T("protocol version %d"), m_ProtocolVersion);
	ceiusb::WriteLog(_T("CCeiUSB::init(%s) end"), devname);
	return S_OK;
}
void CCeiUSB::uninit()
{	
	ceiusb::WriteLog(_T("CCeiUSB::uninit() start"));
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_hd) {
		libusb_release_interface(m_hd, 0);
		libusb_close(m_hd);
	}
	m_hd = NULL;
	if (m_ctx) libusb_exit(m_ctx); //close the session
	m_ctx = NULL;
	if (m_pbuf) delete [] m_pbuf;
	m_pbuf=NULL;
	ceiusb::WriteLog(_T("CCeiUSB::uninit() end"));
}
void CCeiUSB::clear_stall(bool bwrite)
{
	ceiusb::WriteLog(_T("clear_stall() start"));
	clear_stall_1(bwrite);
	ceiusb::WriteLog(_T("clear_stall() end"));
}
void CCeiUSB::clear_stall_1(bool bwrite)
{
	//ceiusb::WriteLog(_T("clear_stall_1() start"));
	char data=0;
	int actual = 0;
	if (bwrite)
	{
		//read
		libusb_bulk_transfer(m_hd, m_endpoint[ENDPOINT_READ], (unsigned char*)&data, sizeof(data), &actual, CCeiUSB::s_timeout);		
		//if (status<=0) ceiusb::WriteLog(_T("Read ERROR:%s\r\n"), usb_strerror());
	}
	else
	{
		//write
		libusb_bulk_transfer(m_hd, m_endpoint[ENDPOINT_WRITE], (unsigned char*)&data, sizeof(data), &actual, CCeiUSB::s_timeout);
		//if (status<=0) ceiusb::WriteLog(_T("Write ERROR:%s\r\n"), usb_strerror());
	}
	libusb_clear_halt(m_hd, m_endpoint[ENDPOINT_READ]);
	libusb_clear_halt(m_hd, m_endpoint[ENDPOINT_WRITE]);
	//ceiusb::WriteLog(_T("clear_stall_1() end"));
}
long CCeiUSB::bulk_write(char *data, long data_size)
{	
	//ceiusb::WriteLog(_T("CCeiUSB::bulk_write(%d) start"), data_size);
	long ret = 0;	
	int actual = 0;
	ret = libusb_bulk_transfer(m_hd, m_endpoint[ENDPOINT_WRITE], (unsigned char*)data, data_size, &actual, CCeiUSB::s_timeout);
	if (ret) {
		ceiusb::WriteLog(_T("libusb_bulk_transfer(write) error %d"), ret);
		clear_stall(true);
		return E_FAIL;
	}
	if (actual!=data_size) {
		//ceiusb::WriteLog(_T("ERROR:%s\r\n"), usb_strerror());
		ceiusb::WriteLog(_T("ERROR:actual!=data_size: return_size is %d, data_size is  %d"), actual, data_size);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::bulk_write() end"));
	return S_OK;
}
long CCeiUSB::bulk_read(char *data, long data_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::bulk_read(%d) start"), data_size);
	long ret = 0;
	int actual = 0;
	ret = libusb_bulk_transfer(m_hd, m_endpoint[ENDPOINT_READ], (unsigned char*)data, data_size, &actual, CCeiUSB::s_timeout);
	if (ret) {
		ceiusb::WriteLog(_T("libusb_bulk_transfer(read)  error %d"), ret);	
		clear_stall(false);
		return E_FAIL;
	}
	if (actual!=data_size) {
		//ceiusb::WriteLog(_T("ERROR:%s\r\n"), usb_strerror());
		ceiusb::WriteLog(_T("ERROR:actual!=data_size: return_size is %d, data_size is  %d"), actual, data_size);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::bulk_read() end"));
	return S_OK;
}
long CCeiUSB::exec_write(char *cdb, long cdb_size, char *data, long data_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_ProtocolVersion) return exec_write2(cdb, cdb_size, data, data_size);
	return exec_write1(cdb, cdb_size, data, data_size);
}
long CCeiUSB::exec_read(char *cdb, long cdb_size, char *data, long data_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_ProtocolVersion) return exec_read2(cdb, cdb_size, data, data_size);
	return exec_read1(cdb, cdb_size, data, data_size);
}
long CCeiUSB::exec_none(char *cdb, long cdb_size)
{
    std::lock_guard<std::mutex> lg(m_mutex);
	if (m_ProtocolVersion) return exec_none2(cdb, cdb_size);
	return exec_none1(cdb, cdb_size);
}
long CCeiUSB::request_sense(char *sense)
{
	char cdb[6] = {0x3, 0, 0, 0, 14, 0};
	return exec_read(cdb, sizeof(cdb), sense, cdb[5]);
}
long CCeiUSB::exec_write1(char *cdb, long cdb_size, char *data, long data_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_write1() start"));
	if (!m_pbuf) {
		ceiusb::WriteLog(_T("ERROR:initialize failed. buffer is not allocated."));
		return E_FAIL;
	}
	if (max_write_buffer_size < data_size + 12) {
		ceiusb::WriteLog(_T("ERROR: not supported command! data size = %d"), data_size);
		return E_FAIL;
	}
	long status = S_OK;
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);
	status = bulk_write((char*)command_container, sizeof(command_container));
	if (status!=S_OK) {
		return status;
	}
	memset(m_pbuf, 0, max_write_buffer_size);
	ceiusb::SetDWORD(m_pbuf, 0, data_size+2+2+4);
	ceiusb::SetWORD(m_pbuf, 4, 2);
	ceiusb::SetWORD(m_pbuf, 6, 0xb000);
	ceiusb::SetDWORD(m_pbuf, 8, 0);
	memcpy(m_pbuf+12, data, data_size);
	status = bulk_write((char*)m_pbuf, data_size+4+2+2+4);
	if (status!=S_OK) {
		return status;
	}
	//response container
	BYTE scanner_status[4]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_write1() end"));
	return S_OK;
}
long CCeiUSB::exec_read1(char *cdb, long cdb_size, char *data, long data_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_read1() start"));	
	long status = S_OK;
	//command container	
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);
	status = bulk_write((char*)command_container, sizeof(command_container));
	if (status!=S_OK) {
		ceiusb::WriteLog(_T("bulk_write(0x%x) error"), cdb[0]);
		return status;
	}
	//data container
	status = bulk_read(data, data_size);
	if (status!=S_OK) {
		ceiusb::WriteLog(_T("bulk_read(0x%x) error"), cdb[0]);
		return status;
	}
	//response container
	BYTE scanner_status[4]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		ceiusb::WriteLog(_T("bulk_read(0x%x) error"), cdb[0]);
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_read1() end"));
	return S_OK;
}
long CCeiUSB::exec_none1(char *cdb, long cdb_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_none1() start"));
	long status = S_OK;
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);			
	status = bulk_write((char*)command_container, sizeof(command_container));
	if (status!=S_OK) {
		return status;
	}
	BYTE scanner_status[4]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_none1() end"));
	return S_OK;
}
long CCeiUSB::exec_write2(char *cdb, long cdb_size, char *data, long data_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_write2(%x, %d, data, %d) start"), cdb[0], cdb_size, data_size);
	if (!m_pbuf) {
		ceiusb::WriteLog(_T("ERROR:initialize failed. buffer is not allocated."));
		return E_FAIL;
	}
	if (max_write_buffer_size < data_size + 12) {
		ceiusb::WriteLog(_T("ERROR: not supported command! data size = %d"), data_size);
		return E_FAIL;
	}
	long status = S_OK;
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);
	bool bContinue=true;
	BYTE resp[8]={0};
	while (bContinue) {
		status = bulk_write((char*)command_container, sizeof(command_container));
		if (status!=S_OK) {
			return status;
		}
		status = bulk_read((char*)resp, sizeof(resp));
		if (status!=S_OK) {
			return status;
		}
		DWORD s = ceiusb::GetDWORD(resp, 0);
		switch (s) {
		case 0:bContinue=false;break;
		case 1:break;
		default:
			if (s&0x80) {
				ceiusb::WriteLog(_T("error(%d) %d %s"), s&0x7f, __LINE__, __FILE__);
			}
			return E_FAIL;
		}
	}
	memset(m_pbuf, 0, max_write_buffer_size);
	ceiusb::SetDWORD(m_pbuf, 0, data_size+2+2+4);
	ceiusb::SetWORD(m_pbuf, 4, 2);
	ceiusb::SetWORD(m_pbuf, 6, 0xb000);
	ceiusb::SetDWORD(m_pbuf, 8, 0);
	memcpy(m_pbuf+12, data, data_size);
	if (data_size>(long)ceiusb::GetDWORD(resp, 4)) data_size=ceiusb::GetDWORD(resp, 4);
	status = bulk_write((char*)m_pbuf, data_size+4+2+2+4);
	if (status!=S_OK) {
		return status;
	}
	BYTE scanner_status[8]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_write2() end"));
	return S_OK;
}
long CCeiUSB::exec_read2(char *cdb, long cdb_size, char *data, long data_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_read2(%x, %d, data, %d) start"), cdb[0], cdb_size, data_size);	
	long status = S_OK;	
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);
	BYTE resp[8]={0};
	bool bContinue=true;
	while (bContinue) {
		status = bulk_write((char*)command_container, sizeof(command_container));
		if (status!=S_OK) {
			return status;
		}
		status = bulk_read((char*)resp, sizeof(resp));
		if (status!=S_OK) {
			return status;
		}
		DWORD s = ceiusb::GetDWORD(resp, 0);
		switch (s) {
		case 0:bContinue=false;break;
		case 1:break;
		default:
			if (s&0x80) {
				ceiusb::WriteLog(_T("error(%d) %d %s"), s&0x7f, __LINE__, __FILE__);
			}
			return E_FAIL;
		}
	}
	if (data_size>(long)ceiusb::GetDWORD(resp, 4)) data_size=ceiusb::GetDWORD(resp, 4);
	status = bulk_read(data, data_size);
	if (status!=S_OK) {
		return status;
	}
	BYTE scanner_status[8]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_read2() end"));
	return S_OK;
}
long CCeiUSB::exec_none2(char *cdb, long cdb_size)
{
	//ceiusb::WriteLog(_T("CCeiUSB::exec_none2(%x, %d) start"), cdb[0], cdb_size);
	long status = S_OK;
	BYTE command_container[24]={0};
	ceiusb::SetDWORD(command_container, 0, 0x14);
	ceiusb::SetWORD(command_container, 4, 1);
	ceiusb::SetWORD(command_container, 6, 0x9000);
	ceiusb::SetDWORD(command_container, 8, 0);
	memcpy(command_container+12, cdb, cdb_size);			
	bool bContinue=true;
	while (bContinue) {
		status = bulk_write((char*)command_container, sizeof(command_container));
		if (status!=S_OK) {
			return status;
		}
		BYTE resp[8]={0};
		status = bulk_read((char*)resp, sizeof(resp));
		if (status!=S_OK) {
			return status;
		}
		DWORD s = ceiusb::GetDWORD(resp, 0);
		switch (s) {
		case 0:bContinue=false;break;
		case 1:break;
		default:
			if (s&0x80) {
				ceiusb::WriteLog(_T("error(%d) %d %s"), s&0x7f, __LINE__, __FILE__);
			}
			return E_FAIL;
		}
	}
	BYTE scanner_status[8]={0};
	status = bulk_read((char*)scanner_status, sizeof(scanner_status));
	if (status!=S_OK) {
		return status;
	}
	DWORD ss = ceiusb::GetDWORD(scanner_status, 0);
	if (ss) {
		if (ss!=2) ceiusb::WriteLog(_T("scanner_status is %d %d %s"), ceiusb::GetDWORD(scanner_status, 0), __LINE__, __FILE__);
		return E_FAIL;
	}
	//ceiusb::WriteLog(_T("CCeiUSB::exec_none2() end"));
	return S_OK;
}
long CreateCeiUSB2(ICeiUSBLinux2** ppObject)
{
	ceiusb::WriteLog("CreateCeiUSB2() start");
	if (ppObject == NULL) {
		ceiusb::WriteLog(_T("ppObject is NULL"));
		return E_INVALIDARG;
	}
	*ppObject = new CCeiUSB;
	if (*ppObject == NULL) {
		ceiusb::WriteLog(_T("memory error"));
		return E_OUTOFMEMORY;
	}
	ceiusb::WriteLog("CreateCeiUSB2() end");
	return S_OK;
}
long CreateCeiUSB(ICeiUSBLinux **ppObject)
{
	//WriteLog("CreateCeiUSB() start");
	long out = CreateCeiUSB2((ICeiUSBLinux2**)ppObject);
	//WriteLog("CreateCeiUSB() end");
	return out;
}
