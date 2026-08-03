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
			class CCollectArrayForSimplex : public CCollectArray
			{
			public:
				CCollectArrayForSimplex(void);
				~CCollectArrayForSimplex(void);
				const char* const getName(void) const {return "CollectArrayForSimplex";}
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
			public:
				static RTN CollectArray(CImg& image, COLLECTARRAYINFO& info);
			};
		}
	}
}
