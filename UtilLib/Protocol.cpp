#include "stdafx.h"
#include "Protocol.h"

// ----------------------------------------------------------------------------
// Protocol 
// ----------------------------------------------------------------------------
struct _ercmd {
	INT  id;
	CHAR *name;
	INT  param;
	BOOL auth;
} cmd_items[] = {
	{ CMD_USER,		"USER",	CMD_PARAM_MUST,	false },
	{ CMD_PASS,		"PASS",	CMD_PARAM_OPT,	false },
	{ CMD_STAT,		"STAT", CMD_PARAM_OPT,	true },
	{ CMD_GETD,		"GETD",	CMD_PARAM_MUST,	true },
	{ CMD_ABOR,		"ABOR",	CMD_PARAM_NONE,	false },
	{ CMD_QUIT,		"QUIT", CMD_PARAM_NONE,	false },
	{ CMD_PASV,		"PASV", CMD_PARAM_NONE, true },
	{ CMD_STOR,		"STOR", CMD_PARAM_MUST, true },
	{ CMD_MFF,		"MFF",	CMD_PARAM_MUST, true },
	{ CMD_SHDN,		"SHDN",	CMD_PARAM_MUST, true },
	{ CMD_HELP,		"HELP",	CMD_PARAM_NONE, false },
	{ CMD_NONE,		NULL, 0, 0 }
};

Protocol::Protocol() {
}

Protocol::~Protocol() {
}

CHAR* Protocol::GetCommand(INT nCmdId) {
	if (nCmdId < 0 || nCmdId > CMD_NONE)
		return NULL;

	return cmd_items[nCmdId].name;
}

INT Protocol::GetCommandId(const CHAR* pszCmd) {
	if( pszCmd == NULL || pszCmd[0] == '\0' )
		return CMD_UNKNOWN;

	INT i = 0;
	while(cmd_items[i].id != CMD_NONE) {
		if( strcmp(pszCmd, cmd_items[i].name) == 0 )
			return cmd_items[i].id;

		i++;
	}

	return CMD_UNKNOWN;
}

INT Protocol::RequestParameter(INT nCmdId) {
	if( nCmdId < 0 || nCmdId > CMD_NONE )
		return -1;

	return cmd_items[nCmdId].param;
}

BOOL Protocol::NeedsAuthority(INT nCmdId) {
	if( nCmdId < 0 || nCmdId > CMD_NONE)
		return true;

	return cmd_items[nCmdId].auth;
}
