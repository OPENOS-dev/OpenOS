/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#pragma once
#include "NormalFilterInfo.h"
#include "IPController.h"


#include <limits.h>

namespace Cei
{
	namespace LLiPm
	{
		class CNormalFilter : public CIPBase
		{
		protected:
			typedef enum tagIMAGESTATE {
				IMAGESTATE_COMPLETE,
				IMAGESTATE_FIRST,
				IMAGESTATE_MIDDLE,
				IMAGESTATE_LAST
			} IMAGESTATE;
		public:
			CNormalFilter(void);
			virtual ~CNormalFilter(void);
            virtual void clear();
		protected:
			virtual RTN setInfo(CImg& image, void* lpInfo);
			virtual RTN setInfoFirst(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
			virtual RTN setInfoMiddle(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
			virtual RTN setInfoLast(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
			NORMALFILTERDUPLEXINFO m_Info;
			virtual RTN cehckInfoSEQError(SIDE side);
			RTN execIP(CIPController<CIPBase>* pIP, CImg& image, void* lpInfo, IMAGESTATE imagestate);
        protected:
            int m_nSensorVer;
		};
	}
}
