/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#include <cstddef>
#include "sense2vs_error.h"

namespace {
	long sense2vserror_senskey1(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x37, (char)0x00, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};
		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}
		return VS3_INITERROR;
	}
	long sense2vserror_senskey2(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x04, (char)0x01, VS3_HARDERROR},
			{(char)0x80, (char)0x00, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};
		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}
		return VS3_INITERROR;
	}
	long sense2vserror_senskey3(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x36, (char)0x00, VS3_NOCARTRIDGE},
			{(char)0x3A, (char)0x00, VS3_NOPAGE},
			{(char)0x80, (char)0x00, VS3_JAM},
			{(char)0x80, (char)0x01, VS3_COVEROPEN},
			{(char)0x81, (char)0x01, VS3_DOUBLEFEED},
			{(char)0x81, (char)0x02, VS3_SKEW},
			{(char)0x81, (char)0x04, VS3_STAPLE},
			{(char)0x60, (char)0x00, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};
		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}
		return VS3_INITERROR;	
	}
	long sense2vserror_senskey4(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x80, (char)0x01, VS3_HARDERROR},
			{(char)0x80, (char)0x02, VS3_HARDERROR},
			{(char)0x80, (char)0x03, VS3_HARDERROR},
			{(char)0x80, (char)0x04, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};

		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}

		return VS3_INITERROR;
	}
	long sense2vserror_senskey5(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x1a, (char)0x00, VS3_HARDERROR},
			{(char)0x20, (char)0x00, VS3_HARDERROR},
			{(char)0x24, (char)0x00, VS3_HARDERROR},
			{(char)0x25, (char)0x00, VS3_HARDERROR},
			{(char)0x26, (char)0x00, VS3_HARDERROR},
			{(char)0x2c, (char)0x00, VS3_HARDERROR},
			{(char)0x2c, (char)0x01, VS3_HARDERROR},
			{(char)0x3A, (char)0x00, VS3_NOPAGE},
			{(char)0x3d, (char)0x00, VS3_HARDERROR},
			{(char)0x55, (char)0x00, VS3_NOMEM},
			{(char)0x0,  (char)0x0,  0}
		};
		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}

		return VS3_INITERROR;	
	}
	long sense2vserror_senskey6(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x29, (char)0x00, VS3_HARDERROR},
			{(char)0x2a, (char)0x00, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};

		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}

		return VS3_INITERROR;
	}

    long sense2vserror_senskey7(CSenseCmd &sns)
    {
        struct {
            char asc;
            char ascq;
            long err;
        }tbl[] = {
            {(char)0x9, (char)0x01, VS3_CANCEL},
            {(char)0x3, (char)0x00, VS3_DEVICE_NOT_FOUND},
            {(char)0x0,  (char)0x0,  0}
        };

        for (long i=0; tbl[i].err; i++) {

            if (tbl[i].asc==(char)sns.additional_sense_code() &&
                tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

        }

        return VS3_INITERROR;
    }
	long sense2vserror_senskeyb(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x00, (char)0x00, VS3_HARDERROR},
			{(char)0x45, (char)0x00, VS3_HARDERROR},
			{(char)0x47, (char)0x00, VS3_HARDERROR},
			{(char)0x48, (char)0x00, VS3_HARDERROR},
			{(char)0x49, (char)0x00, VS3_HARDERROR},
			{(char)0x80, (char)0x00, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};

		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}

		return VS3_INITERROR;
	}
	long sense2vserror_senskeye(CSenseCmd &sns)
	{
		struct {
			char asc;
			char ascq;
			long err;
		}tbl[] = {
			{(char)0x3b, (char)0x0d, VS3_HARDERROR},
			{(char)0x3b, (char)0x0e, VS3_HARDERROR},
			{(char)0x0,  (char)0x0,  0}
		};

		for (long i=0; tbl[i].err; i++) {

			if (tbl[i].asc==(char)sns.additional_sense_code() &&
			    tbl[i].ascq==(char)sns.additional_sense_code_qualifier()) return tbl[i].err;

		}
		return VS3_INITERROR;
	}
}
long sense2vs3_error(CSenseCmd &sns)
{
	long out = VS3_INITERROR;
	char s = sns.sense_key();
	//WriteLog(_T("sense2vserror(%d)"), s);
	switch (s) {
		case 1:out = sense2vserror_senskey1(sns);break;
		case 2:out = sense2vserror_senskey2(sns);break;
		case 3:out = sense2vserror_senskey3(sns);break;
		case 4:out = sense2vserror_senskey4(sns);break;
		case 5:out = sense2vserror_senskey5(sns);break;
		case 6:out = sense2vserror_senskey6(sns);break;
        case 7:out = sense2vserror_senskey7(sns);break;
		case 0xb:out = sense2vserror_senskeyb(sns);break;
		case 0xe:out = sense2vserror_senskeye(sns);break;
		default:break;
	}
	//WriteLog(_T("VS ERROR:%s"), ErrorCodetoString(out));
	return out;
}
