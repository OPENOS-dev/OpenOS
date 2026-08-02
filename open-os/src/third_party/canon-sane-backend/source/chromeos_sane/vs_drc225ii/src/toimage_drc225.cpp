/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <stdio.h>
#include <memory.h>
#include <vector>
#include "ceilogwrite.h"
#include "sdk_image_util.h"
#include "ipsdk.h"
#include "command.h"
long get_scanner_specific_offset(long dpi=1200);
namespace {
	long get_total_size(std::vector<CStreamCmd *> &stream_list)
	{
		long out = 0;
		std::vector<CStreamCmd *>::iterator itr = stream_list.begin();
		for (; itr != stream_list.end(); itr++) {
			out += (*itr)->transfer_length();
		}
		return out;
	}
    void fb2images(ICeiImage* pIn, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        CVSCSDSDKImage* front = create_vscsdsdk_image();
        front->width(pIn->width()/2);
        front->height(pIn->height());
        front->spp(pIn->spp());
        front->bps(pIn->bps());
        front->sync(front->width());
        front->xdpi(pIn->xdpi());
        front->ydpi(pIn->ydpi());
        front->rgb_order(front->spp() == 3 ? 1 : 0);
        front->size(front->sync() * front->height() * front->spp());
        if (front->img() == NULL) return;
        memset(front->img(), 0xff, front->size());
        CVSCSDSDKImage* back = create_vscsdsdk_image();
        back->width(front->width());
        back->height(front->height());
        back->spp(pIn->spp());
        back->bps(pIn->bps());
        back->sync(back->width());
        back->xdpi(pIn->xdpi());
        back->ydpi(pIn->ydpi());
        back->rgb_order(back->spp() == 3 ? 1 : 0);
        back->size(back->sync() * back->height() * back->spp());
        if (back->img() == NULL) return;
        memset(back->img(), 0xff, back->size());
        *ppFront = front;
        *ppBack = back;
    }
    void fb2images(ICeiImage* pIn, CWindow &window, ICeiImage** ppFront, ICeiImage** ppBack, long mud = 1200)
    {
        CVSCSDSDKImage* front = create_vscsdsdk_image();
        front->width(window.width() * window.xdpi() / mud);
        front->height((window.length()+ get_scanner_specific_offset()) * window.ydpi() /mud);
        if (pIn->height()>front->height()) front->height(pIn->height());
        front->spp(pIn->spp());
        front->bps(pIn->bps());
        front->sync(front->width());
        front->xdpi(pIn->xdpi());
        front->ydpi(pIn->ydpi());
        front->rgb_order(front->spp() == 3 ? 1 : 0);
        front->size(front->sync() * front->height() * front->spp());
        if (front->img() == NULL) return;
        memset(front->img(), 0xff, front->size());
        CVSCSDSDKImage* back = create_vscsdsdk_image();
        back->width(front->width());
        back->height(front->height());
        back->spp(pIn->spp());
        back->bps(pIn->bps());
        back->sync(back->width());
        back->xdpi(pIn->xdpi());
        back->ydpi(pIn->ydpi());
        back->rgb_order(back->spp() == 3 ? 1 : 0);
        back->size(back->sync() * back->height() * back->spp());
        if (back->img() == NULL) return;
        memset(back->img(), 0xff, back->size());
        *ppFront = front;
        *ppBack = back;
    }
    void separate(ICeiImage* in, ICeiImage* front, ICeiImage* back)
    {
        char* src[2] = { in->img(), in->img() + in->width() / 2 };
        char* dst[2] = { front->img(), back->img() };
        for (long h = 0; h < in->height(); h++) {
            memcpy(dst[0], src[0], front->sync());
            memcpy(dst[1], src[1], back->sync());
            src[0] += in->sync();
            src[1] += in->sync();
            dst[0] += front->sync();
            dst[1] += back->sync();
        }
    }
    class CImageWrapper : public ICeiImage
    {
    public:
        CImageWrapper(ICeiImage* p) :m_p(p) {}
        long STDMETHODCALLTYPE QueryInterface(REFIID id, void** ppOut) { return -1; }
        unsigned long STDMETHODCALLTYPE AddRef() { return 1; }
        unsigned long STDMETHODCALLTYPE Release() { return 1; }
        char* img() { return m_p->img(); }
        long width() { return m_p->width(); }
        long height() { return m_p->height()*3; }
        long xdpi() { return m_p->xdpi(); }
        long ydpi() { return m_p->ydpi(); }
        long spp() { return 1; }
        long bps() { return m_p->bps(); }
        long sync() { return m_p->sync(); }
        long size() { return m_p->size(); }
        long comptype() {return m_p->comptype();}
        long compinfo() {return m_p->compinfo();}
        long rgb_order() { return 1; }
    private:
        ICeiImage* m_p;
    };
    void to2images_color(ICeiImage* pIn, CWindow &window, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        ICeiImage* front = NULL;
        ICeiImage* back = NULL;
        fb2images(pIn, window, &front, &back);
        CImageWrapper win(pIn), wf(front), wb(back);
        separate(&win, &wf, &wb);
        *ppFront = front;
        *ppBack = back;
    }
    void to2images_gray(ICeiImage* pIn, CWindow &window, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        ICeiImage* front = NULL;
        ICeiImage* back = NULL;
        fb2images(pIn, window, &front, &back);
        separate(pIn, front, back);
        *ppFront = front;
        *ppBack = back;
    }
    void to2images(ICeiImage* pIn, CWindow &window, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        if (pIn->spp() == 3) to2images_color(pIn, window, ppFront, ppBack);
        else                 to2images_gray(pIn, window, ppFront, ppBack);
    }
    void to2images_color(ICeiImage* pIn, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        ICeiImage* front = NULL;
        ICeiImage* back = NULL;
        fb2images(pIn, &front, &back);
        CImageWrapper win(pIn), wf(front), wb(back);
        separate(&win, &wf, &wb);
        *ppFront = front;
        *ppBack = back;
    }
    void to2images_gray(ICeiImage* pIn, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        ICeiImage* front = NULL;
        ICeiImage* back = NULL;
        fb2images(pIn, &front, &back);
        separate(pIn, front, back);
        *ppFront = front;
        *ppBack = back;
    }
    void to2images(ICeiImage* pIn, ICeiImage** ppFront, ICeiImage** ppBack)
    {
        if (pIn->spp() == 3) to2images_color(pIn, ppFront, ppBack);
        else                 to2images_gray(pIn, ppFront, ppBack);
    }
    void to(std::vector<CStreamCmd*>& stream_list, ICeiImage* pimg)
    {
        long total_size = get_total_size(stream_list);
        memset(pimg->img(), 0xff, pimg->size());
        if (total_size > pimg->size()) total_size = pimg->size();
        char* dst = pimg->img();
        long copy_size = 0;
        std::vector<CStreamCmd*>::iterator itr = stream_list.begin();
        while (itr != stream_list.end()) {
            copy_size = (*itr)->transfer_length();
            if (copy_size > total_size) copy_size = total_size;
            memcpy(dst, (*itr)->data(), copy_size);
            dst += copy_size;
            total_size -= copy_size;
            delete (*itr);
            itr = stream_list.erase(itr);
        }
        itr = stream_list.begin();
        for (; itr != stream_list.end(); itr++) {
            delete(*itr);
        }
        stream_list.clear();
    }
    void stream_list2image(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window, long mud, bool duplex)
    {
        long total_size = get_total_size(stream_list);
        long width = (window.width() * window.xdpi() / mud);
        width *= (duplex ? 2 : 1);
        CVSCSDSDKImage* p = create_vscsdsdk_image();
        p->width(width);
        p->spp(window.spp());
        p->sync(width);
        p->bps(window.bps());
        if (p->spp() == 3) {
            p->height(total_size / (p->sync()*3));
        }
        else {
            p->height(total_size / p->sync());
        }
        p->xdpi(window.xdpi());
        p->ydpi(window.ydpi());
        p->rgb_order(p->spp() == 3 ? 1 : 0);
        p->size(p->sync() * p->height() * p->spp());
        if (p->img() == NULL) return;
        to(stream_list, p);
        *ppOut = p;
    }
    void stream_list2image_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window, long mud)
    {
        CVSCSDSDKImage* p = create_vscsdsdk_image();
        p->width(window.width() * window.xdpi() / mud);
        p->height((window.length()+24) * window.ydpi() / mud);
        p->sync(p->width());
        p->spp(window.spp());
        p->bps(window.bps());
        p->xdpi(window.xdpi());
        p->ydpi(window.ydpi());
        p->rgb_order(p->spp() == 3 ? 1 : 0);
        p->size(p->sync() * p->height() * p->spp());
        if (p->img() == NULL) return;
        memset(p->img(), 0xff, p->size());
        to(stream_list, p);
        *ppOut = p;
    }
}
namespace drc225fpgatoimage {
    void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window)
    {
        ICeiImage* pfb = NULL;
        stream_list2image(stream_list, &pfb, window, 1200, true);
        to2images(pfb, window, ppFront, ppBack);
        pfb->Release();
    }
    void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window)
    {
        ICeiImage* pfb = NULL;
        stream_list2image(stream_list, &pfb, window, 1200, true);
        to2images(pfb, ppFront, ppBack);
    }
    void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window)
    {
        stream_list2image(stream_list, ppOut, window, 1200, false);
    }
    void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window)
    {
        stream_list2image_fixedsize(stream_list, ppOut, window, 1200);
    }
}