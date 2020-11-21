#ifndef __TRACE_H__
#define __TRACE_H__
#pragma once

#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include "Utils.h"
#include "cstring.h"
#include "MemFile.h"
#include "LogMap.h"

#define TRACE_ALL		6
#define TRACE_DEBUG		5
#define TRACE_INFO		4
#define TRACE_WARN		3
#define TRACE_ERROR		2
#define TRACE_FATAL		1
#define TRACE_NONE		0

namespace UTL {

template<typename T> struct traitslog;
// ------------------------------------------------------------------
// function for char
// ------------------------------------------------------------------
template<> struct traitslog<char> {
	static void vsnprintf(char* buffer, size_t sizeOfBuffer, const char* format, va_list argptr) {
		vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr);
	}

	static void write(CMemoryFile* pFile, char* buffer) {
		if (pFile != NULL) {
			// Time 
			CStringA tm;
			time_t ltime;
			struct tm createtime;
			time(&ltime);

			// Convert to local time
			errno_t terr = localtime_s(&createtime, &ltime);
			if (terr == 0)
			{
				// ext. 2013/08/09 11:09:21
				tm.Format("%04d/%02d/%02d %02d:%02d:%02d [%04d]-[%04d] ",
					createtime.tm_year + 1900,
					createtime.tm_mon + 1,
					createtime.tm_mday,
					createtime.tm_hour,
					createtime.tm_min,
					createtime.tm_sec,
					::GetCurrentProcessId(),
					::GetCurrentThreadId()
					);
			}
			if (!tm.IsEmpty()) {
				pFile->WriteFile(tm.GetBuffer(), (UINT)tm.GetLength());
			}

			// Write
			pFile->WriteFile(buffer, (UINT)length(buffer));

			// return
			pFile->WriteFile("\r\n", (UINT)2);
		}
	}

	static size_t length(char* buffer) {
		return strlen(buffer);
	}

	static void output_debug_string(char* msg, const char* file, long line) {
		size_t len = strlen(msg);
		if (len > 0) {
			::OutputDebugStringA(msg);
			::OutputDebugStringA("\n");
			//#ifdef _DEBUG
			printf(msg);
			printf("\n");
			//#endif
		}
	}
};

// ------------------------------------------------------------------
// function for wchar_t
// ------------------------------------------------------------------
template<> struct traitslog<wchar_t> {
	static void vsnprintf(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, va_list argptr) {
		_vsnwprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr);
	}

	static void write(CMemoryFile* pFile, wchar_t* buffer) {
		char buf[4096] = { 0 };
		strcpy_s(buf, 4096, _CW2A(buffer));
		if (pFile != NULL) {
			// Time 
			CStringA tm;
			time_t ltime;
			struct tm createtime;
			time(&ltime);

			// Convert to local time
			errno_t terr = localtime_s(&createtime, &ltime);
			if (terr == 0)
			{
				// ext. 2013/08/09 11:09:21
				tm.Format("%04d/%02d/%02d %02d:%02d:%02d [%04d]-[%04d] ",
					createtime.tm_year + 1900,
					createtime.tm_mon + 1,
					createtime.tm_mday,
					createtime.tm_hour,
					createtime.tm_min,
					createtime.tm_sec,
					::GetCurrentProcessId(),
					::GetCurrentThreadId()
					);
			}
			if (!tm.IsEmpty()) {
				pFile->WriteFile(tm.GetBuffer(), (UINT)tm.GetLength());
			}
			// Write
			size_t len = strlen(buf);
			pFile->WriteFile(buf, (UINT)len);

			// return
			pFile->WriteFile("\r\n", (UINT)2);
		}
	}

	static size_t length(wchar_t* buffer) {
		return wcslen(buffer);
	}

	static void output_debug_string(wchar_t* msg, const wchar_t* file, long line) {
		size_t len = wcslen(msg);
		if (len > 0) {
			::OutputDebugStringW(msg);
			::OutputDebugStringW(L"\n");
			//#ifdef _DEBUG
			wprintf(msg);
			wprintf(L"\n");
			//#endif
		}
	}

};

template<class Type>
class afxtrace_output_debug_stringT {
public:
	afxtrace_output_debug_stringT(int logType, Type* pszName = NULL, long line = 0)
		: nLogType(logType)
		, pszFile(pszName)
		, nLine(line)
	{
	}

	void operator() (const int level) {
		CLogMap* pLogMap = CLogMap::getInstance();
		if (pLogMap != NULL) {
			pLogMap->SetLogLevel(level);
		}
	}

	void operator() (const Type*  lpszFormat, ...) {
		// Default Log Level
		int nLogLevel = TRACE_ERROR;
		// Get log level
		CLogMap* pLogMap = CLogMap::getInstance();
		if (pLogMap != NULL) {
			nLogLevel = pLogMap->GetLogLevel();
		}
		// no output
		if (nLogLevel <= 0 || nLogType > nLogLevel) {
			return;
		}

		Type buf[BUFFER_SIZE] = { 0 };

		va_list list;
		va_start(list, lpszFormat);

		// Format
		traitslog<Type>::vsnprintf(buf, BUFFER_SIZE, lpszFormat, list);

		va_end(list);

		// Write to memory
		size_t len = traitslog<Type>::length(buf);
		if (len > 0) {
			CMemoryFile* pMem = pLogMap->GetMemFile();
			if (pMem != NULL) {
				// Check Size (37 : 2013/08/09 11:09:21 [0000]-[0000] + '\r\n')
				if (pMem->GetPosition() + len + 37 >= pMem->GetLength()) {
					// Write to File
					pLogMap->WriteToFile();
				}
				// Write to Buffer
				traitslog<Type>::write(pMem, buf);
			}

			// Output
			traitslog<Type>::output_debug_string(buf, (const Type*)pszFile, nLine);
		}
	}

	void operator() (DWORD dwMsgCode, ...)
	{
		// Default Log Level
		int nLogLevel = TRACE_ERROR;
		// Get log level
		CLogMap* pLogMap = CLogMap::getInstance();
		if (pLogMap != NULL) {
			nLogLevel = pLogMap->GetLogLevel();
		}
		// no output
		if (nLogLevel <= 0 || nLogType > nLogLevel) {
			return;
		}

		CMsgMap* pMsg = CMsgMap::getInstance();
		if (!pMsg)
		{
			return;
		}
		DWORD dwErrorCode = ::GetLastError();
		LPCONVMSG lpMsg = pMsg->GetMsg(dwMsgCode);
		if (lpMsg == NULL)
		{
			return;
		}

		Type buf[BUFFER_SIZE] = { 0 };

		va_list list;
		va_start(list, dwMsgCode);

		// Format
		traitslog<Type>::vsnprintf(buf, BUFFER_SIZE, lpMsg->szMsg, list);

		va_end(list);

		// Write to memory
		size_t len = traitslog<Type>::length(buf);
		if (len > 0 ) {
			CMemoryFile* pMem = pLogMap->GetMemFile();
			if (pMem != NULL) {
				// Check Size (37 : 2013/08/09 11:09:21 [0000]-[0000] + '\r\n')
				if (pMem->GetPosition() + len + 37 >= pMem->GetLength()) {
					// Write to File
					pLogMap->WriteToFile();
				}
				// Write to Buffer
				traitslog<Type>::write(pMem, buf);
			}

			// Output
			traitslog<Type>::output_debug_string(buf, (const Type*)pszFile, nLine);
		}
	}
private:
	int		nLogType;
	Type*	pszFile;
	int		nLine;

	enum {
		BUFFER_SIZE = 4096
	};
};

// dummy trace
inline void __DUMMY_TRACE__(...) {}

} // namespace UTL

// ------------------------------------------------------------------
// function for char
// ------------------------------------------------------------------
#define afxTRACEA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_ALL, __FILE__, __LINE__))
#define afxDEBUGA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_DEBUG, __FILE__, __LINE__))
#define afxWARNA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_WARN, __FILE__, __LINE__))
#define afxERRORA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_ERROR, __FILE__, __LINE__))
#define afxINFOA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_INFO, __FILE__, __LINE__))
#define afxFATALA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_FATAL, __FILE__, __LINE__))
#define afxLEVELA	(UTL::afxtrace_output_debug_stringT<char>(TRACE_NONE, __FILE__, __LINE__))

#define __WLINE__ TOWL1(__LINE__)
#define TOWL1(p) TOWL2(p)
#define TOWL2(p) L#p

// ------------------------------------------------------------------
// function for wchar_t
// ------------------------------------------------------------------
#define afxTRACEW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_ALL, __WLINE__, __LINE__))
#define afxDEBUGW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_DEBUG, __WLINE__, __LINE__))
#define afxWARNW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_WARN, __WLINE__, __LINE__))
#define afxERRORW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_ERROR, __WLINE__, __LINE__))
#define afxINFOW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_INFO, __WLINE__, __LINE__))
#define afxFATALW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_FATAL, __WLINE__, __LINE__))
#define afxLEVELW	(UTL::afxtrace_output_debug_stringT<wchar_t>(TRACE_NONE, __WLINE__, __LINE__))

#ifdef _UNICODE
#define afxTRACE	afxTRACEW
#define afxDEBUG	afxDEBUGW
#define afxWARN		afxWARNW
#define afxERROR	afxERRORW
#define afxINFO		afxINFOW
#define afxFATAL	afxFATALW
#define afxLEVEL	afxLEVELW
#else
#define afxTRACE	afxTRACEA
#define afxDEBUG	afxDEBUGA
#define afxWARN		afxWARNA
#define afxERROR	afxERRORA
#define afxINFO		afxINFOA
#define afxFATAL	afxFATALA
#define afxLEVEL	afxLEVELA
#endif

#endif
