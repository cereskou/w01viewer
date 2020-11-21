#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#pragma once

// ----------------------------------------------------------------------------
// Protocol 
// ----------------------------------------------------------------------------
enum {
	CMD_USER = 0x00,		// User
	CMD_PASS,				// Password
	CMD_STAT,				// Get server status		
	CMD_GETD,				// Get Binary Data
	CMD_ABOR,				// Abort
	CMD_QUIT,				// Quit
	CMD_PASV,				// Pasv
	CMD_STOR,				// Store file
	CMD_MFF,				// Modify File Facts
	CMD_SHDN,				// Shutdown
	CMD_HELP,				// Help
	CMD_NONE = 0xFF,		// None
	CMD_UNKNOWN = -1
};

// Parameter info
#define CMD_PARAM_NONE		0
#define CMD_PARAM_OPT		1
#define CMD_PARAM_MUST		2

class Protocol
{
public:
	Protocol();
	~Protocol();

public:
	static CHAR* GetCommand(INT nCmdId);
	static INT GetCommandId(const CHAR* pszCmd);
	static INT RequestParameter(INT nCmdId);
	static BOOL NeedsAuthority(INT nCmdId);
};

#endif
