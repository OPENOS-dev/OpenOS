/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once



class CCeiRaster
{
public:
	CCeiRaster();
	virtual ~CCeiRaster();

	void Clear(void);

	bool SetSize(unsigned long dwNewLen);
	unsigned long GetSize();

	unsigned char* GetPtr();

private:
	unsigned char* m_Buffer;
	unsigned long m_ulSize;
};



