/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once
#include "IPBase.h"
#include "CeiImgList.h"

namespace Cei
{
	namespace LLiPm
	{
		template<class T>
		class CIPController
		{
		public:
			CIPController(void)
				: m_pIP(0), m_Type(IPANY), m_bFirst(true) {}
			~CIPController(void)
                {clear();}
		public:
			typedef enum tagIPTYPE {
				IPANY,
				IPFIRST,
				IPLAST,
				IPFIRSTLAST, 
				IPCOMPONLY, 
			} IPTYPE;
			void set(T* pIP, IPTYPE type)
				{m_pIP = pIP; m_Type = type; m_bFirst = true;}
			void changeType(IPTYPE type)
				{m_Type = type;}
			bool isNull(void) 
				{return (m_pIP == 0);}
			T* getPtr() const
				{return (m_pIP);}
            void clear(void)
                {if (m_pIP) {delete m_pIP; m_pIP=NULL; m_listImg.PopAll();}}
		public:
			inline RTN IPComplete(CImg& image, void* lpInfo)
			{
				m_bFirst = true;
				m_listImg.PopAll();
				RTN result = m_pIP->IPInterface(image, lpInfo);
				return result;
			}

			inline RTN IPFirst(CImg& image, void* lpInfo)
			{
				m_bFirst = true;
				RTN result = IPMiddle(image, lpInfo);
				return result;
			}

			inline RTN IPMiddle(CImg& image, void* lpInfo)
			{
				RTN result;
				if (m_bFirst) {
					m_listImg.PopAll();
				}
				if (m_Type == IPCOMPONLY) {
					m_listImg.PushBack(image);
					result = RTN_OK;
					m_bFirst = false;
				}
				else {
					if (image.isNull()) {
						result = RTN_OK;
					}
					else {
						if (m_bFirst) {
							result = m_pIP->IPFirstInterface(image, lpInfo);
							m_bFirst = false;
						}
						else {
							result = m_pIP->IPMiddleInterface(image, lpInfo);
						}
					}
				}
				return result;
			}

			RTN IPLast(CImg& image, void* lpInfo)
			{
				RTN result;
				if (m_bFirst) {
					m_listImg.PopAll();
				}
				if (m_Type == IPCOMPONLY) {
					m_listImg.PushBack(image);
					m_listImg.SpliceAndPopAll(image);
					result = m_pIP->IPInterface(image, lpInfo);
				}
				else {
					
					if (m_bFirst) {
						result = m_pIP->IPInterface(image, lpInfo);
					}
					else {
						if (image.isNull() && (m_Type != IPLAST && m_Type != IPFIRSTLAST)) {
							result = RTN_OK;
						}
						else {
							result = m_pIP->IPLastInterface(image, lpInfo);
						}
					}
				}
				m_listImg.PopAll();
				m_bFirst = true;
				return result;
			}
		private:
			T* m_pIP;
			IPTYPE m_Type;
			bool m_bFirst;
			CImgList m_listImg;
		};
	}
}
