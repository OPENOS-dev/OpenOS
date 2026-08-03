/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _CSD_CEIUSB_LINUX_H_INCLUDED
#define _CSD_CEIUSB_LINUX_H_INCLUDED

/********************************************************************/
/*	status							   */
/********************************************************************/
#define CEIUSB_OK 0
#define CEIUSB_DEVICE_NOT_FOUND 1
#define CEIUSB_CANNOT_OPEN_USB  2
#define CEIUSB_NOMEM		3

class ICeiUSBLinux
{
public:
	ICeiUSBLinux(){}
	virtual ~ICeiUSBLinux(){}
	virtual long AddRef()=0;
	virtual long Release()=0;

	virtual long init(char *dev)=0;
	virtual void uninit()=0;

	virtual long exec_write(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_read(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_none(char *cdb, long cdb_size)=0;

	virtual long request_sense(char *sense)=0;
	/*
		if cdb is NULL, exec_read() returns information.
		1:cdb_size is READ_ID_PROTOCOLVERSION --> data is version number, data_size is ignored.			
	*/
	enum {
		READ_ID_PROTOCOLVERSION=0,
		READ_ID_PRODUCTNAME=1
	};
};


class ICeiUSBLinux2 : public ICeiUSBLinux
{
public:
	ICeiUSBLinux2(){}
	virtual ~ICeiUSBLinux2(){}	
	virtual long lock(long timeout)=0;//refernce count is needed. lock() might be called many time. ex. if lock() is called 3 times, the client must call unlock() 3 times.
	virtual long  unlock()=0;//refernce count is needed.
};


class ICeiEnumScanner
{
public:
	ICeiEnumScanner(){}
	
	virtual ~ICeiEnumScanner(){}

	virtual long HasNext()=0;
	
	virtual long pid()=0;//product id

	virtual long Release()=0;
};


class ICeiEnumScanner2 : public ICeiEnumScanner
{
public:
	ICeiEnumScanner2(){}
	
	virtual ~ICeiEnumScanner2(){}

	virtual char *name()=0;
};


/**/
/*
 * Descriptor types
 */
#define CEIUSB_DT_DEVICE			0x01
#define CEIUSB_DT_CONFIG			0x02
#define CEIUSB_DT_STRING			0x03
#define CEIUSB_DT_INTERFACE			0x04
#define CEIUSB_DT_ENDPOINT			0x05

#define CEIUSB_DT_HID			0x21
#define CEIUSB_DT_REPORT			0x22
#define CEIUSB_DT_PHYSICAL			0x23
#define CEIUSB_DT_HUB			0x29

/*
 * Descriptor sizes per descriptor type
 */
#define CEIUSB_DT_DEVICE_SIZE		18
#define CEIUSB_DT_CONFIG_SIZE		9
#define CEIUSB_DT_INTERFACE_SIZE		9
#define CEIUSB_DT_ENDPOINT_SIZE		7
#define CEIUSB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define CEIUSB_DT_HUB_NONVAR_SIZE		7


extern "C" {

long CreateCeiUSB(ICeiUSBLinux **ppObject);

long CreateCeiUSB2(ICeiUSBLinux2 **ppObject);

long CreateEnumScanners(ICeiEnumScanner **ppObject);

long CreateEnumScanners2(ICeiEnumScanner2 **ppObject);

long CeiUsbControlMsg(ICeiUSBLinux *pObject, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);

long CeiUsbGetDescriptor(ICeiUSBLinux *pObject, unsigned char type/*CEIUSB_DT_* is same as USB_DT_* */, unsigned char index, void *buf, int size);

char * Version();

}/* extern "C" */

typedef long (*LPFNCREATECEIUSB)(ICeiUSBLinux **ppObject);
typedef long (*LPFNCREATECEIUSB2)(ICeiUSBLinux2 **ppObject);
typedef long (*LPFNCREATEENUMSCANNERS)(ICeiEnumScanner **ppObject);
typedef long (*LPFNCREATEENUMSCANNERS2)(ICeiEnumScanner2 **ppObject);
typedef long (*LPFNCEIUISBCONTROLMSG)(ICeiUSBLinux *pObject, int requesttype, int request, int value, int index, char *bytes, int size, int timeout);
typedef long (*LPFNCEIUSBGETDESCRIPTOR)(ICeiUSBLinux *pObject, unsigned char type/*CEIUSB_DT_* is same as USB_DT_* */, unsigned char index, void *buf, int size);

#endif	/* _CSD_CEIUSB_LINUX_H_INCLUDED */
