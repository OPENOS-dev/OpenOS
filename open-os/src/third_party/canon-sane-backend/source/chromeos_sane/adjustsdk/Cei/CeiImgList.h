/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/

#pragma once
#include "CeiImg.h"
#include <vector>

namespace Cei
{
	namespace LLiPm
	{
		class CImgList
		{
		public:
			CImgList();
			~CImgList();
		public:
		public:
			bool PushBack(CImg& img);
			void PopAll(void);
			void SpliceAndPopAll(CImg& imgReceiver);

		private:
			std::vector<CImg*> m_listImg;
			IMAGEINFO m_imgSum;
		};
	}
}


