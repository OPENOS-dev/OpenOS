/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <memory.h>
#include <vector>
#include "ceilogwrite.h"
#include "sdk_image_util.h"
#include "sdk_command_util.h"
#include "ipsdk.h"
#include "CeiImg.h"
#include "adjust_config.h"
namespace Cei
{
	namespace LLiPm
	{
		namespace SCANNER_NAME_NAMESPACE
		{
			extern Cei::LLiPm::CImg g_white[2];
			extern Cei::LLiPm::CImg g_black[2];
			enum {
				FRONT=0,
				BACK
			};
		}
	}
	template<class T>
	T ceimax(T a, T b)
	{
		return (((a) > (b)) ? (a) : (b));
	}
	template<class T>
	T ceimin(T a, T b)
	{
		return (((a) < (b)) ? (a) : (b));
	}
}
namespace {	
	void shading_gray(unsigned char* lpSrc, unsigned short* lpWhite, unsigned short* lpBlack, long w)
	{
		while (w--) {
			unsigned long iPix = *lpSrc;
			
			iPix -= *lpBlack;
			lpBlack++;

			iPix = Cei::ceimax(iPix, (unsigned long)0);
			unsigned long iWhite = *lpWhite;
			iPix = (iPix * iWhite) >> 12;
			lpWhite++;

			iPix = Cei::ceimin(iPix, (unsigned long)0xff);
			*lpSrc = (unsigned char)iPix;
			lpSrc++;
		}
	}
	void shading_gray(ICeiImage *pimg, Cei::LLiPm::CImg &white, Cei::LLiPm::CImg &black)
	{
		long lLine = pimg->height();
		long width = pimg->width();
		unsigned char* ptr=(unsigned char*)pimg->img();
		unsigned short* pBlack = (unsigned short*)black.getImagePtr();
		unsigned short* pWhite = (unsigned short*)white.getImagePtr();
		long sync = pimg->sync();
		while (lLine--) {
			shading_gray(ptr, pWhite, pBlack, width);
			ptr += sync;
		}
	}
	
	void shading_lineorder_color(ICeiImage *pimg, Cei::LLiPm::CImg &white, Cei::LLiPm::CImg &black)
	{
		enum {
			RED=0,
			GREEN,
			BLUE,
			RED_GREEN_BLUE_SIZE
		};
		long lLine = pimg->height();
		long width = pimg->width();
		unsigned char* ptr[RED_GREEN_BLUE_SIZE]={0};
		ptr[RED] = (unsigned char*)pimg->img();
		ptr[GREEN] = ptr[RED] + width;
		ptr[BLUE] = ptr[GREEN] + width;
		unsigned short* pblack[RED_GREEN_BLUE_SIZE]={0};
		pblack[RED] = (unsigned short*)black.getImagePtr();// r, g, b is all same
		pblack[GREEN] = (unsigned short*)black.getImagePtr();// r, g, b is all same
		pblack[BLUE] =  (unsigned short*)black.getImagePtr();// r, g, b is all same
		unsigned short* pwhite[RED_GREEN_BLUE_SIZE] = {0};
		pwhite[RED] = (unsigned short*)white.getImagePtr();
		pwhite[GREEN] = pwhite[RED]+white.getSync()/2;
		pwhite[BLUE] =  pwhite[GREEN]+white.getSync()/2;

		while (lLine--) {
			shading_gray(ptr[RED], pwhite[RED], pblack[RED], width);
			shading_gray(ptr[GREEN], pwhite[GREEN], pblack[GREEN], width);
			shading_gray(ptr[BLUE], pwhite[BLUE], pblack[BLUE], width);
			ptr[RED] += width * 3;
			ptr[GREEN] += width * 3;
			ptr[BLUE] += width * 3;
		}

	}
	void shading(ICeiImage *pimg, Cei::LLiPm::CImg &white, Cei::LLiPm::CImg &black)
	{
		if (pimg->width()!=white.getWidth()) {
			printf("ERROR:image width:%ld != white width:%ld\r\n", pimg->width(), white.getWidth());
			return;
		}
		if (pimg->width()!=black.getWidth()) {
			printf("ERROR:image width:%ld != black width:%ld\r\n", pimg->width(), black.getWidth());
			return;
		}
		if (pimg->spp()==3) {
			shading_lineorder_color(pimg, white, black);
		} else {
			shading_gray(pimg, white, black);
		}
	}
}
void shading_front(ICeiImage *pimg)
{
	shading(pimg, Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::SCANNER_NAME_NAMESPACE::FRONT], Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::SCANNER_NAME_NAMESPACE::FRONT]);
}
void shading_back(ICeiImage *pimg)
{
	shading(pimg, Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::SCANNER_NAME_NAMESPACE::BACK], Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::SCANNER_NAME_NAMESPACE::BACK]);
}
namespace {
	long send_shading_data(CScsiCommand& scanner, CStreamCmd& strm, Cei::LLiPm::CImg& data, bool black_or_white)
	{
		//WriteLog(_T("send_shading_data()(2) start"));

		long ret = 0;
		strm.black_or_white(black_or_white);
		if (data.getSpp() != 3) {
			//gray
			strm.rgb(CStreamCmd::RED);
			strm.shading((char*)data.getImagePtr(), data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
			strm.rgb(CStreamCmd::BLUE);
			strm.shading((char*)data.getImagePtr(), data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
			strm.rgb(CStreamCmd::GREEN);
			strm.shading((char*)data.getImagePtr(), data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
		}
		else {
			//color		
			strm.rgb(CStreamCmd::RED);
			strm.shading((char*)data.getImagePtr(), data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
			strm.rgb(CStreamCmd::BLUE);
			strm.shading((char*)data.getImagePtr() + data.getSync(), data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
			strm.rgb(CStreamCmd::GREEN);
			strm.shading((char*)data.getImagePtr() + data.getSync() * 2, data.getSync());
			ret = scanner.write(strm);
			if (ret) {
				//WriteLog(_T("%d %s"), __LINE__, __FILE__);
				return ret;
			}
		}
		//WriteLog(_T("send_shading_data()(2) end"));
		return 0;
	}
	long send_shading_data(CScsiCommand& scanner, Cei::LLiPm::CImg& white, Cei::LLiPm::CImg& black, long back)
	{
		//WriteLog(_T("send_shading_data()(1) start"));
		CStreamCmd strm(10244);
		strm.transfer_data_type(CStreamCmd::SHADING);
		long ret = 0;
		//side
		strm.side(back ? false/*back*/ : true/*front*/);
		ret = send_shading_data(scanner, strm, white, false);
		if (ret) {
			//WriteLog(_T("%d %s"), __LINE__, __FILE__);
			return ret;
		}
		ret = send_shading_data(scanner, strm, black, true);
		if (ret) {
			//WriteLog(_T("%d %s"), __LINE__, __FILE__);
			return ret;
		}
		//WriteLog(_T("send_shading_data()(1) end"));
		return 0;
	}
}
void send_shading_data_front(CScsiCommand& scanner)
{
	send_shading_data(scanner, Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::SCANNER_NAME_NAMESPACE::FRONT], Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::SCANNER_NAME_NAMESPACE::FRONT], Cei::LLiPm::SCANNER_NAME_NAMESPACE::FRONT);
}
void send_shading_data_back(CScsiCommand& scanner)
{
	send_shading_data(scanner, Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::SCANNER_NAME_NAMESPACE::BACK], Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::SCANNER_NAME_NAMESPACE::BACK], Cei::LLiPm::SCANNER_NAME_NAMESPACE::BACK);
}