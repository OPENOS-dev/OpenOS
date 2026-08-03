/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_TAG_SDK_OPTIONS_TAG_HEADER_DEFINED__
#define __CSD_TAG_SDK_OPTIONS_TAG_HEADER_DEFINED__

#include <vector>
#include "sdk_tag.h"
#include "virtual_scanner_interface.h"

class CSkipBlankPage : public CCsdTagLong
{
public:
	CSkipBlankPage(ICsdTags2 *parent);
	virtual ~CSkipBlankPage();
	char *id_name();
private:
	long m_choice[2];
};
class CDetectBlankPage : public CCsdTagLong
{
public:
	CDetectBlankPage(ICsdTags2 *parent);
	virtual ~CDetectBlankPage();
	char *id_name();
private:
	long m_choice[2];
};
class CSkipBlankPageParam : public CCsdTagLong
{
public:
	CSkipBlankPageParam(ICsdTags2 *parent);
	virtual ~CSkipBlankPageParam();
	char *id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
private:
	long m_choice[3];
};
class CScanAhead : public CCsdTagLong
{
public:
	CScanAhead(ICsdTags2 *parent);
	virtual ~CScanAhead();
	//char *id_name();
private:
	long m_choice[2];
};
class CMaxScanAheadPages : public CCsdTagLong
{
public:
	CMaxScanAheadPages(ICsdTags2 *parent);
	virtual ~CMaxScanAheadPages();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
};
class CEdgeEmphasis : public CCsdTagLong
{
public:
	CEdgeEmphasis(ICsdTags2 *parent);
	virtual ~CEdgeEmphasis();
	char *id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
private:
	long m_choice[5];
};
class CMoireRemoval : public CCsdTagLong
{
public:
	CMoireRemoval(ICsdTags2 *parent);
	virtual ~CMoireRemoval();
	char *id_name();
private:
	long m_choice[3];
};
class CPunchHoleRemoval : public CCsdTagLong
{
public:
	CPunchHoleRemoval(ICsdTags2 *parent);
	virtual ~CPunchHoleRemoval();
	char *id_name();
private:
	long m_choice[2];
};
class CAutoResolution : public CCsdTagLong
{
public:
	CAutoResolution(ICsdTags2 *parent);
	virtual ~CAutoResolution();
	char *id_name();
private:
	long m_choice[2];
};
class CAutoSize : public CCsdTagLong
{
public:
	CAutoSize(ICsdTags2 *parent);
	virtual ~CAutoSize();
	char *id_name();
private:
	long m_choice[2];
};
class CDeskew : public CCsdTagLong
{
public:
	CDeskew(ICsdTags2 *parent);
	virtual ~CDeskew();
	char *id_name();
private:
	long m_choice[2];
};
class CDeskewMethod : public CCsdTagLong
{
public:
	CDeskewMethod(ICsdTags2 *parent);
	virtual ~CDeskewMethod();
	char *id_name();
private:
	long m_choice[2];
};
class CRotation : public CCsdTagLongMulti
{
public:
	CRotation(ICsdTags2 *parent);
	virtual ~CRotation();
	char *id_name();
private:
	long m_choice[4];
};
class CAutoRotation : public CCsdTagLongMulti
{
public:
	CAutoRotation(ICsdTags2 *parent);
	virtual ~CAutoRotation();
	char *id_name();
private:
	long m_choice[2];
};
class CTextMethod : public CCsdTagLongMulti
{
public:
	CTextMethod(ICsdTags2 *parent);
	virtual ~CTextMethod();
	char *id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
private:
	long m_choice[3];
};
class CDither : public CCsdTagLongMulti
{
public:
	CDither(ICsdTags2 *parent);
	virtual ~CDither();
	char *id_name();
private:
	long m_choice[2];
};
class CDoubleFeedDetectionUltrasonic : public CCsdTagLong
{
public:
	CDoubleFeedDetectionUltrasonic(ICsdTags2 *parent);
	virtual ~CDoubleFeedDetectionUltrasonic();
	char *id_name();
private:
	long m_choice[2];
};
class CDoubleFeedDetectionUltrasonicDisablePositionStart : public CCsdTagLong
{
public:
	CDoubleFeedDetectionUltrasonicDisablePositionStart(ICsdTags2* parent);
	virtual ~CDoubleFeedDetectionUltrasonicDisablePositionStart();
	char* id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
};
class CDoubleFeedDetectionUltrasonicDisablePositionEnd : public CCsdTagLong
{
public:
	CDoubleFeedDetectionUltrasonicDisablePositionEnd(ICsdTags2* parent);
	virtual ~CDoubleFeedDetectionUltrasonicDisablePositionEnd();
	char* id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
}; 
class CDoubleFeedDetectionLength : public CCsdTagLong
{
public:
	CDoubleFeedDetectionLength(ICsdTags2 *parent);
	virtual ~CDoubleFeedDetectionLength();
	char *id_name();
private:
	long m_choice[2];
};
class CFolio : public CCsdTagLong
{
public:
	CFolio(ICsdTags2 *parent);
	virtual ~CFolio();
	char *id_name();
private:
	long m_choice[2];
};
class CFolioOption : public CCsdTagLong
{
public:
	CFolioOption(ICsdTags2 *parent);
	virtual ~CFolioOption();
	char *id_name();
private:
	long m_choice[6];
};
class CSplitImage : public CCsdTagLong
{
public:
	CSplitImage(ICsdTags2 *parent);
	virtual ~CSplitImage();
	char *id_name();
private:
	long m_choice[2];
};
class CSplitImageOption : public CCsdTagLong
{
public:
	CSplitImageOption(ICsdTags2 *parent);
	virtual ~CSplitImageOption();
	char *id_name();
private:
	long m_choice[2];
};
class CColorEmphasis : public CCsdTagLongMulti
{
public:
	CColorEmphasis(ICsdTags2* parent);
	virtual ~CColorEmphasis();
	char* id_name();
private:
	long m_choice[6];
};
class CDropout : public CCsdTagLongMulti
{
public:
	CDropout(ICsdTags2 *parent);
	virtual ~CDropout();
	char *id_name();
private:
	long m_choice[6];
};
class CBlackAndWhiteReversal : public CCsdTagLong
{
public:
	CBlackAndWhiteReversal(ICsdTags2 *parent);
	virtual ~CBlackAndWhiteReversal();
	char *id_name();
private:
	long m_choice[2];
};
class CEraseDot : public CCsdTagLong
{
public:
	CEraseDot(ICsdTags2 *parent);
	virtual ~CEraseDot();
	char *id_name();
private:
	long m_choice[2];
};
class CEraseNotch : public CCsdTagLong
{
public:
	CEraseNotch(ICsdTags2 *parent);
	virtual ~CEraseNotch();
	char *id_name();
private:
	long m_choice[2];
};
class CPrescan : public CCsdTagLong
{
public:
	CPrescan(ICsdTags2 *parent);
	virtual ~CPrescan();
	char *id_name();
private:
	long m_choice[2];
}; 
class CPrescanOption : public CCsdTagLong
{
public:
	CPrescanOption(ICsdTags2* parent);
	virtual ~CPrescanOption();
	char* id_name();
private:
	long m_choice[2];
}; 
class CFeedingOption : public CCsdTagLong
{
public:
	CFeedingOption(ICsdTags2 *parent);
	virtual ~CFeedingOption();
	char *id_name();
private:
	long m_choice[4];
};
class CAutoStartWaitTime : public CCsdTagLong
{
public:
	CAutoStartWaitTime(ICsdTags2* parent);
	virtual ~CAutoStartWaitTime();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	char* id_name();
private:
	long m_choice[3];
}; 
class CBleedthrough : public CCsdTagLong
{
public:
	CBleedthrough(ICsdTags2* parent);
	virtual ~CBleedthrough();
	char* id_name();
private:
	long m_choice[2];
};
class CBleedthroughLevel : public CCsdTagLong
{
public:
	CBleedthroughLevel(ICsdTags2* parent);
	virtual ~CBleedthroughLevel();
	char* id_name();
private:
	long m_choice[7];
};
class CCharacterEmphasis : public CCsdTagLong
{
public:
    CCharacterEmphasis(ICsdTags2* parent);
    virtual ~CCharacterEmphasis();
    char* id_name();
private:
    long m_choice[7];
};
class CBackgroundSmoothing : public CCsdTagLong
{
public:
	CBackgroundSmoothing(ICsdTags2* parent);
	virtual ~CBackgroundSmoothing();
	char* id_name();
private:
	long m_choice[2];
};
class CRemoveShadow : public CCsdTagLong
{
public:
    CRemoveShadow(ICsdTags2* parent);
    virtual ~CRemoveShadow();
    char* id_name();
private:
    long m_choice[2];
};
class CNoiseReduction : public CCsdTagLong
{
public:
    CNoiseReduction(ICsdTags2* parent);
    virtual ~CNoiseReduction();
    char* id_name();
private:
    long m_choice[2];
};
class CNoiseReductionLevel : public CCsdTagLong
{
public:
    CNoiseReductionLevel(ICsdTags2* parent);
    virtual ~CNoiseReductionLevel();
    char* id_name();
private:
    long m_choice[4];
};
class CPageSizeWidth1200dpi : public CCsdTagLong
{
public:
	CPageSizeWidth1200dpi(ICsdTags2* parent);
	virtual ~CPageSizeWidth1200dpi();
	char* id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	void update(ICsdTag* sender);
	void update_def(ICsdTag* sender);
};
class CPageSizeLength1200dpi : public CCsdTagLong
{
public:
	CPageSizeLength1200dpi(ICsdTags2* parent);
	virtual ~CPageSizeLength1200dpi();
	char* id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
	void update(ICsdTag* sender);
	void update_def(ICsdTag* sender);
};
class CRapidRecovery : public CCsdTagLong
{
public:
	CRapidRecovery(ICsdTags2* parent);
	virtual ~CRapidRecovery();
	char* id_name();
private:
	long m_choice[2];
};
class CPassport : public CCsdTagLong
{
public:
	CPassport(ICsdTags2* parent);
	virtual ~CPassport();
	char* id_name();
private:
	long m_choice[2];
};
class CCarrierSheet : public CCsdTagLong
{
public:
	CCarrierSheet(ICsdTags2* parent);
	virtual ~CCarrierSheet();
	char* id_name();
private:
	long m_choice[3];
};
class CCustom3Gamma : public CCsdTagLong
{
public:
	CCustom3Gamma(ICsdTags2* parent);
	virtual ~CCustom3Gamma();
	char* id_name();
private:
	long m_choice[2];
};
class CCustom3GammaFilePath : public CCsdTagAsci
{
public:
	CCustom3GammaFilePath(ICsdTags2* parent);
	virtual ~CCustom3GammaFilePath();
	int set(long long lParam);
	char *id_name();
};
class CDriverVersion : public CCsdTagAsci
{
public:
	CDriverVersion(ICsdTags2* parent);
	~CDriverVersion();
	int set(long long lParam);
	//char* id_name();
};
class CScannerModel : public CCsdTagAsciScanner
{
public:
	CScannerModel(ICsdTags2* parent, IVirtualScanner* pscanner);
	virtual ~CScannerModel();
	int get(void* lpParam);
private:
	char m_s[16 + 1];
};
class CMirror : public CCsdTagLong
{
public:
	CMirror(ICsdTags2* parent);
	virtual ~CMirror();
	char* id_name();
private:
	long m_choice[2];
};
class CMaxDocuments : public CCsdTagLong
{
public:
	CMaxDocuments(ICsdTags2* parent);
	virtual ~CMaxDocuments();
	char* id_name();
	ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
private:
};
#endif
