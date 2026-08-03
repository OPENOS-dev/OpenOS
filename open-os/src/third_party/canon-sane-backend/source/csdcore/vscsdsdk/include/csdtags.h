/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD_SDK_CSDTAGS_DEFINED_HEADER__
#define __CSD_SDK_CSDTAGS_DEFINED_HEADER__

#ifndef CSDP_WIDTH

#define CSDP_WIDTH				   1
#define CSDP_PAGEWIDTH			   CSDP_WIDTH/*do not use this tag. use CSDP_IMAGEWIDTH instead.*/
#define CSDP_LENGTH				   2
#define CSDP_PAGELENGTH			   CSDP_LENGTH/*do not use this tag. use CSDP_IMAGELENGTH instead.*/
#define CSDP_FEEDER				   3
#define CSDP_BRIGHTNESS			   5
#define CSDP_XRESOLUTION			   6
#define CSDP_YRESOLUTION           7
#define CSDP_COMPRESSION           8
#define CSDP_AUTOSIZE             12
#define CSDP_READAHEAD            17
#define CSDP_FILETYPE             19
#define CSDP_BITPERPIXEL          20
#define CSDP_BITSPERSAMPLE		  CSDP_BITPERPIXEL
#define CSDP_CONTRAST             21
#define CSDP_DBLFEEDLENGTH        30
#define CSDP_XOFFSET              40
#define CSDP_YOFFSET              41
#define CSDP_JPEGQUALITY	         109
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
#define CSDP_PRESCAN					309
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
#endif
#endif
