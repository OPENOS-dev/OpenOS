/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef _SANEWRAPPER_FUNCTIONS_LINUX_H_INCLUDED_
#define _SANEWRAPPER_FUNCTIONS_LINUX_H_INCLUDED_
void SaneWriteLog_init(const char *backend);
int SaneWriteLog(const char *fmt, ...);
void SaneWriteLog_uninit();
bool IsSaneWriteLog();
#endif	/* _SANEWRAPPER_FUNCTIONS_LINUX_H_INCLUDED_ */
