/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "CeiImg.h"

#include <new>
#define IP_TRY_BADALLOC			try {
#define IP_CATCH_BADALLOC		} catch(std::bad_alloc &) {return RTN_NOMEM;}

#ifdef USE_TEST
#include <limits.h>
#include <stdio.h>
#include "CeiImgList.h"
#include "CeiImgFile.h"
#define TEST_DIR "/tmp"
#define PUSH_IMAGE(img) pushImageStack(img)
#define EXPORT_IMAGE()   exportProcessedImage()
#else
#define PUSH_IMAGE(img)
#define EXPORT_IMAGE()
#endif


namespace Cei
{
	namespace LLiPm
	{
		typedef struct tagDUMMYPIXELS {
			long lLeft;
			long lMiddle;
			long lRight;
		} DUMMYPIXELS, *LPDUMMYPIXELS;

		class CIPBase
		{
		public:
			CIPBase(void) {}
			virtual ~CIPBase(void) {}
			virtual const char* const getName(void) const = 0;
		public:
			RTN IPInterface(CImg& image, void* lpInfo) {
				RTN result = RTN_OK;
				result = setInfo(image, lpInfo);
				if (result == RTN_OK) {
					result = IP(image);
                    PUSH_IMAGE(image);
                    EXPORT_IMAGE();
				}
				return result;
			}
			RTN IPFirstInterface(CImg& image, void* lpInfo) {
				RTN result = RTN_OK;
				result = setInfoFirst(image, lpInfo);
				if (result == RTN_OK) {
					result = IPFirst(image);
                    PUSH_IMAGE(image);
				}
				return result;
			}
			RTN IPMiddleInterface(CImg& image, void* lpInfo) {
				RTN result = RTN_OK;
				result = setInfoMiddle(image, lpInfo);
				if (result == RTN_OK) {
					result = IPMiddle(image);
                    PUSH_IMAGE(image);
				}
				return result;
			}
			RTN IPLastInterface(CImg& image, void* lpInfo) {
				RTN result = RTN_OK;
				result = setInfoLast(image, lpInfo);
				if (result == RTN_OK) {
					result = IPLast(image);
                    PUSH_IMAGE(image);
                    EXPORT_IMAGE();
				}
				return result;
			}
		protected:
			virtual RTN IP(CImg& image) = 0;
			virtual RTN IPFirst(CImg& image) = 0;
			virtual RTN IPMiddle(CImg& image) = 0;
			virtual RTN IPLast(CImg& image) = 0;
			virtual RTN setInfo(CImg& image, void* lpInfo) = 0;
			virtual RTN setInfoFirst(CImg& image, void* lpInfo) = 0;
			virtual RTN setInfoMiddle(CImg& image, void* lpInfo) = 0;
			virtual RTN setInfoLast(CImg& image, void* lpInfo) = 0;
#ifdef USE_TEST
        private:
            void pushImageStack(CImg &img) {
                CImg copy = img;
                m_listImg.PushBack(copy);
            }
            void exportProcessedImage() {
                char path[PATH_MAX];
                sprintf(path, "%s/%s.cei", TEST_DIR, getName());
                CImgFile output;
                m_listImg.SpliceAndPopAll(output);
                output.saveCei(path);
            }
            CImgList m_listImg;
#endif

		};
        
        class CIPDummy : public CIPBase
        {
        public:
            CIPDummy(void) {}
            ~CIPDummy(void) {}
            const char* const getName(void) const { return "IPDummy"; }
        protected:
            RTN IP(CImg& image) { return RTN_OK; }
            RTN IPFirst(CImg& image) { return RTN_OK; }
            RTN IPMiddle(CImg& image) { return RTN_OK; }
            RTN IPLast(CImg& image) { return RTN_OK; }
            RTN setInfo(CImg& image, void* lpInfo) { return RTN_OK; }
            RTN setInfoFirst(CImg& image, void* lpInfo) { return RTN_OK; }
            RTN setInfoMiddle(CImg& image, void* lpInfo) { return RTN_OK; }
            RTN setInfoLast(CImg& image, void* lpInfo) { return RTN_OK; }
        };
	}
}
