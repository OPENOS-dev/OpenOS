/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SDK_PAGESIZE_TABLE_HEADER_INCLUDE_HEADER__
#define __SDK_PAGESIZE_TABLE_HEADER_INCLUDE_HEADER__

long pagesize_table_count();
void pagesize_table_choice(long index, char *ppagesize, long *pwidth_1200dpi, long *pheight_1200dpi);
void pagesize_table_choice(long index, char **pppagesize, long *pwidth_1200dpi, long *pheight_1200dpi);
void pagesize_table_get(char *ppagesize/*in*/, long *pwidth_1200dpi/*out*/, long *pheight_1200dpi/*out*/);
void maximum_pagesize_get(long *pwidth_1200dpi/*out*/, long *pheight_1200dpi/*out*/);
void maximum_pagesize_set(long width_1200dpi/*in*/, long height_1200dpi/*in*/);
#endif