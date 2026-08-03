/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include <memory.h>

template <class T>
void InitStruct(T *lpStruct)
{
	memset((char *)lpStruct+4,0,(*(unsigned long *)lpStruct)-4);
}
template <class T>
void InitStructEx(T *lpStruct)
{
	*((unsigned long *)lpStruct) = sizeof(T);	//lpStruct->dwSize = sizeof(T);
	InitStruct(lpStruct);
}
