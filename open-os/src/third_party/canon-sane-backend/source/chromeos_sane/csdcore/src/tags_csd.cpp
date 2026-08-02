/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <list>
#include <string.h>
#include "ceilogwrite.h"
#include "sdk_tag_basic.h"
#include "sdk_tag_pagesize.h"
#include "sdk_tag_options.h"
#include "sdk_tag_scan.h"
#include "sdk_tag_scanner.h"
#include "global_apis.h"
#include "tag_enum_interface.h"
#include "scanctrl_interface.h"
#include "virtual_scanner_interface.h"
#include "sdk_pagesize_table.h"
#include "csdtags.h"
#include "csdtags.org.h"
namespace csdtag
{
	class CDoubleFeedDetectionUltrasonic : public CCsdTagLong
	{
	public:
		CDoubleFeedDetectionUltrasonic(ICsdTags2* parent);
		virtual ~CDoubleFeedDetectionUltrasonic();
		char* id_name();
	private:
		long m_choice[2];
	};
	CDoubleFeedDetectionUltrasonic::CDoubleFeedDetectionUltrasonic(ICsdTags2* parent) :CCsdTagLong(CSDP_DBLFEEDUSS, parent)
	{
		m_choice[0] = 0;
		m_choice[1] = 1;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
	}
	CDoubleFeedDetectionUltrasonic::~CDoubleFeedDetectionUltrasonic()
	{
	}
	char* CDoubleFeedDetectionUltrasonic::id_name()
	{
		return (char*)"CSDP_DBLFEEDUSS";
	}
	class CDoubleFeedDetectionLength : public CCsdTagLong
	{
	public:
		CDoubleFeedDetectionLength(ICsdTags2* parent);
		virtual ~CDoubleFeedDetectionLength();
		char* id_name();
	private:
		long m_choice[2];
	};
	CDoubleFeedDetectionLength::CDoubleFeedDetectionLength(ICsdTags2* parent) :CCsdTagLong(CSDP_DBLFEEDLENGTH, parent)
	{
		m_choice[0] = 0;
		m_choice[1] = 1;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
	}
	CDoubleFeedDetectionLength::~CDoubleFeedDetectionLength()
	{
	}
	char* CDoubleFeedDetectionLength::id_name()
	{
		return (char*)"CSDP_DBLFEEDLENGTH";
	}
	class CAutoSize : public CCsdTagLong
	{
	public:
		CAutoSize(ICsdTags2* parent);
		virtual ~CAutoSize();
		char* id_name();
	private:
		long m_choice[2];
	};
	CAutoSize::CAutoSize(ICsdTags2* parent) :CCsdTagLong(CSDP_AUTOSIZE, parent)
	{
		m_choice[0] = 0;
		m_choice[1] = 1;
		m_def =*m_v.rbegin() = 1;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
	}
	CAutoSize::~CAutoSize()
	{
	}
	char* CAutoSize::id_name()
	{
		return (char*)"CSDP_AUTOSIZE";
	}
	class CDeskew : public CCsdTagLong
	{
	public:
		CDeskew(ICsdTags2* parent);
		virtual ~CDeskew();
		char* id_name();
	private:
		long m_choice[2];
	};
	CDeskew::CDeskew(ICsdTags2* parent) :CCsdTagLong(CSDP_DESKEW, parent)
	{
		m_choice[0] = 0;
		m_choice[1] = 1;
		m_pchoice = m_choice;
		m_def = *m_v.rbegin() = 1;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
	}
	CDeskew::~CDeskew()
	{
	}
	char* CDeskew::id_name()
	{
		return (char*)"CSDP_DESKEW";
	}
	class CSpp : public CCsdTagLong
	{
	public:
		CSpp(ICsdTags2* parent);
		virtual ~CSpp();
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
		char* id_name();
	private:
		long m_choice[2];
	};
	CSpp::CSpp(ICsdTags2* parent) :CCsdTagLong(CSDP_SAMPLESPERPIXEL, parent)
	{
		m_choice[0] = 1;
		m_choice[1] = 3;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
		m_def = 3;
		*m_v.rbegin() = m_def;
	}
	CSpp::~CSpp()
	{
	}
	ICsdTag::CSDTAG_CHOICE_FLAG CSpp::choice_flag()
	{
		return ICsdTag::CHOICE_LIST;
	}
	char* CSpp::id_name()
	{
		return (char*)"CSDP_SAMPLESPERPIXEL";
	}

	class CBps : public CCsdTagLong
	{
	public:
		CBps(ICsdTags2* parent);
		virtual ~CBps();
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
		char* id_name();
	private:
		long m_choice[2];
	};
	CBps::CBps(ICsdTags2* parent) :CCsdTagLong(CSDP_BITSPERSAMPLE, parent)
	{
		m_choice[0] = 1;
		m_choice[1] = 8;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
		m_def = 8;
		*m_v.rbegin() = m_def;
	}
	CBps::~CBps()
	{
	}
	ICsdTag::CSDTAG_CHOICE_FLAG CBps::choice_flag()
	{
		return ICsdTag::CHOICE_LIST;
	}
	char* CBps::id_name()
	{
		return (char*)"CSDP_BITSPERSAMPLE";
	}
	class CBrightness : public CCsdTagLong
	{
	public:
		CBrightness(ICsdTags2* parent);
		virtual ~CBrightness();
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
		char* id_name();
	private:
		long m_choice[3];
	};
	CBrightness::CBrightness(ICsdTags2* parent) :CCsdTagLong(CSDP_BRIGHTNESS, parent)
	{
		m_choice[0] = 1;//low
		m_choice[1] = 1;//step
		m_choice[2] = 255;//high
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
		m_def = 128;
		*m_v.rbegin() = m_def;
	}
	CBrightness::~CBrightness()
	{
	}
	ICsdTag::CSDTAG_CHOICE_FLAG CBrightness::choice_flag()
	{
		return ICsdTag::CHOICE_RANGE;
	}
	char* CBrightness::id_name()
	{
		return(char*)"CSDP_BRIGHTNESS";
	}
	class CContrast : public CCsdTagLong
	{
	public:
		CContrast(ICsdTags2* parent);
		virtual ~CContrast();
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag();
		char* id_name();
	private:
		long m_choice[3];
	};
	CContrast::CContrast(ICsdTags2* parent) :CCsdTagLong(CSDP_CONTRAST, parent)
	{
		m_choice[0] = 1;//low
		m_choice[1] = 1;//step
		m_choice[2] = 255;//high
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
		m_def = 128;
		*m_v.rbegin() = m_def;
	}
	CContrast::~CContrast()
	{
	}
	ICsdTag::CSDTAG_CHOICE_FLAG CContrast::choice_flag()
	{
		return ICsdTag::CHOICE_RANGE;
	}
	char* CContrast::id_name()
	{
		return(char*)"CSDP_CONTRAST";
	}
	class CXDpi : public CCsdTagLong
	{
	public:
		CXDpi(ICsdTags2* parent);
		virtual ~CXDpi();
		char* id_name();
	private:
	};
	CXDpi::CXDpi(ICsdTags2* parent) :CCsdTagLong(CSDP_XRESOLUTION, parent)
	{
		m_def = 300;
		set(m_def);
	}
	CXDpi::~CXDpi()
	{
	}
	char* CXDpi::id_name()
	{
		return (char*)"CSDP_XRESOLUTION";
	}
	class CScanAhead : public CCsdTagLong
	{
	public:
		CScanAhead(ICsdTags2* parent);
		virtual ~CScanAhead();
		//char *id_name();
	private:
		long m_choice[2];
	};
	CScanAhead::CScanAhead(ICsdTags2* parent) :CCsdTagLong(CSDP_READAHEAD, parent)
	{
		m_choice[0] = 0;
		m_choice[1] = 1;
		m_pchoice = m_choice;
		m_choice_size = (long)(sizeof(m_choice) / sizeof(m_choice[0]));
		*m_v.rbegin() = m_def = 1;
	}
	CScanAhead::~CScanAhead()
	{
	}
	class CCsdTagLongAreaMeta1200dpi : public CCsdTagBase
	{
	public:
		CCsdTagLongAreaMeta1200dpi(long id, ICsdTags2* parent) :
			CCsdTagBase(id, parent),
			m_page(NULL)
		{
			m_page = parent->get(CSDP_PAGESIZE);
		}
		~CCsdTagLongAreaMeta1200dpi()
		{}
		virtual long which(long w, long h) = 0;
		int get(void* lpParam)
		{
			long* pout = (long*)lpParam;
			char s[32] = { 0 };
			m_page->get(s);
			long w1200dpi = 0;
			long h1200dpi = 0;
			pagesize_table_get(s, &w1200dpi, &h1200dpi);
			*pout = which(w1200dpi, h1200dpi);
			return 0;
		}
		int set(long long lParam)
		{
			return 0;
		}
		int choice(int index, void* lpParam)
		{
			long* pout = (long*)lpParam;
			long v = 0;
			get(&v);
			switch ((CSDTAG_CHOICE_RANGE)index) {
			case ICsdTag::RANGE_LOW: *pout = 0; break;
			case ICsdTag::RANGE_STEP:*pout = 1; break;
			case ICsdTag::RANGE_HIGH:*pout = v; break;
			default:
				*pout = 0;
				break;
			}
			return 0;
		}
		int choice_count(long* lpCount)
		{
			*lpCount = 3;
			return 0;
		}
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag()
		{
			return ICsdTag::CHOICE_RANGE;
		}
	protected:
		ICsdTag* m_page;
	};
	class CXPos1200dpi : public CCsdTagLongAreaMeta1200dpi
	{
	public:
		CXPos1200dpi(ICsdTags2* parent) :CCsdTagLongAreaMeta1200dpi(CSDP_XOFFSET1200DPI, parent)
		{
		}
		~CXPos1200dpi()
		{
		}
		long which(long , long ) { return 0; }
	};
	class CYPos1200dpi : public CCsdTagLongAreaMeta1200dpi
	{
	public:
		CYPos1200dpi(ICsdTags2* parent) :CCsdTagLongAreaMeta1200dpi(CSDP_YOFFSET1200DPI, parent)
		{
		}
		~CYPos1200dpi()
		{
		}
		long which(long , long ) { return 0; }
	};
	class CImageWidth1200dpi : public CCsdTagLongAreaMeta1200dpi
	{
	public:
		CImageWidth1200dpi(ICsdTags2* parent) :CCsdTagLongAreaMeta1200dpi(CSDP_IMAGEWIDTH1200DPI, parent)
		{
		}
		~CImageWidth1200dpi()
		{
		}
		long which(long w, long) { return w; }
	};
	class CImageLength1200dpi : public CCsdTagLongAreaMeta1200dpi
	{
	public:
		CImageLength1200dpi(ICsdTags2* parent) :CCsdTagLongAreaMeta1200dpi(CSDP_IMAGELENGTH1200DPI, parent)
		{
		}
		~CImageLength1200dpi()
		{
		}
		long which(long, long h) { return h; }
	};




	class CCsdTagLongAreaMeta : public CCsdTagBase
	{
	public:
		CCsdTagLongAreaMeta(long id, ICsdTags2* parent) :
			CCsdTagBase(id, parent),
			m_dpi(NULL),
			m_1200dpi(NULL)
		{}
		~CCsdTagLongAreaMeta()
		{}
		int get(void* lpParam)
		{
			long* pout = (long*)lpParam;
			long dpi = 300;
			m_dpi->get(&dpi);
			long v1200dpi = 0;
			m_1200dpi->get(&v1200dpi);
			*pout = v1200dpi * dpi / 1200;
			return 0;
		}
		int set(long long lParam)
		{
			return 0;
		}
		int choice(int index, void* lpParam)
		{
			long* pout = (long*)lpParam;
			long v = 0;
			get(&v);
			switch ((CSDTAG_CHOICE_RANGE)index) {
			case ICsdTag::RANGE_LOW: *pout = 0; break;
			case ICsdTag::RANGE_STEP:*pout = 1; break;
			case ICsdTag::RANGE_HIGH:*pout = v; break;
			default:
				*pout = 0;
				break;
			}
			return 0;
		}
		int choice_count(long* lpCount)
		{
			*lpCount = 3;
			return 0;
		}
		ICsdTag::CSDTAG_CHOICE_FLAG choice_flag()
		{
			return ICsdTag::CHOICE_RANGE;
		}
	protected:
		ICsdTag* m_dpi;
		ICsdTag* m_1200dpi;
	};
	class CXPos : public CCsdTagLongAreaMeta
	{
	public:
		CXPos(ICsdTags2* parent) :CCsdTagLongAreaMeta(CSDP_XOFFSET, parent)
		{
			m_dpi = parent->get(CSDP_XRESOLUTION);
			m_1200dpi = parent->get(CSDP_XOFFSET1200DPI);
		}
		~CXPos()
		{
		}};
	class CYPos : public CCsdTagLongAreaMeta
	{
	public:
		CYPos(ICsdTags2* parent) :CCsdTagLongAreaMeta(CSDP_YOFFSET, parent)
		{
			m_dpi = parent->get(CSDP_YRESOLUTION);
			m_1200dpi = parent->get(CSDP_YOFFSET1200DPI);
		}
		~CYPos()
		{
		}
	};
	class CImageWidth : public CCsdTagLongAreaMeta
	{
	public:
		CImageWidth(ICsdTags2* parent) :CCsdTagLongAreaMeta(CSDP_IMAGEWIDTH, parent)
		{
			m_dpi = parent->get(CSDP_XRESOLUTION);
			m_1200dpi = parent->get(CSDP_IMAGEWIDTH1200DPI);
		}
		~CImageWidth()
		{
		}
	};
	class CImageLength : public CCsdTagLongAreaMeta
	{
	public:
		CImageLength(ICsdTags2* parent) :CCsdTagLongAreaMeta(CSDP_IMAGELENGTH, parent)
		{
			m_dpi = parent->get(CSDP_YRESOLUTION);
			m_1200dpi = parent->get(CSDP_IMAGELENGTH1200DPI);
		}
		~CImageLength()
		{
		}
	};
}
long enum_csdtags(ICsdTags2 *parent, IScanCtrl *pscan, IVirtualScanner *pscanner, IUnknown *handle)
{
	//WriteLog((char*)"enum_csdtags() start");
	parent->add(new csdtag::CXDpi(parent));
	parent->add(new CYDpi(parent));    
	parent->add(new CPageSize(parent, pscanner));
	parent->add(new csdtag::CXPos1200dpi(parent));
	parent->add(new csdtag::CYPos1200dpi(parent));
	parent->add(new csdtag::CImageWidth1200dpi(parent));
	parent->add(new csdtag::CImageLength1200dpi(parent));
	parent->add(new csdtag::CXPos(parent));
	parent->add(new csdtag::CYPos(parent));
	parent->add(new csdtag::CImageWidth(parent));
	parent->add(new csdtag::CImageLength(parent));
	parent->add(new csdtag::CSpp(parent));
	parent->add(new csdtag::CBps(parent));
	parent->add(new csdtag::CBrightness(parent));
	parent->add(new csdtag::CContrast(parent));
	parent->add(new CScanSide(parent));
	parent->add(new csdtag::CScanAhead(parent));
	parent->add(new csdtag::CDoubleFeedDetectionUltrasonic(parent));
	parent->add(new csdtag::CDoubleFeedDetectionLength(parent));
	parent->add(new csdtag::CAutoSize(parent));
	parent->add(new csdtag::CDeskew(parent));
	parent->add(new CSerialNumber(parent, pscanner));
	parent->add(new CTotalCounter(parent, pscanner));
	parent->add(new CScannerButton(parent, pscanner));
	parent->add(new CFeederLoaded(parent, pscanner));
	return 0;
}
