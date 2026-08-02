/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SANE_SANE_GLOBAL_APIS_INTERFACE_HEADER_DEFINED__
#define __SANE_SANE_GLOBAL_APIS_INTERFACE_HEADER_DEFINED__

namespace sanesdk {
void ceisdk_set_module_name(char *s);//ex: vs or csdcore
char *ceisdk_get_module_name();
void  ceisdk_set_scanner_name(char *s);//ex: s is drm260 or DR-M260, 660h, ....
char *ceisdk_get_scanner_name();
char *ceisdk_get_temp_folderpath(char *s);
char *ceisdk_get_library_path(char *s);
void  ceisdk_set_library_path(char *s);
char *ceisdk_get_ini_path(char *s, size_t size);
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def="");
int ceisdk_get_private_profile_int(const char *section_name, const char *key, int ndefault=0);
double ceisdk_get_private_profile_double(const char *section_name, const char *key, double ndefault=0.0);
long ceisdk_get_private_profile_string(const char *section_name, const char *key, char *returned_string, long size_of_returned_string, const char *def, const char *path);
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string, const char *path);
long ceisdk_write_private_profile_string(const char *section_name, const char *key, char *in_string);
}

#endif