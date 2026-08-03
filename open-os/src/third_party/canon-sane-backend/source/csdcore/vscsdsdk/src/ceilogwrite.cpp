/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <mutex>
#include <fcntl.h>      // O_CREAT
#include <sys/file.h>   // flock
#include "global_apis.h"

namespace {
	std::mutex g_ceilogwriter_mutex;
	char g_path[64] = {0};
	char g_logname[32] = {0};
	char g_sdklogname[32] = {0};
	struct {
#ifdef _WIN32
		long id = 0;
#else
		pthread_t id;
#endif
		char name[32 + 2];
	} g_th_names[4];
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
#ifdef _WIN32
	char *th2name(long id)
#else
	char *th2name(pthread_t id)
#endif
	{
		if (id==0) return NULL;
		for (unsigned long i=0; i<sizeof(g_th_names)/sizeof(g_th_names[0]); i++) {
			if (g_th_names[i].id==id) {
				return g_th_names[i].name;
			}
		}
		//printf("th2name(%ld) return NULL\r\n", id);
		return NULL;
	}
	int WriteLogToFile(char *str)
	{
	#ifdef _WIN32
		long id = 0;
	#else
		pthread_t id = pthread_self();
	#endif
		int out = 0;
		char path[256];
		sprintf(path, "%s%s.log", g_path, g_logname);
	    FILE *fp = fopen(path, "a");
		if (fp==NULL) {
			printf("fopen(%s) error %s\r\n", path, strerror(errno));
			return 0;
		}
		//fseek(fp, 0, SEEK_END);
		time_t timer;
		time(&timer);
		char strtm[32];
		sprintf(strtm, "%s", ctime(&timer));
		strtm[strlen(strtm)-1]=0;
		char *pname = th2name(id);
		if (pname) {
			fprintf(fp, "%s [%s]%s\r\n", strtm, pname, str);
		} else {
			fprintf(fp, "%s [0x%lx]%s\r\n", strtm, (unsigned long)id, str);
		}
		fclose(fp);
		return out;
	}
	int SDKWriteLogToFile(char *str)
	{
	#ifdef _WIN32
		long id = 0;
	#else
		pthread_t id = pthread_self();
	#endif
		int out = 0;
		char path[256];
		sprintf(path, "%s%s.log", g_path, g_sdklogname);
	    FILE *fp = fopen(path, "a");
		if (fp==NULL) {
			printf("fopen(%s) error %s\r\n", path, strerror(errno));
			return 0;
		}
		//fseek(fp, 0, SEEK_END);
		time_t timer;
		time(&timer);
		char strtm[32];
		sprintf(strtm, "%s", ctime(&timer));
		strtm[strlen(strtm)-1]=0;
		fprintf(fp, "%s [0x%lx]%s\r\n", strtm, (unsigned long)id, str);
		fclose(fp);
		return out;
	}
}
bool IsLogMode()
{
	return g_logname[0]>0;
}
void WriteLog_init()
{
	//printf(" WriteLog_init()\r\n");
	for (unsigned long i=0; i < sizeof(g_th_names)/sizeof(g_th_names[0]); i++) {
		g_th_names[i].id = 0;
		g_th_names[i].name[0] = 0;
	}
	char *s = ceisdk_get_scanner_name();
	if (s[0]) {
		char path[64];
		char name[32];
		sprintf(name, "%s_%s", ceisdk_get_module_name(), s);
		sprintf(path, "/tmp/%s.log", name);
		//printf("path:%s\r\n", path);
		if (FileExists(path)) {
			strcpy(g_path, "/tmp/");
			strcpy(g_logname, name);
			strcpy(g_sdklogname, name);
			strcat(g_sdklogname, ".sdk");
			printf("log mode on:%s\r\n", path);
		} else {
			sprintf(path, "/usr/local/etc/%s.log", name);
			//printf("path:%s\r\n", path);
			if (FileExists(path)) {
				strcpy(g_path, "/usr/local/etc/");
				strcpy(g_logname, name);
				strcpy(g_sdklogname, name);
				strcat(g_sdklogname, ".sdk");
				printf("log mode on:%s\r\n", path);
			}
		}
	} else {
		//printf("s[0] is NULL\r\n");
	}
}
void WriteLog_uninit()
{
}
void WriteLog_setname(const char *name)
{
	if (!IsLogMode()) return;
#ifdef _WIN32
	long id = 0;
#else
	pthread_t id = pthread_self();
#endif
	if (name) {
		for (unsigned long i=0; i<sizeof(g_th_names)/sizeof(g_th_names[0]); i++) {
			if (g_th_names[i].id==0) {
				g_th_names[i].id = id;
				strcpy(g_th_names[i].name, name);
				//printf("WriteLog_setname(%s) succeeded\r\n", name);
				//printf("id is %ld\r\n", g_th_names[i].id);
				//printf("name is %s\r\n", g_th_names[i].name);
				break;
			}
		}
	} else {
		for (unsigned long i=0; i<sizeof(g_th_names)/sizeof(g_th_names[0]); i++) {
			if (g_th_names[i].id && g_th_names[i].id == id) {
				g_th_names[i].id=0;
				g_th_names[i].name[0]=0;
				break;
			}
		}		
	}
}
int WriteLog(const char *fmt, ...)
{
	if (!IsLogMode()) return 0;
	std::lock_guard<std::mutex> lg(g_ceilogwriter_mutex);

	char *buf=new char[1024*2];
	if (buf==NULL) return 0;
	va_list app;
	va_start(app, fmt);
	vsprintf(buf, fmt, app);
	va_end(app);
	WriteLogToFile(buf);
	delete [] buf;
	return 0;
}
int SDKWriteLog(const char *fmt, ...)
{
	if (!IsLogMode()) return 0;
	std::lock_guard<std::mutex> lg(g_ceilogwriter_mutex);

	char *buf=new char[1024*2];
	if (buf==NULL) return 0;
	va_list app;
	va_start(app, fmt);
	vsprintf(buf, fmt, app);
	va_end(app);
	SDKWriteLogToFile(buf);
	delete [] buf;
	return 0;	
}
