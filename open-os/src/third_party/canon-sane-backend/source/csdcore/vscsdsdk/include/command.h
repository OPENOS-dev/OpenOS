/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _CEI_CSDCORE2_COMMAND_CLASS_HEADER_
#define _CEI_CSDCORE2_COMMAND_CLASS_HEADER_

#include <vector>
#include <string>
#include <memory.h>

typedef enum enumtagOperationCode {
	OPERATION_MIN,
	opTestUnitReady		=0x00,
	opRequestSense		=0x03,
	opInquiry			=0x12,
	opModeSelect		=0x15,
	opReserveUnit		=0x16,
	opReleaseUnit		=0x17,
	opModeSense			=0x1A,
	opScan				=0x1B,
	opReceiveDiagnostic	=0x1C,
	opSendDiagnostic	=0x1D,	
	opSetWindow			=0x24,
	opGetWindow			=0x25,
	opRead				=0x28,
	opSend				=0x2A,
	opObjectPosition	=0x31,
	opRescan			=0xC3,
	opDiscard			=0xC4,
    opGetScannerStatus  =0xC5,
	opErrorClear    	=0xC7,
	opGetScanMode		=0xD5,
	opDefineScanMode	=0xD6,
	opStopBatch			=0xD8,
	opSleepControl		=0xD9,
	opGetAdjustData		=0xE0,
	opSetAdjustData		=0xE1,
	opCheckScanSize     =0xC6,
	opService			=0xFF,
	opGetMemory			=0x3B,
	opLockOperationPanel=0xE2,
	opUnlockOperationPanel=0xE3,
	opGetScanParameter	=0xE4,
	opSetScanParameter	=0xE5,
	opSetImprinter		=0xE5,
	opGetImprinter		=0xE4,
	opRunSubsidiary     =0xE9,
	OPERATION_MAX
}OperationCode;
class CCommand
{
public:
	CCommand();
	virtual ~CCommand();
	char *  cdb();
	long    cdb_size();
	char*   data();
	long    data_size();
	unsigned char transfer_data_type();
	void           transfer_data_type(unsigned char s);
	long    transfer_length();
	void    transfer_length(long s);
	void    transfer_identification(short id);
	short   transfer_identification();
	unsigned char allocation_length();
	void          allocation_length(unsigned char s);
	void copy(CCommand &src);
	bool operator==(CCommand &src);
	bool operator!=(CCommand& src);
	typedef enum tagEXEC_TYPE{
		EXEC_NONE=0,
		EXEC_WRITE=1,
		EXEC_READ=2
	}EXEC_TYPE;
	typedef enum tagCMD_SCSI_ERROR
	{
		CMD_OK=0,
		CMD_CHECKCONDITION=1,
		CMD_NOMEM=2
	}CMD_SCSI_ERROR;
	virtual void I_am_in(EXEC_TYPE type);
	virtual CCommand* clone();
	virtual void serialize(FILE *fp);
	virtual void deserialize(FILE *fp);
protected:
	unsigned char m_cdb[10];
	long m_cdb_size;
	unsigned char * m_pdata;
protected:
	void input(char * cdb, long cdb_size, char * data, long data_size);
};
class CTestUnitReadyCmd : public CCommand
{
public:
	CTestUnitReadyCmd();
	~CTestUnitReadyCmd(){}
};
class CInquiryCmd : public CCommand
{
public:
	CInquiryCmd();
	CInquiryCmd(CInquiryCmd& in);
	CInquiryCmd(char * cdb, long cdb_size, char * data, long data_size);
	~CInquiryCmd();
	CInquiryCmd& operator = (CInquiryCmd& in);
	void evpd(bool sw);
	bool evpd();
	void page_code(char code);
	void allocation_length(long v);
	long allocation_length();
	bool wireless();
	void wireless(bool sw);
	long window_width();
	void window_width(long v);
	long window_length();
	void window_length(long v);
	long real_window_width();
	void real_window_width(long v);
	long basic_xdpi();
	void basic_xdpi(long v);
	long basic_ydpi();	
	void basic_ydpi(long v);
	bool has_flatbed();
	void scanner_name(char *in);
	char *scanner_name();
	void product_revision_level(char *in);
	char *product_revision_level();
private:
	unsigned char m_data[64];
	char m_out[16 + 2];
};
class CReserveUnitCmd : public CCommand
{
public:
	CReserveUnitCmd();
	~CReserveUnitCmd(){}
};
class CReleaseUnitCmd : public CCommand
{
public:
	CReleaseUnitCmd();
	~CReleaseUnitCmd(){}
};
class CStopBatchCmd : public CCommand
{
public:
	CStopBatchCmd();
	~CStopBatchCmd(){}
};
class CDiscardCmd : public CCommand
{
public:
	CDiscardCmd();
	CDiscardCmd(CDiscardCmd& in);
	CDiscardCmd(char* cdb, long cdb_size, char* data, long data_size);
	~CDiscardCmd();
	CDiscardCmd& operator = (CDiscardCmd& in);
public:
	void window_identifier(char v);
	char window_identifier();
	void resampling_after_scan(bool v);
	bool resampling_after_scan();
	void resampling(bool v);
	bool resampling();
	void side(bool v);//false:front, true:back
	bool side();
public:
	void main_window(char v);
private:
	unsigned char m_data[1];
};
class CObjectPositionCmd : public CCommand
{
public:
	CObjectPositionCmd(long type);
	CObjectPositionCmd(char * cdb, long cdb_size);
	~CObjectPositionCmd();
	typedef enum enumType {
		AbortScan = 0,
		MediumPosition = 1,
		PreFeed = 2,
		StartAdjustLight = 5,
		Reject = 4	
	}TYPE;
	TYPE position_type();
};
class CAbortCmd : public CObjectPositionCmd
{
public:
	CAbortCmd();
	CAbortCmd(CAbortCmd &in);
	~CAbortCmd(){}
};
class CScanCmd : public CCommand
{
public:
	CScanCmd();
	CScanCmd(char * cdb, long cdb_size, char * data, long data_size);
	CScanCmd(CScanCmd &w);
	~CScanCmd();
	bool duplex();
	void duplex(bool on);
	CScanCmd& operator = (CScanCmd& in);
public:
	void window_identifier(char v);// == main window id
	char window_identifier();//==main window id
	void extention(char number, bool v);
	bool extention(char number);
	void side(bool v);
	bool side();
public:
	char main_window();
	char sub_window();
	void main_window(char id);
	void sub_window(char id);
private:
	unsigned char m_data[2];
};
#if 0
class CScanCmd2 : public CCommand
{
public:
	CScanCmd2();
	CScanCmd2(char* cdb, long cdb_size, char* data, long data_size);
	CScanCmd2(CScanCmd2& w);
	~CScanCmd2();
	void window_identifier(int index, char v);
	char window_identifier(int index);
	void extention(int index, int number, bool v);
	bool extention(int index, int number);
	void side(int index, bool v);
	bool side(int index);
	void set_scanner_key(char* v);
	void get_scanner_key(char* v);
private:
	unsigned char m_data[256];
	enum {
		SCANNER_KEY_SIZE = 128
	};
};
#endif
class CSenseCmd : public CCommand
{
public:
	CSenseCmd();
	CSenseCmd(CSenseCmd& in);
	CSenseCmd(char * cdb, long cdb_size, char * data, long data_size);
	~CSenseCmd();
	CSenseCmd& operator =(CSenseCmd& in);
	char sense_key();
	void sense_key(char key);
	short code();
	char additional_sense_code();
	char additional_sense_code_qualifier();
	void valid(char v);
	char valid();
	void error_code(char v);
	char error_code();
	void additional_sense_length(char v);
	char additional_sense_length();
	bool ILI();
	void ILI(bool sw);
	long information_bytes(); 
	int information_bytes(long size);
	int bad_sequence();
	int nomemory();
	int invalid_param();
	int jam();
	int nopaper();
	int cover_open();
	int doublefeed();
    int cancel();
	bool is_no_paper();
	bool is_double_feed_error();
	bool is_bad_sequence_error();
	bool is_power_on_reset_error();
    bool is_bad_cdb_error();
	bool is_jam_error();
    bool is_cover_open();
	bool has_error();
	void clear();
private:
	unsigned char m_data[14];
	int set_error(char key, char code, char qual);
};
class CStreamCmd : public CCommand
{
public:
	CStreamCmd();
	CStreamCmd(CStreamCmd& in);
	CStreamCmd(char * cdb, long cdb_size, char * data, long data_size);
	CStreamCmd(char * data, long data_size);
	CStreamCmd(long data_size);
	CStreamCmd(long transfer_data_type, long transfer_identification);
	~CStreamCmd();
	
	typedef enum enumTransferDataType {
		IMAGE=0,
		AREAINFO=0x80,
		EJECT=0xA1,
        COLORCOMPOSITION=0xA7,
        COMPLETE_IMAGEINFO=0xA8,
		PANEL=0x84,
		SHADING=0x90,
		PAPER=0x8B,
		GAMMA=0x3,
		USERDATA=0x8C,
		SERVICEDATA=0x8C,
		NETWORK=0xb0,
		NETWORKEX=0xb1,
		PATCHCODE=0x85,
		MICR=0x86,
		IMPRINTER=0x8E,
		COLOR_DETECTION=0x9A,
		BLANKPAGE_DETECTION=0x9B,
		FEEDING_OPTION=0xD1
	}TransferDataType;
	typedef enum enumTransferIdentification {
		IMAGEAREA=0,
		MARGIN=1,
		PAPERINFO=4,

		GAMMA_GRAY=2,				
		GAMMA_BLUE=0x4,
		GAMMA_GREEN=0x8,
		GAMMA_RED=0x10,
		
		GAMMA_FULLCOLOR=0x1c,
		GAMMA_FULLCOLOR_DROPOUT=0x1d,
		GAMMA_FULLCOLOR_EMPHASIS=0x3c,
        
        AREAINFO_4POINTS_AFTER=5,
        AREAINFO_4POINTS_BEFORE=7,
        
        AREAINFO_IMAGEAREA2_AFTER=0x10,
        AREAINFO_IMAGEAREA2_BEFORE=0x12,
        AREAINFO_MARGIN2_AFTER=0x11,
        AREAINFO_MARGIN2_BEFORE=0x13,
        AREAINFO_4POINTS2_AFTER=0x15,
        AREAINFO_4POINTS2_BEFORE=0x17,
        AREAINFO_PAPERINFO2_AFTER=0x14,
        AREAINFO_PAPERINFO2_BEFORE=0x16,
        AREAINFO_PAPERAREA=0x40,
        AREAINFO_VALIDAREA=0x41,
        
        MICR_ALLGAIN=0x80,
		MICR_MIDGAIN=0x81,
		MICR_HIGHGAIN=0x82,
		MICR_LOWGAIN=0x83,

		VS_ORIGINAL_MICR_RESULT=0x97,
		VS_ORIGINAL_IMAGEINFO=0x98,
		VS_ORIGINAL=0x99,
	}TransferIdentification;

	typedef enum enumRGB_TYPE{
		RED=0,
		BLUE=1,
		GREEN=2
	}RGB_TYPE;

	typedef enum enumSIDE_TYPE{
		FRONT=0,
		BACK=1
	}SIDE_TYPE;

	CStreamCmd& operator = (CStreamCmd& src);
	CCommand *clone();

	//shading
	void black_or_white(bool black);
	void black();
	void white();
	void rgb(RGB_TYPE rgb);
	void side(bool front);
	void shading(char * pdata, long size);

	//eject
	void eject(bool eject);
	bool eject();
	void doublefeed(bool v);
	bool doublefeed();

	//area
	long areainfo_upperleftx();
	long areainfo_upperlefty();
	long areainfo_width();
	long areainfo_length();
	void areainfo_upperleftx(long v);
	void areainfo_upperlefty(long v);
	void areainfo_width(long v);
	void areainfo_length(long v);
	//autosize info
	long autosize_upperleftx();
	long autosize_upperlefty();
	long autosize_width();
	long autosize_length();
	void autosize_upperleftx(long v);
	void autosize_upperlefty(long v);
	void autosize_width(long v);
	void autosize_length(long v);

	//margin
	long margin_left();
	long margin_top();
	long margin_right();
	long margin_bottom();
	void margin_left(long v);
	void margin_top(long v);
	void margin_right(long v);
	void margin_bottom(long v);

	//4points
	long p4_upperleftx();
	long p4_upperlefty();
	long p4_upperrightx();
	long p4_upperrighty();
	long p4_lowerleftx();
	long p4_lowerlefty();
	long p4_lowerrightx();
	long p4_lowerrighty();
	void p4_upperleftx(long v);
	void p4_upperlefty(long v);
	void p4_upperrightx(long v);
	void p4_upperrighty(long v);
	void p4_lowerleftx(long v);
	void p4_lowerlefty(long v);
	void p4_lowerrightx(long v);
	void p4_lowerrighty(long v);

	//paper info
	long paper_length();
	void paper_length(long v);

	//paper
	void paper_is(bool bpaper);
	bool paper_is_detected();
	
	//gamma
	bool gamma_download();
	bool gamma_back();
	char gamma_colortype();

	//patchcode
	bool patchcode();
	void patchcode(bool v);
	long patchcode_type();
	void patchcode_type(long type);

	//color detection
	char front_result();
	void front_result(char v);
	char front_color_pixels();
	void front_color_pixels(char v);
	long front_color_lines();
	void front_color_lines(long v);
	char back_result();
	void back_result(char v);
	char back_color_pixels();
	void back_color_pixels(char v);
	long back_color_lines();
	void back_color_lines(long v);

	//detect blank paper
	long number_of_change_points_x_front();
	long number_of_change_points_y_front();
	long number_of_change_points_x_back();
	long number_of_change_points_y_back();


	//user data
	long maximum_paper_length();
	void maximum_paper_length(long v);
	long paper_counter();
	void paper_counter(long v);
	long parts1_counter();
	void parts1_counter(long v);
	void parts1_time(long v);
	long parts1_time();
	long vertical_scaling();
	void vertical_scaling(long v);
	char *serial_number();
	void serial_number(char *s);
	long poweroff_time();
	void poweroff_time(long time);
	long sleep_time();
	void sleep_time(long time);
	long parts1_counter_limit();
	void parts1_counter_limit(long v);
	long enable_parts1_warning();
	void enable_parts1_warning(long v);

	long skipped_paper_counter();
	long paper_counter2();
	void skipped_paper_counter(long v);
	void paper_counter2(long v);
	void status_is(long status);//0 is no error, 6 is doublefeed error.
	long status_is();
	void is_scan_done(bool v);
	bool is_scan_done();
	//FEEDING_OPTION
	char feeding_option();
	void feeding_option(char v);
	char prescan();
	void prescan(char v);
	//user data(vsoriginal)VS_ORIGINAL_IMAGEINFO
	void image_is_blankpage_front(long v);
	long image_is_blankpage_front();
	void image_is_blankpage_back(long v);
	long image_is_blankpage_back();
	void image_is(long front);
	long image_is();
	void angle_of_rotation_is(long angle);//0=0, 1=90, 2=180, 3=270
	long angle_of_rotation_is();

	//user data(vsoriginal)VS_ORIGINAL_MICR_RESULT
	char *micr_text();
	void micr_text(char *text);

	//
	void attach_buffer(char *buffer, long size);

	//panel
	bool start_key();
	bool stop_key();
	bool up_key();
	bool down_key();
	bool left_key();
	bool right_key();
    bool dfr_key();
    bool non_sep_key();
	
	void enable_stop_key(bool v);
	bool enalbe_stop_key();
	void set_or_clear(bool v);
	bool set_or_clear();
	void mode(char v);
	char mode();
	void title(wchar_t  *s);
    void panel_json(wchar_t  *sjson);

    //imprint
    char *data_of_imprint(char *work);	
private:	
	void I_am_in(CCommand::EXEC_TYPE type);
private:
	unsigned char *m_buffer;
};
class CMode : public CCommand
{
public:
	CMode();
	CMode(CMode &in);
	CMode(char * cdb, long cdb_size, char * data, long data_size);
	~CMode();
	long mud();
	void mud(long v);
	CMode& operator = (CMode& src);
private:
	void I_am_in(CCommand::EXEC_TYPE type);
private:
	unsigned char m_data[12];
};
class CWindow : public CCommand
{
public:
	CWindow();
	CWindow(char * cdb, long cdb_size, char * data, long data_size);
	CWindow(CWindow &w);
	~CWindow();

	CWindow  &operator = (CWindow &src);

	short xdpi();
	void xdpi(short dpi);

	short ydpi();
	void ydpi(short dpi);

	short dpi();
	void dpi(short dpi);

	long width();
	long length();
	long spp();
	long bps();
	bool error_diffusion();
	void error_diffusion(bool sw);
	bool ateii();
	void ateii(bool sw);

	void AEmode(char v);
	char AEmode();

	void bpp(char bpp);
	char bpp();

	void width(long w);
	void length(long l);
	
	void spp(char v);
	void bps(char v);

	char image_composition();
	void image_composition(char ic);

	void window_identifier(char v);
	char window_identifier();

	void side(bool v);
	bool side();
	void resampling(bool v);
	bool resampling();
	void resampling_after_scan(bool v);
	bool resampling_after_scan();

	void image_processing_progress(long v);
	long image_processing_progress();

	bool I_am_front_window();
	bool I_am_back_window();

	void xoffset(long x);
	void yoffset(long y);
	
	long xoffset();
	long yoffset();

	void grc(bool v);
	bool grc();
	bool through_grc();
	void through_grc(bool v);

	long brightness();
	long contrast();

	void brightness(long v);
	void contrast(long v);

	long compression_type();
	void compression_type(long v);

	long compression_argument();
	void compression_argument(long v);

	char threshold();
	void threshold(char v);

	long rotation();
	void rotation(long v);

	bool high_speed();
	void high_speed(bool v);

	bool rif();
	void rif(bool v);
	
private:
	void I_am_in(CCommand::EXEC_TYPE type);
private:
	unsigned char m_data[52];
	char line_5_and_8();
};
class CScanMode : public CCommand
{
public:
	typedef enum tagPAGE_CODE_TYPE{
		PAGE_CODE_OPTION=0x30,
		PAGE_CODE_SCAN=0x32,
		PAGE_CODE_FILTER=0x36,
		PAGE_CODE_DATE=0x37,
		PAGE_CODE_MICR=0x3a,
		PAGE_CODE_OCR=0x10,//vs original page code.
		PAGE_CODE_FILTER2=0x11,//vs original page code.
		PAGE_CODE_SCAN2=0x12//vs originCal page code.
	}PAGE_CODE_TYPE;
	CScanMode(PAGE_CODE_TYPE page_code);
	CScanMode(char * cdb, long cdb_size, char * data, long data_size);
	CScanMode(CScanMode &sm);
	CScanMode();
	~CScanMode();

	CScanMode& operator = (CScanMode& in);

	void page_code(char code);
	char page_code();

	//PAGE_CODE_SCAN
	bool batch();
	void batch(bool sw);

	long maxdocument();
	void maxdocument(long count);
	long maximum_number_of_documents_in_batch_mode();
	void maximum_number_of_documents_in_batch_mode(long v);
	

	char scanside();
	void scanside(char v);

	typedef enum tagCOLOR_TYPE{
		NONE=0,
		RED,
		GREEN,
		BLUE
	}COLOR_TYPE;
	typedef enum tagSIDE_TYPE{
		FRONT = 0,
		BACK  = 1
	}SIDE_TYPE;
		
	//PAGE_CODE_OPTION
	bool micr();
	void micr(bool v);
	bool imprinter();
	void imprinter(bool v);

    bool dfd_uss();
    void dfd_uss(bool v);
    bool dfd_length();
    void dfd_length(bool v);



	//PAGE_CODE_SCAN
	bool autosize();
	void autosize(bool a);

	bool platen();
	void platen(bool p);

	bool duplex();
	void duplex(bool on);

	bool deskew();
	void deskew(bool on);

	char bothscanmode();
	void bothscanmode(char v);

	bool feeding_direction();
	void feeding_direction(bool sw);

	char detect_slant_option();
	void detect_slant_option(char v);

	char autosize_option();
	void autosize_option(char v);

	char deskew_option();
	void deskew_option(char v);

	bool folio();
	void folio(bool v);

	bool skip_blank_page();
	void skip_blank_page(bool v);

	long blank_page_param();
	void blank_page_param(long v);
	

	//PAGE_CODE_FILTER
	bool edgeemphasis();
	void edgeemphasis(bool v);

	long intensity_of_edgeemphasis();
	void intensity_of_edgeemphasis(long v);

	COLOR_TYPE drop_out(SIDE_TYPE type);
	void drop_out(SIDE_TYPE type, COLOR_TYPE col);
	
	COLOR_TYPE emphasis(SIDE_TYPE type);
	void emphasis(SIDE_TYPE type, COLOR_TYPE col);

	bool notch_erasure();
	void notch_erasure(bool v);

	bool dot_erasure();
	void dot_erasure(bool v);

	//PAGE_CODE_FILTER2
	char autocolor();
	void autocolor(char v);

	char autocolor_type();
	void autocolor_type(char type);

	char autocolor_binary_type();
	void autocolor_binary_type(char type);

	char sensitivity_of_colorbinary();
	void sensitivity_of_colorbinary(char v);

	char intensity_of_colorbinary();
	void intensity_of_colorbinary(char v);

	char sensitivity_of_colorgray();
	void sensitivity_of_colorgray(char v);

	char intensity_of_colorgray();
	void intensity_of_colorgray(char v);
	
	bool patch();
	void patch(bool sw);

	long patch_orientation();//0:0, 1:90, 2:180, 3:270
	void patch_orientation(long ori);

	bool auto_rotation();//PAGE_CODE_SCAN_BOTH_RESCAN vs original
	void auto_rotation(bool on);//PAGE_CODE_SCAN_BOTH_RESCAN vs original

	char erase_bleedthrough();
	void erase_bleedthrough(char v);

	char erase_bleedthrough_level();
	void erase_bleedthrough_level(char v);

	void background_color_equalization(bool v);
	bool background_color_equalization();

	long ftf();
	void ftf(long v);

	//PAGE_CODE_SCAN2
	void max_ahead_pages(long v);
	long max_ahead_pages();
	void disable_error_recovery_ex(bool v);
	bool disable_error_recovery_ex();

					
	//PAGE_CODE_MICR
	bool micr_wave();
	void micr_wave(bool v);

	//PAGE_CODE_OCR
	bool ocr();
	void ocr(bool v);
private:
	void I_am_in(CCommand::EXEC_TYPE type);
private:
	unsigned char m_data[128];
	
	enum {
		DATA_BLOCK_OFFSET=4//Reserved(0)Reserved(0)Reserved(0)BlockDescriptorLength(0)
	};
	
	char length(char page_code);
};
class CScanParam : public CCommand
{
public:

	typedef enum tagPAGE_CODE_TYPE{
		PAGE_CODE_OPTION=0x0,
		PAGE_CODE_DOUBLEFEED=0x1,
		PAGE_CODE_SCAN_BOTH=0x2,
		PAGE_CODE_SCAN_BOTH_RESCAN=0x3,
		PAGE_CODE_SCAN_SEP=0x4,
		PAGE_CODE_SCAN_SEP_RESCAN=0x06,
		PAGE_CODE_DATE=0x7,
		PAGE_CODE_PAPER_SIZE=0x11,
		PAGE_CODE_MARGIN=0x12,
		PAGE_CODE_IMPRINTER_HEAD=0x33,
		PAGE_CODE_IMPRINTER_TEXT=0x34,
	}PAGE_CODE_TYPE;
	typedef enum tagCOLOR_TYPE {
		NONE = 0,
		RED,
		GREEN,
		BLUE
	}COLOR_TYPE;
	typedef enum tagSIDE_TYPE {
		FRONT = 0,
		BACK = 1
	}SIDE_TYPE;

	CScanParam(PAGE_CODE_TYPE page_code, short id=0);
	CScanParam(char * cdb, long cdb_size, char * data, long data_size);
	CScanParam();
	CScanParam(CScanParam &in);
	~CScanParam();

	CScanParam& operator =(CScanParam &in);

	bool side();
	void side(bool v);
    
    long window_identifier();
    void window_identifier(long v);
	
	void page_code(char code);
	char page_code();

	void identification(short id);
	short identification();

	/////////////////data////////////////////////
	bool patch();//PAGE_CODE_OPTION
	void patch(bool sw);//PAGE_CODE_OPTION
	bool skew_detection();//PAGE_CODE_OPTION
	void skew_detection(bool v);//PAGE_CODE_OPTION
	bool double_feed_detection_length();//PAGE_CODE_OPTION
	void double_feed_detection_length(bool v);//PAGE_CODE_OPTION
	bool double_feed_detection_ultrasonic();//PAGE_CODE_OPTION
	void double_feed_detection_ultrasonic(bool v);//PAGE_CODE_OPTION
	bool imprinter();//PAGE_CODE_OPTION
	void imprinter(bool v);//PAGE_CODE_OPTION
	bool error_recovery();//PAGE_CODE_OPTION
	void error_recovery(bool v);//PAGE_CODE_OPTION
	bool error_recovery_ex();//PAGE_CODE_OPTION
	void error_recovery_ex(bool v);//PAGE_CODE_OPTION
	bool continue_scan();//PAGE_CODE_OPTION
	void continue_scan(bool v);//PAGE_CODE_OPTION
	long dfd_retry();//PAGE_CODE_OPTION
	void dfd_retry(long v);//PAGE_CODE_OPTION

	bool batch();//PAGE_CODE_SCAN_BOTH
	void batch(bool sw);//PAGE_CODE_SCAN_BOTH
	long maximum_number_of_documents_in_batch_mode();//PAGE_CODE_SCAN_BOTH
	void maximum_number_of_documents_in_batch_mode(long v);//PAGE_CODE_SCAN_BOTH
	long maximum_paper_length();//PAGE_CODE_SCAN_BOTH
	void maximum_paper_length(long v);//PAGE_CODE_SCAN_BOTH
	bool platen();//PAGE_CODE_SCAN_BOTH
	void platen(bool v);//PAGE_CODE_SCAN_BOTH
	bool passport_carriersheet();//PAGE_CODE_SCAN_BOTH
	void passport_carriersheet(bool v);//PAGE_CODE_SCAN_BOTH
	bool standard_carriersheet();//PAGE_CODE_SCAN_BOTH
	void standard_carriersheet(bool v);//PAGE_CODE_SCAN_BOTH
	bool nsf();//PAGE_CODE_SCAN_BOTH nsf is non seperation feed
	void nsf(bool v);//PAGE_CODE_SCAN_BOTH
	bool thinpaper();//PAGE_CODE_SCAN_BOTH
	void thinpaper(bool v);//PAGE_CODE_SCAN_BOTH
	bool thickpaper();//PAGE_CODE_SCAN_BOTH
	void thickpaper(bool v);//PAGE_CODE_SCAN_BOTH

	char autosize();//PAGE_CODE_SCAN_BOTH_RESCAN
	void autosize(char a);//PAGE_CODE_SCAN_BOTH_RESCAN
	bool deskew(); //PAGE_CODE_SCAN_BOTH_RESCAN
	void deskew(bool on); //PAGE_CODE_SCAN_BOTH_RESCAN
	char deskew_option();//PAGE_CODE_SCAN_BOTH_RESCAN
	void deskew_option(char v);//PAGE_CODE_SCAN_BOTH_RESCAN
	char drop_out();//PAGE_CODE_SCAN_SEP_RESCAN
	void drop_out(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char emphasis();//PAGE_CODE_SCAN_SEP_RESCAN
	void emphasis(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char emphasis_color();//PAGE_CODE_SCAN_SEP_RESCAN
	void emphasis_color(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char gamma_mode();//PAGE_CODE_SCAN_SEP_RESCAN
	void gamma_mode(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char color_gamma_mode();//PAGE_CODE_SCAN_SEP_RESCAN
	void color_gamma_mode(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char erase_bleedthrough();//PAGE_CODE_SCAN_SEP_RESCAN
	void erase_bleedthrough(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	char erase_bleedthrough_level();//PAGE_CODE_SCAN_SEP_RESCAN
	void erase_bleedthrough_level(char v);//PAGE_CODE_SCAN_SEP_RESCAN
	long intensity_of_bleed_through();//PAGE_CODE_SCAN_SEP_RESCAN
	void intensity_of_bleed_through(long v);//PAGE_CODE_SCAN_SEP_RESCAN
	bool edgeemphasis();//PAGE_CODE_SCAN_SEP_RESCAN
	void edgeemphasis(bool v);//PAGE_CODE_SCAN_SEP_RESCAN
	long intensity_of_edgeemphasis();//PAGE_CODE_SCAN_SEP_RESCAN
	void intensity_of_edgeemphasis(long v);//PAGE_CODE_SCAN_SEP_RESCAN
	void white_dot_erasure(bool v);//PAGE_CODE_SCAN_SEP_RESCAN
	bool white_dot_erasure();//PAGE_CODE_SCAN_SEP_RESCAN
	void black_dot_erasure(bool v);//PAGE_CODE_SCAN_SEP_RESCAN
	bool black_dot_erasure();//PAGE_CODE_SCAN_SEP_RESCAN
	void notch_erasure(bool v);//PAGE_CODE_SCAN_SEP_RESCAN
	bool notch_erasure();//PAGE_CODE_SCAN_SEP_RESCAN
	void moire_reduction(bool b);//PAGE_CODE_SCAN_SEP_RESCAN
	bool moire_reduction();//PAGE_CODE_SCAN_SEP_RESCAN
	void detect_blank_paper(bool v);//PAGE_CODE_SCAN_SEP_RESCAN sep resampling reading option
	bool detect_blank_paper();//PAGE_CODE_SCAN_SEP_RESCAN sep resampling reading option

	char autocolor();//PAGE_CODE_SCAN_SEP
	void autocolor(char v);//PAGE_CODE_SCAN_SEP
	char sensitivity_of_autocolor();//PAGE_CODE_SCAN_SEP
	void sensitivity_of_autocolor(char v);//PAGE_CODE_SCAN_SEP
	char intensity_of_autocolor();//PAGE_CODE_SCAN_SEP
	void intensity_of_autocolor(char v);//PAGE_CODE_SCAN_SEP
	char sensitivity_of_colorbinary();//PAGE_CODE_SCAN_SEP
	void sensitivity_of_colorbinary(char v);//PAGE_CODE_SCAN_SEP
	char intensity_of_colorbinary();//PAGE_CODE_SCAN_SEP
	void intensity_of_colorbinary(char v);//PAGE_CODE_SCAN_SEP
	bool noise_remove();//PAGE_CODE_SCAN_SEP
	void noise_remove(bool v);//PAGE_CODE_SCAN_SEP
	char noise_remove_level();//PAGE_CODE_SCAN_SEP
	void noise_remove_level(char v);//PAGE_CODE_SCAN_SEP
	bool background();//PAGE_CODE_SCAN_SEP
	void background(bool v);//PAGE_CODE_SCAN_SEP

	long disable_dfd_starty();//PAGE_CODE_DOUBLEFEED
	void disable_dfd_starty(long v);//PAGE_CODE_DOUBLEFEED
	long disable_dfd_endy();//PAGE_CODE_DOUBLEFEED
	void disable_dfd_endy(long v);//PAGE_CODE_DOUBLEFEED
	void indifferent_start_position_of_Y_axis(unsigned short v);//PAGE_CODE_DOUBLEFEED
	unsigned short indifferent_start_position_of_Y_axis();//PAGE_CODE_DOUBLEFEED
	void indifferent_end_position_of_Y_axis(unsigned short v);//PAGE_CODE_DOUBLEFEED
	unsigned short indifferent_end_position_of_Y_axis();//PAGE_CODE_DOUBLEFEED
	void indifferent_start_position_of_X_axis(unsigned short v);//PAGE_CODE_DOUBLEFEED
	unsigned short indifferent_start_position_of_X_axis();//PAGE_CODE_DOUBLEFEED
	void indifferent_end_position_of_X_axis(unsigned short v);//PAGE_CODE_DOUBLEFEED
	unsigned short indifferent_end_position_of_X_axis();//PAGE_CODE_DOUBLEFEED

	long ulx_of_paper();//PAGE_CODE_PAPER_SIZE
	void ulx_of_paper(long v);//PAGE_CODE_PAPER_SIZE
	long uly_of_paper();//PAGE_CODE_PAPER_SIZE
	void uly_of_paper(long v);//PAGE_CODE_PAPER_SIZE
	long width_of_paper();//PAGE_CODE_PAPER_SIZE
	void width_of_paper(long v);//PAGE_CODE_PAPER_SIZE
	long length_of_paper();//PAGE_CODE_PAPER_SIZE
	void length_of_paper(long v);//PAGE_CODE_PAPER_SIZE

	long left_margin();//PAGE_CODE_MARGIN
	void left_margin(long v);//PAGE_CODE_MARGIN
	long top_margin();//PAGE_CODE_MARGIN
	void top_margin(long v);//PAGE_CODE_MARGIN
	long right_margin();//PAGE_CODE_MARGIN
	void right_margin(long v);//PAGE_CODE_MARGIN
	long bottom_margin();//PAGE_CODE_MARGIN
	void bottom_margin(long v);//PAGE_CODE_MARGIN
		
	void year(short v);//PAGE_CODE_DATE
	void month(char v);//PAGE_CODE_DATE
	void day(char v);//PAGE_CODE_DATE
	void hour(char v);//PAGE_CODE_DATE
	void minutes(char v);//PAGE_CODE_DATE
	void second(char v);//PAGE_CODE_DATE
	short year();//PAGE_CODE_DATE
	char month();//PAGE_CODE_DATE
	char day();//PAGE_CODE_DATE
	char hour();//PAGE_CODE_DATE
	char minutes();//PAGE_CODE_DATE
	char second();	//PAGE_CODE_DATE

	void headid(char id);//PAGE_CODE_IMPRINTER_HEAD
	char headid();//PAGE_CODE_IMPRINTER_HEAD
	char print_timing();//PAGE_CODE_IMPRINTER_HEAD
	void print_timing(char v);//PAGE_CODE_IMPRINTER_HEAD

	void xdpi(long v);//PAGE_CODE_IMPRINTER_TEXT
	long xdpi();//PAGE_CODE_IMPRINTER_TEXT
	void ydpi(long v);//PAGE_CODE_IMPRINTER_TEXT
	long ydpi();//PAGE_CODE_IMPRINTER_TEXT
	void format(char v);//PAGE_CODE_IMPRINTER_TEXT
	char format();//PAGE_CODE_IMPRINTER_TEXT
	char *strings(char *work, long work_size);//PAGE_CODE_IMPRINTER_TEXT
	void strings(char *s);//PAGE_CODE_IMPRINTER_TEXT
	void fontsize(char v);//PAGE_CODE_IMPRINTER_TEXT
	char fontsize();//PAGE_CODE_IMPRINTER_TEXT
    void countup_timing1(char v);//PAGE_CODE_IMPRINTER_TEXT
    char countup_timing1();//PAGE_CODE_IMPRINTER_TEXT
    void countup_amount1(long v);//PAGE_CODE_IMPRINTER_TEXT
    long countup_amount1();//PAGE_CODE_IMPRINTER_TEXT
    void reset_timing1(char v);//PAGE_CODE_IMPRINTER_TEXT
    char reset_timing1();//PAGE_CODE_IMPRINTER_TEXT
    void reset_value1(long v);//PAGE_CODE_IMPRINTER_TEXT
    long reset_value1();//PAGE_CODE_IMPRINTER_TEXT
   	void reset_timing2(char v);//PAGE_CODE_IMPRINTER_TEXT
    char reset_timing2();//PAGE_CODE_IMPRINTER_TEXT
	bool imp_enable();//PAGE_CODE_IMPRINTER_TEXT and AGE_CODE_IMPRINTER_HEAD
	void imp_enable(bool b);//PAGE_CODE_IMPRINTER_TEXT and AGE_CODE_IMPRINTER_HEAD
private:
	void I_am_in(CCommand::EXEC_TYPE type);
private:
	unsigned char m_data[256];
};
class CAdjustCmd : public CCommand
{
public:
	CAdjustCmd(long length=40, long id=3);
	CAdjustCmd(CAdjustCmd& in);
	~CAdjustCmd();

	CAdjustCmd& operator = (CAdjustCmd& src);

    void parameter_list_length(long v);
    void transfer_identification(long v);

	void I_am_in(CCommand::EXEC_TYPE type);

	char gain1_f();
	char gain2_f();
	char gain3_f();
	char gain4_f();
	char offset1_f();
	char offset2_f();
	char offset3_f();
	char offset4_f();
	short red_led_f();
	short green_led_f();
	short blue_led_f();
	short led_f1();
	short led_f2();
	short led_f3();

	void gain1_f(char v);
	void gain2_f(char v);
	void gain3_f(char v);
	void gain4_f(char v);
	void offset1_f(char v);
	void offset2_f(char v);
	void offset3_f(char v);
	void offset4_f(char v);
	void red_led_f(short v);
	void green_led_f(short v);
	void blue_led_f(short v);
	void led_f1(short v);
	void led_f2(short v);
	void led_f3(short v);

	char gain1_b();
	char gain2_b();
	char gain3_b();
	char gain4_b();
	char offset1_b();
	char offset2_b();
	char offset3_b();
	char offset4_b();
	short red_led_b();
	short green_led_b();
	short blue_led_b();	
	short led_b1();
	short led_b2();
	short led_b3();

	void gain1_b(char v);
	void gain2_b(char v);
	void gain3_b(char v);
	void gain4_b(char v);
	void offset1_b(char v);
	void offset2_b(char v);
	void offset3_b(char v);
	void offset4_b(char v);
	void red_led_b(short v);
	void green_led_b(short v);
	void blue_led_b(short v);	
	void led_b1(short v);
	void led_b2(short v);
	void led_b3(short v);


	//for carol
	void offset1_f2(char v);
	void offset2_f2(char v);
	void offset3_f2(char v);
	void offset4_f2(char v);
	void offset5_f2(char v);
	void offset6_f2(char v);
	void red_led_f2(short v);
	void green_led_f2(short v);
	void blue_led_f2(short v);
	void current_led_f(short v);

	void offset1_b2(char v);
	void offset2_b2(char v);
	void offset3_b2(char v);
	void offset4_b2(char v);
	void offset5_b2(char v);
	void offset6_b2(char v);
	void red_led_b2(short v);
	void green_led_b2(short v);
	void blue_led_b2(short v);
	void current_led_b(short v);	
private:
	unsigned char m_data[96];
	long m_id;
};
class CGetScannerStatusCmd : public CCommand
{
public:
	CGetScannerStatusCmd();
	CGetScannerStatusCmd(CGetScannerStatusCmd& in);
	CGetScannerStatusCmd(char * cdb, long cdb_size, char * data, long data_size);
	~CGetScannerStatusCmd();
	bool error();
	void error(bool err);
	long bufferred_image_count();
	void bufferred_image_count(long v);
	CGetScannerStatusCmd& operator = (CGetScannerStatusCmd& src);
private:
	unsigned char m_data[8];
};
class CBufferCmd : public CCommand
{
public:
	CBufferCmd();
	CBufferCmd(CBufferCmd& in);
	CBufferCmd(char * data, long data_size);
	~CBufferCmd();
	
	CBufferCmd& operator = (CBufferCmd& src);
};
class CBufferCmd2 : public CCommand
{
public:
	CBufferCmd2(char * data, long data_size);
	CBufferCmd2(CBufferCmd2&in);
	~CBufferCmd2();

	bool end();
	void next();
	
	CBufferCmd2& operator = (CBufferCmd2& src);
private:
	char * m_pdata2;
	char * m_cur;
	long m_total_size;
	long m_offset;
};
class CServiceCmd : public CCommand
{
public:
	CServiceCmd(char mode, long submode, char *data, long size);
	~CServiceCmd();
	char *firm_name(char *wk);
	char *firm_version(char *wk);
};
class CShadingDataCmd : public CCommand
{
public:
	CShadingDataCmd();
	CShadingDataCmd(CShadingDataCmd& in);
	~CShadingDataCmd();

	typedef struct {
		short dpi;
		short mode;
		bool front;
		bool white;
	}KEYINFO;

	char *adjust_data(KEYINFO &key);
	void read(KEYINFO &key, char *ptr, long size);
private:
	std::vector<char> m_buffer;
private:
	char *search(KEYINFO &key);
private:
	void first();
	bool eof();
	char *next();
	char *m_cur;
};
class CErrorHistoryCmd : public CCommand
{
public:
	CErrorHistoryCmd();
	CErrorHistoryCmd(CErrorHistoryCmd& in);
	CErrorHistoryCmd(char *pdata/*size must be 192*/);
	~CErrorHistoryCmd();
	CErrorHistoryCmd& operator = (CErrorHistoryCmd& in);
private:	
	void I_am_in(CCommand::EXEC_TYPE type);
	unsigned char m_data[192];
};
class CCheckScanSize : public CCommand
{
public:
	CCheckScanSize();
	CCheckScanSize(CCheckScanSize& in);
	CCheckScanSize(char* cdb, long cdb_size, char* data, long data_size);
	~CCheckScanSize();
	CCheckScanSize& operator = (CCheckScanSize& in);
	bool duplex();
	void duplex(bool v);
	void main_windowid(unsigned char v);
	unsigned char main_windowid();	
private:
	void sub_windowid(unsigned char v);
	unsigned char sub_windowid();
private:
	unsigned char m_data[2];
};
class CErrorClear : public CCommand
{
public:
    CErrorClear();
	CErrorClear(CErrorClear &in);
    ~CErrorClear();
};
#endif //_CEI_CSDCORE2_COMMAND_CLASS_HEADER_
