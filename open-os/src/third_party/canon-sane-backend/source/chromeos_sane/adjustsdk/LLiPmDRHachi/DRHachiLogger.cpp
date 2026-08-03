/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "DRHachiLogger.h"
#include <fstream>
using namespace Cei;
using namespace LLiPm;
using namespace DR_NAMESPACE;
void DRHachiLogger::createLogger(const char* szFilename)
{
}
void DRHachiLogger::writeADJUSTINFO(const ADJUSTINFO& info)
{
}
void DRHachiLogger::writeFILTERSIMPLEXINFO(FILTERSIMPLEXINFO& info)
{
}
void DRHachiLogger::writeFILTERDUPLEXINFO(FILTERDUPLEXINFO& info)
{
}
void DRHachiLogger::writeSPECIALFILTERINFO(SPECIALFILTERINFO& info)
{
}
void DRHachiLogger::writeNORMALFILTERINFO(NORMALFILTERINFO& info)
{
}
void DRHachiLogger::writeIMAGEINFO(IMAGEINFO* info)
{
}
void DRHachiLogger::writeCei(const CImg& img, const char* szFilename)
{
}
static void dumpBytePixel(CeiLogger* logger, void* data, int offset)
{
}
static void dumpShortPixel(CeiLogger* logger, void* data, int offset)
{
}
void DRHachiLogger::dumpFirstLine(CeiLogger* logger, IMAGEINFO* info)
{
}
DRHachiLogger::DRHachiLogger()
{
}
DRHachiLogger::~DRHachiLogger()
{
}
