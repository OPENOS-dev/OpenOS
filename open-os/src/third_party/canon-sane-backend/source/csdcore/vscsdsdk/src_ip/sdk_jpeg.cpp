/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <errno.h>
#include <memory>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include "jpeglib.h"
#include "jerror.h"
#elif __ANDROID__
#include <dlfcn.h>
#include "jpeglib.h"
#include "jerror.h"
#else
#include <dlfcn.h>
#include <jpeglib.h>
#include <jerror.h>
#endif
#include "ipsdk.h"
#include "global_apis.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"

#ifdef _WIN32
extern HMODULE g_hModule;
#endif

namespace {
#ifdef _WIN32
	bool FileExists(char* path)
	{
		FILE* fp;
		bool out = false;
		fp = fopen(path, "r");
		if (fp == NULL) {
			out = false;
		}
		else {
			out = true;
			fclose(fp);
		}
		return out;
	}
#endif
	long get_file_size(char* fileName)
	{
	    struct stat st={0};
	    if (stat(fileName, &st) != 0) {
	        return 0;
	    }
	    if ((st.st_mode & S_IFMT) != S_IFREG) {
	        return 0;
	    }
	    return st.st_size;
	}
	unsigned char *searchApp0(unsigned char *ptr, long size)
	{
		for (long i=0; i<size-1; i++) {
			if (ptr[i]==0xff && ptr[i+1]==0xe0) return &ptr[i];
		}
		return NULL;
	}
	unsigned char *searchSOF(unsigned char *ptr, long size)
	{
		for (long i=0; i<size-1; i++) {
			if (ptr[i]==0xff && ptr[i+1]==0xc0) return &ptr[i];
			if (ptr[i]==0xff && ptr[i+1]==0xc2) return &ptr[i];
		}
		return NULL;
	}
	void Set2BYTE(char * pData, int nIndex, short wData)
	{
		pData[nIndex]   = (char)((wData) >> 8);
		pData[nIndex+1] = (char)((wData) );
	}
	long get_xdpi_from_jpg(unsigned char *ptr, long size)
	{
		unsigned char *sof =searchApp0(ptr, size);
		if (sof==NULL) {return -1;}	
		return  ((unsigned int)sof[12]<<8)|sof[13];
	}
	long get_ydpi_from_jpg(unsigned char *ptr, long size)
	{
		unsigned char *sof =searchApp0(ptr, size);
		if (sof==NULL) {return -1;}
		return  ((unsigned int)sof[14]<<8)|sof[15];
	}
	#if 0
	void set_xdpi_from_jpg(unsigned char *ptr, long size, unsigned short v)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {WriteLog(((char*)"searchSOF() return NULL"));return;}	
		Set2BYTE((char*)sof, 12, v);
	}
	void set_ydpi_from_jpg(unsigned char *ptr, long size, unsigned short v)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {WriteLog(((char*)"searchSOF() return NULL"));return;}	
		Set2BYTE((char*)sof, 14, v);		
	}	
	#endif
	long get_width_from_jpg(unsigned char *ptr, long size, long def=100)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {return def;}	
		return  ((unsigned int)sof[7]<<8)|sof[8];
	}
	long get_height_from_jpg(unsigned char *ptr, long size, long def=100)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {return def;}
		return  ((unsigned int)sof[5]<<8)|sof[6];
	}
	#if 0
	void set_width_to_jpg(unsigned char *ptr, long size, unsigned short v)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {WriteLog(((char*)"searchSOF() return NULL"));return;}	
		Set2BYTE((char*)sof, 7, v);
	}
    #endif
	void set_height_to_jpg(unsigned char *ptr, long size, unsigned short v)
	{
		unsigned char *sof =searchSOF(ptr, size);
		if (sof==NULL) {WriteLog(((char*)"searchSOF() return NULL"));return;}
		Set2BYTE((char*)sof, 5, v);
	}
#if defined(__APPLE__) || defined(__ANDROID__)
class CCeiLibJpegDll
{
public:
    CCeiLibJpegDll();
    ~CCeiLibJpegDll();
    struct jpeg_error_mgr * jpeg_std_error(struct jpeg_error_mgr *err);
    void jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize);
    void jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize);
    void jpeg_set_defaults(j_compress_ptr cinfo);
    void jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline);
    void jpeg_write_marker(j_compress_ptr cinfo, int marker, const JOCTET *dataptr, unsigned int datalen);
    void jpeg_destroy_compress(j_compress_ptr cinfo);
    void jpeg_destroy_decompress(j_decompress_ptr cinfo);
    void jpeg_mem_dest(j_compress_ptr cinfo, unsigned char **outbuffer, unsigned long *outsize);
    void jpeg_mem_src(j_decompress_ptr cinfo, const unsigned char *inbuffer, unsigned long insize);
    void jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables);
    boolean jpeg_start_decompress(j_decompress_ptr cinfo);
    JDIMENSION jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines);
    JDIMENSION jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines);
    void jpeg_finish_compress(j_compress_ptr cinfo);
    boolean jpeg_finish_decompress(j_decompress_ptr cinfo);
    int jpeg_read_header(j_decompress_ptr cinfo, boolean require_image);
};
CCeiLibJpegDll::CCeiLibJpegDll()
{
}
CCeiLibJpegDll::~CCeiLibJpegDll()
{
}
struct jpeg_error_mgr * CCeiLibJpegDll::jpeg_std_error(struct jpeg_error_mgr *err)
{
    return ::jpeg_std_error(err);
}
void CCeiLibJpegDll::jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize)
{
    return ::jpeg_CreateCompress(cinfo, version, structsize);
}
void CCeiLibJpegDll::jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize)
{
    return ::jpeg_CreateDecompress(cinfo, version, structsize);
}
void CCeiLibJpegDll::jpeg_set_defaults(j_compress_ptr cinfo)
{
    return ::jpeg_set_defaults(cinfo);
}
void CCeiLibJpegDll::jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline)
{
    return ::jpeg_set_quality(cinfo, quality, force_baseline);
}
void CCeiLibJpegDll::jpeg_write_marker(j_compress_ptr cinfo, int marker, const JOCTET *dataptr, unsigned int datalen)
{
    return ::jpeg_write_marker(cinfo, marker, dataptr, datalen);
}
void CCeiLibJpegDll::jpeg_destroy_compress(j_compress_ptr cinfo)
{
    return ::jpeg_destroy_compress(cinfo);
}
void CCeiLibJpegDll::jpeg_destroy_decompress(j_decompress_ptr cinfo)
{
    return ::jpeg_destroy_decompress(cinfo);
}
void CCeiLibJpegDll::jpeg_mem_dest(j_compress_ptr cinfo, unsigned char **outbuffer, unsigned long *outsize)
{
#ifdef __ANDROID__
    return ::jpeg_mem_dest(cinfo, outbuffer, (size_t*)outsize);
#else
    return ::jpeg_mem_dest(cinfo, outbuffer, outsize);
#endif
}
void CCeiLibJpegDll::jpeg_mem_src(j_decompress_ptr cinfo, const unsigned char *inbuffer, unsigned long insize)
{
    return ::jpeg_mem_src(cinfo, inbuffer, insize);
}
void CCeiLibJpegDll::jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables)
{
    return ::jpeg_start_compress(cinfo, write_all_tables);
}
boolean CCeiLibJpegDll::jpeg_start_decompress(j_decompress_ptr cinfo)
{
    return ::jpeg_start_decompress(cinfo);
}
JDIMENSION CCeiLibJpegDll::jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines)
{
    return ::jpeg_write_scanlines(cinfo, scanlines, num_lines);
}
JDIMENSION CCeiLibJpegDll::jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines)
{
    return ::jpeg_read_scanlines(cinfo, scanlines, max_lines);

}
void CCeiLibJpegDll::jpeg_finish_compress(j_compress_ptr cinfo)
{
    return ::jpeg_finish_compress(cinfo);
}
boolean CCeiLibJpegDll::jpeg_finish_decompress(j_decompress_ptr cinfo)
{
   return ::jpeg_finish_decompress(cinfo);
}
int CCeiLibJpegDll::jpeg_read_header(j_decompress_ptr cinfo, boolean require_image)
{
    return ::jpeg_read_header(cinfo, require_image);
}
#else
class CCeiLibJpegDll
{
public:
	CCeiLibJpegDll();
	~CCeiLibJpegDll();
	void load();
	void unload();	
	struct jpeg_error_mgr * jpeg_std_error(struct jpeg_error_mgr *err);
	void jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize);
	void jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize);
	void jpeg_set_defaults(j_compress_ptr cinfo);
	void jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline);
	void jpeg_write_marker(j_compress_ptr cinfo, int marker, const JOCTET *dataptr, unsigned int datalen);
	void jpeg_destroy_compress(j_compress_ptr cinfo);
	void jpeg_destroy_decompress(j_decompress_ptr cinfo);
	void jpeg_mem_dest(j_compress_ptr cinfo, unsigned char **outbuffer, unsigned long *outsize);
	void jpeg_mem_src(j_decompress_ptr cinfo, const unsigned char *inbuffer, unsigned long insize);
	void jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables);
	boolean jpeg_start_decompress(j_decompress_ptr cinfo);
	JDIMENSION jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines);
	JDIMENSION jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines);
	void jpeg_finish_compress(j_compress_ptr cinfo);
	boolean jpeg_finish_decompress(j_decompress_ptr cinfo);
	int jpeg_read_header(j_decompress_ptr cinfo, boolean require_image);
private:
	void *get_proc_address(const char *api_name);
	static char *load_error();
	typedef struct jpeg_error_mgr * (*lpfn_jpeg_std_error)(struct jpeg_error_mgr *err);
	typedef void (*lpfn_jpeg_CreateCompress)(j_compress_ptr cinfo, int version, size_t structsize);
	typedef void (*lpfn_jpeg_CreateDecompress)(j_decompress_ptr cinfo, int version, size_t structsize);
	typedef void (*lpfn_jpeg_set_defaults)(j_compress_ptr cinfo);
	typedef void (*lpfn_jpeg_set_quality)(j_compress_ptr cinfo, int quality, boolean force_baseline);
	typedef void (*lpfn_jpeg_write_marker)(j_compress_ptr cinfo, int marker, const JOCTET *dataptr, unsigned int datalen);
	typedef void (*lpfn_jpeg_destroy_compress)(j_compress_ptr cinfo);
	typedef void (*lpfn_jpeg_destroy_decompress)(j_decompress_ptr cinfo);
	typedef void (*lpfn_jpeg_mem_dest)(j_compress_ptr cinfo, unsigned char **outbuffer, unsigned long *outsize);
	typedef void (*lpfn_jpeg_mem_src)(j_decompress_ptr cinfo, const unsigned char *inbuffer, unsigned long insize);
	typedef void (*lpfn_jpeg_start_compress)(j_compress_ptr cinfo, boolean write_all_tables);
	typedef boolean (*lpfn_jpeg_start_decompress)(j_decompress_ptr cinfo);
	typedef JDIMENSION (*lpfn_jpeg_write_scanlines)(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines);
	typedef JDIMENSION (*lpfn_jpeg_read_scanlines)(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines);
	typedef void (*lpfn_jpeg_finish_compress)(j_compress_ptr cinfo);
	typedef boolean (*lpfn_jpeg_finish_decompress)(j_decompress_ptr cinfo);
	typedef int (*lpfn_jpeg_read_header)(j_decompress_ptr cinfo, boolean require_image);
	struct {
		lpfn_jpeg_std_error jpeg_std_error;
		lpfn_jpeg_CreateCompress jpeg_CreateCompress;
		lpfn_jpeg_CreateDecompress jpeg_CreateDecompress;
		lpfn_jpeg_set_defaults jpeg_set_defaults;
		lpfn_jpeg_set_quality jpeg_set_quality;
		lpfn_jpeg_write_marker jpeg_write_marker;
		lpfn_jpeg_destroy_compress jpeg_destroy_compress;
		lpfn_jpeg_destroy_decompress jpeg_destroy_decompress;
		lpfn_jpeg_mem_dest jpeg_mem_dest;
		lpfn_jpeg_mem_src jpeg_mem_src;
		lpfn_jpeg_start_compress jpeg_start_compress;
		lpfn_jpeg_start_decompress jpeg_start_decompress;
		lpfn_jpeg_write_scanlines jpeg_write_scanlines;
		lpfn_jpeg_read_scanlines jpeg_read_scanlines;
		lpfn_jpeg_finish_compress jpeg_finish_compress;
		lpfn_jpeg_finish_decompress jpeg_finish_decompress;
		lpfn_jpeg_read_header jpeg_read_header;
	}m_lpfn;
	void *m_hd;
};
CCeiLibJpegDll::CCeiLibJpegDll(): m_hd(NULL)
{
	memset(&m_lpfn, 0 , sizeof(m_lpfn));
	load();
}
CCeiLibJpegDll::~CCeiLibJpegDll()
{
	unload();
}
void *CCeiLibJpegDll::get_proc_address(const char *api_name)
{
#ifdef _WIN32
	HMODULE hm = (HMODULE)m_hd;
	return GetProcAddress(hm, api_name);
#else
	return dlsym(m_hd, api_name);
#endif
}
char *CCeiLibJpegDll::load_error()
{
#ifdef _WIN32
	return (char*)"error";
#else
	return dlerror();
#endif
}
void CCeiLibJpegDll::load()
{
	if (m_hd) return;
#ifdef _WIN32
	char path[256];
	GetModuleFileName(g_hModule, path, sizeof(path));
	char* p = strrchr(path, '\\');
	if (p) {
		*p = 0;
	} else{
		p = strrchr(path, '/');
		if (p) *p = 0;
	}
	strcat(path, "\\libjpeg.dll");
	if (FileExists(path)) {
		m_hd = LoadLibrary(path);
	}
	else {
		ceisdk_get_library_path(path);
		strcat(path, "libjpeg.dll");
		m_hd = LoadLibrary(path);
	}
#else
	char path[] = "libjpeg.so";
	m_hd = dlopen(path, RTLD_LAZY);
#endif
	if (m_hd == NULL) {
		#ifdef _WIN32
		#else
		printf("libjpeg can't be loaded:%s\r\n", dlerror());
		#endif
		return;
	}

	m_lpfn.jpeg_std_error = (lpfn_jpeg_std_error)get_proc_address("jpeg_std_error");
	if (m_lpfn.jpeg_std_error == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_CreateCompress = (lpfn_jpeg_CreateCompress)get_proc_address("jpeg_CreateCompress");
	if (m_lpfn.jpeg_CreateCompress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_CreateDecompress = (lpfn_jpeg_CreateDecompress)get_proc_address("jpeg_CreateDecompress");
	if (m_lpfn.jpeg_CreateDecompress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_set_defaults = (lpfn_jpeg_set_defaults)get_proc_address("jpeg_set_defaults");
	if (m_lpfn.jpeg_set_defaults == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_set_quality = (lpfn_jpeg_set_quality)get_proc_address("jpeg_set_quality");
	if (m_lpfn.jpeg_set_quality == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_write_marker = (lpfn_jpeg_write_marker)get_proc_address("jpeg_write_marker");
	if (m_lpfn.jpeg_write_marker == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_destroy_compress = (lpfn_jpeg_destroy_compress)get_proc_address("jpeg_destroy_compress");
	if (m_lpfn.jpeg_destroy_compress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_destroy_decompress = (lpfn_jpeg_destroy_decompress)get_proc_address("jpeg_destroy_decompress");
	if (m_lpfn.jpeg_destroy_decompress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_mem_dest = (lpfn_jpeg_mem_dest)get_proc_address("jpeg_mem_dest");
	if (m_lpfn.jpeg_mem_dest == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_mem_src = (lpfn_jpeg_mem_src)get_proc_address("jpeg_mem_src");
	if (m_lpfn.jpeg_mem_src == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_start_compress = (lpfn_jpeg_start_compress)get_proc_address("jpeg_start_compress");
	if (m_lpfn.jpeg_start_compress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_start_decompress = (lpfn_jpeg_start_decompress)get_proc_address("jpeg_start_decompress");
	if (m_lpfn.jpeg_start_decompress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_write_scanlines = (lpfn_jpeg_write_scanlines)get_proc_address("jpeg_write_scanlines");
	if (m_lpfn.jpeg_write_scanlines == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_read_scanlines = (lpfn_jpeg_read_scanlines)get_proc_address("jpeg_read_scanlines");
	if (m_lpfn.jpeg_read_scanlines == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_finish_compress = (lpfn_jpeg_finish_compress)get_proc_address("jpeg_finish_compress");
	if (m_lpfn.jpeg_finish_compress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_finish_decompress = (lpfn_jpeg_finish_decompress)get_proc_address("jpeg_finish_decompress");
	if (m_lpfn.jpeg_finish_decompress == NULL) printf("dlsym() error %s\r\n", load_error());

	m_lpfn.jpeg_read_header = (lpfn_jpeg_read_header)get_proc_address("jpeg_read_header");
	if (m_lpfn.jpeg_read_header == NULL) printf("dlsym() error %s\r\n", load_error());

}
void CCeiLibJpegDll::unload()
{
#ifdef _WIN32
	if (m_hd) {
		HMODULE hm = (HMODULE)m_hd;
		FreeLibrary(hm);
		m_hd = NULL;
	}
#else
	if (m_hd) {
		dlclose(m_hd);
		m_hd=NULL;
	}
#endif
}
struct jpeg_error_mgr * CCeiLibJpegDll::jpeg_std_error(struct jpeg_error_mgr *err)
{
	if (m_lpfn.jpeg_std_error) return m_lpfn.jpeg_std_error(err);
	return NULL;
}
void CCeiLibJpegDll::jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize)
{
	if (m_lpfn.jpeg_CreateCompress) return m_lpfn.jpeg_CreateCompress(cinfo, JPEG_LIB_VERSION, structsize);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize)
{	
	if (m_lpfn.jpeg_CreateDecompress) return m_lpfn.jpeg_CreateDecompress(cinfo, JPEG_LIB_VERSION, structsize);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_set_defaults(j_compress_ptr cinfo)
{
	if (m_lpfn.jpeg_set_defaults) return m_lpfn.jpeg_set_defaults(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline)
{
	if (m_lpfn.jpeg_set_quality) return m_lpfn.jpeg_set_quality(cinfo, quality, force_baseline);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_write_marker(j_compress_ptr cinfo, int marker, const JOCTET *dataptr, unsigned int datalen)
{
	if (m_lpfn.jpeg_write_marker) return m_lpfn.jpeg_write_marker(cinfo, marker, dataptr, datalen);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_destroy_compress(j_compress_ptr cinfo)
{
	if (m_lpfn.jpeg_destroy_compress) return m_lpfn.jpeg_destroy_compress(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_destroy_decompress(j_decompress_ptr cinfo)
{
	if (m_lpfn.jpeg_destroy_decompress) return m_lpfn.jpeg_destroy_decompress(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_mem_dest(j_compress_ptr cinfo, unsigned char **outbuffer, unsigned long *outsize)
{
	if (m_lpfn.jpeg_mem_dest) return m_lpfn.jpeg_mem_dest(cinfo, outbuffer, outsize);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_mem_src(j_decompress_ptr cinfo, const unsigned char *inbuffer, unsigned long insize)
{
	if (m_lpfn.jpeg_mem_src) return m_lpfn.jpeg_mem_src(cinfo, inbuffer, insize);
	printf("ERROR:L:%d\r\n", __LINE__);
}
void CCeiLibJpegDll::jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables)
{
	if (m_lpfn.jpeg_start_compress) return m_lpfn.jpeg_start_compress(cinfo, write_all_tables);
	printf("ERROR:L:%d\r\n", __LINE__);
}
boolean CCeiLibJpegDll::jpeg_start_decompress(j_decompress_ptr cinfo)
{
	if (m_lpfn.jpeg_start_decompress) return m_lpfn.jpeg_start_decompress(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
	return (boolean)0;
}
JDIMENSION CCeiLibJpegDll::jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines)
{
	if (m_lpfn.jpeg_write_scanlines) return m_lpfn.jpeg_write_scanlines(cinfo, scanlines, num_lines);
	printf("ERROR:L:%d\r\n", __LINE__);
	JDIMENSION out={0};
	return out;
}
JDIMENSION CCeiLibJpegDll::jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines)
{
	if (m_lpfn.jpeg_read_scanlines) return m_lpfn.jpeg_read_scanlines(cinfo, scanlines, max_lines);
	printf("ERROR:L:%d\r\n", __LINE__);
	JDIMENSION out={0};
	return out;
}
void CCeiLibJpegDll::jpeg_finish_compress(j_compress_ptr cinfo)
{
	if (m_lpfn.jpeg_finish_compress) return m_lpfn.jpeg_finish_compress(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
	return;
}
boolean CCeiLibJpegDll::jpeg_finish_decompress(j_decompress_ptr cinfo)
{
	if (m_lpfn.jpeg_finish_decompress) return m_lpfn.jpeg_finish_decompress(cinfo);
	printf("ERROR:L:%d\r\n", __LINE__);
	return  (boolean)0;;
}
int CCeiLibJpegDll::jpeg_read_header(j_decompress_ptr cinfo, boolean require_image)
{
	if (m_lpfn.jpeg_read_header) return m_lpfn.jpeg_read_header(cinfo, require_image);
	printf("ERROR:L:%d\r\n", __LINE__);
	return -1;
}
#endif
}

long ceisdk_jpeg_comp(LPCEICSDSDKJPEGIMAGEINFO dst, LPCEICSDSDKJPEGIMAGEINFO src, int quality, char *comment, void *plib)
{
	//WriteLog("ceisdk_jpeg_comp() start");
	//printf("ceijpeg_comp() start\r\n");
	CCeiLibJpegDll *pdll=NULL;
	std::unique_ptr<CCeiLibJpegDll>dll;
	if (plib==NULL) {
		dll.reset(new CCeiLibJpegDll);
		pdll = dll.get();
	} else {
		pdll = (CCeiLibJpegDll*)plib;
	}
	//unsigned long jSize = 0;
	//unsigned char*jBuf = NULL;
	unsigned char* image=src->lpImage;
	int width=(int)src->lWidth;
	int height=(int)src->lHeight;
	int spp=(int)src->lSpp;
	unsigned long jpegSize=0;
	unsigned char *jpegBuffer=NULL;
	struct jpeg_compress_struct cinfo={0};
	struct jpeg_error_mgr jerr={0};
	JSAMPROW row_pointer[1]={0};
	int row_stride=0;
	cinfo.err = pdll->jpeg_std_error(&jerr);
	pdll->jpeg_create_compress(&cinfo);
	cinfo.image_width = width;
	cinfo.image_height = height;
	if (spp==1) {
		// Input is greyscale, 1 byte per pixel
		cinfo.input_components = 1;
		cinfo.in_color_space = JCS_GRAYSCALE;
		// 1 BPP
		row_stride = width;
	} else {
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		row_stride = width * 3;
	}
	pdll->jpeg_set_defaults(&cinfo);
	pdll->jpeg_set_quality(&cinfo, quality, TRUE);
#ifdef _WIN32
	unsigned long original_jpegSize = 0;
	unsigned char* original_jpegBuffer = NULL;
	original_jpegSize  =jpegSize = src->lSync * src->lHeight;
	original_jpegBuffer = jpegBuffer = (unsigned char*)malloc(jpegSize);
#endif
	pdll->jpeg_mem_dest(&cinfo, &jpegBuffer, &jpegSize);//library will alloc memory. client must free this memory after using it.
	pdll->jpeg_start_compress(&cinfo, TRUE);
	if (comment) {
		pdll->jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET*)comment, (unsigned int)strlen(comment));
	}
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &image[cinfo.next_scanline * row_stride];
		pdll->jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	pdll->jpeg_finish_compress(&cinfo);
	pdll->jpeg_destroy_compress(&cinfo);
#ifdef _WIN32
	if (original_jpegSize > jpegSize) {
		jpegBuffer = (unsigned char*)malloc(jpegSize);
		memcpy(jpegBuffer, original_jpegBuffer, jpegSize);
		free(original_jpegBuffer);
	}
#endif
	dst->lpImage = jpegBuffer;
	dst->tImageSize = jpegSize;
	dst->lWidth = src->lWidth;
	dst->lHeight = src->lHeight;
	dst->lSpp = src->lSpp;
	dst->lBps = src->lBps;
	dst->lXResolution = src->lXResolution;
	dst->lYResolution = src->lYResolution;
	dst->lSync = src->lSync;
	//printf("ceijpeg_comp() end\r\n");
	//WriteLog("ceisdk_jpeg_comp() end");
	return 0;
}
long ceisdk_jpeg_decomp(LPCEICSDSDKJPEGIMAGEINFO dst, LPCEICSDSDKJPEGIMAGEINFO src, void *plib)
{
	//printf("ceijpeg_decomp() start\r\n");
	CCeiLibJpegDll *pdll=NULL;
	std::unique_ptr<CCeiLibJpegDll>dll;
	if (plib==NULL) {
		dll.reset(new CCeiLibJpegDll);
		pdll = dll.get();
	} else {
		pdll = (CCeiLibJpegDll*)plib;
	}
    long src_len = get_height_from_jpg(src->lpImage, (long)src->tImageSize);
    if (src_len>65500) {
        set_height_to_jpg(src->lpImage, (long)src->tImageSize, 65500);
    } else if (src_len<=0) {
        set_height_to_jpg(src->lpImage, (long)src->tImageSize, (unsigned short)src->lHeight);
    }
	//int rc=0;
	struct jpeg_decompress_struct cinfo={0};
	struct jpeg_error_mgr jerr={0};
	cinfo.err = pdll->jpeg_std_error(&jerr);	
	pdll->jpeg_create_decompress(&cinfo);
	pdll->jpeg_mem_src(&cinfo, src->lpImage, (unsigned long)src->tImageSize);
	pdll->jpeg_read_header(&cinfo, TRUE);
	pdll->jpeg_start_decompress(&cinfo);
	dst->lWidth = cinfo.output_width;
	dst->lHeight = cinfo.output_height;
	if (cinfo.output_components==1) {
		dst->lSpp=1;
		dst->lBps=8;
		dst->lSync=dst->lWidth;
	} else {
		dst->lSpp=3;
		dst->lBps=8;
		dst->lSync=dst->lWidth*3;
	}
	//printf("jpeg image\r\n");
	//printf("width:%ld, height:%ld\r\n", dst->lWidth, dst->lHeight);
	//printf("pixel size:%d\r\n", cinfo.output_components);
	dst->tImageSize = dst->lSync * dst->lHeight;
	dst->lpImage = (unsigned char*)malloc(dst->tImageSize);
	if (dst->lpImage==NULL) {
		pdll->jpeg_finish_decompress(&cinfo);
		pdll->jpeg_destroy_decompress(&cinfo);		
		return ENOMEM;
	}
	unsigned char * ptr = dst->lpImage;
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		pdll->jpeg_read_scanlines(&cinfo, &ptr, 1);
		ptr+=dst->lSync;
	}
	pdll->jpeg_finish_decompress(&cinfo);
	pdll->jpeg_destroy_decompress(&cinfo);

	dst->lXResolution = get_xdpi_from_jpg((unsigned char*)src->lpImage, (long)src->tImageSize);
	dst->lYResolution = get_ydpi_from_jpg((unsigned char*)src->lpImage, (long)src->tImageSize);
	//printf("ceijpeg_decomp() end\r\n");
	return 0;
}

class CJpegComp : public ICeiJpeg
{
public:
	CJpegComp(long quality, char *comment=(char*)"", void *plib=NULL);
	virtual ~CJpegComp();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	long run(ICeiImage **ppInOut);
private:
	long m_ref;
	long m_quality;
	char *m_comment;
	void *m_plib;
	CCeiLibJpegDll m_dll;
};
CJpegComp::CJpegComp(long quality, char *comment, void *plib):m_ref(1), m_quality(quality), m_comment(comment), m_plib(plib)
{}
CJpegComp::~CJpegComp()
{}
long CJpegComp::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CJpegComp::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CJpegComp::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CJpegComp::run(ICeiImage **ppInOut)
{
	ICeiImage *pin = *ppInOut;
	if (pin->bps()==1) return 0;
	if (pin->comptype()) return 0;

	CEICSDSDKJPEGIMAGEINFO dst={sizeof(dst)};
	CEICSDSDKJPEGIMAGEINFO src={sizeof(src)};
	src.lpImage=(unsigned char*)pin->img();	
	src.lWidth=pin->width();
	src.lHeight=pin->height();
	src.lSync=pin->sync();	
	src.tImageSize=pin->size();
	src.lBps=pin->bps();
	src.lSpp=pin->spp();
	src.lXResolution=pin->xdpi();
	src.lYResolution=pin->ydpi();
	ceisdk_jpeg_comp(&dst, &src, (int)m_quality , m_comment?m_comment:(char*)"", m_plib?m_plib:(void *)&m_dll);
	pin->Release();
    CVSCSDSDKImage *pout = create_vscsdsdk_image();   
    pout->width(dst.lWidth);
    pout->height(dst.lHeight);
    pout->xdpi(dst.lXResolution);
    pout->ydpi(dst.lYResolution);
    pout->sync(src.lSync);
    pout->spp(dst.lSpp);
    pout->bps(dst.lBps);
    pout->comptype(1);
    pout->compinfo(m_quality);
    pout->attach((char *)dst.lpImage, (long)dst.tImageSize, true);
    *ppInOut = pout;
    return 0;
}
class CJpegDecomp : public ICeiJpeg
{
public:
	CJpegDecomp(void *plib=NULL);
	virtual ~CJpegDecomp();
	long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut);
	unsigned long STDMETHODCALLTYPE AddRef();
	unsigned long STDMETHODCALLTYPE Release();
	long run(ICeiImage **ppInOut);
private:
	long m_ref;
	void *m_plib;
	CCeiLibJpegDll m_dll;	
};
CJpegDecomp::CJpegDecomp(void *plib):m_ref(1), m_plib(plib)
{}
CJpegDecomp::~CJpegDecomp()
{}
long CJpegDecomp::QueryInterface(REFIID id, void **ppOut)
{
	return -1;
}
unsigned long CJpegDecomp::AddRef()
{
	m_ref++;
	return m_ref;
}
unsigned long CJpegDecomp::Release()
{
	m_ref--;
	if (m_ref<=0) {
		delete this;
		return 0;
	}
	return m_ref;
}
long CJpegDecomp::run(ICeiImage **ppInOut)
{
	long ret = 0;
	ICeiImage *pin = *ppInOut;
	if (pin->bps()==1) return 0;
	if (pin->comptype()==0) return 0;
	CEICSDSDKJPEGIMAGEINFO dst={sizeof(dst)};
	CEICSDSDKJPEGIMAGEINFO src={sizeof(src)};
	src.lpImage=(unsigned char*)pin->img();	
	src.lWidth=pin->width();
	src.lHeight=pin->height();
	src.lSync=pin->sync();	
	src.tImageSize=pin->size();
	src.lBps=pin->bps();
	src.lSpp=pin->spp();
	src.lXResolution=pin->xdpi();
	src.lYResolution=pin->ydpi();
	ret = ceisdk_jpeg_decomp(&dst, &src, m_plib?m_plib:(void *)&m_dll);
	if (ret) return ret;
	pin->Release();
    CVSCSDSDKImage *pout = create_vscsdsdk_image(); 
	if (pout == NULL) {
		printf("no memory L:%d F:%s\r\n", __LINE__, __FILE__);
		return -1;
	}
    pout->width(dst.lWidth);
    pout->height(dst.lHeight);
    pout->xdpi(dst.lXResolution);
    pout->ydpi(dst.lYResolution);
    pout->sync(dst.lSync);
    pout->spp(dst.lSpp);
    pout->bps(dst.lBps);
    pout->attach((char*)dst.lpImage, (long)dst.tImageSize, true);
    *ppInOut = pout;
    return 0;
}
ICeiJpeg *jpeg_comp(long quality)
{
	return new CJpegComp(quality);
}
ICeiJpeg *jpeg_decmp()
{
	return new CJpegDecomp();
}
long ceisdk_jpeg_comp(ICeiImage **ppInOut, int quality, char *comment, void *plib)
{
	CJpegComp cmp(quality, comment, plib);
	return cmp.run(ppInOut);
}
long ceisdk_jpeg_decomp(ICeiImage **ppInOut, void *plib)
{
	CJpegDecomp decmp(plib);
	return decmp.run(ppInOut);
}
long read_jpeg_file(char *pfilename, ICeiImage **ppOut, void *plib)
{
	long sz = get_file_size(pfilename);
	if (sz==0) return errno;

	FILE *fp = fopen(pfilename, "rb");
	if (fp==NULL) {
		return errno;
	}
	char *ptr = new char [sz+8];
	size_t rd = fread(ptr, sz, 1, fp);
	if (rd==0) printf("fread() error %s\r\n", strerror(errno));
	fclose(fp);
	CVSCSDSDKImage *pout = create_vscsdsdk_image();
	if (pout) {
		
		pout->width(get_width_from_jpg((unsigned char*)ptr, sz));
		pout->height(get_height_from_jpg((unsigned char*)ptr, sz));
		pout->xdpi(get_xdpi_from_jpg((unsigned char*)ptr, sz));
		pout->ydpi(get_ydpi_from_jpg((unsigned char*)ptr, sz));
		pout->spp(3);
		pout->bps(8);
		pout->sync(pout->width()*3);
		pout->comptype(1);
		pout->compinfo(80);
		pout->attach(ptr, sz, false);

		*ppOut = pout;
	}
	
	return 0;
}
//JPEGファイル読み込み
long ceisdk_read_jpeg_file(const char *pfilename, ICeiImage **ppOut)
{
	return read_jpeg_file((char*)pfilename, ppOut, NULL);
}
//JPEGファイル書き出し
long ceisdk_write_jpeg_file(const char *pfilename, ICeiImage *pIn)
{
	if (pIn->comptype() == 0) return  -1;
	FILE *fp = fopen(pfilename, "wb");
	if (fp) {
		fwrite(pIn->img(), pIn->size(), 1, fp);
		fclose(fp);
	}
	return 0;
}
