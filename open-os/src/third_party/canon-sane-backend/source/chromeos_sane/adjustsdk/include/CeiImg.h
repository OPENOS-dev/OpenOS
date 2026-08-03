/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "ceiinf.h"

namespace Cei
{
	namespace LLiPm
	{
		class CImg
		{
		public:
			CImg(void);
			virtual ~CImg(void);
			CImg(const CImg& rhs);
			CImg& operator =(const CImg& rhs);
			operator IMAGEINFO*(void);
		public:
           
			bool createImg(long lWidth, long lHeight, long lBps, long lSpp, unsigned long ulRGBOrder);
			bool createImg(long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder);
			bool createImg(long lWidth, long lHeight, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution);
			bool createImg(long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution);
			bool createImg(long lXPos, long lYPos, long lWidth, long lHeight, long lSync, long lBps, long lSpp, unsigned long ulRGBOrder, long lXResolution, long lYResolution);
			bool createImg(IMAGEINFO& Info);
			bool createImg(CImg& img);
            
            
            bool createJpg(long lWidth, long lBps, long lSpp, long lXResolution, long lYResolution , unsigned long tImageSize);
            
			void deleteImg(void);
			bool isNull(void) const;
		public:
			void attachImg(CImg& rhs);		
			bool appendImg(CImg& rhs);		
		public:
			long getXPos(void) const {return m_Img.lXpos;}
			long getYPos(void) const {return m_Img.lYpos;}
			long getWidth(void) const {return m_Img.lWidth;}
			long getHeight(void) const {return m_Img.lHeight;}
			long getSync(void) const {return m_Img.lSync;}
			unsigned long getImageSize(void) const {return m_Img.tImageSize;}
			long getBps(void) const {return m_Img.lBps;}
			long getSpp(void) const {return m_Img.lSpp;}
			unsigned long getRGBOrder(void) const {return m_Img.ulRGBOrder;}
			long getXResolution(void) const {return m_Img.lXResolution;}
			long getYResolution(void) const {return m_Img.lYResolution;}
			unsigned char* getImagePtr(void) const {return m_Img.lpImage;}
			unsigned long getBpp(void) const {return m_Img.lBps * m_Img.lSpp;}
			unsigned long getRGBSync(void) const {return (m_Img.lSpp == 3 && m_Img.ulRGBOrder == LINE_ORDER) ? m_Img.lSync * m_Img.lSpp : m_Img.lSync;}
            
        public:
            bool convertToJpg(int quality);
            
		protected:
			bool checkInfo(const IMAGEINFO& Info);
			bool allocImgData(void);
		public:
			static long calcMinSync(long lWidth, long lBps, long lSpp, unsigned long ulRGBOrder); // return < 0 is error
			static unsigned long calcSize(long lSync, long lHeight, long lSpp, unsigned long ulRGBOrder); // return < 0 is error
		protected:
			IMAGEINFO m_Img;
		};
	}
}
