/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <list>
#include <string>
#include <memory>
#include "sane/sane.h"
#ifdef _WIN32
#ifndef BACKEND_NAME
#define BACKEND_NAME winsane
#endif
#else
#ifndef BACKEND_NAME
#define BACKEND_NAME framework
#endif
#endif
#include "sane_ctrl_interface.h"
#include "devices_interface.h"
#include "sane_global_apis.h"
#include "log.h"
#define VENDOR_ID 0x1083
#define VENDOR_NAME "Canon"
#define INI_FILE_PATH "/run/imageloader/sane-backends-canon/package/root/canonlibs"
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
namespace {
	long get_product_id()
	{
		char s[8]={0};
		char path[256]={0};
		sprintf(path, "%s/%s.sane.ini", INI_FILE_PATH, STRINGIFY(BACKEND_NAME));
		sanesdk::ceisdk_get_private_profile_string("SANE", "pid", s, sizeof(s), "5741", path);
		return atol(s);
	}
	char *get_product_name(char *s)
	{
		char path[256]={0};
		sprintf(path, "%s/%s.sane.ini", INI_FILE_PATH, STRINGIFY(BACKEND_NAME));
		sanesdk::ceisdk_get_private_profile_string("SANE", "name", s, 32, "DR-M260", path);
		return s;
	}
	char *get_type_desc(char *s)
	{
		char path[256]={0};
		sprintf(path, "%s/%s.sane.ini", INI_FILE_PATH, STRINGIFY(BACKEND_NAME));
		sanesdk::ceisdk_get_private_profile_string("SANE", "type", s, 32, "sheetfed scanner", path);
		return s;
	}
 #if 0
    const char *unit2string(SANE_Unit unit)
    {   
        switch (unit) {
        case SANE_UNIT_NONE:return "SANE_UNIT_NONE";
        case SANE_UNIT_PIXEL:return "SANE_UNIT_PIXEL";
        case SANE_UNIT_BIT:return "SANE_UNIT_BIT";
        case SANE_UNIT_MM:return "SANE_UNIT_MM";
        case SANE_UNIT_DPI:return "SANE_UNIT_DPI";
        case SANE_UNIT_PERCENT:return "SANE_UNIT_PERCENT";
        case SANE_UNIT_MICROSECOND:return "SANE_UNIT_MICROSECOND";   
        }
        return "unknown sane unit";
    }
    const char *type2string(SANE_Value_Type type)
    {
        switch (type) {
        case SANE_TYPE_BOOL:return "SANE_TYPE_BOOL";
        case SANE_TYPE_INT:return "SANE_TYPE_INT";
        case SANE_TYPE_FIXED:return "SANE_TYPE_FIXED";
        case SANE_TYPE_STRING:return "SANE_TYPE_STRING";
        case SANE_TYPE_BUTTON:return "SANE_TYPE_BUTTON";
        case SANE_TYPE_GROUP:return "SANE_TYPE_GROUP";
        }
        return "unknown sane type";
    }
    #endif
    char *tolower_str(char *s)
    {
        char *out = s;
        while (*s) {
            *s = ::tolower(*s);
            s++;
        }
        return out;
    }
    char *rm_ch(char *s, char c)
    {
        char *dst = s;
        char *src = s;
        while (*src) {
            if (*src == c) {
                src++;
            }
            else {
                *dst = *src;
                dst++;
                src++;
            }
        }
        *dst = 0;
        return s;
    }       
    char *get_driver_library_path(char *lib_path)
    {
        // ChromeOS makes use of the RPATH
        sprintf(lib_path, "");
        return lib_path;
    }
    ISaneDevices* g_devices=NULL;
}

extern "C" {
//////////////////////////////////////////////////
// EXPORT APIs
#define _SANE_OP_FULL(x, op) sane_ ## x ## _##op
// We need this inner macro for the blue paint rule.
// See https://en.wikipedia.org/wiki/Painted_blue
#define _SANE_OP_BACKEND_NAME_EXPANSION(x, op) _SANE_OP_FULL(x, op)
#define SANE_OP(op) _SANE_OP_BACKEND_NAME_EXPANSION(BACKEND_NAME, op)
SANE_Status SANE_OP(init)(SANE_Int *version_code, SANE_Auth_Callback authorize)
{
	SaneWriteLog_init(STRINGIFY(BACKEND_NAME));
	SaneWriteLog("%s() start", __func__);
	authorize = authorize;/* to shut up compiler */
	if (version_code) {
		char v[32];
		char *s = v;
		strcpy(v, STRINGIFY(SANE_INIT_VERSION));
		long v1=1;
		long v2=0;
		long v3=0;
		char *p=strstr(s, ".");
		if (p) {
			*p=0;
			v1 = atoi(s);
			s = p+1;
		}
		p = strstr(s, ".");
		if (p) {
			*p=0;
			v2 = atoi(s);
			s = p+1;
		}
		v3 = atoi(s);

		SaneWriteLog("version:%ld.%ld.%ld", v1, v2, v3);
		*version_code = SANE_VERSION_CODE(v1, v2, v3);
	}
	SaneWriteLog("%s() end", __func__);
	return SANE_STATUS_GOOD;
}
void SANE_OP(exit)(void)
{
	SaneWriteLog("%s() start", __func__);
	if (g_devices) {
		g_devices->Release();
		g_devices=NULL;
	}
	SaneWriteLog("%s() end", __func__);
	SaneWriteLog_uninit();
}
SANE_Status SANE_OP(get_devices)(const SANE_Device *** device_list, SANE_Bool local_only)
{
	SaneWriteLog("%s() start", __func__);
	SANE_Status status = SANE_STATUS_GOOD;
	if (device_list == NULL) {
		SaneWriteLog("device_list is NULL");
		return SANE_STATUS_INVAL;
	}
	if (g_devices) g_devices->Release();
	char pn[32]={0};
	char td[64]={0};
	g_devices=create_devices(VENDOR_NAME, VENDOR_ID, get_product_name(pn), get_product_id(), get_type_desc(td));
	if (g_devices == NULL) {
		return SANE_STATUS_NO_MEM;
	}

	*device_list = (const SANE_Device **)g_devices->get_devices(SANE_TRUE);
	if (*device_list == NULL) {
		SaneWriteLog("*device_list is NULL");
		return SANE_STATUS_INVAL;
	}
	const SANE_Device **pp = *device_list;
	if (*pp == NULL) {
		SaneWriteLog("**pp is NULL");
		return SANE_STATUS_INVAL;
	}
	while (*pp) {
		SaneWriteLog("  SANE_Device::name   %s", pp[0]->name);
		SaneWriteLog("  SANE_Device::model  %s", pp[0]->model);
		SaneWriteLog("  SANE_Device::type   %s", pp[0]->type);
		SaneWriteLog("  SANE_Device::vendor %s", pp[0]->vendor);
		SaneWriteLog("");
		pp++;
	}
	SaneWriteLog("%s() end\r\n", __func__);
	return status;
}
SANE_Status SANE_OP(open)(SANE_String_Const devicename/*ex libusb:001:006*/, SANE_Handle * handle)
{
	SaneWriteLog("%s(%s, handle) start", __func__, devicename?devicename:"null");
	SANE_Status status = SANE_STATUS_GOOD;
	if (devicename == NULL) {SaneWriteLog("ERROR: LINE:%d", __LINE__); return SANE_STATUS_INVAL;}
	if (handle == NULL) {SaneWriteLog("ERROR: LINE:%d", __LINE__); return SANE_STATUS_INVAL;}
	char lib_path[256];
	char pn[32];
	ICeiSane* p = NULL;
	if (strcmp(VENDOR_NAME, "simulation")==0) {
		p = create_sane_ctrl("simulation", get_product_name(pn), get_driver_library_path(lib_path));
    } else {
		p = create_sane_ctrl(devicename, get_product_name(pn), get_driver_library_path(lib_path));
	}
	if (p == NULL)  {SaneWriteLog("ERROR: LINE:%d", __LINE__); return SANE_STATUS_NO_MEM;}
	*handle = (SANE_Handle)p;
	SaneWriteLog("%s() end: handle is 0x%x\r\n", __func__, *handle);
	return status;
}
void SANE_OP(close)(SANE_Handle handle)
{
	SaneWriteLog("%s(0x%x) start", __func__, handle);
	if (handle == NULL) return;
	ICeiSane *p = (ICeiSane*)handle;
	p->Release();
	SaneWriteLog("%s() end\r\n", __func__);
}
const SANE_Option_Descriptor *SANE_OP(get_option_descriptor)(SANE_Handle handle, SANE_Int option)
{
	SaneWriteLog("%s(0x%x, %d) start", __func__, handle, option);
	if (handle == NULL) return NULL;
	ICeiSane *p = (ICeiSane*)handle;
	const SANE_Option_Descriptor *out = p->get_option_descriptor(option);
	SaneWriteLog("%s() end\r\n", __func__);
	return out;
}
SANE_Status SANE_OP(control_option)(SANE_Handle handle, SANE_Int option, SANE_Action action, void *value, SANE_Int *info)
{
	SaneWriteLog("%s(0x%x, %d, %s, 0x%x, 0x%x) start", __func__, handle, option, action == SANE_ACTION_GET_VALUE ? "get" : "set", value, info);
	SANE_Status status = SANE_STATUS_GOOD;
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	status = p->control_option(option, action, value, info);
	SaneWriteLog("%s() end\r\n", __func__);
	return status;
}
SANE_Status SANE_OP(get_parameters)(SANE_Handle handle, SANE_Parameters * params)
{
	SaneWriteLog("%s(0x%x) start", __func__, handle);
	SANE_Status status = SANE_STATUS_GOOD;
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	status = p->get_parameters(params);
	SaneWriteLog("%s end\r\n", __func__);
	return status;
}
SANE_Status SANE_OP(start)(SANE_Handle handle)
{
	SaneWriteLog("%s(0x%x) start", __func__, handle);
	SANE_Status status = SANE_STATUS_GOOD;
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	status = p->start();
	SaneWriteLog("%s() end\r\n", __func__);
	return status;
}
SANE_Status SANE_OP(read)(SANE_Handle handle, SANE_Byte * data, SANE_Int max_length, SANE_Int * length)
{
	SaneWriteLog("%s() start", __func__);
	SANE_Status status = SANE_STATUS_GOOD;
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	status = p->read(data, max_length, length);
	if (status) {
          SaneWriteLog("%s() end status=%d\r\n", __func__, status);
	} else {
          SaneWriteLog("%s() end status=%d", __func__, status);
	}
	return status;
}
void SANE_OP(cancel)(SANE_Handle handle)
{
	SaneWriteLog("%s() start", __func__);
	if (handle == NULL) return;
	ICeiSane *p = (ICeiSane*)handle;
	p->cancel();
	SaneWriteLog("%s() end\r\n", __func__);
}
SANE_Status SANE_OP(set_io_mode)(SANE_Handle handle, SANE_Bool non_blocking)
{
	SaneWriteLog("%s() start", __func__);
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	SANE_Status status = p->set_io_mode(non_blocking);
	SaneWriteLog("%s() end\r\n", __func__);
	return status;
}
SANE_Status SANE_OP(get_select_fd)(SANE_Handle handle, SANE_Int *fd)
{
	SaneWriteLog("%s() start", __func__);
	SANE_Status status = SANE_STATUS_GOOD;
	if (handle == NULL) return SANE_STATUS_INVAL;
	ICeiSane *p = (ICeiSane*)handle;
	status = p->get_select_fd(fd);
	SaneWriteLog("%s() end\r\n", __func__);
	return status;
}
SANE_String_Const SANE_OP(strstatus)(SANE_Status status)
{
	switch (status) {
	case SANE_STATUS_GOOD:
		return "Success";
	case SANE_STATUS_UNSUPPORTED:
		return "Not supported";
	case SANE_STATUS_CANCELLED:
		return "Cancelled";
	case SANE_STATUS_DEVICE_BUSY:
		return "Busy";
	case SANE_STATUS_INVAL:
		return "Invalid data";
	case SANE_STATUS_EOF:
		return "End of file";
	case SANE_STATUS_JAMMED:
		return "Jammed";
	case SANE_STATUS_NO_DOCS:
		return "No documents";
	case SANE_STATUS_COVER_OPEN:
		return "Cover open";
	case SANE_STATUS_IO_ERROR:
		return "I/O error";
	case SANE_STATUS_NO_MEM:
		return "Out of memory";
	case SANE_STATUS_ACCESS_DENIED:
		return "Access denied";
	default:
		break;
	}
	return "Unknown error";
}
}
