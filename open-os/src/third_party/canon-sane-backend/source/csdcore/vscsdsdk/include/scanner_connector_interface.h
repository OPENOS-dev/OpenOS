/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SCANNER_CONNECTOR_INTERFACE_HEADER__
#define __SCANNER_CONNECTOR_INTERFACE_HEADER__

#include "unknown.h"

class IScannerConnector : public IUnknown
{
public:
	virtual long lock(long timeout)=0;
	virtual void unlock()=0;

	virtual long  exec_write(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long  exec_read(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long  exec_none(char *cdb, long cdb_size)=0;
};


IScannerConnector *create_simulation_scanner(char *sjson=(char*)"{\"count\":4}");
/*
{
	"count":4,
	"vendor","Canon",
	"model","DR-C230",
	"printf":1,
	"servicedata":{
	"paper_counter":100
	},
	"userdata":{
	"poweroff_time":14400000,
	"sleep_time":600000
	},
	"mode":{
	"mud":1200
	},
	"inquiryex":{
	"basic_x_resolution":600,
	"basic_y_resolution":600,
	"window_width":4960,
	"window_length":7014
	}
	"imprint":{
	"imprinted_string":""
	}
}

example
create_simulation_scanner("{\"count\":10}");
create_simulation_scanner("{\"count\":1, \"vendor\":\"Winmage\",\"inquiryex\":{\"basic_x_resolution\":1200,\"basic_y_resolution\":1200}}");

*/

#endif