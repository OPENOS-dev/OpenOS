/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include "ceidbg.h"
#include "CImgInf.h"
#include <new>


class CImageInfoPtr {
protected:
	CImageInfo *m_pImg;
public :
	CImageInfoPtr();
	virtual ~CImageInfoPtr();

	BOOL CreateImage(LONG w, LONG sync, LONG h, LONG b, LONG s, LONG xres, LONG yres);

	// CImageInfo‚ÌˆÚ“®
	void Attach(CImageInfo *pImg);
	CImageInfo *Detach();
	CImageInfo * operator ->() {	return m_pImg;	}
	operator CEIIMAGEINFO *() {	return (CEIIMAGEINFO *)*m_pImg;	}
	void ReleaseImage();

	BOOL Rotate90R();
	BOOL Rotate90L();
	BOOL ReverseColor();

	LONG Width()	{	_ASSERT(m_pImg);	return m_pImg->Width();	}
	LONG Height()	{	_ASSERT(m_pImg);	return m_pImg->Height();	}
	LONG Bps()		{	_ASSERT(m_pImg);	return m_pImg->Bps();	}
	LONG Spp()	{	_ASSERT(m_pImg);	return m_pImg->Spp();	}
	LONG XResolution()	{	_ASSERT(m_pImg);	return m_pImg->XResolution();	}
	LONG YResolution()		{	_ASSERT(m_pImg);	return m_pImg->YResolution();	}
	LONG Sync()		{	_ASSERT(m_pImg);	return m_pImg->Sync();	}
	LPBYTE GetPtr()	{	_ASSERT(m_pImg);	return m_pImg->GetPtr();	}
	DWORD GetSize()	{	_ASSERT(m_pImg);	return m_pImg->GetSize();	}
	DWORD RGBOrder() {	_ASSERT(m_pImg);	return m_pImg->RGBOrder();	}
	CImageInfo *GetImage() {	return m_pImg;	}

#ifdef USE_WIN//_WIN32
	BOOL SaveBmp(LPCTSTR szFileName) {
		_ASSERT(m_pImg);
		#ifdef UNICODE
			return m_pImg->SaveBmp((LPTSTR)szFileName);
		#else
			return m_pImg->SaveBmp((LPSTR)szFileName);
		#endif
	}
#endif //USE_WIN//_WIN32
	
	BOOL SetExternalImage(CEIIMAGEINFO &iInfo)
	{
		CImageInfo *pImg = new(std::nothrow) CImageInfo(&iInfo);
		if (pImg == NULL) return FALSE;
		Attach(pImg);
		return TRUE;
	}
};

BOOL CopyCImageInfo(CImageInfoPtr &Dst, CImageInfoPtr &Src);
BOOL SameSizeCImageInfo(CImageInfoPtr &Dst, CImageInfoPtr &Src);
int CompareImages(CImageInfoPtr &Img1, CImageInfoPtr &Img2);

