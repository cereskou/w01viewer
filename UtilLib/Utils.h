#ifndef __UTILFUNC_H__
#define __UTILFUNC_H__
#pragma once

#include "cstring.h"

// ----------------------------------------------------------------------------
// define
// ----------------------------------------------------------------------------
#define BIT_SET(a,b)	((a) |= (1 << (b)))
#define BIT_CLEAR(a,b)	((a) &= ~(1 << (b)))
#define BIT_FLIP(a, b)	((a) ^= (1 << (b)))
#define BIT_CHECK(a, b)	((a) & (1 << (b)))

#define DATA_UNIT_UNKNOW	0
#define DATA_UNIT_KB		1			// kilobyte
#define DATA_UNIT_MB		2			// megabyte
#define DATA_UNIT_GB		3			// gigabyte
#define DATA_UNIT_TB		4			// terabyte
#define DATA_UNIT_PB		5			// petabyte
#define DATA_UNIT_EB		6			// exabyte
#define DATA_UNIT_ZB		7			// zettabyte
#define DATA_UNIT_YB		8			// yottabyte

namespace UTL {

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------
INT IsValidSpaceFormat(CString spcFmt);
BOOL GetDataSpace(CString space, INT fmt, double& value);
BOOL GetDataSpace(double space, INT fmt, double& value);
BOOL GetDataSpace(CString space, double& value);

BOOL CheckHalfWidth(LPCWSTR pszText);
BOOL isValidNumber(LPCWSTR lpszNums);
BOOL isNumber(LPWSTR lpszNums);
BOOL isNumber(LPCWSTR pwszTxt, int off);

double roundd(double x);

CString GetWin32Message(DWORD dwMessageId);
//DWORD GetProcessIdByModuleName(LPCTSTR pszModule);
void OutputSystemErrorMessage(DWORD dwErrCode, int nLevel = 1);

BOOL GetSystemErrorMessageA(DWORD dwErrCode, LPSTR pszMsg, DWORD nSize);
BOOL GetSystemErrorMessageW(DWORD dwErrCode, LPWSTR pszMsg, DWORD nSize);

HMODULE GetCurrentModule();
CString GetCurrentModulePath();
CString GetCurrentModuleName();

// TerminateThread
BOOL ForceTerminateThread(HANDLE hHandel, DWORD dwCode = 13);

// Set file permission
DWORD SetFilePermissionA(LPCSTR lpszFileName);
DWORD SetFilePermissionW(LPCWSTR lpszFileName);

BOOL GetModuleVersionA(LPCSTR lpModule, unsigned short* pMajor, unsigned short* pMinor, unsigned short* pRevision, unsigned short* pBuild);
BOOL GetModuleVersionW(LPCWSTR lpModule, unsigned short* pMajor, unsigned short* pMinor, unsigned short* pRevision, unsigned short* pBuild);

DWORD GetFullPathNameExA(LPCSTR pcszPath, LPSTR pszFullPath, DWORD dwSize = _MAX_PATH);
DWORD GetFullPathNameExW(LPCWSTR pcszPath, LPWSTR pszFullPath, DWORD dwSize = _MAX_PATH);

BOOL FileLikeA(LPCSTR lpPath);
BOOL FileLikeW(LPCWSTR lpPath);

#ifdef _UNICODE
#define GetFullPathNameEx		GetFullPathNameExW
#define GetModuleVersion		GetModuleVersionW
#define FileLike				FileLikeW
#define GetSystemErrorMessage	GetSystemErrorMessageW
#define SetFilePermission		SetFilePermissionW
#else
#define GetFullPathNameEx		GetFullPathNameExA
#define GetModuleVersion		GetModuleVersionA
#define FileLike				FileLikeA
#define GetSystemErrorMessage	GetSystemErrorMessageA
#define SetFilePermission		SetFilePermissionA
#endif

} // namespace UTL

#endif
