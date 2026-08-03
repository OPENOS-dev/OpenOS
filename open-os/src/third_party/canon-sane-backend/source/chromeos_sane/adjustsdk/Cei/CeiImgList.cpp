/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "CeiImgList.h"
#include "CeiLogger.h"
#include <algorithm>
#include <string.h>

using namespace Cei;
using namespace LLiPm;

CImgList::CImgList()
{
	memset(&m_imgSum, 0, sizeof(m_imgSum));
	m_imgSum.ulSize = sizeof(m_imgSum);
}

CImgList::~CImgList()
{
	PopAll();
}

bool CImgList::PushBack(CImg& img)
{
	if (img.isNull()) {
		return true;
	}

	if (m_listImg.empty()) {
		m_imgSum.lXpos = img.getXPos();
		m_imgSum.lYpos = img.getYPos();
		m_imgSum.lWidth = img.getWidth();
		m_imgSum.lSync = img.getSync();
		m_imgSum.lBps = img.getBps();
		m_imgSum.lSpp = img.getSpp();
		m_imgSum.lXResolution = img.getXResolution();
		m_imgSum.lYResolution = img.getYResolution();
		m_imgSum.ulRGBOrder = img.getRGBOrder();
	}
	else {
		if (m_imgSum.lBps != img.getBps() || 
			m_imgSum.lSpp != img.getSpp() || 
			m_imgSum.lWidth != img.getWidth() || 
			m_imgSum.lSync != img.getSync() || 
			m_imgSum.lXResolution != img.getXResolution() || 
			m_imgSum.lYResolution != img.getYResolution() || 
			((m_imgSum.lSpp == 3) && (m_imgSum.ulRGBOrder != img.getRGBOrder())))
		{
			return false;
		}
	}
	m_imgSum.lHeight += img.getHeight();
    m_imgSum.tImageSize += img.getImageSize();

	CImg *pImg = new(std::nothrow) CImg;
	if (!pImg) {
		return false;
	}
	pImg->attachImg(img);
	m_listImg.push_back(pImg);
	return true;
}

void CImgList::PopAll(void)
{
	if (!m_listImg.empty()) {
        while (!m_listImg.empty()) {
            delete m_listImg.back();
            m_listImg.pop_back();
        }
		m_listImg.clear();
	}
	memset(&m_imgSum, 0, sizeof(m_imgSum));
	m_imgSum.ulSize = sizeof(m_imgSum);
}

void CImgList::SpliceAndPopAll(CImg& imgReceiver)
{
	if (m_listImg.empty()) {
		CeiLogger::writeLog("CImgList::SpliceAndPopAll unexpected case. m_listImg.empty()");
		return;
	}
	if (m_listImg.size() == 1) {
		imgReceiver.attachImg(*(m_listImg.front()));
        delete m_listImg.front();
		m_listImg.pop_back();   // back is front
		return;
	}

    if (IS_JPEG_ORDER(m_imgSum.ulRGBOrder)) {
        imgReceiver.createJpg(m_imgSum.lWidth, m_imgSum.lBps, m_imgSum.lSpp, m_imgSum.lXResolution, m_imgSum.lYResolution, m_imgSum.tImageSize);
        if (imgReceiver.isNull()) {
            throw std::bad_alloc();
        }
    } else {
        m_imgSum.tImageSize = CImg::calcSize(m_imgSum.lSync, m_imgSum.lHeight, m_imgSum.lSpp, m_imgSum.ulRGBOrder);
        
        imgReceiver.createImg(m_imgSum);
        if (imgReceiver.isNull()) {
            throw std::bad_alloc();
        }
        
    }

	unsigned long pos = 0;
    for (size_t i = 0; i < m_listImg.size(); i++) {
		memcpy(&imgReceiver.getImagePtr()[pos], m_listImg.at(i)->getImagePtr(), m_listImg.at(i)->getImageSize());
		pos += m_listImg.at(i)->getImageSize();
    }
	while (!m_listImg.empty()) {
		delete m_listImg.back();
		m_listImg.pop_back();
	}
}

