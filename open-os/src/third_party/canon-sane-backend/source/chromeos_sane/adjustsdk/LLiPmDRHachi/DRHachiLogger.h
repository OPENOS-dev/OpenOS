/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#if !defined(AFX_DRHachiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_)
#define AFX_DRHachiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_

#pragma once

#include "CeiLogger.h"
#include "Dependencies.h"
#include DRFilterInfoHeader

namespace Cei
{
	namespace LLiPm
	{
		namespace DR_NAMESPACE
		{
			class DRHachiLogger
			{
			public:
				static void createLogger(const char* szFilename);
				static void writeADJUSTINFO(const ADJUSTINFO& info);
				static void writeFILTERSIMPLEXINFO(FILTERSIMPLEXINFO& info);
				static void writeFILTERDUPLEXINFO(FILTERDUPLEXINFO& info);
				static void writeSPECIALFILTERINFO(SPECIALFILTERINFO& info);
				static void writeNORMALFILTERINFO(NORMALFILTERINFO& info);
				static void writeIMAGEINFO(IMAGEINFO* info);
				static void writeCei(const CImg& img, const char* szFilename);
                static void dumpFirstLine(CeiLogger* logger, IMAGEINFO* info);
			private:
				DRHachiLogger();
				virtual ~DRHachiLogger();
			};
		}
	}
}

#endif // !defined(AFX_DRHachiLogger_H__128203F4_1814_40DC_A8E9_473268B7EAF0__INCLUDED_)
