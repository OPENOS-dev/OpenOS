/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <string.h>
#include "sdk_pagesize_table.h"

namespace {
	long g_maximum_width_1200dpi=0;
	long g_maximum_length_1200dpi=0;
	struct {
		char *paper_size;
		long width_1200dpi;
		long length_1200dpi;
	}g_pagesize[] = {
	{(char*)"A3", 14031, 19842},
	{(char*)"A4",  9921, 14031},
	{(char*)"A4R",14031,  9921},
	{(char*)"A5",  6992,  9921},
	{(char*)"A5R", 9921,  6992},
	{(char*)"A6",  4960,  6992},
	{(char*)"A6R", 6992,  4960},
	{(char*)"B4", 12141, 17196},
	{(char*)"B5",  8598, 12141},
	{(char*)"B5R", 12141, 8598},
	{(char*)"B6",   6047, 8598},
	{(char*)"B6R",  8598, 6047},
	{(char*)"LEGAL",  10200, 16800},
	{(char*)"LETTER", 10200, 13200},
	{(char*)"LETTERR",13200, 10200},
	{(char*)"DOUBLE LETTER", 13200, 20400}
	};

}


long pagesize_table_count()
{
	return (long)(sizeof(g_pagesize)/sizeof(g_pagesize[0]));
}
void pagesize_table_choice(int index, char *ppagesize, long *pwidth_1200dpi, long *pheight_1200dpi)
{
	if (index<0) return;
	if (index>=pagesize_table_count()) return;
	
	if (ppagesize) strcpy(ppagesize, g_pagesize[index].paper_size);
	if (pwidth_1200dpi) *pwidth_1200dpi = g_pagesize[index].width_1200dpi;
	if (pheight_1200dpi) *pheight_1200dpi = g_pagesize[index].length_1200dpi;
}

void pagesize_table_choice(long index, char **pppagesize, long *pwidth_1200dpi, long *pheight_1200dpi)
{
	if (index<0) return;
	if (index>=pagesize_table_count()) return;
	if (pppagesize) *pppagesize = g_pagesize[index].paper_size;
	if (pwidth_1200dpi) *pwidth_1200dpi = g_pagesize[index].width_1200dpi;
	if (pheight_1200dpi) *pheight_1200dpi = g_pagesize[index].length_1200dpi;	
}

void maximum_pagesize_get(long* pwidth_1200dpi/*out*/, long* pheight_1200dpi/*out*/)
{
	if (pwidth_1200dpi) *pwidth_1200dpi = g_maximum_width_1200dpi;
	if (pheight_1200dpi) *pheight_1200dpi = g_maximum_length_1200dpi;
}
void maximum_pagesize_set(long width_1200dpi/*in*/, long height_1200dpi/*in*/)
{
	if (width_1200dpi>0) g_maximum_width_1200dpi = width_1200dpi;
	if (height_1200dpi>0) g_maximum_length_1200dpi = height_1200dpi;
}

void pagesize_table_get(char *ppagesize/*in*/, long *pwidth_1200dpi/*out*/, long *pheight_1200dpi/*out*/)
{
	if (strcmp(ppagesize, "Maximum")==0) {
		maximum_pagesize_get(pwidth_1200dpi, pheight_1200dpi);	
	} else {
		long max_table = pagesize_table_count();
		for (long i=0; i<max_table; i++) {
			if (strcmp(ppagesize, g_pagesize[i].paper_size)==0) {
				if (pwidth_1200dpi) *pwidth_1200dpi = g_pagesize[i].width_1200dpi;
				if (pheight_1200dpi) *pheight_1200dpi = g_pagesize[i].length_1200dpi;	
				break;
			}
		}
	}
}
