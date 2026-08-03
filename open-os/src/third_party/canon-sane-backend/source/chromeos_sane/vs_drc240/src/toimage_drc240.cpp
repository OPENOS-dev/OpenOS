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

long sensor_len_600dpi(); 
long dummy_pixel_600dpi()
{
	return 26;
}
long valid_width_600dpi();
void sort_duplex_line(char* f, char* b, char* line, long xdpi);
namespace {
	long get_total_size(std::vector<CStreamCmd*>& stream_list)
	{
		long out = 0;
		std::vector<CStreamCmd*>::iterator itr = stream_list.begin();
		for (; itr != stream_list.end(); itr++) {
			out += (*itr)->transfer_length();
		}
		return out;
	}
	class CLine
	{
	public:
		CLine(std::vector<CStreamCmd*>& stream_list);
		~CLine();
		bool hasnext();
		void read(char* pline, long size);
	private:
		std::vector<CStreamCmd*>& m_stream_list;
		long m_index;
		long m_offset;
		long m_left;
	};
	CLine::CLine(std::vector<CStreamCmd*>& stream_list) :m_stream_list(stream_list), m_index(-1), m_offset(0), m_left(0)
	{

		if (stream_list.size()) {
			m_index = 0;
			m_left = stream_list[m_index]->transfer_length();
		}

	}
	CLine::~CLine()
	{
		m_stream_list.clear();
	}
	bool CLine::hasnext()
	{
		return m_index >= 0;
	}
	void CLine::read(char* pline, long size)
	{
		long copy_size = size;
		long total_copy_size = 0;
		long pIndex = 0;
		while ((size_t)m_index < m_stream_list.size()) {
			if (copy_size > m_left) {
				copy_size = m_left;
			}
			memcpy(pline + pIndex, m_stream_list[m_index]->data() + m_offset, copy_size);
			total_copy_size += copy_size;
			m_offset += copy_size;
			m_left -= copy_size;
			if (total_copy_size == size) {
				if (m_left) {
					break;
				}
				delete m_stream_list[m_index];
				m_stream_list[m_index] = NULL;
				m_index++;
				if ((size_t)m_index < m_stream_list.size()) {
					m_offset = 0;
					m_left = m_stream_list[m_index]->transfer_length();
				}
				else {
					m_index = -1;
				}
				break;
			}
			else {
				delete m_stream_list[m_index];
				m_stream_list[m_index] = NULL;
				m_index++;
				if ((size_t)m_index < m_stream_list.size()) {
					m_offset = 0;
					m_left = m_stream_list[m_index]->transfer_length();
					copy_size = size - total_copy_size;
					pIndex = total_copy_size;
				}
				else {
					m_index = -1;
					break;
				}
			}
		}
	}
	bool toImage(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window, long mud, long fixed)
	{
		//printf("toImage() start\r\n");
		if (window.width() != (sensor_len_600dpi() * 3 * 2)) {
			printf("invalid window.width():%ld, sensor width * 3:%ld\r\n", window.width(), sensor_len_600dpi() * 3 * 2);
			return false;
		}
		long total_size = 0;
		long height = 0;
		long width_with_dummy_pixel = sensor_len_600dpi() * 3 * window.xdpi() / 600;
		long width = valid_width_600dpi() * window.xdpi() / 600;

		total_size = get_total_size(stream_list);
		total_size /= 2;

		height = total_size / (width_with_dummy_pixel * window.spp());
		if (fixed) {
			long height2 = window.length() * window.ydpi() / mud;
			if (height < height2) height = height2;
		}

		CVSCSDSDKImage* pfront = create_vscsdsdk_image();
		pfront->width(width);
		pfront->height(height);
		pfront->sync(width);
		pfront->spp(window.spp());
		pfront->bps(window.bps());
		pfront->xdpi(window.xdpi());
		pfront->ydpi(window.ydpi());
		pfront->rgb_order(pfront->spp() == 3 ? 1 : 0);
		pfront->size(pfront->sync() * pfront->height() * pfront->spp());
		if (pfront->img() == NULL) {
			printf("out of memory\r\n");
			return false;
		}
		memset(pfront->img(), 0xff, pfront->size());
		CVSCSDSDKImage* pback = create_vscsdsdk_image();
		pback->width(width);
		pback->height(height);
		pback->sync(width);
		pback->spp(window.spp());
		pback->bps(window.bps());
		pback->xdpi(window.xdpi());
		pback->ydpi(window.ydpi());
		pback->rgb_order(pback->spp() == 3 ? 1 : 0);
		pback->size(pback->sync() * pback->height() * pback->spp());
		if (pback->img() == NULL) {
			printf("out of memory\r\n");
			return false;
		}
		memset(pback->img(), 0xff, pback->size());
		long line_size = width_with_dummy_pixel * 2;
		enum {
			FRONT = 0,
			BACK,
			BOTH
		};
		char* pline[3];
		pline[BOTH] = new char[line_size];
		if (pline[BOTH] == NULL) {
			printf("out of memory\r\n");
			return false;
		}
		pline[FRONT] = new char[width_with_dummy_pixel];
		if (pline[FRONT] == NULL) {
			printf("out of memory\r\n");
			return false;
		}
		pline[BACK] = new char[width_with_dummy_pixel];
		if (pline[BACK] == NULL) {
			printf("out of memory\r\n");
			return false;
		}
		long xdpi = window.xdpi();
		char* dst[2] = { pfront->img(), pback->img() };
		long offset = dummy_pixel_600dpi() * xdpi / 600;
		CLine line(stream_list);
		while (line.hasnext()) {
			line.read(pline[BOTH], line_size);
			sort_duplex_line(pline[FRONT], pline[BACK], pline[BOTH], xdpi);
			memcpy(dst[FRONT], pline[FRONT] + offset, width);
			memcpy(dst[BACK], pline[BACK] + offset, width);
			dst[FRONT] += width;
			dst[BACK] += width;
		}
		delete[] pline[FRONT];
		delete[] pline[BACK];
		delete[] pline[BOTH];
		*ppFront = pfront;
		*ppBack = pback;
		//printf("toImage() end\r\n");
		return true;
	}
	void toImage(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window, long mud, long fixed)
	{
		long total_size = get_total_size(stream_list);
		long width = (window.width() * window.xdpi() / mud);
		long height = total_size / (width * window.spp());
		if (fixed) {
			long height2 = (window.length()+24) * window.ydpi() / mud;
			if (height < height2) height = height2;
		}
		CVSCSDSDKImage* p = create_vscsdsdk_image();
		p->width(width);
		p->height(height);
		p->sync(width);
		p->spp(window.spp());
		p->bps(window.bps());
		p->xdpi(window.xdpi());
		p->ydpi(window.ydpi());
		p->rgb_order(p->spp() == 3 ? 1 : 0);
		p->size(p->sync() * p->height() * p->spp());
		if (p->img() == NULL) return;
		memset(p->img(), 0xff, p->size());
		char* dst = p->img();
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
		*ppOut = p;
		//printf("toImage() end\r\n");
	}
}
void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window)
{
	toImage(stream_list, ppOut, window, 1200, 1);
}
void toImage_fixedsize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window)
{
	toImage(stream_list, ppFront, ppBack, window, 1200, 1);
}
void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppOut, CWindow& window)
{
	toImage(stream_list, ppOut, window, 1200, 0);
}
void toImage_autosize(std::vector<CStreamCmd*>& stream_list, ICeiImage** ppFront, ICeiImage** ppBack, CWindow& window)
{
	toImage(stream_list, ppFront, ppBack, window, 1200, 0);
}
