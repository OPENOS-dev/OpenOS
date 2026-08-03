/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once

#include "CeiImg.h"


namespace Cei
{
    namespace LLiPm
    {
        class CImgEdit
        {
        private:
            static const char BIT_TABLE[];
            
        public:
            static bool FillColor(CImg& img, RGBQUAD fill, RECT& ignore);
            
            static bool ToColor(CImg& img);
            static bool ToGray(CImg& img);
            static bool ToBinary(CImg& img);
            
            static void MemOr(unsigned char* lpDst, unsigned char* lpOr, long lWidth);
            static void MemAnd(unsigned char* lpDst, unsigned char* lpAnd, long lWidth);

        private:
            static bool ColorToGray(CImg& img);
            static bool GrayToColor(CImg& img);
            static bool GrayToBinary(CImg& img);
            static bool BinaryToGray(CImg& img);

        private:
            CImgEdit();
            ~CImgEdit();
        };
    }
}