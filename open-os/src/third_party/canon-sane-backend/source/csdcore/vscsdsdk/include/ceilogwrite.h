/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CEI_LOG_WRITER_UTILITY_HEADER_INCLUDED__
#define __CEI_LOG_WRITER_UTILITY_HEADER_INCLUDED__

void WriteLog_init();
void WriteLog_uninit();
int WriteLog(const char *fmt, ...);
int SDKWriteLog(const char *fmt, ...);//this is for SDK. do not use
bool IsLogMode();
void WriteLog_setname(const char *name/*up to 32*/);


#endif
