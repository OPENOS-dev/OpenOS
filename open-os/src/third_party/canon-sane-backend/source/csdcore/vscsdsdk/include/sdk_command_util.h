/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __SCSI_COMMAND_CONNECTOR_UTILITY_HEADER__
#define __SCSI_COMMAND_CONNECTOR_UTILITY_HEADER__

#include "command.h"
#include "scanner_connector_interface.h"
#include "virtual_scanner_interface.h"

class CScsiCommand
{
public:
	CScsiCommand(IScannerConnector*p):m_psc(p){}
	~CScsiCommand(){}
	long write(CCommand &cmd)
	{
		cmd.I_am_in(CCommand::EXEC_WRITE);
		return m_psc->exec_write(cmd.cdb(), cmd.cdb_size(), cmd.data(), cmd.data_size());
	}
	long read(CCommand &cmd)
	{
		cmd.I_am_in(CCommand::EXEC_READ);
		return m_psc->exec_read(cmd.cdb(), cmd.cdb_size(), cmd.data(), cmd.data_size());		
	}
	long none(CCommand &cmd)
	{
		return m_psc->exec_none(cmd.cdb(), cmd.cdb_size());		
	}	
private:
	IScannerConnector *m_psc;
};


class CScsiVSCommand
{
public:
	CScsiVSCommand(IVirtualScanner *p):m_psc(p){}
	~CScsiVSCommand(){}
	long write(CCommand &cmd)
	{
		cmd.I_am_in(CCommand::EXEC_WRITE);
		return m_psc->exec_write(cmd.cdb(), cmd.cdb_size(), cmd.data(), cmd.data_size());
	}
	long read(CCommand &cmd)
	{
		cmd.I_am_in(CCommand::EXEC_READ);
		return m_psc->exec_read(cmd.cdb(), cmd.cdb_size(), cmd.data(), cmd.data_size());		
	}
	long none(CCommand &cmd)
	{
		return m_psc->exec_none(cmd.cdb(), cmd.cdb_size());		
	}	
private:
	IVirtualScanner *m_psc;
};


#endif