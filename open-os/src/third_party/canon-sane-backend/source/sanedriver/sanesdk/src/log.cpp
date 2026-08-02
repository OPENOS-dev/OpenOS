/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include <memory.h>
#include <thread>
#include <vector>
#include <thread>
#include <mutex>
#include <string>

namespace {
	char g_backend[32]={0};
	std::mutex g_logmt;
	bool FileExists(char *path)
	{
		FILE* fp;
		bool out=false;
		fp = fopen(path, "r" );
		if( fp == NULL ){
			out = false;
		}
		else{
			out=true;
			fclose( fp );
		}
		return out;
	}
	bool IsLogMode()
	{
		static bool c_once=true;
		static bool c_logflag=false;
		if (c_once) {
			c_once=false;
			char path[128];
			sprintf(path, "/tmp/libsane-%s.log", g_backend);
			//printf("%s\r\n", path);
			if (FileExists(path)) {
				printf("log mode on:/tmp/libsane-%s.log\r\n", g_backend);
				c_logflag=true;
			}
		}
		return c_logflag;
	}
	int WriteLogToFile(char *str)
	{
		char path[64];
		sprintf(path, "/tmp/libsane-%s.log", g_backend);
	    FILE *fp = fopen(path, "a");
		if (fp==NULL) return 0;
		fseek(fp, 0, SEEK_END);
		time_t timer;
		time(&timer);
		char strtm[32];
		sprintf(strtm, "%s", ctime(&timer));
		strtm[strlen(strtm)-1]=0;
		fprintf(fp, "%s %s\r\n", strtm, str);
		fclose(fp);
		return 0;
	}
}
int SaneWriteLog(const char *fmt, ...)
{
	if (!IsLogMode()) return 0;
	std::lock_guard<std::mutex> lg(g_logmt);
	char *buf=new char[1024];
	if (buf==NULL) return 0;
	va_list app;
	va_start(app, fmt);
	vsprintf(buf, fmt, app);
	va_end(app);
	int out = WriteLogToFile(buf);
	delete []buf;
	return out;
}
bool IsSaneWriteLog()
{
	return IsLogMode();
}
void SaneWriteLog_init(const char *backend)
{
	//printf("SaneWriteLog_init(%s)\r\n", backend);
	strcpy(g_backend, backend);
}
void SaneWriteLog_uninit()
{
	//printf("SaneWriteLog_uninit()\r\n");
}