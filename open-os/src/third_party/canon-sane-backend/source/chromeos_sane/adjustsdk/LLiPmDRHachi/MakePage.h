/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "IPBase.h"

namespace Cei
{
	namespace LLiPm
	{
		typedef struct tagMAKEPAGEINFO
        {
            int RGBOrder;
		} MAKEPAGEINFO, *LPMAKEPAGEINFO;
        
		class CMakePage : public CIPBase
		{
		public:
			CMakePage(void);
			~CMakePage(void);
			const char* const getName(void) const {return "MakePage";}
		protected:
			RTN IP(CImg& image) { return MakePage(image); }
			RTN IPFirst(CImg& image) { return RTN_NOSPT; }
			RTN IPMiddle(CImg& image) { return RTN_NOSPT; }
			RTN IPLast(CImg& image) { return RTN_NOSPT; }
			RTN setInfo(CImg& image, void* lpInfo);
			RTN setInfoFirst(CImg& image, void* lpInfo) { return setInfo(image, lpInfo); }
			RTN setInfoMiddle(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
			RTN setInfoLast(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
            MAKEPAGEINFO m_Info;
        private:
            RTN MakePage(CImg& image);
            RTN toPixelOrder(CImg& image);
            RTN toLineOrder(CImg& image);
            RTN toJpegOrder(CImg& image);
		};
	}
}
