/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __GLOBAL_FUNCIONS_DEFINE_HEADER__
#define __GLOBAL_FUNCIONS_DEFINE_HEADER__

void ceisdk_set_module_name(char *s);//ex: vs or csdcore
char *ceisdk_get_module_name();
void  ceisdk_set_scanner_name(char *s);//ex: s is DR-M260, 660h, ....
char *ceisdk_get_scanner_name();//drm260, 660h....
char* ceisdk_get_scanner_name_full();//DR-M260
char *ceisdk_get_temp_folderpath(char *s);
char *ceisdk_get_library_path(char *s);
void  ceisdk_set_library_path(char *s);
char *ceisdk_get_ceiip_path(char *s);
char *ceisdk_get_ini_path(char *s, size_t size);
char *ceisdk_get_srgb_binary_path(char *s);
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def="");
int ceisdk_get_private_profile_int(const char *section_name, const char *key, int ndefault=0);
double ceisdk_get_private_profile_double(const char *section_name, const char *key, double ndefault=0.0);
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def, const char *path);
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string, const char *path);
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string);
bool ceisdk_is_scanner_background_white();
bool ceisdk_is_scanner_frontpage_shadow_top();

void *ceisdk_load_module(char * path);
void ceisdk_print_loadmodule_error();
void *ceisdk_get_proc_address(void * hd, const char *func_name);
void ceisdk_close_module(void * hd);

#endif