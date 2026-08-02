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
#include "devices_interface.h"

namespace {
    char *extract(char *src, char *dst, const char *skey, const char *ekey)
    {
        char *p = strstr(src, skey);
        if (p) {
            p++;
            if (p) {
                char *s = p;
                p = strstr(s, ekey);
                if (p) {
                    *p=0;
                    strcpy(dst, s);
                }
            }
        }
        return dst;
    }
   class CEnumDevices
    {
    public:
        CEnumDevices(long vendor_id):m_bfirst(true), m_vendor_id(vendor_id)
        {
        }
        ~CEnumDevices()
        {
        }
        bool hasnext()
        {
            //WriteLog("CEnumDevices::hasnext() start");
            bool out = false;
            if (m_bfirst) {
                out = hasnext_first();
                m_bfirst=false;
            } else {
                m_infos.pop_front();
                out = m_infos.size()>0;
            }
            //WriteLog("CEnumDevices::hasnext() end");
            return out;
        }
        long pid()
        {
            return m_infos.front().pid;
        }
        long busid()
        {
            return m_infos.front().busid;
        }
        long devid()
        {
            return m_infos.front().devid;
        }
    private:
        bool m_bfirst;
        long m_vendor_id;
        typedef struct tagDEVINFO
        {
            long pid;
            long busid;
            long devid;
        }DEVINFO;
        std::list<DEVINFO>m_infos;
        bool hasnext_first()
        {
#ifndef _WIN32
            FILE *fp=popen("lsusb","r");
            if (fp ==NULL) {
                return false;
            }
            char buf[256];
            char svid[8]={0};
            sprintf(svid, "%lx:", m_vendor_id);
            while(fgets(buf, sizeof(buf), fp) != NULL) {
                char *p = strstr(buf, svid);
                if (p) {
                    DEVINFO info={0};
                    char s[8]={0};
                    //
                    memcpy(s, p + strlen(svid), 4);
                    info.pid = strtol(s, NULL, 16);
                    memset(s, 0, sizeof(s));
                    //
                    p = strstr(buf, "Bus ");
                    if (p) {
                        memcpy(s, p + strlen("Bus "), 3);
                        info.busid = strtol(s, NULL, 16);
                        memset(s, 0, sizeof(s));
                    }
                    //
                    p = strstr(buf, "Device ");
                    if (p) {
                        memcpy(s, p + strlen("Device "), 3);
                        info.devid = strtol(s, NULL, 16);
                        memset(s, 0, sizeof(s));
                    }
                    if (info.pid>0&&info.busid>0&&info.devid>0) {
                        m_infos.push_back(info);
                    }    
                }
                memset(buf, 0, sizeof(buf));
            }   
            pclose(fp);
#endif
            return m_infos.size()>0;     
        }
    };
class CEnumDevicesNet
    {
    public:
        CEnumDevicesNet(char*pn):m_bfirst(true)
        {
            strcpy(m_product_name, pn);
        }
        ~CEnumDevicesNet()
        {
        }
        bool hasnext()
        {
            //WriteLog("CEnumDevices::hasnext() start");
            bool out = false;
            if (m_bfirst) {
                out = hasnext_first();
                m_bfirst=false;
            } else {
                m_infos.pop_front();
                out = m_infos.size()>0;
            }
            //WriteLog("CEnumDevices::hasnext() end");
            return out;
        }
        const char* ip()
        {
            return m_infos.front().ip.c_str();
        }
        const char* hostname()
        {
            return m_infos.front().hostname.c_str();
        }
        long ip_type()
        {
            return m_infos.front().ip_type;
        }
    private:
        bool m_bfirst;
        char m_product_name[32];
        typedef struct tagDEVINFO
        {
            long ip_type;
            std::string ip;
            std::string hostname;
        }DEVINFO;
        std::list<DEVINFO>m_infos;
        bool hasnext_first()
        {
#ifndef _WIN32
            FILE *fp=popen("avahi-browse -d local _scanner._tcp --resolve -t","r");
            if (fp ==NULL) {
                return false;
            }
            char buf[512]={0};
            struct {
                char product_name[32];
                char address[256];
                char hostname[256];
                short port;
                char ipv;
            }result;
            char *p=NULL;
            char mdl[256];
            sprintf(mdl, "mdl=CANON %s", m_product_name);
            memset(&result, 0, sizeof(result));
            while(fgets(buf, sizeof(buf), fp) != NULL) {
                p = strstr(buf, mdl);
                if (p) {
                    strcpy(result.product_name, m_product_name);
                } else {
                    p = strstr(buf, "address");
                    if (p) {
                        extract(p, result.address, "[", "]");
                    } else {
                        p = strstr(buf, "hostname");
                        if (p) {
                            extract(p, result.hostname, "[", "]");
                        } else {
                            p = strstr(buf, "port");
                            if (p) {
                                result.port = 80;
                            } else {
                                p = strstr(buf, "IPv4 Canon Scanner");
                                if (p) {
                                    memset(&result, 0, sizeof(result));
                                    result.ipv=4;
                                } 
                                p = strstr(buf, "IPv6 Canon Scanner");
                                if (p) {
                                    memset(&result, 0, sizeof(result));
                                    result.ipv=6;
                                }
                            }
                        }
                    }
                }
                if (result.product_name[0]&&result.address[0]&&result.hostname[0]&&result.ipv==4) {
                    DEVINFO info;
                    info.ip_type = result.ipv;
                    info.ip = result.address;
                    info.hostname = result.hostname;
                    m_infos.push_back(info);
                    memset(&result, 0, sizeof(result)); 
                }
                memset(buf, 0, sizeof(buf));
            }   
            pclose(fp);
#endif
            return m_infos.size()>0;     
        }
    };    
    class CSANE_Device
    {
    public:
        CSANE_Device(char *vendor_name)
        {
            m_name="";
            m_model="";
            m_type="";
            m_vendor_name = vendor_name;
            memset(&m_dev, 0, sizeof(m_dev));
        }
        ~CSANE_Device()
        {
        }
        operator SANE_Device * () {return ptr();}
        void name(char *s){m_name=s;}
        std::string name(){return m_name;}
        void type(char *s){m_type=s;}
        std::string type(){return m_type;}
        void model(char *s){m_model=s;}
        std::string model(){return m_model;}
    private:
        std::string m_name;  /* unique device name */
        std::string m_model;    /* device model name */
        std::string m_type; /* device type (e.g., "flatbed scanner") */
        std::string m_vendor_name;
        SANE_Device m_dev;
        SANE_Device *ptr()
        {
            m_dev.name   = m_name.c_str();
            m_dev.vendor= m_vendor_name.c_str();
            m_dev.model = m_model.c_str();
            m_dev.type  = m_type.c_str();
            return &m_dev;
        }    
    };    
}
class CDevices : public ISaneDevices
{
    typedef std::vector< SANE_Device *> SANE_DEVICEPTRS;
    typedef std::list< CSANE_Device > CSANE_DEVICES;
public:
    CDevices(const char *vendor_name, long vendor_id, const char *product_name, long product_id, const char *type);
    virtual ~CDevices();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();    
    SANE_Device **get_devices(SANE_Bool local_only=SANE_FALSE);
private:
	long m_ref;
	SANE_DEVICEPTRS m_devptrs;
	CSANE_DEVICES m_devs;
	char m_vendor_name[16];
	long m_vendor_id;
	char m_product_name[32];
	long m_product_id;
    char m_type[128];
	void search(SANE_Bool local_only);
		void search_main(SANE_Bool local_only);
		void search_simulation();
	void clear();
};
CDevices::CDevices(const char *vendor_name, long vendor_id, const char *product_name, long product_id, const char *type):m_ref(1), m_vendor_id(vendor_id), m_product_id(product_id)
{
	strcpy(m_vendor_name, vendor_name);
	strcpy(m_product_name, product_name);
    strcpy(m_type, type);
}
CDevices::~CDevices()
{
	clear();
}
long CDevices::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CDevices::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CDevices::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return  0;
	}
	return m_ref;
} 
SANE_Device **CDevices::get_devices(SANE_Bool local_only)
{
	clear();
	search(local_only);
	if (m_devptrs.size()==0) {
	    m_devptrs.push_back(NULL);
	}
	return &m_devptrs[0];
}
void CDevices::clear()
{
    m_devptrs.clear();
    m_devs.clear();
}	
void CDevices::search(SANE_Bool local_only)
{
	if (strcmp(m_vendor_name, "simulation")==0) {
		search_simulation();
	} else {
		search_main(local_only);
	}
}   
void CDevices::search_simulation()
{
    CSANE_Device dev(m_vendor_name);
    char name[64];
    sprintf(name, "usb:4604:9999");
    dev.name(name);
	dev.model((char*)"IMS");
	dev.type((char*)"simulation scanner");
    m_devs.push_back(dev);
    m_devptrs.push_back((SANE_Device*)m_devs.back());
     m_devptrs.push_back(NULL);
}
void CDevices::search_main(SANE_Bool local_only)
{
    CSANE_Device dev(m_vendor_name);
    {
        CEnumDevices devs(m_vendor_id);
        while (devs.hasnext()) {
            //WriteLog("   PID:0x%x", devs.pid());
            if (devs.pid()==m_product_id) {
                char name[64];
                sprintf(name, "usb:%03ld:%03ld", devs.busid(), devs.devid());
                dev.name(name);
                dev.model(m_product_name);
                dev.type(m_type);
                m_devs.push_back(dev);
                m_devptrs.push_back((SANE_Device*)m_devs.back());
                break;
            }
        }
    }
    if (!local_only) {
        CEnumDevicesNet devs(m_product_name);
        while (devs.hasnext()) {
            //WriteLog("   PID:0x%x", devs.pid());
            char name[512];
            sprintf(name, "network:%ld:%s:%s", devs.ip_type(), devs.ip(), devs.hostname());
            dev.name(name);
            dev.model(m_product_name);
            dev.type(m_type);
            m_devs.push_back(dev);
            m_devptrs.push_back((SANE_Device*)m_devs.back());
        }
    }    
    m_devptrs.push_back(NULL);
}        
////////////
ISaneDevices *create_devices(const char *vendor_name, long vendor_id, const char *product_name, long product_id, const char *type)
{
	return new CDevices(vendor_name, vendor_id, product_name, product_id, type);
}