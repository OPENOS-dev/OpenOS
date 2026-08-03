/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __COMMAND_HOOK_INTERFACE_HEADER_DEFINED__
#define __COMMAND_HOOK_INTERFACE_HEADER_DEFINED__

#include "unknown.h"
#include "scanner_connector_interface.h"

class ICommandHook : public IUnknown
{
public:
	virtual long exec_write(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_read(char *cdb, long cdb_size, char *data, long data_size)=0;
	virtual long exec_none(char *cdb, long cdb_size)=0;
};


ICommandHook *commandhook(IScannerConnector *s, IUnknown *h);
ICommandHook *commandhook(unsigned char cdb, IScannerConnector *s, IUnknown *h);
#endif