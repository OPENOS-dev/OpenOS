/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#pragma once

class fixed
{
public:
	fixed(void) : value(0) {}
	fixed(const fixed& val) : value(val.value.data) {}
	explicit fixed(const long val) : value(val, 0) {}
	explicit fixed(const double val) : value((long long)(val * (double)0x10000) << 16) {}
private:
	explicit fixed(const long long val) : value(val) {}
public:
	~fixed(void) {}
public:
	inline fixed& operator=(const fixed& val)
	{
		value.data = val.value.data;
		return *this;
	}

	inline fixed& operator+=(const fixed& val)
	{
		value.data += val.value.data;
		return *this;
	}

	inline fixed& operator-=(const fixed& val)
	{
		value.data -= val.value.data;
		return *this;
	}

	inline fixed operator+(const fixed& val) const
	{
		return fixed(value.data + val.value.data);
	}

	inline fixed operator-(const fixed&val) const
	{
		return fixed(value.data - val.value.data);
	}

	inline fixed operator*(const fixed& val) const
	{
		return fixed((value.data >> 16) * (val.value.data >> 16));
	}

	inline fixed operator/(const fixed& val) const
	{
		return fixed((value.data / (val.value.data >> 16)) << 16);
	}

	inline unsigned long getInteger(void) const
	{
		return value.integer;
	}

	inline unsigned short getDecimal(void) const
	{
		return value.decimal;
	}

	inline operator long(void) const
	{
		return value.integer;
	}

	inline operator double(void) const
	{
		return (double)value.integer + (double)value.decimal / (double)0x10000;
	}

private:
	union FIXEDDATA {
		long long data;
#ifdef	__BIG_ENDIAN__
		struct {
			long integer;
			unsigned short decimal;
			unsigned short padding;
		};
#else
		struct {
			unsigned short padding;
			unsigned short decimal;
			long integer;
		};
#endif
		FIXEDDATA(const long long& ll) : data(ll) {}
		FIXEDDATA(const long& l, const unsigned short& d) : integer(l), decimal(d), padding(0) {}
	} value;
};