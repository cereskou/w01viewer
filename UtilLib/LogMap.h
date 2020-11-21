#ifndef __LOGMAP_H__
#define __LOGMAP_H__
#pragma once

#include "MemFile.h"

#define _MAX_PREFIX		32
#define LOG_EXT			_T("log")
namespace UTL {

class CLogMap
{
public:
#ifdef _USRDLL
	static void destroy();
#endif
	static CLogMap* getInstance();
	static LONG volatile getGlobalIndex();

private:
	CLogMap();
	~CLogMap();

public:
	void FlushMemeFile(HANDLE hHandle);
	void ClearMemFile();
	void SaveMemFile(HANDLE hHandle, BOOL bClose = TRUE);
	void SaveMemFile(FILE* pf, BOOL bClose = TRUE);

	CMemoryFile* GetMemFile(BOOL bCreate = TRUE);

	DWORD GetLastErrorCode();

	// Reserved Integer
	void SetLogLevel(int newValue);
	int GetLogLevel();

	// Log file
	void SetLogFile(LPCTSTR pszPath, LPCTSTR pszPrefix = NULL, LPCTSTR pszExt = LOG_EXT, BOOL bDate = TRUE);
	CString GetLogFile();
	void SetMemorySize(ULONG ulSize);
	BOOL MakeLogFile();

	void WriteToFile(BOOL bClose = FALSE);

	// for Used Memory
	void SetAllocMemory(SIZE_T stSize);
	void SetFreeMemory(SIZE_T stSize);
	SIZE_T GetAllocedMemory();
	void ClearMemoryCount();

	void Lock();
	void Unlock();
protected:
	void Clear();

protected:
	CMemoryFile*	pMemFile;
	SIZE_T		m_WorkingSetSize;
	DWORD		m_dwErrorCode;
	int			m_nLogLevel;
	BOOL		m_bLogDate;		// Append Date?
	TCHAR		m_szLogPath[_MAX_PATH];
	TCHAR		m_szLogPrefix[_MAX_PATH];
	TCHAR		m_szLogExt[_MAX_EXT];
	ULONG		m_ulMemSize;

private:
	CRITICAL_SECTION pCriticalSection;
	static LONG volatile nGlobalIndex;
#ifdef _USRDLL
	static CLogMap* volatile pInstance;
#endif
};

} // namespace UTL

#endif
