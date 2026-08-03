/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include "ipsdk.h"
#include "sdk_image_util.h"
#include "ceilogwrite.h"

#ifndef FAILED
#define FAILED(x) (x<0)
#endif

namespace {
	long round_dpi(long v)
	{
		if (v==0) {
			v=300;
		} else if (v>100) {
			v/=100;
			v*=100;
		}
		return v;
	}
}
#pragma pack(push,2)
    typedef struct tag_BITMAPFILEHEADER {
        unsigned short	bfType;
        unsigned int	bfSize;
        unsigned short	bfReserved1;
        unsigned short	bfReserved2;
        unsigned int	bfOffBits;
    } _BITMAPFILEHEADER;
#pragma pack(pop)
    
typedef struct tag_BITMAPINFOHEADER {
    unsigned int	biSize;
    int			biWidth;
    int			biHeight;
    unsigned short	biPlanes;
    unsigned short	biBitCount;
    unsigned int	biCompression;
    unsigned int	biSizeImage;
    int			biXPelsPerMeter;
    int			biYPelsPerMeter;
    unsigned int	biClrUsed;
    unsigned int	biClrImportant;
} _BITMAPINFOHEADER;

typedef struct tag_RTBQUAD {
    unsigned char	rgbBlue;
    unsigned char	rgbGreen;
    unsigned char	rgbRed;
    unsigned char	rgbReserved;
} _RGBQUAD;
namespace bmputil {

	class CFile
	{
	public:
		CFile():m_hd(NULL){}
		~CFile() {
			close();
		}
		long open(const char *pfilename, long bread) 
		{
			//printf("CFile::open(%s, %ld) start\r\n", pfilename, bread);
			close();
			m_hd = fopen(
				pfilename, 
				bread?"rb":"wb");
			if (m_hd==NULL) {
				long out = errno;
				//printf("fopen(%s, \"%s\") error %s\r\n", pfilename, bread?"rb":"wb", strerror(errno));
				return out;
			}
			//printf("CFile::open() end\r\n");
			return 0;
		}
		void close() {
			if (m_hd) fclose(m_hd);
			m_hd=NULL;
		}
		long write(void *pbuf, long size) {
			//printf("CFile::write(pbuf, %ld) start\r\n", size);
			if (invalid_handle()) return -1;
			fwrite(pbuf, size, 1, m_hd);
			//printf("CFile::write() end\r\n");
			return 0;
		}
		long read(void *pbuf, long size) {
			if (invalid_handle()) return -1;
			size_t ret = fread(pbuf, size, 1, m_hd);
			return ret!=(size_t)(size*1);
		}
		bool invalid_handle() {return m_hd==NULL;}
	private:
		FILE *m_hd;
	};

}
namespace {
void exchangeRB(unsigned char * p, long width)
{
	long max = width*3;
	for (long i=0; i < max; i+=3) {
		unsigned char buff = p[i];
		p[i] = p[i+2];
		p[i+2] = buff;
	}
/*
	for (long i=0; i<width; i++) {
		unsigned char buff = p[i*3];
		p[i*3] = p[i*3 + 2];
		p[i*3 + 2] = buff;
	}
*/
/*
	for (long i=0; i<width; i++) {
		unsigned char buff = *p;
		*p = *(p+2);
		*(p+2) = buff;
		p+=3;
	}
*/
/*
	while (width--) {
		unsigned char buff = *p;
		*p = *(p+2);
		*(p+2) = buff;
		p+=3;
	}
*/
}
long read_image(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	bool color = pbm->bpp==24;
	pbm->pimg = new unsigned char[pbm->sync * pbm->height];
	if (pbm->pimg==NULL) return ENOMEM;
	unsigned char *buff = (unsigned char*)pbm->pimg;
	buff += pbm->sync * (pbm->height-1);
	for (int h = 0; h < pbm->height; h++){
		long hr = f.read(buff, pbm->sync);
		if (FAILED(hr)) {
			delete [] (unsigned char*)pbm->pimg;
			return hr;
		}
		if (color) exchangeRB(buff, pbm->width);
		buff -= pbm->sync;
	}	
	return 0;
}
void revise_binary_image(unsigned char *colormap, CCeiBitMapImg *pbm)
{
	if (colormap[0]!=0xff) {
		unsigned char *p = (unsigned char*)pbm->pimg;
		long max = pbm->sync*pbm->height;
		for (long i=0; i<max; i++) {
			p[i] = ~p[i];
		}
	}
}
long read_binary(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	unsigned char colormap[2*4]={0};
	long hr = f.read(colormap, sizeof(colormap));
	if (FAILED(hr)) return hr;
	pbm->sync = ((pbm->width+7)/8 + 3)/4*4;
	hr = read_image(f, pbm);
	if (FAILED(hr)) return hr;
	revise_binary_image(colormap, pbm);
	return 0;
}
void revise_gray_image(unsigned char *colormap, CCeiBitMapImg *pbm)
{
	if (colormap[0]!=0) {
		unsigned char *p = (unsigned char*)pbm->pimg;
		long max = pbm->sync*pbm->height;
		for (long i=0; i<max; i++) {
			p[i] = ~p[i];
		}
	}
}
long read_gray(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	unsigned char colormap[256*4]={0};
	long hr = f.read(colormap, sizeof(colormap));
	if (FAILED(hr)) return hr;
	pbm->sync = (pbm->width + 3) / 4 * 4;
	hr = read_image(f, pbm);
	if (FAILED(hr)) return hr;
	revise_gray_image(colormap, pbm);
	return 0;
}


long read_4bitgray(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	unsigned char colormap[16*4]={0};
	long hr = f.read(colormap, sizeof(colormap));
	if (FAILED(hr)) return hr;
	pbm->sync = ((pbm->width + 1) / 2 + 3) / 4 * 4;
	hr = read_image(f, pbm);
	if (FAILED(hr)) return hr;
	revise_gray_image(colormap, pbm);
	return 0;
}
long read_color(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	pbm->sync = (pbm->width*3+3)/4*4;
	long hr = read_image(f, pbm);
	if (FAILED(hr)) return hr;
	return 0;
}
}
long read_bmp_info(const char *pfilename, CCeiBitMapImg *pbm)
{
	if (pfilename==NULL) return EINVAL;
	if (pbm==NULL) return EINVAL;

	long hr = 0;
	bmputil::CFile f;
	hr = f.open(pfilename, 1);
	_BITMAPFILEHEADER bfh={0};
	hr = f.read(&bfh, sizeof(bfh));
	if (FAILED(hr)) return hr;
	unsigned char *cmp = (unsigned char *)&bfh.bfType;
	if (cmp[0] != 'B' || cmp[1] != 'M') {
		return -1;
	}
	_BITMAPINFOHEADER bih={0};
	hr = f.read(&bih, sizeof(_BITMAPINFOHEADER));
	if (FAILED(hr)) return hr;
	pbm->width = bih.biWidth;
	pbm->height = bih.biHeight;
	pbm->bpp = bih.biBitCount;
	pbm->xdpi = (bih.biXPelsPerMeter*254+9999)/10000;
	pbm->ydpi = (bih.biYPelsPerMeter*254+9999)/10000;
	return 0;	
}
long read_bmp_file(const char *pfilename, CCeiBitMapImg *pbm)
{
	//printf("read_bmp_file(%s, pbm) start\r\n", pfilename);
	if (pfilename==NULL) return EINVAL;
	if (pbm==NULL) return EINVAL;

	long hr = 0;
	bmputil::CFile f;
	hr = f.open(pfilename, 1);
	_BITMAPFILEHEADER bfh={0};
	hr = f.read(&bfh, sizeof(bfh));
	if (FAILED(hr)) return hr;
	unsigned char *cmp = (unsigned char *)&bfh.bfType;
	if (cmp[0] != 'B' || cmp[1] != 'M') {
		return -1;
	}
	_BITMAPINFOHEADER bih={0};
	hr = f.read(&bih, sizeof(_BITMAPINFOHEADER));
	if (FAILED(hr)) return hr;
	pbm->width = bih.biWidth;
	pbm->height = bih.biHeight;
	pbm->bpp = bih.biBitCount;
	pbm->xdpi = (bih.biXPelsPerMeter*254+9999)/10000;
	pbm->ydpi = (bih.biYPelsPerMeter*254+9999)/10000;
	switch (bih.biBitCount) {
	case  1:hr = read_binary(f, pbm);break;
	case  8:hr = read_gray(f, pbm);  break;
	case 24:hr = read_color(f, pbm); break;
	case  4:hr = read_4bitgray(f, pbm);break;
	default:return -1;
	}
	//printf("read_bmp_file() end\r\n");
	return hr;
}

namespace {
long write_binary(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	long hr=0;
	unsigned char * pimg = pbm->pimg + pbm->sync * (pbm->height-1);
	long buffer_size = (pbm->width + 7) / 8;
	buffer_size = (buffer_size +3) / 4 * 4;
	unsigned char * pbuffer = new unsigned char[buffer_size];
	if (pbuffer==NULL) return ENOMEM;
	long copy_size = (pbm->width + 7) / 8;
  memset(pbuffer, 0, buffer_size);
	for (long h=0; h<pbm->height; h++) {
		memcpy(pbuffer, pimg, copy_size);
		hr = f.write(pbuffer, buffer_size);
		if (FAILED(hr)) {
			delete []pbuffer;
			return hr;
		}
		pimg -= pbm->sync;
	}
	delete []pbuffer;
	return hr;
}
long write_gray(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	long hr=0;
	unsigned char * pimg = pbm->pimg + pbm->sync * (pbm->height-1);
	long buffer_size = (pbm->width + 3) / 4 * 4;
	unsigned char * pbuffer = new unsigned char[buffer_size];
	if (pbuffer==NULL) return ENOMEM;

	long copy_size = pbm->width;
	for (long h=0; h<pbm->height; h++) {
		memcpy(pbuffer, pimg, copy_size);
		hr = f.write(pbuffer, buffer_size);
		if (FAILED(hr)) {
			delete []pbuffer;
			return hr;
		}
		pimg -= pbm->sync;
	}
	delete []pbuffer;
	return hr;
}
long write_4bitgray(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	long hr=0;
	unsigned char * pimg = pbm->pimg + pbm->sync * (pbm->height-1);
	long buffer_size = ((pbm->width + 1) / 2 + 3) / 4 * 4;
	unsigned char * pbuffer = new unsigned char[buffer_size];
	if (pbuffer==NULL) return ENOMEM;
	long copy_size =(pbm->width + 1) / 2;
	for (long h=0; h<pbm->height; h++) {
		memcpy(pbuffer, pimg, copy_size);
		hr = f.write(pbuffer, buffer_size);
		if (FAILED(hr)) {
			delete []pbuffer;
			return hr;
		}
		pimg -= pbm->sync;
	}
	delete []pbuffer;
	return hr;
}
long write_color(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	//printf("write_color() start\r\n");
	long hr=0;
	unsigned char * pimg = pbm->pimg + pbm->sync * (pbm->height-1);
	long buffer_size = ((pbm->width * 3) + 3) / 4 * 4;
	unsigned char * pbuffer = new unsigned char[buffer_size];
	if (pbuffer==NULL) return ENOMEM;
	for (long h=0; h<pbm->height; h++) {
		memcpy(pbuffer, pimg, pbm->width * 3);
		exchangeRB(pbuffer, pbm->width);
		hr = f.write(pbuffer, buffer_size);
		if (FAILED(hr)) {
			delete []pbuffer;
			return hr;
		}
		pimg -= pbm->sync;
	}
	delete []pbuffer;
	//printf("write_color() end\r\n");
	return hr;
}
long get_off_bits(CCeiBitMapImg *pbm)
{
	long out = sizeof(_BITMAPFILEHEADER)+ sizeof(_BITMAPINFOHEADER);
	switch (pbm->bpp) {
		case 1: out+=2*4;break;
		case 8: out+=256*4;break;
		case 4: out+=16*4;break;
		case 24:break;
		default:break;
	}
	return out;
}
long write_bmp_header(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	long hr=0;
	_BITMAPFILEHEADER bfh={0};
	unsigned char * pbfType = (unsigned char*)&bfh.bfType;
	pbfType[0] = 'B';
	pbfType[1] = 'M';
	bfh.bfOffBits=(unsigned int)get_off_bits(pbm);
	hr = f.write(&bfh, sizeof(bfh));
	if (FAILED(hr)) return hr;
	_BITMAPINFOHEADER bih={sizeof(_BITMAPINFOHEADER)};
	bih.biWidth = (int)pbm->width;
	bih.biHeight = (int)pbm->height;
    bih.biPlanes = 1;
	switch (pbm->bpp) {
	case 1:bih.biBitCount = 1;break;
	case 8:bih.biBitCount = 8;break;
	case 24:bih.biBitCount = 24;break;
	case 4:bih.biBitCount = 4;break;
	default:return EINVAL;
	}
    bih.biCompression = 0;//BI_RGB;
    bih.biSizeImage = 0;
	bih.biXPelsPerMeter = (int)(pbm->xdpi*10000+253)/254;
	bih.biYPelsPerMeter = (int)(pbm->ydpi*10000+253)/254;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	hr = f.write(&bih, sizeof(bih));
	if (FAILED(hr)) return hr;

	return hr;
}
long write_color_table(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	unsigned char colormap[256*4] = {0};
	int i=0;
	long hr = 0;
	switch(pbm->bpp){
	case 24:;break;
	case 8:
		for(i=0; i<256; i++){
			colormap[i*4]=i;
			colormap[i*4+1]=i;
			colormap[i*4+2]=i;
		}	
		hr = f.write(colormap, sizeof (colormap));
		if (FAILED(hr)) return hr;
		break;
	case 4:
		{
			long val[16];
			for (long m=0; m<16; m++) {
				val[m]=(255 * m) / 15;
				if (val[m]>255) val[m]=255;
			}
			for(i=0; i<16; i++){
				colormap[i*4]   = (unsigned char)val[i];
				colormap[i*4+1] = (unsigned char)val[i];
				colormap[i*4+2] = (unsigned char)val[i];
			}
		}
		hr = f.write(colormap, sizeof (colormap[0])*16*4);
		if (FAILED(hr)) return hr;
		break;
	case 1:
		colormap[4]=colormap[5]=colormap[6]=0;
		colormap[0]=colormap[1]=colormap[2]=(char)0xff;
		hr = f.write(colormap, 8);
		if (FAILED(hr)) return hr;
		break;
	default:return EINVAL;
	}
	return hr;
}
long write_image(bmputil::CFile &f, CCeiBitMapImg *pbm)
{
	long hr=0;
	switch(pbm->bpp){
	case 24:hr = write_color(f, pbm);break;
	case 8:hr = write_gray(f, pbm);break;
	case 1:hr = write_binary(f, pbm);break;
	case 4:hr = write_4bitgray(f, pbm);break;
	default:return EINVAL;
	}
	return hr;
}
bool FileExists(char *path)
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
}
long write_bmp_file(const char *pfilename, CCeiBitMapImg *pbm)
{
	//printf("write_bmp_file(%s, pbm) start\r\n", pfilename);
	//printf("w:%ld h:%ld sync:%ld xdpi:%ld ydpi:%ld bpp:%ld\r\n", pbm->width, pbm->height, pbm->sync, pbm->xdpi, pbm->ydpi, pbm->bpp);
	if (pbm==NULL) return EINVAL;
	if (pfilename==NULL) return EINVAL;
	long hr = 0;
	bmputil::CFile f;
	hr = f.open(pfilename, 0);
	if (FAILED(hr)) return hr;
	hr = write_bmp_header(f, pbm);
	if (FAILED(hr)) return hr;
	hr = write_color_table(f, pbm);
	if (FAILED(hr)) return hr;
	hr = write_image(f, pbm);
	if (FAILED(hr)) return hr;
	//printf("write_bmp_file() end\r\n");
	return hr;
}

long read_bmp_file(const char *pfilename, ICeiImage **ppOut)
{
	CCeiBitMapImg bm;
	long out = read_bmp_file(pfilename, &bm);
	if (out) return out;

	CVSCSDSDKImage *pout = create_vscsdsdk_image();
	if (pout) {
		pout->width(bm.width);
		pout->height(bm.height);
		pout->xdpi(round_dpi(bm.xdpi));
		pout->ydpi(round_dpi(bm.ydpi));
		pout->spp(bm.bpp==24?3:1);
		pout->bps(bm.bpp==1?1:8);
		pout->sync(bm.sync);
		pout->comptype(0);
		pout->attach((char*)bm.pimg, bm.sync*bm.height, false);
		*ppOut = pout;
	}
	bm.pimg=NULL;
	return 0;
}
long write_bmp_file(const char *pfilename, ICeiImage *pbm)
{
	CCeiBitMapImg bm;
	bm.pimg = (unsigned char*)pbm->img();
	bm.width = pbm->width();
	bm.height = pbm->height();
	bm.sync = pbm->sync();
	bm.xdpi = pbm->xdpi();
	bm.ydpi = pbm->ydpi();
	bm.bpp = pbm->spp() * pbm->bps();
	long out = write_bmp_file(pfilename, &bm);
	bm.pimg=NULL;
	return out;	
}
//BMPファイル読み込み
long ceisdk_read_bmp_file(const char *pfilename, ICeiImage **ppOut)
{
	return read_bmp_file(pfilename, ppOut);
}
//BMPファイル書き出し
long ceisdk_write_bmp_file(const char *pfilename, ICeiImage *pIn)
{
	return  write_bmp_file(pfilename, pIn);
}

#ifdef _WIN32
#include <windows.h>
#endif

void get_temp_directory(char * pdir, unsigned int count)
{
#ifdef _WIN32
	GetTempPathA(count, pdir);
#else
	strcpy(pdir, "/tmp/");
#endif
}

bool is_write_debug_bmp_file(const char * pfilestr)
{
#ifndef _DEBUG
	bool bIsWrite = false;
	// Debug時でないときに判定する
#ifdef _WIN32
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Canon Electronics Inc.\\ipsdk", 0, KEY_ALL_ACCESS, &hKey);
	if (lRes == S_OK)
	{
		DWORD dwType = 0;
		DWORD dwCount = sizeof(DWORD);
		DWORD dwValue = 0;
		lRes = RegQueryValueExA(hKey, pfilestr, NULL, &dwType, (LPBYTE)&dwValue, &dwCount);
		if (lRes == S_OK) {
			if (dwValue != 0) { bIsWrite = true; }
		}
		RegCloseKey(hKey);
	}
#else
	char path[1024] = {0};
	sprintf(path, "/tmp/%s", pfilestr);
	bIsWrite = FileExists(path);
#endif
#else
	//bool bIsWrite = true;
	bool bIsWrite = false;
#endif
	return bIsWrite;
}


void write_debug_bmp_file(const char * pfilestr, ICeiImage * pIn)
{
	// debug用のBMPファイル書き出し関数
	// pfilestrは画処理名を入れるべき
	bool bIsWrite = false;
	if (!is_write_debug_bmp_file(pfilestr)) { return; }
	
	char path[1024] = {0};
	char tmpdir[256] = {0};
	get_temp_directory(tmpdir, 256);

	for (int i = 1; i < 100; i++) {
		sprintf(path, "%s%s_%03d.bmp", tmpdir, pfilestr, i);
		if (!FileExists(path)) {
			bIsWrite = true;
			break;
		}
	}
	if (!bIsWrite) return;	// 画像が多過ぎる場合は書き込まない

	write_bmp_file(path, pIn);
}