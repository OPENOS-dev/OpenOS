/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CSD3_ERROR_DEFINE_HEADER_DEFINED__
#define __CSD3_ERROR_DEFINE_HEADER_DEFINED__

#ifndef CSD3_OK
#define CSD3_OK                     0
#define CSD3_NOPAGE                1
#define CSD3_NODEVICE              2
#define CSD3_BADPARMNO             3
#define CSD3_BADFILE               4
#define CSD3_BADPARM               5
#define CSD3_NOPAPER               6
#define CSD3_JAM                    7
#define CSD3_COVEROPEN             8
#define CSD3_POWERON               9
#define CSD3_BADFILE0             10
#define CSD3_BADFILE1             11
#define CSD3_COUNTONLY            12
#define CSD3_COUNTMISS            13
#define CSD3_ABORTED              14
#define CSD3_RESFAIL              15
#define CSD3_NOTREADY             16
#define CSD3_HARDERROR            17
#define CSD3_NOTSELECTED          18
#define CSD3_NEWFILE              19
#define CSD3_DOUBLEFEED           20
#define CSD3_SKEWFEED             21
#define CSD3_FILMEND				 22
#define CSD3_NOCAMERA			 	 23
#define CSD3_BADLOGFILE			 24
#define CSD3_FILMERROR			 25
#define CSD3_NOMEM				 26
#define CSD3_UNKNOWN				 27
#define CSD3_ENDOFPAGE			 28
#define CSD3_CANCEL				 29
#define CSD3_NOCARTRIDGE			 30
#define CSD3_COUNTMISSTOOMANY	 31
#define CSD3_COUNTMISSTOOFEW		 32
#define CSD3_STAPLEDETECTED		 33
#define CSD3_DELIVERYFULL		 34
#define CSD3_DETECTED_BATCHSEP    35
#define CSD3_COMM				 	 36
#define CSD3_SCANNER_NOMEM		 37
#define CSD3_FEEDERRORDETECTED	 38
#define CSD3_CONNECT_ERROR_WIFI	 39
#define CSD3_CONNECT_ERROR_USB	 40
#define CSD3_SCANNER_LOCKED		 41

#define CSD3_SOFTWARE				1000
#define CSD3_NOTFINDMODULE		1001
#define CSD3_DRIVERBUSY			1002
#define CSD3_SEQUENCEERR			1003

#define CSD3_BADPATH 			    2000
#define CSD3_BADACCESS 			2001
#define CSD3_DISKFULL 			2002

#endif
#endif