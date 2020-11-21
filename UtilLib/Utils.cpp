#include "stdafx.h"
#include "Utils.h"
#include <math.h>
//#include <TlHelp32.h>
#include <AccCtrl.h>
#include <AclAPI.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// version
#pragma comment(lib, "version.lib")

namespace UTL {

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------
INT IsValidSpaceFormat(CString spcFmt) {
	INT nRet = 0;
	if (spcFmt.GetLength() != 2) {
		return nRet;
	}

	// 単位
	if (spcFmt.CompareNoCase(_T("KB")) == 0) {
		nRet = DATA_UNIT_KB;
	}
	else if (spcFmt.CompareNoCase(_T("MB")) == 0) {
		nRet = DATA_UNIT_MB;
	}
	else if (spcFmt.CompareNoCase(_T("GB")) == 0) {
		nRet = DATA_UNIT_GB;
	}
	else if (spcFmt.CompareNoCase(_T("TB")) == 0) {
		nRet = DATA_UNIT_TB;
	}
	else if (spcFmt.CompareNoCase(_T("PB")) == 0) {
		nRet = DATA_UNIT_PB;
	}
	else if (spcFmt.CompareNoCase(_T("EB")) == 0) {
		nRet = DATA_UNIT_EB;
	}
	else if (spcFmt.CompareNoCase(_T("ZB")) == 0) {
		nRet = DATA_UNIT_ZB;
	}
	else if (spcFmt.CompareNoCase(_T("YB")) == 0) {
		nRet = DATA_UNIT_YB;
	}

	return nRet;
}

// Get Data Space
// [IN ] space  - "100"
// [IN ] format - DATA_UNIT_xB
// [OUT] value
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL GetDataSpace(CString space, INT fmt, double& value) {
	LPTSTR pszEnd = NULL;
	value = 0.0;
	errno = 0;
	double dbval = _tcstod(space, &pszEnd);
	if (errno != ERANGE) {
		if (*pszEnd != _T('\0')) {
			return FALSE;
		}
	}
	else if (dbval == HUGE_VAL) {
		return FALSE;
	}

	return GetDataSpace(dbval, fmt, value);
}

// Get Data Space
// [IN ] space  - 100
// [IN ] format - DATA_UNIT_xB
// [OUT] value
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL GetDataSpace(double space, INT fmt, double& value) {
	errno = 0;
	value = space * pow(1024.0, fmt);
	if (errno != 0)
		return FALSE;

	return TRUE;
}

// GetDataSpace
// [IN ] space -   "100MB"
// [OUT] value
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL GetDataSpace(CString space, double& value) {
	space.Trim();
	// Length Check
	int spclen = (int)space.GetLength() - 2;
	if (spclen <= 0) {
		return FALSE;
	}
	// Get unit
	CString szUnit = space.Right(2);
	// Get Value
	CString szValue = space.Mid(0, spclen);
	// Check data unit
	INT unit = IsValidSpaceFormat(szUnit);
	if (unit == DATA_UNIT_UNKNOW) {
		return FALSE;
	}

	// Return
	return GetDataSpace(szValue, unit, value);
}

/*
// ChangeFileTime
// [IN ] path -   "c:\\sample\\sample.txt"
// [IN ] ftm  -   ""
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL ChangeFileTime(LPTSTR path, LPTSTR ftm) {
	// F
	HANDLE hFile = ::CreateFile(path,
		FILE_WRITE_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	FILETIME cTime;
	FILETIME aTime;
	FILETIME uTime;
	if (!::GetFileTime(hFile, &cTime, &aTime, &uTime)) {
		::CloseHandle(hFile);

		return FALSE;
	}

	// parse
	LPTSTR seps = _T(";");
	LPTSTR token = NULL;
	TCHAR ch = _T('=');
	LPTSTR next = NULL;

	TCHAR sets[8][32];
	memset(&sets, 0x00, sizeof(sets));
	token = _tcstok_s(ftm, seps, &next);
	while (token != NULL) {
		LPTSTR val = NULL;
		val = _tcschr(token, ch);
		int pos = (int)(val - token);
		if (pos > 0)
		{
			val++;
			if (_tcsncmp(_T("Create"), token, pos) == 0) {
				_tcscpy_s(sets[0], 32, val);
			}
			else if (_tcsncmp(_T("Modify"), token, pos) == 0) {
				_tcscpy_s(sets[1], 32, val);
			}
			else if (_tcsncmp(_T("Access"), token, pos) == 0) {
				_tcscpy_s(sets[2], 32, val);
			}
		}

		// Get Next Token
		token = _tcstok_s(NULL, seps, &next);
	}

	// 作成日時
	if (sets[0] != 0) {
		//Create
		if (!StrToFileTime(sets[0], &cTime)) {
			::CloseHandle(hFile);

			return FALSE;
		}
	}

	// 最終更新日時
	if (sets[1] != 0) {
		//Modify
		if (!StrToFileTime(sets[1], &uTime)) {
			::CloseHandle(hFile);

			return FALSE;
		}
	}

	// 最終アクセス日時
	if (sets[2] != 0) {
		//Access
		if (!StrToFileTime(sets[2], &aTime)) {
			::CloseHandle(hFile);

			return FALSE;
		}
	}

	// SetFileTime
	// ファイルの作成日時、最終アクセス日時、最終更新日時を設定します
	if (!::SetFileTime(hFile, &cTime, &aTime, &uTime)){
		::CloseHandle(hFile);
		return FALSE;
	}

	::CloseHandle(hFile);

	return TRUE;
}

// StrToTimeT
// [IN ] ftm  -   ""
// [OUT] lptm  -  time_t
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL StrToTimeT(LPTSTR ftm, time_t* lptm)
{
	struct tm tmm = { 0 };
	TCHAR s[8] = { 0 };
	LPTSTR p = ftm;
	// Year Year[since 1900]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 4);
	tmm.tm_year = _tstoi(s);
	tmm.tm_year -= 1900;
	p += 4;

	// Mon [0-11]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	tmm.tm_mon = _tstoi(s);
	tmm.tm_mon--;
	p += 2;

	// Day[1-31]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	tmm.tm_mday = _tstoi(s);
	p += 2;

	// Hour[0-23]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	tmm.tm_hour = _tstoi(s);
	p += 2;

	// Min[0-59]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	tmm.tm_min = _tstoi(s);
	p += 2;

	// Sec[0-61]
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	tmm.tm_sec = _tstoi(s);

	*lptm = mktime(&tmm);
	if (*lptm == -1)
		return FALSE;

	return TRUE;
}

// StrToTimeT
// [IN ] ftm  -   ""
// [OUT] lptm  -  FILETIME
// Return
//     TRUE - Ok
//     FALSE - Failed
BOOL StrToFileTime(LPTSTR ftm, LPFILETIME lpftm)
{
	SYSTEMTIME st = { 0 };
	TCHAR s[8] = { 0 };
	LPTSTR p = ftm;
	// Year
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 4);
	st.wYear = _tstoi(s);
	p += 4;

	// Mon
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	st.wMonth = _tstoi(s);
	p += 2;

	// Day
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	st.wDay = _tstoi(s);
	p += 2;

	// Hour
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	st.wHour = _tstoi(s);
	p += 2;

	// Min
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	st.wMinute = _tstoi(s);
	p += 2;

	// Sec
	memset(s, 0x00, sizeof(s) * sizeof(TCHAR));
	_tcsncpy_s(s, 8, p, 2);
	st.wSecond = _tstoi(s);

	// SystemTimeTo File Time
	if (!::SystemTimeToFileTime(&st, lpftm)) {
		return FALSE;
	}

	// Local FileTiem To FileTime
	if (!::LocalFileTimeToFileTime(lpftm, lpftm)) {
		return FALSE;
	}

	return TRUE;
}
*/

//BOOL ListProcessModule(DWORD dwPID, LPCTSTR pszModule) {
//	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
//	MODULEENTRY32 me32;
//
//	hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
//	if (hModuleSnap == INVALID_HANDLE_VALUE) {
//		//afxERROR(_T("CreateToolhelp32Snapshot<TH32CS_SNAPMODULE> failed with error %d"), ::GetLastError());
//		return FALSE;
//	}
//
//	me32.dwSize = sizeof(MODULEENTRY32);
//
//	if (!::Module32First(hModuleSnap, &me32)) {
//		//afxERROR(_T("Module32First failed with error %d"), ::GetLastError());
//		::CloseHandle(hModuleSnap);
//		return FALSE;
//	}
//
//	BOOL bFound = FALSE;
//	do {
//		if (_tcsicmp(pszModule, me32.szModule) == 0 ||
//			_tcsicmp(pszModule, me32.szExePath) == 0) {
//			bFound = TRUE;
//			break;
//		}
//		//afxTRACE(_T("Module Name  : %s"), me32.szModule);
//		//afxTRACE(_T("Executable   : %s"), me32.szExePath);
//		//afxTRACE(_T("Process ID   : 0x%08X"), me32.th32ProcessID);
//		//afxTRACE(_T("Ref count(g) : 0x%04X"), me32.GlblcntUsage);
//		//afxTRACE(_T("Ref count(p) : 0x%04X"), me32.ProccntUsage);
//		//afxTRACE(_T("Base address : 0x%08X"), me32.modBaseAddr);
//		//afxTRACE(_T("Base size    : %d"), me32.modBaseSize);
//
//	} while (::Module32Next(hModuleSnap, &me32));
//	::CloseHandle(hModuleSnap);
//
//	return bFound;
//}

//BOOL ListProcessThreads(DWORD dwOwnerPID) {
//	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
//	THREADENTRY32 te32;
//
//	hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
//	if (hThreadSnap == INVALID_HANDLE_VALUE) {
//		afxERROR(_T("CreateToolhelp32Snapshot<TH32CS_SNAPTHREAD> failed with error %d"), ::GetLastError());
//		return FALSE;
//	}
//
//	te32.dwSize = sizeof(THREADENTRY32);
//
//	if (!::Thread32First(hThreadSnap, &te32)) {
//		afxERROR(_T("Thread32First failed with error %d"), ::GetLastError());
//		::CloseHandle(hThreadSnap);
//		return FALSE;
//	}
//
//	do {
//		if (te32.th32OwnerProcessID == dwOwnerPID) {
//			afxTRACE(_T("Thread ID      : 0x%08X"), te32.th32ThreadID);
//			afxTRACE(_T("Base priority  : %d"), te32.tpBasePri);
//			afxTRACE(_T("Delta priority : %d"), te32.tpDeltaPri);
//		}
//
//	} while (::Thread32Next(hThreadSnap, &te32));
//
//	::CloseHandle(hThreadSnap);
//	return TRUE;
//}

//DWORD GetProcessIdByModuleName(LPCTSTR pszModule) {
//	HANDLE hProcessSnap;
//
//	PROCESSENTRY32 pe32;
//
//	hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//	if (hProcessSnap == INVALID_HANDLE_VALUE) {
//		//afxERROR(_T("CreateToolhelp32Snapshot<TH32CS_SNAPPROCESS> failed with error %d"), ::GetLastError());
//		return FALSE;
//	}
//
//	pe32.dwSize = sizeof(PROCESSENTRY32);
//
//	if (!::Process32First(hProcessSnap, &pe32)) {
//		//afxERROR(_T("Process32First failed with error %d"), ::GetLastError());
//		::CloseHandle(hProcessSnap);
//		return FALSE;
//	}
//
//	DWORD dwProcessId = 0;
//	//HANDLE hProcess = INVALID_HANDLE_VALUE;
//	do {
//		//afxTRACE(_T("Process Name : %s"), pe32.szExeFile);
//		if (ListProcessModule(pe32.th32ProcessID, pszModule)) {
//			// Found
//			dwProcessId = pe32.th32ProcessID;
//			break;
//		}
//	} while (::Process32Next(hProcessSnap, &pe32));
//
//	::CloseHandle(hProcessSnap);
//
//	return dwProcessId;
//}


CString GetWin32Message(DWORD dwMessageId) {
	CString szMsg;
	LPVOID lpMsgBuf = NULL;

	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwMessageId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL) > 0 ) {

		szMsg = (LPTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
	}

	return szMsg;
}

void OutputSystemErrorMessage(DWORD dwErrCode, int nLevel)
{
	CString szMsg = GetWin32Message(dwErrCode);

	switch (nLevel)
	{
	case TRACE_FATAL:
		afxFATAL(_T("%s"), szMsg);
		break;
	case TRACE_ERROR:
		afxERROR(_T("%s"), szMsg);
		break;
	case TRACE_WARN:
		afxWARN(_T("%s"), szMsg);
		break;
	case TRACE_INFO:
		afxINFO(_T("%s"), szMsg);
		break;
	case TRACE_DEBUG:
		afxDEBUG(_T("%s"), szMsg);
		break;
	case TRACE_ALL:
		afxTRACE(_T("%s"), szMsg);
		break;
	default:
		break;
	}
}

BOOL GetSystemErrorMessageA(DWORD dwErrCode, LPSTR pszMsg, DWORD nSize)
{
	LPVOID lpMsgBuff = NULL;

	::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(DWORD)dwErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuff,
		0,
		NULL);
	if (lpMsgBuff != NULL)
	{
		// Remove new line code
		LPSTR p = strchr((LPSTR)lpMsgBuff, '\r');
		if (p != NULL)
		{
			*p = '\0';
		}
		strcpy_s(pszMsg, nSize - 1, (LPSTR)lpMsgBuff);

		LocalFree(lpMsgBuff);

		return TRUE;
	}
	return FALSE;
}

BOOL GetSystemErrorMessageW(DWORD dwErrCode, LPWSTR pszMsg, DWORD nSize)
{
	LPVOID lpMsgBuff = NULL;

	::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(DWORD)dwErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuff,
		0,
		NULL);
	if (lpMsgBuff != NULL)
	{
		// Remove new line code
		LPWSTR p = wcschr((LPWSTR)lpMsgBuff, L'\r');
		if (p != NULL)
		{
			*p = L'\0';
		}
			
		wcscpy_s(pszMsg, nSize - 1, (LPWSTR)lpMsgBuff);

		LocalFree(lpMsgBuff);

		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// Get dll module handle
HMODULE GetCurrentModule()
{
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	HMODULE hModule = NULL;
	if (::GetModuleHandleEx(flags, (LPCTSTR)GetCurrentModule, &hModule))
	{
		return hModule;
	}

	return NULL;
}

CString GetCurrentModuleName() {
	TCHAR szModule[_MAX_PATH] = { 0 };
	CString szPath;

	if (::GetModuleFileName(GetCurrentModule(), szModule, _MAX_PATH)) {
		szPath = szModule;
	}

	return szPath;
}

CString GetCurrentModulePath() {
	TCHAR szModule[_MAX_PATH] = { 0 };
	CString szPath;

	if (!::GetModuleFileName(NULL, szModule, _MAX_PATH)) {
		return _T("");
	}

	if (::PathRemoveFileSpec(szModule)) {
		::PathAddBackslash(szModule);
		szPath = szModule;
	}

	return szPath;
}

BOOL ForceTerminateThread(HANDLE hHandel, DWORD dwCode) {
	BOOL bRet = TRUE;
	DWORD dwExitCode = 0;
	if (::GetExitCodeThread(hHandel, &dwExitCode)) {
		if (dwExitCode == STILL_ACTIVE) {
			afxTRACE(_T("TerminateThread thread %d"), hHandel);
			if (!::TerminateThread(hHandel, dwCode)) {
				afxTRACE(_T("TerminateThread thread %d failed with error %d"), hHandel, ::GetLastError());
				bRet = FALSE;
			}
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------------
// SetFilePermission
DWORD SetFilePermissionA(LPCSTR lpszFileName)
{
	PSID pEvryoneSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea[1];
	DWORD dwErrorCode = ERROR_SUCCESS;

	// EveryOne
	SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_WORLD_SID_AUTHORITY;

	// Create a well-known SID for the Everyone group
	if (!::AllocateAndInitializeSid(&sidAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEvryoneSID))
	{
		return ::GetLastError();
	}

	// Init an EXPLICIT_ACCESS structure for an ACE
	// The ACE will allow Everyone read access to the key
	ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = 0xFFFFFFFF;
	ea[0].grfAccessMode = GRANT_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pEvryoneSID;

	// Create a new ACL that contains the new ACEs
	if (::SetEntriesInAcl(1, ea, NULL, &pACL) == ERROR_SUCCESS)
	{
		// initialize a security descriptor
		pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (pSD != NULL)
		{
			if (::InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
			{
				// Add the ACL to the security descriptor
				if (::SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
				{
					// Change the security attributes
					::SetFileSecurityA(lpszFileName, DACL_SECURITY_INFORMATION, pSD);
				}
			}
		}
	}
	dwErrorCode = ::GetLastError();

	if (pEvryoneSID)
		FreeSid(pEvryoneSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);

	return dwErrorCode;
}

DWORD SetFilePermissionW(LPCWSTR lpszFileName)
{
	PSID pEvryoneSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea[1];
	DWORD dwErrorCode = ERROR_SUCCESS;

	// EveryOne
	SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_WORLD_SID_AUTHORITY;

	// Create a well-known SID for the Everyone group
	if (!::AllocateAndInitializeSid(&sidAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEvryoneSID))
	{
		return ::GetLastError();
	}

	// Init an EXPLICIT_ACCESS structure for an ACE
	// The ACE will allow Everyone read access to the key
	ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = 0xFFFFFFFF;
	ea[0].grfAccessMode = GRANT_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pEvryoneSID;

	// Create a new ACL that contains the new ACEs
	if (::SetEntriesInAcl(1, ea, NULL, &pACL) == ERROR_SUCCESS)
	{
		// initialize a security descriptor
		pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (pSD != NULL)
		{
			if (::InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
			{
				// Add the ACL to the security descriptor
				if (::SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
				{
					// Change the security attributes
					::SetFileSecurityW(lpszFileName, DACL_SECURITY_INFORMATION, pSD);
				}
			}
		}
	}
	dwErrorCode = ::GetLastError();

	if (pEvryoneSID)
		FreeSid(pEvryoneSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);

	return dwErrorCode;
}

DWORD GetFullPathNameExW(LPCWSTR pcszPath, LPWSTR pszFullPath, DWORD dwSize)
{
	DWORD dwLen = 0;
	WCHAR szPath[_MAX_PATH] = { 0 };
	if (::PathIsUNCW(pcszPath))
	{
		afxWARN(_T("Does not support UNC path.\n"));
		return 0;
	}

	if (!::PathIsRelativeW(pcszPath))
	{
		wcscpy_s(pszFullPath, dwSize, pcszPath);
	}
	else
	{
		if (!::GetModuleFileNameW(GetCurrentModule(), szPath, _MAX_PATH))
		{
			if (!::GetModuleFileNameW(NULL, szPath, _MAX_PATH))
			{
				afxWARN(_T("Failed to get module path\n"));
				return 0;
			}
		}

		if (!::PathRemoveFileSpecW(szPath))
		{
			afxWARN(_T("PathRemoveFileSpec failed.\n"));
			return 0;
		}

		::PathAddBackslashW(szPath);
		wcscat_s(szPath, _MAX_PATH, pcszPath);

		if (::PathIsRelativeW(pcszPath))
		{
			WCHAR** lppPart = { NULL };
			WCHAR buf[_MAX_PATH] = { 0 };

			if (::GetFullPathNameW(szPath, _MAX_PATH, buf, lppPart))
			{
				wcscpy_s(szPath, _MAX_PATH, buf);
			}
		}
		wcscpy_s(pszFullPath, dwSize, szPath);
	}
	dwLen = (DWORD)wcslen(pszFullPath);

	return dwLen;
}

DWORD GetFullPathNameExA(LPCSTR pcszPath, LPSTR pszFullPath, DWORD dwSize)
{
	DWORD dwLen = 0;
	CHAR szPath[_MAX_PATH] = { 0 };
	if (::PathIsUNCA(pcszPath))
	{
		afxWARN(_T("Does not support UNC path.\n"));
		return 0;
	}

	if (!::PathIsRelativeA(pcszPath))
	{
		strcpy_s(pszFullPath, dwSize, pcszPath);
	}
	else
	{
		if (!::GetModuleFileNameA(GetCurrentModule(), szPath, _MAX_PATH))
		{
			if (!::GetModuleFileNameA(NULL, szPath, _MAX_PATH))
			{
				afxWARN(_T("Failed to get module path\n"));
				return 0;
			}
		}

		if (!::PathRemoveFileSpecA(szPath))
		{
			afxWARN(_T("PathRemoveFileSpec failed.\n"));
			return 0;
		}

		::PathAddBackslashA(szPath);
		strcat_s(szPath, _MAX_PATH, pcszPath);

		if (::PathIsRelativeA(pcszPath))
		{
			CHAR** lppPart = { NULL };
			CHAR buf[_MAX_PATH] = { 0 };

			if (::GetFullPathNameA(szPath, _MAX_PATH, buf, lppPart))
			{
				strcpy_s(szPath, _MAX_PATH, buf);
			}
		}
		strcpy_s(pszFullPath, dwSize, szPath);
	}
	dwLen = (DWORD)strlen(pszFullPath);

	return dwLen;
}

BOOL FileLikeA(LPCSTR lpPath)
{
	BOOL bFound = FALSE;
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = ::FindFirstFileA(lpPath, &ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		bFound = TRUE;

		::FindClose(hFind);
	}
	return bFound;
}

BOOL FileLikeW(LPCWSTR lpPath)
{
	BOOL bFound = FALSE;
	WIN32_FIND_DATAW ffd;
	HANDLE hFind = ::FindFirstFileW(lpPath, &ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		bFound = TRUE;

		::FindClose(hFind);
	}
	return bFound;
}

BOOL GetModuleVersionA(LPCSTR lpModule, unsigned short* pMajor, unsigned short* pMinor, unsigned short* pRevision, unsigned short* pBuild)
{
	BOOL bReturn = FALSE;
	DWORD dwHandle;
	DWORD dwSize = ::GetFileVersionInfoSizeA(lpModule, &dwHandle);
	if (dwSize > 0)
	{
		LPVOID lpMem = NULL;
		HANDLE hMem = NULL;
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
		if (hMem == NULL)
			return bReturn;

		lpMem = ::GlobalLock(hMem);
		if (lpMem == NULL)
		{
			::GlobalFree(hMem);
			return bReturn;
		}

		if (::GetFileVersionInfoA(lpModule, dwHandle, dwSize, lpMem))
		{
			VS_FIXEDFILEINFO* pFileInfo = NULL;
			UINT nSize = 0;
			BOOL bRet = ::VerQueryValueA(lpMem, "\\", (LPVOID*)&pFileInfo, &nSize);
			if (bRet)
			{
#ifdef WIN64
				*pMajor = (pFileInfo->dwFileVersionMS >> 16) & 0xff;
				*pMinor = (pFileInfo->dwFileVersionMS >> 0) & 0xff;
				*pRevision = (pFileInfo->dwFileVersionLS >> 16) & 0xff;
				*pBuild = (pFileInfo->dwFileVersionLS >> 0) & 0xff;
#else
				*pMajor = HIWORD(pFileInfo->dwFileVersionMS);
				*pMinor = LOWORD(pFileInfo->dwFileVersionMS);
				*pRevision = HIWORD(pFileInfo->dwFileVersionLS);
				*pBuild = LOWORD(pFileInfo->dwFileVersionLS);
#endif

				bReturn = TRUE;
			}
		}
		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
	return bReturn;
}

BOOL GetModuleVersionW(LPCWSTR lpModule, unsigned short* pMajor, unsigned short* pMinor, unsigned short* pRevision, unsigned short* pBuild)
{
	BOOL bReturn = FALSE;
	DWORD dwHandle;
	DWORD dwSize = ::GetFileVersionInfoSizeW(lpModule, &dwHandle);
	if (dwSize > 0)
	{
		LPVOID lpMem = NULL;
		HANDLE hMem = NULL;
		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
		if (hMem == NULL)
			return bReturn;

		lpMem = ::GlobalLock(hMem);
		if (lpMem == NULL)
		{
			::GlobalFree(hMem);
			return bReturn;
		}

		if (::GetFileVersionInfoW(lpModule, dwHandle, dwSize, lpMem))
		{
			VS_FIXEDFILEINFO* pFileInfo = NULL;
			UINT nSize = 0;
			BOOL bRet = ::VerQueryValueW(lpMem, L"\\", (LPVOID*)&pFileInfo, &nSize);
			if (bRet)
			{
#ifdef WIN64
				*pMajor = (pFileInfo->dwFileVersionMS >> 16) & 0xff;
				*pMinor = (pFileInfo->dwFileVersionMS >> 0) & 0xff;
				*pRevision = (pFileInfo->dwFileVersionLS >> 16) & 0xff;
				*pBuild = (pFileInfo->dwFileVersionLS >> 0) & 0xff;
#else
				*pMajor = HIWORD(pFileInfo->dwFileVersionMS);
				*pMinor = LOWORD(pFileInfo->dwFileVersionMS);
				*pRevision = HIWORD(pFileInfo->dwFileVersionLS);
				*pBuild = LOWORD(pFileInfo->dwFileVersionLS);
#endif

				bReturn = TRUE;
			}
		}
		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
	return bReturn;
}

// ------------------------------------------------------------------
// 半角・全角判定 (UNICODE)
// ------------------------------------------------------------------
BOOL CheckHalfWidth(LPCWSTR pszText)
{
	// 文字列の文字数
	size_t length = wcslen(pszText);

	// 文字情報の配列
	WORD *type = new WORD[length];

	BOOL bHalf = FALSE;
	// 文字情報を取得
	if (GetStringTypeExW(0, CT_CTYPE3, pszText, (int)length, type))
	{
		// 取得成功
		for (size_t i = 0; i < length; i++)
		{
			// 1文字毎に判定
			if ((type[i] & C3_FULLWIDTH) == C3_FULLWIDTH ||
				(type[i] & C3_IDEOGRAPH) == C3_IDEOGRAPH ||
				(type[i] & C3_HIRAGANA) == C3_HIRAGANA)
			{
				// 全角英数、全角カタカナ、全角スペース
				// 全角漢字
				// 全角ひらがな
				bHalf = FALSE;
				break;
			}

			if ((type[i] & C3_HALFWIDTH) == C3_HALFWIDTH ||
				(type[i] & C3_KATAKANA) == C3_KATAKANA)
			{
				// 半角英数、半角カタカナ、半角スペース
				// 半角カタカナ、全角カタカナ
				bHalf = TRUE;
				break;
			}
		}
	}
	else
	{
		// 取得失敗
	}

	// 文字情報の解放
	delete[] type;

	return bHalf;
}


BOOL isNumber(LPCWSTR pwszTxt, int off)
{
	LPWSTR lpEndPtr = NULL;
	// 数字？
	errno = 0;
	double dbchar = wcstod(pwszTxt, &lpEndPtr);
	if (dbchar != 0)
		return TRUE;

	if (!_IsEmptyW(lpEndPtr))
		return FALSE;

	if (dbchar == 0.0 && errno == 0)
		return TRUE;

	if (_wcsicmp(pwszTxt, L"0") == 0)
		return TRUE;

	if (off == 0)
	{
		if (_wcsicmp(pwszTxt, L"-") == 0 ||
			_wcsicmp(pwszTxt, L".") == 0)
			return TRUE;
	}
	else
	{
		if (_wcsicmp(pwszTxt, L",") == 0 ||
			_wcsicmp(pwszTxt, L".") == 0)
			return TRUE;
	}

	return FALSE;
}

BOOL isValidNumber(LPCWSTR lpszNums)
{
	int cntDot = 0;
	int cntMinus = 0;
	int posCanma = 0;

	size_t i = 0;
	size_t len = wcslen(lpszNums);
	if (len > 1)
	{
		if (len > 2)
		{
			// 0.1234以外
			if (lpszNums[0] == L'0' &&
				lpszNums[1] != L'.')
				return FALSE;

			if (lpszNums[0] == L'-' &&
				!iswdigit(lpszNums[1]))
				return FALSE;

			if (lpszNums[0] == L'+' &&
				!iswdigit(lpszNums[1]))
				return FALSE;
		}
		// 061222
		if (lpszNums[0] == L'0' &&
			lpszNums[1] != L'.')
			return FALSE;
	}

	while (i < len)
	{
		if (!iswdigit(lpszNums[i]))
		{
			// 符号のみ？
			if (len == 1)
				return FALSE;

			if (lpszNums[i] == L'.')
			{
				if (i == 0)
					return FALSE;
				if (cntDot > 0)
					return FALSE;

				cntDot++;
			}
			else if (lpszNums[i] == L'-')
			{
				if (i != 0)
					return FALSE;
				if (cntMinus > 0)
					return FALSE;

				cntMinus++;
			}
			else if (lpszNums[i] == L'+')
			{
				if (i != 0)
					return FALSE;
				if (cntMinus > 0)
					return FALSE;

				cntMinus++;
			}
			else if (lpszNums[i] == L',')
			{
				if (i == 0 ||
					(i == len - 1))
					return FALSE;
				if (posCanma != 0 &&
					posCanma != (i - 4))
					return FALSE;

				posCanma = (int)i;
			}
			else
			{
				return FALSE;
			}
		}

		i++;
	}

	return TRUE;
}

BOOL isNumber(LPWSTR lpszNums)
{
	int count = 0;
	LPCWSTR lpSub = NULL;
	LPWSTR lpNextToken = NULL;

	// データチェック
	lpSub = wcstok_s(lpszNums, L" ", &lpNextToken);
	while (lpSub != NULL)
	{
		// 符号チェック
		if (!isValidNumber(lpSub))
			return FALSE;

		lpSub = wcstok_s(NULL, L" ", &lpNextToken);
	}

	// サイズチェック
	lpSub = wcschr(lpszNums, L'.');
	if (lpSub != NULL)
	{
		// 小数点前１５桁まで
		int pos = (int)(lpSub - lpszNums);
		if (pos > 15)
			return FALSE;
	}
	else
	{
		size_t slen = wcslen(lpszNums);
		// double 15桁
		if (slen > 40)
			return FALSE;
	}

	return TRUE;
}

double roundd(double x)
{
	if (x > 0.0)
		return floor(x + 0.5);
	else
		return -1.0 * floor(fabs(x) + 0.5);
}

} // namespace UTL