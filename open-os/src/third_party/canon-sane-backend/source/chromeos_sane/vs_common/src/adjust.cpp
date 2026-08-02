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
#include "adjust_config.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace SCANNER_NAME_NAMESPACE
		{
			Cei::LLiPm::CImg g_white[2];
			Cei::LLiPm::CImg g_black[2];
		}
	}
#ifdef _WIN32
	typedef unsigned char BYTE;
	typedef unsigned char * LPBYTE;
#endif
}
namespace {
	#if 0
	void mirror(Cei::LLiPm::CImg &img)
	{
		if (img.getBps()!=16) {
			printf("ERROR L:%d F:%s\r\n", __LINE__, __FILE__);
			return;
		}
		unsigned char *ptr = (unsigned char *)img.getImagePtr();
		unsigned char tmp=0;
		long mwidth = img.getWidth() * 2;//because of 16bit(2byte)
		for (long w=0; w<mwidth/2; w++) {
			tmp = p[w];
			p[w] = p[mwidth - 1 - w];
			p[mwidth - 1 -w] = tmp;
		}


	}
	#endif
	#if 0
void printf_CImg(Cei::LLiPm::CImg &img)
{
	printf("w:%ld\r\n", img.getWidth());
	printf("l:%ld\r\n", img.getHeight());
	printf("spp:%ld\r\n", img.getSpp());
	printf("bps:%ld\r\n", img.getBps());
	printf("sync:%ld\r\n", img.getSync());
}
#endif
Cei::LLiPm::ColorMode get_colormode(long spp, long bps)
{
	Cei::LLiPm::ColorMode out=Cei::LLiPm::GRAY;
	if (bps==1) {
		out = Cei::LLiPm::BINARY;
		//WriteLog(("Cei::LLiPm::BINARY"));
	} else {
		if (spp==3) {
			out = Cei::LLiPm::COLOR;
			//WriteLog(("Cei::LLiPm::COLOR"));
		} else {
			out = Cei::LLiPm::GRAY;
			//WriteLog(("Cei::LLiPm::GRAY"));
		}
	}
	return out;
}
void dropout_emphasis_front( Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO &adj, CScanMode::COLOR_TYPE dropout, CScanMode::COLOR_TYPE emphasis)
{
	//WriteLog(("dropout_emphasis_front(adj, %d, %d)"), dropout, emphasis);
	if (dropout!=CScanMode::NONE) {
		switch (dropout) {
		case CScanMode::NONE:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::LIGHT_NORMAL;break;
		case CScanMode::RED:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_RED;break;
		case CScanMode::GREEN:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_GREEN;break;
		case CScanMode::BLUE:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_BLUE;break;
		default:break;
		}
	} else if (emphasis!=CScanMode::NONE) {
		switch (emphasis) {
		case CScanMode::NONE:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::LIGHT_NORMAL;break;
		case CScanMode::RED:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_RED;break;
		case CScanMode::GREEN:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_GREEN;break;
		case CScanMode::BLUE:	adj.FrontLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_BLUE;break;
		default:break;
		}
	}
}
void dropout_emphasis_back( Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO &adj, CScanMode::COLOR_TYPE dropout, CScanMode::COLOR_TYPE emphasis)
{
	//WriteLog(("dropout_emphasis_back(adj, %d, %d)"), dropout, emphasis);
	if (dropout!=CScanMode::NONE) {
		switch (dropout) {
		case CScanMode::NONE:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::LIGHT_NORMAL;break;
		case CScanMode::RED:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_RED;break;
		case CScanMode::GREEN:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_GREEN;break;
		case CScanMode::BLUE:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::DROPOUT_BLUE;break;
		default:break;
		}
	} else if (emphasis!=CScanMode::NONE) {
		switch (emphasis) {
		case CScanMode::NONE:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::LIGHT_NORMAL;break;
		case CScanMode::RED:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_RED;break;
		case CScanMode::GREEN:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_GREEN;break;
		case CScanMode::BLUE:	adj.BackLightSorce= Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO::EMPHASIS_BLUE;break;
		default:break;
		}
	}
}
void set( Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO &adj, CAdjustCmd &adjcmd)
{
	adjcmd.gain1_f(adj.FrontAdjustInfo.Gain1);
	adjcmd.gain2_f(adj.FrontAdjustInfo.Gain2);
	adjcmd.gain3_f(adj.FrontAdjustInfo.Gain3);
	adjcmd.offset1_f(adj.FrontAdjustInfo.Offset1);
	adjcmd.offset2_f(adj.FrontAdjustInfo.Offset2);
	adjcmd.offset3_f(adj.FrontAdjustInfo.Offset3);
	adjcmd.red_led_f(adj.FrontAdjustInfo.RedLED);
	adjcmd.green_led_f(adj.FrontAdjustInfo.GreenLED);
	adjcmd.blue_led_f(adj.FrontAdjustInfo.BlueLED);

	adjcmd.gain1_b(adj.BackAdjustInfo.Gain1);
	adjcmd.gain2_b(adj.BackAdjustInfo.Gain2);
	adjcmd.gain3_b(adj.BackAdjustInfo.Gain3);
	adjcmd.offset1_b(adj.BackAdjustInfo.Offset1);
	adjcmd.offset2_b(adj.BackAdjustInfo.Offset2);
	adjcmd.offset3_b(adj.BackAdjustInfo.Offset3);
	adjcmd.red_led_b(adj.BackAdjustInfo.RedLED);
	adjcmd.green_led_b(adj.BackAdjustInfo.GreenLED);
	adjcmd.blue_led_b(adj.BackAdjustInfo.BlueLED);

#if 0
	WriteLog("////////////////////adjust////////////////");
	WriteLog(("FrontAdjustInfo.Gain1 %d"), adj.FrontAdjustInfo.Gain1);
	WriteLog(("FrontAdjustInfo.Gain2 %d"), adj.FrontAdjustInfo.Gain2);

	WriteLog(("FrontAdjustInfo.Gain3 %d"), adj.FrontAdjustInfo.Gain3);
	WriteLog(("FrontAdjustInfo.Offset1 %d"), adj.FrontAdjustInfo.Offset1);
	WriteLog(("FrontAdjustInfo.Offset2 %d"), adj.FrontAdjustInfo.Offset2);
	WriteLog(("FrontAdjustInfo.Offset3 %d"), adj.FrontAdjustInfo.Offset3);
	WriteLog(("FrontAdjustInfo.RedLED %d"), adj.FrontAdjustInfo.RedLED);

	WriteLog(("FrontAdjustInfo.GreenLED %d"), adj.FrontAdjustInfo.GreenLED);
	WriteLog(("FrontAdjustInfo.BlueLED %d"), adj.FrontAdjustInfo.BlueLED);
	
	WriteLog(("BackAdjustInfo.Gain1 %d"), adj.BackAdjustInfo.Gain1);
	WriteLog(("BackAdjustInfo.Gain2 %d"), adj.BackAdjustInfo.Gain2);

	WriteLog(("BackAdjustInfo.Gain3 %d"), adj.BackAdjustInfo.Gain3);
	WriteLog(("BackAdjustInfo.Offset1 %d"), adj.BackAdjustInfo.Offset1);
	WriteLog(("BackAdjustInfo.Offset2 %d"), adj.BackAdjustInfo.Offset2);
	WriteLog(("BackAdjustInfo.Offset3 %d"), adj.BackAdjustInfo.Offset3);
	WriteLog(("BackAdjustInfo.RedLED %d"), adj.BackAdjustInfo.RedLED);

	WriteLog(("BackAdjustInfo.GreenLED %d"), adj.BackAdjustInfo.GreenLED);
	WriteLog(("BackAdjustInfo.BlueLED %d"), adj.BackAdjustInfo.BlueLED);
#endif
}
long create_buffer(Cei::LLiPm::CImg &img, CWindow &window, bool duplex, long mud)
{
	Cei::IMAGEINFO imginfo={sizeof(imginfo)};
	imginfo.lWidth=(window.width() * window.xdpi() / mud) * (duplex?2:1);
	imginfo.lHeight=window.length() * window.ydpi() / mud;
	imginfo.lBps=12;
	imginfo.lSpp=window.spp();
	imginfo.ulRGBOrder=LINE_ORDER;
	imginfo.lXResolution=window.xdpi();
	imginfo.lYResolution=window.ydpi();
	imginfo.lSync=Cei::LLiPm::CImg::calcMinSync(imginfo.lWidth, imginfo.lBps, imginfo.lSpp, imginfo.ulRGBOrder);
	imginfo.tImageSize=Cei::LLiPm::CImg::calcSize(imginfo.lSync, imginfo.lHeight, imginfo.lSpp, imginfo.ulRGBOrder);
	const unsigned long MEGA4 = 1024 * 1024 * 4;
	while ((unsigned long)imginfo.tImageSize>MEGA4) {
		imginfo.lHeight--;
		imginfo.tImageSize=Cei::LLiPm::CImg::calcSize(imginfo.lSync, imginfo.lHeight, imginfo.lSpp, imginfo.ulRGBOrder);
	}	
	img.createImg(imginfo);
	if (img.isNull()) {
		WriteLog(("no memory %d %s"), __LINE__, __FILE__);
		return -1;
	}
	return 0;
}
void truncate_cimg(Cei::LLiPm::CImg &img, CSenseCmd &sense)
{
	//WriteLog(("truncate_cimg(%d)"), img.getHeight());
	Cei::IMAGEINFO*pinfo = img;
	long new_size = img.getImageSize()-sense.information_bytes();
	if (pinfo->lSpp==3) {
		//color
		if (pinfo->ulRGBOrder==LINE_ORDER) {
			pinfo->lHeight 	  = new_size/(pinfo->lSync*pinfo->lSpp);
			pinfo->tImageSize = pinfo->lSync*pinfo->lSpp*pinfo->lHeight;
		} else {
			pinfo->lHeight = new_size/pinfo->lSync;
			pinfo->tImageSize = pinfo->lSync*pinfo->lHeight;
		}
	} else {
		//gray or binary
		pinfo->lHeight = new_size/pinfo->lSync;
		pinfo->tImageSize = pinfo->lSync*pinfo->lHeight;
	}
	//WriteLog(("truncate_cimg(%d)"), img.getHeight());
}
long scan_adjust_data(CScsiCommand &scanner, Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO &adj, Cei::LLiPm::CImg &img, CWindow window, CScanMode mode_filter, bool duplex, long mud, long count, CSenseCmd &sense)
{
	long ret =0;
	if (count>1&&count<6) {
		mode_filter.drop_out(CScanMode::FRONT, CScanMode::NONE);
		mode_filter.drop_out(CScanMode::BACK,  CScanMode::NONE);
		mode_filter.emphasis(CScanMode::FRONT, CScanMode::NONE);
		mode_filter.emphasis(CScanMode::BACK,  CScanMode::NONE);
	}
	scanner.write(mode_filter);
	window.length(32 * mud / window.ydpi());
	window.side(false);
	if (count>1&&count<6) {
		window.spp(3);
		window.bps(8);
	} 
	window.bpp(12);
	scanner.write(window);//front
	if (duplex) {
		window.side(true);
		scanner.write(window);//back
	}
	ret = create_buffer(img, window, duplex, mud);
	if (ret) {
		WriteLog("ERROR L:%d F:%s", __LINE__, __FILE__);
		return sense.nomemory();
	}
	CScanCmd scan;
	scan.duplex(duplex);
	scan.main_window(adj.ScanInfo.MainWindowID);	
	scan.sub_window(adj.ScanInfo.SubWindowID);
	ret = scanner.write(scan);
	if (ret) {
		scanner.read(sense);
		return ret;
	}
	CStreamCmd stream((char*)img.getImagePtr(), (long)img.getImageSize());
	ret = scanner.read(stream);
	if (ret) {
		scanner.read(sense);
		if (!sense.ILI()) {
			WriteLog("ERROR L:%d F:%s", __LINE__, __FILE__);
			return ret;
		}
		truncate_cimg(img, sense);
	}		
	CAbortCmd abort;
	scanner.none(abort);
	return 0;
}
long scan_shading_data(CScsiCommand &scanner, Cei::LLiPm::CImg &img, CWindow window, bool duplex, long mud, bool black, CSenseCmd &sense)
{
	long ret = 0;
	window.length(100 * mud / window.ydpi());
	window.side(false);
	window.bpp(12);
	scanner.write(window);
	if (duplex) {
		window.side(true);//back
		ret = scanner.write(window);
	}
	ret = create_buffer(img, window, duplex, mud);
	if (ret) {
		WriteLog(("ERROR L:%d F:%s"), __LINE__, __FILE__);
		return sense.nomemory();
	}
	CScanCmd scan;
	scan.duplex(duplex);
	if (black) {
		scan.main_window((char)0xff);
		scan.sub_window((char)0xff);
	} else {
		scan.main_window((char)0xfe);
		scan.sub_window((char)0xfe);
	}	
	ret = scanner.write(scan);
	if (ret) {
		scanner.read(sense);
		return ret;
	}
	//read
	CStreamCmd stream((char*)img.getImagePtr(), (long)img.getImageSize());
	ret = scanner.read(stream);
	if (ret) {
		scanner.read(sense);
		if (!sense.ILI()) {
			return ret;
		}
		truncate_cimg(img, sense);
	}
	//abort		
	CAbortCmd abort;
	scanner.none(abort);
	return 0;
}
void printf_adj(const char *title, Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO &adj)
{
	#if 0
	printf("///////////adjust:%s////////////////\r\n", title);
	printf("bDuplex:%s\r\n", adj.bDuplex?"true":"false");
	printf("lXResolution:%ld\r\n", adj.lXResolution);
	printf("ScanMode:%d\r\n", adj.ScanMode);

	printf(("FrontAdjustInfo.Gain1 %d\r\n"), adj.FrontAdjustInfo.Gain1);
	printf(("FrontAdjustInfo.Gain2 %d\r\n"), adj.FrontAdjustInfo.Gain2);

	printf(("FrontAdjustInfo.Gain3 %d\r\n"), adj.FrontAdjustInfo.Gain3);
	printf(("FrontAdjustInfo.Offset1 %d\r\n"), adj.FrontAdjustInfo.Offset1);
	printf(("FrontAdjustInfo.Offset2 %d\r\n"), adj.FrontAdjustInfo.Offset2);
	printf(("FrontAdjustInfo.Offset3 %d\r\n"), adj.FrontAdjustInfo.Offset3);
	printf(("FrontAdjustInfo.RedLED %d\r\n"), adj.FrontAdjustInfo.RedLED);

	printf(("FrontAdjustInfo.GreenLED %d\r\n"), adj.FrontAdjustInfo.GreenLED);
	printf(("FrontAdjustInfo.BlueLED %d\r\n"), adj.FrontAdjustInfo.BlueLED);
	
	printf(("BackAdjustInfo.Gain1 %d\r\n"), adj.BackAdjustInfo.Gain1);
	printf(("BackAdjustInfo.Gain2 %d\r\n"), adj.BackAdjustInfo.Gain2);

	printf(("BackAdjustInfo.Gain3 %d\r\n"), adj.BackAdjustInfo.Gain3);
	printf(("BackAdjustInfo.Offset1 %d\r\n"), adj.BackAdjustInfo.Offset1);
	printf(("BackAdjustInfo.Offset2 %d\r\n"), adj.BackAdjustInfo.Offset2);
	printf(("BackAdjustInfo.Offset3 %d\r\n"), adj.BackAdjustInfo.Offset3);
	printf(("BackAdjustInfo.RedLED %d\r\n"), adj.BackAdjustInfo.RedLED);

	printf(("BackAdjustInfo.GreenLED %d\r\n"), adj.BackAdjustInfo.GreenLED);
	printf(("BackAdjustInfo.BlueLED %d\r\n"), adj.BackAdjustInfo.BlueLED);
	#endif
}
long adjust_scanner_main(CScsiCommand &scanner, CMode &mode, CWindow window, CScanMode mode_scan, CSenseCmd &sense, long sensor_version)
{
	long ret=0;
	bool duplex = mode_scan.duplex();
	CScanMode mode_filter;
	mode_filter.page_code(CScanMode::PAGE_CODE_FILTER);
	scanner.read(mode_filter);
	 Cei::LLiPm::SCANNER_NAME_NAMESPACE::ADJUSTINFO adj={sizeof(adj)};
	adj.bDuplex=duplex;
	adj.lXResolution=window.xdpi();
	adj.ScanMode=get_colormode(window.spp(), window.bps());
	dropout_emphasis_front(adj, mode_filter.drop_out(CScanMode::FRONT), mode_filter.emphasis(CScanMode::FRONT));
	dropout_emphasis_back(adj, mode_filter.drop_out(CScanMode::BACK), mode_filter.emphasis(CScanMode::BACK));
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::AdjustLightFirst(&adj, sensor_version);
	CAdjustCmd adjcmd;
	long count = 1;
	while (!adj.bUse) {
		set(adj, adjcmd);
		scanner.write(adjcmd);
		Cei::LLiPm::CImg img;
		ret = scan_adjust_data(scanner, adj, img, window, mode_filter, duplex, mode.mud(), count, sense);
		if (ret) {
			WriteLog("ERROR L:%d F:%s", __LINE__, __FILE__);
			return ret;
		}
		Cei::LLiPm::SCANNER_NAME_NAMESPACE::AdjustLightNext(img, &adj);
		count++;
	}
	printf_adj("end", adj);
	set(adj, adjcmd);
	scanner.write(adjcmd);
	Cei::LLiPm::CImg srcWhite, srcBlack;
	ret = scan_shading_data(scanner, srcWhite, window, duplex, mode.mud(), false, sense);
	if (ret) {
		WriteLog("ERROR L:%d F:%s", __LINE__, __FILE__);
		return ret;
	}
	ret = scan_shading_data(scanner, srcBlack, window, duplex, mode.mud(), true, sense);
	if (ret) {
		WriteLog("ERROR L:%d F:%s", __LINE__, __FILE__);
		return ret;
	}
	Cei::LLiPm::CImg white[2];
	Cei::LLiPm::CImg black[2];
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::AdjustLightLast(white[Cei::LLiPm::FRONT], white[Cei::LLiPm::BACK], srcWhite, &adj);
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::AdjustLightLast(black[Cei::LLiPm::FRONT], black[Cei::LLiPm::BACK], srcBlack, &adj);
	long buffer_size = 0x80000;
	Cei::LPBYTE pbuffer = new Cei::BYTE[buffer_size+256];
	if (pbuffer==NULL) {
		return sense.nomemory();
	}
	CBufferCmd2 bufcmd((char*)pbuffer, (long)buffer_size);
	while (bufcmd.end()) {
		scanner.read(bufcmd);
		bufcmd.next();
	}
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::AdjustLightFix(white[Cei::LLiPm::FRONT], black[Cei::LLiPm::FRONT], &adj, Cei::LLiPm::FRONT, pbuffer, buffer_size);
	if (duplex) {
		AdjustLightFix(white[Cei::LLiPm::BACK], black[Cei::LLiPm::BACK], &adj, Cei::LLiPm::BACK, pbuffer, buffer_size);
	}
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::FRONT] = white[Cei::LLiPm::FRONT];
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_white[Cei::LLiPm::BACK] = white[Cei::LLiPm::BACK];
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::FRONT] = black[Cei::LLiPm::FRONT];
	Cei::LLiPm::SCANNER_NAME_NAMESPACE::g_black[Cei::LLiPm::BACK] = black[Cei::LLiPm::BACK];
	return 0;
}
}
long adjust_scanner(CScsiCommand &scanner, CSenseCmd &sense, long sensor_version)
{
	WriteLog("adjust_scanner() start");
	long ret = 0;
	CMode mode;
	scanner.read(mode);
	CWindow window;
	scanner.read(window);
	CScanMode mode_scan;
	mode_scan.page_code(CScanMode::PAGE_CODE_SCAN);
	scanner.read(mode_scan);
	mode_scan.autosize(false);
	scanner.write(mode_scan);
	ret = adjust_scanner_main(scanner, mode, window, mode_scan, sense, sensor_version);
	mode_scan.autosize(true);
	scanner.write(mode_scan);
	scanner.write(window);
	window.side(true);
	scanner.write(window);
	WriteLog("adjust_scanner() end %ld", ret);
	return ret;
}