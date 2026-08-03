/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "CollectArray.h"

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{
			class CCollectArrayForDuplex : public CCollectArray
			{
			public:
				CCollectArrayForDuplex(void);
				~CCollectArrayForDuplex(void);
				const char* const getName(void) const {return "CollectArrayForDuplex";}
			protected:
				RTN IP(CImg& image);
				RTN IPFirst(CImg& image);
				RTN IPMiddle(CImg& image);
				RTN IPLast(CImg& image);
				RTN setInfo(CImg& image, void* lpInfo);
				RTN setInfoFirst(CImg& image, void* lpInfo) {return setInfo(image, lpInfo);}
				RTN setInfoMiddle(CImg& image, void* lpInfo) {return RTN_OK;}
				RTN setInfoLast(CImg& image, void* lpInfo) {return RTN_OK;}
				COLLECTARRAYINFO m_Info;
				CImg m_imgBack;
			public:
				void getBackImage(CImg& image);
			public:
				static RTN CollectArray(CImg& image, CImg& imgBack, COLLECTARRAYINFO& info);
			};
		}
	}
}
