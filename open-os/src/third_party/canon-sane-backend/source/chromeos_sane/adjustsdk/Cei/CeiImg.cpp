/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CeiImg.h"
#include "ceilib.h"
#include <memory.h>
#include <algorithm>
using namespace Cei;
using namespace LLiPm;

//
// Constructor & Destructor & Operator
//

CImg::CImg(void)
{
	memset(&m_Img, 0, sizeof(m_Img));
}

CImg::~CImg(void)
{
	deleteImg();
}

CImg::CImg(const CImg& rhs)
{
	memcpy((void*)&m_Img, (void*)&rhs.m_Img, sizeof(IMAGEINFO));
	m_Img.lpImage = 0;
	if (allocImgData()) {
		memcpy(m_Img.lpImage, rhs.m_Img.lpImage, m_Img.tImageSize);
	}
	else {
		deleteImg();
	}
}

CImg& CImg::operator =(const CImg& rhs)
{
	if (this != &rhs) {
        deleteImg();
		m_Img = rhs.m_Img;
		m_Img.lpImage = 0;
		if (allocImgData()) {
			memcpy(m_Img.lpImage, rhs.m_Img.lpImage, m_Img.tImageSize);
		}
		else {
			deleteImg();
		}
	}
	return *this;
}

CImg::operator IMAGEINFO*(void)
{
	return &m_Img;
}

//
// Create & Delete
//

bool CImg::createImg(long lWidth, long lHeight, long lBps, long lSpp, unsigned long ulRGBOrder)
{
	long lSync = calcMinSync(lWidth, lBps, lSpp, ulRGBOrder);
	return createImg(lWidth, lHeight, lSync, lBps, lSpp, ulRGBOrder);
}

bool CImg::createImg(long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder)
{
	long lXResolution = 0;
	long lYResolution = 0;
	return createImg(lWidth, lHeight, lSync, lBps, lSpp, ulRGBOrder, lXResolution, lYResolution);
}

bool CImg::createImg(long lWidth, long lHeight, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution)
{
	long lSync = calcMinSync(lWidth, lBps, lSpp, ulRGBOrder);
	return createImg(lWidth, lHeight, lSync, lBps, lSpp, ulRGBOrder, lXResolution, lYResolution);
}

bool CImg::createImg(long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution)
{
	long lXPos = 0;
	long lYPos = 0;
	return createImg(lXPos, lYPos, lWidth, lHeight, lSync, lBps, lSpp, ulRGBOrder, lXResolution, lYResolution);
}

bool CImg::createImg(long lXPos, long lYPos, long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution)
{
	IMAGEINFO Info;
	Info.ulSize = sizeof(Info);
	Info.lpImage = 0;
	Info.lXpos = lXPos;
	Info.lYpos = lYPos;
	Info.lWidth = lWidth;
	Info.lHeight = lHeight;
	Info.lSync = lSync;
	Info.lBps = lBps;
	Info.lSpp = lSpp;
	Info.ulRGBOrder = ulRGBOrder;
	Info.lXResolution = lXResolution;
	Info.lYResolution = lYResolution;
	if (Info.ulRGBOrder == PIXEL_ORDER) {
		Info.tImageSize = Info.lSync * Info.lHeight;
	}
	else if (Info.ulRGBOrder == LINE_ORDER) {
		Info.tImageSize = Info.lSync * Info.lHeight * Info.lSpp;
	}
	else {
		Info.tImageSize = 0;
	}
	return createImg(Info);
}

bool CImg::createImg(IMAGEINFO& Info)
{
	if (!checkInfo(Info)) {
		return false;
	}
	
	deleteImg();
	m_Img = Info;

	if (!allocImgData()) {
		deleteImg();
	}

	return true;
}

bool CImg::createImg(CImg& rhs)
{
	IMAGEINFO Info = rhs.m_Img;
	Info.lpImage = 0;
	return createImg(Info);
}

bool CImg::createJpg(long lWidth, long lBps, long lSpp, long lXResolution, long lYResolution , unsigned long tImageSize)
{
    deleteImg();
    
	m_Img.ulSize = sizeof(m_Img);
	m_Img.lpImage = 0;
	m_Img.lXpos = 0;
	m_Img.lYpos = 0;
	m_Img.lWidth = lWidth;
	m_Img.lHeight = -1;
	m_Img.lSync = -1;
	m_Img.lBps = lBps;
	m_Img.lSpp = lSpp;
	m_Img.ulRGBOrder = JPEG_ORDER;
	m_Img.lXResolution = lXResolution;
	m_Img.lYResolution = lYResolution;
    m_Img.tImageSize = tImageSize;
    
	if (!allocImgData()) {
		deleteImg();
        return false;
	}
    
	return true;
}

void CImg::deleteImg(void)
{
	if (m_Img.lpImage) {
		delete[] m_Img.lpImage;
	}
	memset(&m_Img, 0, sizeof(m_Img));
}

bool CImg::isNull(void) const
{
	return (m_Img.lpImage == 0);
}

//
// Attach & Dettach
//

void CImg::attachImg(CImg& rhs)
{
	deleteImg();
	m_Img = rhs.m_Img;
	memset(&rhs.m_Img, 0, sizeof(rhs.m_Img));
}

bool CImg::appendImg(CImg& rhs)
{
	if (rhs.isNull()) {
		return true;
	}
	if (isNull()) {
		*this = rhs;
		return true;
	}

	if (m_Img.lBps != rhs.m_Img.lBps ||
		m_Img.lSpp != rhs.m_Img.lSpp ||
		m_Img.ulRGBOrder != rhs.m_Img.ulRGBOrder ||
		m_Img.lXResolution != rhs.m_Img.lXResolution ||
		m_Img.lYResolution != rhs.m_Img.lYResolution ||
		m_Img.ulRGBOrder != rhs.m_Img.ulRGBOrder
		) {
			return false;
	}
	if (rhs.m_Img.lXpos != 0 || rhs.m_Img.lYpos != 0) {
		//not supported
		return false;
	}

	CImg imgNew;
	if (!imgNew.createImg(
					std::max(getWidth(), rhs.getWidth()), 
					getHeight() + rhs.getHeight(), 
					std::max(getSync(), rhs.getSync()), 
					getBps(),
					getSpp(),
					getRGBOrder(),
					getXResolution(),
					getYResolution()))
	{
		return false;
	}
	if (imgNew.isNull()) {
		deleteImg();
		return false;
	}

	unsigned char* pDst = imgNew.getImagePtr();
	unsigned char* pSrc = getImagePtr();
	long lDstSync = imgNew.getSync();
	long lSrcSync = getSync();
	long lLine = getHeight();
	if (getSpp() == 3 && getRGBOrder() == LINE_ORDER) {
		lLine *= getSpp();
	}
	while (lLine--) {
		memcpy(pDst, pSrc, lSrcSync);
		pDst += lDstSync;
		pSrc += lSrcSync;
	}
	pSrc = rhs.getImagePtr();
	lSrcSync = rhs.getSync();
	lLine = rhs.getHeight();
	if (rhs.getSpp() == 3 && rhs.getRGBOrder() == LINE_ORDER) {
		lLine *= rhs.getSpp();
	}
	while (lLine--) {
		memcpy(pDst, pSrc, lSrcSync);
		pDst += lDstSync;
		pSrc += lSrcSync;
	}
	attachImg(imgNew);
	return true;
}

#if defined(JPEG_EXPORT)
#include "CeiImgJpg.h"
bool CImg::convertToJpg(int quality)
{
    return CImgJpg::Compress(*this, quality);
}
#else
bool CImg::convertToJpg(int quality)
{
    return false;
}
#endif




//
// Helper
//

bool CImg::checkInfo(const IMAGEINFO& Info)
{
	if (Info.lWidth <= 0 || Info.lHeight <= 0) {
		return false;
	}
	if (Info.lSpp != 1 &&
		Info.lSpp != 3 &&
		Info.lBps != 1 &&
		Info.lBps != 4 &&
		Info.lBps != 8 &&
		Info.lBps != 16) {
		return false;
	}

	long lMinSync = calcMinSync(Info.lWidth, Info.lBps, Info.lSpp, Info.ulRGBOrder);
	if (Info.lSync < 0 || Info.lSync < lMinSync) {
		return false;
	}
	if (Info.tImageSize != calcSize(Info.lSync, Info.lHeight, Info.lSpp, Info.ulRGBOrder)) {
		return false;
	}

	return true;
}

bool CImg::allocImgData(void)
{
	if (m_Img.lpImage) {
		delete[] m_Img.lpImage;
		m_Img.lpImage = 0;
	}
	if (m_Img.tImageSize == 0) {
		return false;
	}

    try {
        m_Img.lpImage = new unsigned char[m_Img.tImageSize];
    } catch (std::exception& ) {
        m_Img.lpImage = 0;
        return false;
    }

	memset(m_Img.lpImage, 0, m_Img.tImageSize);
	return true;
}

long CImg::calcMinSync(long lWidth, long lBps, long lSpp, unsigned long ulRGBOrder)
{
	long lMinSync = -1;
	if (ulRGBOrder == PIXEL_ORDER) {
		lMinSync = lSpp * lBps * lWidth;
		lMinSync = PACKING8(lMinSync) / 8;
	}
	else if (ulRGBOrder == LINE_ORDER) {
		lMinSync = lBps * lWidth;
		lMinSync = PACKING8(lMinSync) / 8;
	}
	return lMinSync;
}

unsigned long CImg::calcSize(long lSync, long lHeight, long lSpp, unsigned long ulRGBOrder)
{
	unsigned long ulSize = 0;
	if (ulRGBOrder == PIXEL_ORDER) {
		ulSize = lSync * lHeight;
	}
	else if (ulRGBOrder == LINE_ORDER) {
		ulSize = lSync * lHeight * lSpp;
	}
	return ulSize;
}

