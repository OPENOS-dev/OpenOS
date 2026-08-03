/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#include "CeiLoadlib.h"
#else
#include <dirent.h>
#include <dlfcn.h>
#include "unknown.h"
#endif
#include "global_apis.h"

namespace {
	char g_modulename[8]={0};
	char g_scannername[16]={0};
	char g_scannername_full[sizeof(g_scannername)] = { 0 };
	char g_lib[256] = { 0 };
	int g_isScannerBackGroundWhite = -1;
	int g_isScannerFrontShadowTop = -1;
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
	char *rm_ch(char *s, char c)
	{
		char *dst = s;
		char *src = s;
		while (*src) {
			if (*src == c) {
				src++;
			}
			else {
				*dst = *src;
				dst++;
				src++;
			}
		}
		*dst = 0;
		return s;
	}
	char *tolower_str(char *s)
	{
		char *out = s;
		while (*s) {
			*s = ::tolower(*s);
			s++;
		}
		return out;
	}
	char* toupper_str(char* s)
	{
		char* out = s;
		while (*s) {
			*s = ::toupper(*s);
			s++;
		}
		return out;
	}
#if 0
	bool DirectoryExists( const char* pzPath )
	{
#ifdef _WIN32
		return false;
#else
		bool bExists = false;
	    if ( pzPath == NULL) return false;
	    DIR *pDir;
	    pDir = opendir (pzPath);
	    if (pDir != NULL)
	    {
	        bExists = true;    
	        (void) closedir (pDir);
	    }
	    return bExists;
#endif
	}
#endif
}
void ceisdk_set_module_name(char *s)
{
	if (g_modulename[0]) return;
	strcpy(g_modulename, s);
}
char *ceisdk_get_module_name()
{
	return g_modulename;
}
void ceisdk_set_scanner_name(char *s)//ex: s is drm260, 660h, ....
{
	if (g_scannername[0]) return;
	strcpy(g_scannername, s);
	strcpy(g_scannername_full, s);
	tolower_str(g_scannername);
	rm_ch(g_scannername, '-');
	toupper_str(g_scannername_full);
}
char *ceisdk_get_scanner_name()
{
	return g_scannername;
}
char* ceisdk_get_scanner_name_full()
{
	return g_scannername_full;
}
char *ceisdk_get_temp_folderpath(char *s)
{
#ifdef _WIN32
	GetTempPath(MAX_PATH, s);
#else
	strcpy(s, "/tmp/");
#endif
	return s;
}
char *ceisdk_get_library_path(char *s)
{
	//printf("ceisdk_get_library_path() start\r\n");
	if (g_lib[0]) {
		strcpy(s, g_lib);
	}
	else {
		char *sn = ceisdk_get_scanner_name();
#ifdef _WIN32
		if (sn[0]) {
			GetTempPath(MAX_PATH, s);
			strcat(s, sn);
			strcat(s, "\\");
		}
		else {
			GetTempPath(MAX_PATH, s);
		}
#else
        if (sn[0]) {
			strcpy(s, "/tmp/");
            strcat(s, sn);
            strcat(s, "/");
        } else {
            strcpy(s, "/tmp/");
        }
#endif
	}
	//printf("ceisdk_get_library_path() end %s\r\n", s);
	return s;
}
void  ceisdk_set_library_path(char *s)
{
	if (s && s[0]) {
		strcpy(g_lib, s);
		char *end = g_lib + strlen(g_lib);
		end--;
		if (*end == '\\' || *end == '/') {

		}
		else {
			end++;
			*end = '/';
		}
	}
}
char *ceisdk_get_ceiip_path(char *s)
{
	ceisdk_get_library_path(s);
#ifdef _WIN32
	strcat(s, "ceiip.dll");
#elif __APPLE__
    strcat(s, "ceiip.framework/ceiip");
#else
	strcat(s, "libceiip.so");
#endif
	return s;
}
char *ceisdk_get_ini_path(char *s, size_t size)
{
	snprintf(s, size, "%s/", "/run/imageloader/sane-backends-canon/package/root/canonlibs");
	char *n = ceisdk_get_scanner_name();
	if (n[0]) {
#ifdef __APPLE__
        strcat(s, n);
        strcat(s, ".ini");
        if (!FileExists(s)) {
            char *p = strrchr(s, '/');
            if (*p) *p = 0;
            strcat(s, "/../Resources/");
            strcat(s, n);
            strcat(s, ".ini");
        }
#else
		strncat(s, n, size - strlen(s));
		strncat(s, ".ini", size - strlen(s));
#endif
	}
	else {
		strncat(s, "profiles.ini", size - strlen(s));
	}
	static bool c_once = true;
	if (c_once) {
		if (!FileExists(s)) {
			printf("%s is not found.\r\n", s);
		}
	}
	c_once=false;	
	return s;
}

#ifdef __ANDROID__
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def, const char *path)
{
	long out = 0;
	typedef long (*lpfnandroid_get_private_profile_string)(const char* section_name, const char* key, char* returned_string, long size_of_returned_string, const char* def, const char* path);
	void *hm = dlopen("libceiandroid_assets.so", RTLD_LAZY);
	if (hm) {
		lpfnandroid_get_private_profile_string lpfn = (lpfnandroid_get_private_profile_string)dlsym(hm, "android_get_private_profile_string");
		if (lpfn) {
			out = lpfn(section_name, key, returned_string, size_of_returned_string, def, path);
		}
		dlclose(hm);
	}
	return out;
}
#else
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def, const char *path)
{
	size_of_returned_string -= 1;
	if (size_of_returned_string <= 0) {
		strcpy(returned_string, def);
		return (long)strlen(returned_string);
	}
	char sec[32];
	sprintf(sec, "[%s]", section_name);
	char k[128];
	sprintf(k, "%s=", key);
	std::ifstream ifs(path);
	std::string line;
	bool bloop = true;
	bool bNotFound = true;
	while (bloop && std::getline(ifs, line)) {
		if (line == sec)
		{
			while (std::getline(ifs, line)) {
				const char *p2 = strstr(line.c_str(), k);
				if (p2) {
					if (strlen(line.c_str() + strlen(k)) < (size_t)size_of_returned_string) {
						size_of_returned_string = (long)strlen(line.c_str() + strlen(k));
					}
					strncpy(returned_string, line.c_str() + strlen(k), size_of_returned_string);
					bloop = false;
					bNotFound = false;
					break;
				}
			}
		}
	}
	if (bNotFound) {
		strcpy(returned_string, def);
	}
	return (long)strlen(returned_string);
}
#endif
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string, const char *path)
{
	return -1;
}
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def)
{
	char path[256] = { 0 };
	return ceisdk_get_private_profile_string(section_name, key, returned_string, size_of_returned_string, def, ceisdk_get_ini_path(path, sizeof(path)));
}
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string)
{
	char path[256] = { 0 };
	return ceisdk_write_private_profile_string(section_name, key, in_string, ceisdk_get_ini_path(path, sizeof(path)));
}
int ceisdk_get_private_profile_int(const char *section_name, const char *key, int ndefault)
{
	char buf[128] = { 0 };
	char def_str[16] = { 0 };
	sprintf(def_str, "%d", ndefault);
	ceisdk_get_private_profile_string(section_name, key, buf, 128, def_str);

	return (int)(atoi(buf));
}
double ceisdk_get_private_profile_double(const char *section_name, const char *key, double ndefault)
{
	char buf[128] = { 0 };
	char def_str[16] = { 0 };
	sprintf(def_str, "%f", ndefault);
	ceisdk_get_private_profile_string(section_name, key, buf, 128, def_str);
	return (double)(atof(buf));
}
void *ceisdk_load_module(char * s)
{
#ifdef _WIN32
	HMODULE hm = LoadLibrary(ceisdk_get_ceiip_path(s));
	return (void *)hm;
#else
	return dlopen(ceisdk_get_ceiip_path(s), RTLD_LAZY);
#endif	
}
void ceisdk_print_loadmodule_error()
{
#ifdef _WIN32
	printf("LoadLibrary() error %d\r\n", GetLastError());
#else
	printf("dlopen() error %s\r\n", dlerror());
#endif
}
void * ceisdk_get_proc_address(void * hd, const char * apiname)
{
#ifdef _WIN32
	HMODULE hm = (HMODULE)hd;
	return GetProcAddress(hm, apiname);
#else
	return dlsym(hd, apiname);
#endif	
}
void ceisdk_close_module(void * hd)
{
	if (hd) {
#ifdef _WIN32
		FreeLibrary((HMODULE)hd);
#else
		dlclose(hd);
#endif
	}
}

char *ceisdk_get_srgb_binary_path(char *s)
{
	ceisdk_get_library_path(s);

	char buf[32] = {0};
	char *scanner = ceisdk_get_scanner_name();
	if (scanner && scanner[0]) {
		/* Scanner名がある場合はそれ用のモノを使う */
		sprintf(buf, "%s_srgb.dat", scanner);
	}
	else {
		strcpy(buf, "srgb.dat");
	}
	strcat(s, buf);
#ifdef __APPLE__
    if (!FileExists(s)) {
        char *p = strrchr(s, '/');
        if (*p) *p = 0;
        strcat(s, "/../Resources/");
        strcat(s, buf);
    }
#endif
	return s;
}
bool ceisdk_is_scanner_background_white()
{
	if (g_isScannerBackGroundWhite < 0) {
		/* 負値なら初期化する */
		g_isScannerBackGroundWhite = ceisdk_get_private_profile_int("SCANNER", "BackGroundIsWhite", 1);
	}
	return (g_isScannerBackGroundWhite != 0) ? true : false;
}

bool ceisdk_is_scanner_frontpage_shadow_top()
{
	if (g_isScannerFrontShadowTop < 0) {
		/* 負値なら初期化する */
		g_isScannerFrontShadowTop = ceisdk_get_private_profile_int("SCANNER", "FrontShadowIsTop", 1);
	}
	return (g_isScannerFrontShadowTop != 0) ? true : false;
}
#ifdef _WIN32
REFIID IID_IVirtualScanner2 = { 0x7a047d06, 0x1476, 0x4e13, { 0xac, 0xe6, 0x1c, 0xb1, 0x8f, 0x54, 0xbb, 0x91 } };
#else
REFIID IID_IVirtualScanner2 = 100001;
#endif
#ifdef _WIN32
REFIID IID_IInternalVirtualScanner = { 0xeef3118c, 0xd16d, 0x48d4, { 0xa8, 0xee, 0xa0, 0xe, 0xef, 0xf4, 0x9e, 0x8b } };
#else
REFIID IID_IInternalVirtualScanner = 100002;
#endif
