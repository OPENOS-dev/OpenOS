/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once

template <class T>
static DWORD MemSum(T * lpBuff, long lSize)
{
	DWORD dwSum = 0;
	while(lSize--) dwSum += (DWORD)*lpBuff++;
	return dwSum;
}

template <class T>
static long Count(T * lptData, T tData, long lSize)
{
	long lCount = 0;
	while(lSize--) {
		if(*lptData==tData) ++lCount;
	}
	return lCount;
}

template <class T>
static double GetGravity(T * lpBuff, long lSize)
{
	double dbG = 0;
	long i, lSum = 0;
	for(i=0; i<lSize; i++) {
		lSum += *lpBuff;
		dbG += (double) (i * *lpBuff++);
	}
	if(lSum)
		dbG /= lSum;
	return dbG;
}

template <class T>
static long GetMaxDataPos(T * lpData, long lSize)
{
	T tMax = *lpData++;
	long l, lPos = 0;
	for(l=1; l<lSize; l++, lpData++) {
		if(tMax < *lpData) {
			tMax = *lpData;
			lPos = l;
		}
	}
	return lPos;
}

template <class T>
static long GetMinDataPos(T * lpData, long lSize)
{
	T tMin = *lpData++;
	long l, lPos = 0;
	for(l=1; l<lSize; l++, lpData++) {
		if(tMin < *lpData) {
			tMin = *lpData;
			lPos = l;
		}
	}
	return lPos;
}

template <class T>
void SWAP(T * a, T * b)
{
	T temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

template <class T>
T * Search(T nData, T *nTable, long lCnt)
{
	while(lCnt--) {
		if(nData==*nTable) return nTable;
		++nTable;
	}
	return NULL;
}

