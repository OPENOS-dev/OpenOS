/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _CSD_SCAN_H_INCLUDED
#define _CSD_SCAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CCSIZEOF_STRUCT
#define CCSIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

/* //////////////////////////////////////////////////////////
//
// CSD Status
//
////////////////////////////////////////////////////////// */
/* {{ CSD_ERROR_CODE */
#define CSD_OK                    0
#define CSD_NOPAGE                1
#define CSD_NODEVICE              2
#define CSD_BADPARMNO             3
#define CSD_BADFILE               4
#define CSD_BADPARM               5
#define CSD_NOPAPER               6
#define CSD_JAM                   7
#define CSD_COVEROPEN             8
#define CSD_POWERON               9
#define CSD_BADFILE0             10
#define CSD_BADFILE1             11
#define CSD_COUNTONLY            12
#define CSD_COUNTMISS            13
#define CSD_ABORTED              14
#define CSD_RESFAIL              15
#define CSD_NOTREADY             16
#define CSD_HARDERROR            17
#define CSD_NOTSELECTED          18
#define CSD_NEWFILE              19
#define CSD_DOUBLEFEED           20
#define CSD_SKEWFEED             21
#define CSD_FILMEND        22
#define CSD_NOCAMERA       23
#define CSD_BADLOGFILE       24
#define CSD_FILMERROR      25
#define CSD_NOMEM        26
#define CSD_UNKNOWN        27
#define CSD_ENDOFPAGE      28
#define CSD_CANCEL         29
#define CSD_NOCARTRIDGE      30
#define CSD_COUNTMISSTOOMANY   31
#define CSD_COUNTMISSTOOFEW    32
#define CSD_STAPLEDETECTED     33
#define CSD_DELIVERYFULL     34
#define CSD_DETECTED_BATCHSEP    35
#define CSD_COMM         36
#define CSD_SCANNER_NOMEM    37
#define CSD_FEEDERRORDETECTED  38
#define CSD_CONNECT_ERROR_WIFI   39
#define CSD_CONNECT_ERROR_USB  40

#define CSD_SOFTWARE      1000
#define CSD_NOTFINDMODULE   1001
#define CSD_DRIVERBUSY      1002
#define CSD_SEQUENCEERR     1003

#ifndef CSD_BADPATH
#define CSD_BADPATH 2000
#endif
#ifndef CSD_BADACCESS
#define CSD_BADACCESS 2001
#endif
#ifndef CSD_DISKFULL
#define CSD_DISKFULL 2002
#endif  
/* CSD_ERROR_CODE }} */



/* //////////////////////////////////////////////////////////
//
// Parameter type
//
////////////////////////////////////////////////////////// */
#define CSDTAG_ASCII        2
#define CSDTAG_LONG         4
#define CSDTAG_RATIONAL     5



/* //////////////////////////////////////////////////////////
//
// Parameter choices flags
//
////////////////////////////////////////////////////////// */
#define CSDCHOICEFLAG_RANGE     0x00020000L
#define CSDCHOICEFLAG_LIST      0x00040000L
#define CSDCHOICEFLAG_ANY   0x00080000L
#define CSDCHOICEFLAG_SYMRANGE  0x00100000L



/* //////////////////////////////////////////////////////////
//
// Parameter choices index
//
////////////////////////////////////////////////////////// */
#define CSDCHOICE_LOW   0xffffffff
#define CSDCHOICE_HIGH  0xfffffffe
#define CSDCHOICE_STEP  0xfffffffd



/* //////////////////////////////////////////////////////////
//
// CsdParSet/Get parameters
//
////////////////////////////////////////////////////////// */
/* {{ CSD_PARAMETER */
#define CSDP_FEEDER				   3
#define      CSD_FEEDER_SIMPLEX            0
#define      CSD_FEEDER_DUPLEX             1
#define CSDP_BRIGHTNESS			   5
#define CSDP_XRESOLUTION			   6
#define CSDP_YRESOLUTION           7
#define CSDP_AUTOSIZE             12
#define CSDP_READAHEAD            17
#define CSDP_BITPERPIXEL          20
#define CSDP_BITSPERSAMPLE		  CSDP_BITPERPIXEL
#define CSDP_CONTRAST             21
#define CSDP_DBLFEEDLENGTH        30
#define CSDP_XOFFSET              40
#define CSDP_YOFFSET              41
#define CSDP_SAMPLESPERPIXEL	169
#define CSDP_DBLFEEDUSS			174
#define CSDP_IMAGEWIDTH			180
#define CSDP_IMAGELENGTH		181
#define CSDP_WINDOW				183
#define CSDP_WINDOWCOUNT_FRONT	184
#define CSDP_WINDOWCOUNT_BACK	 185
#define CSDP_FEEDER_LOADED		198
#define CSDP_DESKEW					257
#define CSDP_SERIAL_NUMBER			290
#define CSDP_SKIPBLANKPAGE          301
#define CSDP_TOTALPAGECOUNT				310
#define CSDP_FOLIO						321
#define CSDP_PAGESIZE					323
#define CSDP_FIRMVERSION				324
#define CSDP_DBLFEEDUSS_LENGTH			338
#define CSDP_ROLLER_COUNTER				494
#define CSDP_LASTPAGE_SIDE				353
#define CSDP_DETECT_BLANKPAGE				507
#define CSDP_BLANKPAGE_DETECTED				508
#define CSDP_SCANNER_BUTTON			   569
#define    CSD_SCANNER_BUTTON_START	      1
#define    CSD_SCANNER_BUTTON_STOP        2
#define    CSD_SCANNER_BUTTON_DFD         4
#define    CSD_SCANNER_BUTTON_NON_SEP     8
#define CSDP_MAX_AHEAD_PAGES		   570
#define CSDP_IS_SCAN_DONE			   571
#define CSDP_FEEDER_OPTION			   575
#define CSDP_SPLITIMAGE				   602
#define CSDP_IMAGEWIDTH1200DPI		   604
#define CSDP_IMAGELENGTH1200DPI		   605
#define CSDP_XOFFSET1200DPI			   606
#define CSDP_YOFFSET1200DPI			   607
#define CSDP_PAGESIZE_WIDTH1200DPI                632
#define CSDP_PAGESIZE_LENGTH1200DPI               633
#define CSDP_PRESCAN_OPTION                       646
#define CSDP_MAXPAPERLENGTH1200DPI                648
#define CSDP_MAX_ROLLER_COUNTER                   650

/* //////////////////////////////////////////////////////////
//
// API Functions
//
////////////////////////////////////////////////////////// */
#ifndef _CEIIMGINFO_H
#define _CEIIMGINFO_H

typedef struct tagCEIIMAGEINFO {
  size_t  cbSize;       /* size of CEIIMAGEINFO */
  BYTE  *lpImage;     /* ptr of Image buffer */
  long  lXpos;        /* start dot of image */
  long  lYpos;        /* start line of image */
  long  lWidth;       /* width of image (dot) */
  long  lHeight;      /* heigth of image (line) */
  long  lSync;        /* line bytes */
  size_t  tImageSize;     /* buffer size */
  long  lBps;       /* bits per sample */
  long  lSpp;       /* samples per pixel */
  DWORD dwRGBOrder;     /*  */
  long  lXResolution;   /* resolution x */
  long  lYResolution;   /* resolution y */
} CEIIMAGEINFO, *LPCEIIMAGEINFO;

#define CEIIMAGEINFO_V1_SIZE  CCSIZEOF_STRUCT(CEIIMAGEINFO, lYResolution)

typedef struct tagCEIIMAGEINFO2 {
  size_t  cbSize;       /* size of CEIIMAGEINFO2 */
  BYTE  *lpImage;     /* ptr of Image buffer */
  long  lXpos;        /* start dot of image */
  long  lYpos;        /* start line of image */
  long  lWidth;       /* width of image (dot) */
  long  lHeight;      /* heigth of image (line) */
  long  lSync;        /* line bytes */
  size_t  tImageSize;     /* buffer size */
  long  lBps;       /* bits per sample */
  long  lSpp;       /* samples per pixel */
  DWORD dwRGBOrder;     /*  */
  long  lXResolution;   /* resolution x */
  long  lYResolution;   /* resolution y */
  /* V2 */
  long  nFileType;      /* File Format */
  long  nCompType;      /* Compression Type �@�@COMPTYPE_XXX */
  long  nJpegQuality;   /* Quality of JPEG compression */
  DWORD dwReserved;     /* Reserved ( must be 0 ) */
} CEIIMAGEINFO2, *LPCEIIMAGEINFO2;

#define CEIIMAGEINFO_V2_SIZE  CCSIZEOF_STRUCT(CEIIMAGEINFO2, nJpegQuality)

#ifndef RGB_ORDER
#define RGB_ORDER
  #define PIXEL_ORDER     0
  #define LINE_ORDER      1
  #define PSEUDOCOLOR     2
#endif  /* RGB_ORDER */

#define COMPTYPE_NONE 0x00
#define COMPTYPE_JPEG 0x80
#define COMPTYPE_MH   0x01
#define COMPTYPE_MR   0x02
#define COMPTYPE_MMR  0x03

#endif  /* _CEIIMGINFO_H */



#ifndef _INIT_INFORMTAION_DEFINED_
#define _INIT_INFORMTAION_DEFINED_
typedef struct tagINIT_INFORMATION {
  DWORD dwSize;
  LPCTSTR szProductName;
  LPCTSTR szLocalizeFilePath;
    LPCTSTR szLibraryFilePath;
}INIT_INFORMATION, *LPINIT_INFORMATION;
#endif


#ifndef _ICEISTI_DEFINED_
#define _ICEISTI_DEFINED_
class ICeiSti
{
public:
  virtual INT32 STDMETHODCALLTYPE InitSTI(TCHAR *pConnectionName)=0;
  virtual void  STDMETHODCALLTYPE UninitSTI()=0;
  virtual INT32 STDMETHODCALLTYPE Lock(DWORD timeout)=0;
  virtual void  STDMETHODCALLTYPE Unlock()=0;
  virtual INT32 STDMETHODCALLTYPE  SendCustom(void *pCmdBuf, int nSize, int Direction, void *data, int Length)=0;
  virtual INT32 STDMETHODCALLTYPE  ExecWrite(void *lpCDB, void *lpData, unsigned long dwDataLen)=0;
  virtual INT32 STDMETHODCALLTYPE  ExecRead(void *lpCDB, void *lpData, unsigned long dwDataLen)=0;
  virtual INT32 STDMETHODCALLTYPE  ExecNone(void *lpCDB, unsigned int wCdbLen)=0;
  virtual INT32 STDMETHODCALLTYPE  GetSenseData(BYTE *lpSenseData, unsigned long dwDataLen)=0;
  virtual DWORD STDMETHODCALLTYPE  GetInformationByte()=0;
};
#endif
#ifndef _PROBE_INFORMATION_DEFINED_
#define _PROBE_INFORMATION_DEFINED_
typedef struct tagPROBE_INFORMATION {
  DWORD dwSize;
  LPCTSTR szProductName;
  ICeiSti *pSti;
  bool    SimulationMode; 
}PROBE_INFORMATION, *LPPROBE_INFORMATION;
#define PROBE_INFORMATION_V1_SIZE CCSIZEOF_STRUCT(PROBE_INFORMATION, SimulationMode)
#endif 

/* //////////////////////////////////////////////////////////
//
// API Functions
//
////////////////////////////////////////////////////////// */
CSD_API
INT32 WINAPI CsdInit(LPINIT_INFORMATION pInfo);

CSD_API
INT32 WINAPI CsdProbeEx(LPPROBE_INFORMATION pInfo);

CSD_API
INT32 WINAPI CsdTerminate();

CSD_API
INT32 WINAPI CsdUninit();

CSD_API
INT32 WINAPI CsdParGetA(UINT uiParam, LPVOID lpParam);

CSD_API
INT32 WINAPI CsdParSetA(UINT uiParam, LPARAM lParam);

CSD_API
INT32 WINAPI CsdParGetType(UINT uiParNo, UINT32 *lpType);

CSD_API
INT32 WINAPI CsdParGetChoiceFlags(UINT uiParNo, UINT32 *lpFlags);

CSD_API
INT32 WINAPI CsdParGetChoiceCount(UINT uiParNo, UINT32 *lpCount);

CSD_API
INT32 WINAPI CsdParGetChoiceA(UINT uiParNo, INT32 iIndex, LPVOID lpVoid);

CSD_API
INT32 WINAPI CsdParGetChoiceLenA(UINT uiParNo, INT32 iIndex, UINT32 *lpLength);

CSD_API
INT32 WINAPI CsdStartScanA( LPCSTR lpFileName, LPVOID lpReserved1, LPVOID lpReserved2 );

CSD_API
INT32 WINAPI CsdStartPrescanA( LPCSTR lpFileName, LPVOID lpReserved1, LPVOID lpReserved2 );

CSD_API
INT32 WINAPI CsdReadPage( LPCEIIMAGEINFO lpImage );

CSD_API
INT32 WINAPI CsdReleaseImage( LPCEIIMAGEINFO lpInfo );

CSD_API
INT32 WINAPI CsdFlashScannedImage();

CSD_API
INT32 WINAPI CsdStopScan();

CSD_API
INT32 WINAPI CsdAbortScan();


#ifdef __cplusplus
}
#endif

#endif  /* _CSD_SCAN_H_INCLUDED */
