#include "stdafx.h"
#include "Exception.h"
#include <DbgHelp.h>
#include <Shlwapi.h>

namespace UTL {

CUnhandledException _exception;

#define EXCEPTION_LOG		_T("exception.log")

#define MSG_LENGTH	512
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

void CreateMiniDump(EXCEPTION_POINTERS* pExcPtrs)
{
	if (pExcPtrs == NULL)
		return;

	TCHAR dump[_MAX_PATH] = { 0 };
	TCHAR modName[_MAX_PATH] = { 0 };
	HMODULE hModule = NULL;
	HANDLE hFile = NULL;
	MINIDUMP_EXCEPTION_INFORMATION mei;
	MINIDUMP_CALLBACK_INFORMATION  mci;

	hModule = ::LoadLibrary(_T("dbghelp.dll"));
	if (hModule == NULL)
		return;

	// MiniDumpWriteDump
	typedef BOOL(WINAPI *LPMINIDUMPWRITEDUMP)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump =
		(LPMINIDUMPWRITEDUMP)GetProcAddress(hModule, "MiniDumpWriteDump");
	if (!pfnMiniDumpWriteDump)
	{
		// Bad MiniDumpWriteDump function
		::FreeLibrary(hModule);

		return;
	}

	BOOL bFullName = FALSE;
	if (::GetModuleFileName((HINSTANCE)&__ImageBase, modName, _countof(modName))) {

		TCHAR* pt = _tcsrchr(modName, _T('\\'));
		if (pt != NULL) {
			pt = _tcsinc(pt);
			*pt = _T('\0');

			_stprintf_s(dump, _MAX_PATH, _T("%s\\crashdump_%d.dmp"), modName, ::GetCurrentProcessId());
			bFullName = TRUE;
		}
	}

	if (!bFullName) {
		_stprintf_s(dump, _MAX_PATH, _T("crashdump_%d.dmp"), ::GetCurrentProcessId());
	}

	afxERROR(_T("MiniDumpWriteDump --> Dump filename: %s"), dump);

	hFile = ::CreateFile(dump,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::FreeLibrary(hModule);

		return;
	}

	// Write minidump to file
	mei.ThreadId = ::GetCurrentThreadId();
	mei.ExceptionPointers = pExcPtrs;
	mei.ClientPointers = FALSE;
	mci.CallbackRoutine = NULL;
	mci.CallbackParam = NULL;

	HANDLE hProcess = GetCurrentProcess();
	DWORD dwProcessId = GetCurrentProcessId();

	BOOL bWriteDump = pfnMiniDumpWriteDump(
		hProcess,
		dwProcessId,
		hFile,
		MiniDumpNormal,
		&mei,
		NULL,
		&mci);

	// Close file
	::CloseHandle(hFile);

	// Unload dbghelp.dll
	::FreeLibrary(hModule);
}

CStringA FormatExceptionMessage(PEXCEPTION_RECORD pRecord)
{
	CStringA str;
	CStringA pFmt;
	CHAR msg[MSG_LENGTH + 1] = { 0 };
	DWORD_PTR dwAddress = 0;
	DWORD_PTR dwCode = 0;

	if (pRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		if (pRecord->NumberParameters == 2)
		{
			if (pRecord->ExceptionInformation[0] == 0)
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Access violation reading 0x%08Ix.\r\n";
			else if (pRecord->ExceptionInformation[0] == 8)
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Access violation DEP 0x%08Ix.\r\n";
			else
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Access violation writing 0x%08Ix.\r\n";

			dwAddress = pRecord->ExceptionInformation[1];
		}
		else
			pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Access violation\r\n";

		sprintf_s(msg, MSG_LENGTH, pFmt.GetBuffer(), pRecord->ExceptionAddress, dwAddress);

		str = msg;
	}
	else if (pRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
	{
		if (pRecord->NumberParameters == 3)
		{
			if (pRecord->ExceptionInformation[0] == 0)
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000006: Page fault reading 0x%08Ix with code 0x%08Ix.\r\n";
			else if (pRecord->ExceptionInformation[0] == 8)
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Page fault DEP 0x%08Ix with code 0x%08Ix.\r\n";
			else
				pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Page fault writing 0x%08Ix with code 0x%08Ix.\r\n";

			dwAddress = pRecord->ExceptionInformation[1];
			dwCode = pRecord->ExceptionInformation[3];
		}
		else
			pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0xC0000005: Page fault";

		sprintf_s(msg, MSG_LENGTH, pFmt.GetBuffer(), pRecord->ExceptionAddress, dwAddress, dwCode);

		str = msg;
	}
	else
	{
		LPSTR pMessage = NULL;
		int len = ::FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			::GetModuleHandleA("ntdll.dll"),
			pRecord->ExceptionCode,
			0,
			(LPSTR)&pMessage,
			0,
			NULL);
		if (pMessage)
			pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0x%08x: %s\r\n";
		else
			pFmt = "Unhandled exception at 0x%08Ix:\r\nCode 0x%08x\r\n";

		sprintf_s(msg, MSG_LENGTH, pFmt.GetBuffer(), pRecord->ExceptionAddress, pRecord->ExceptionCode, pMessage);

		str = msg;

		if (pMessage)
			::LocalFree(pMessage);
	}

	return str;
}

LONG WINAPI _exceptionFilter(struct _EXCEPTION_POINTERS *info)
{
	//CHAR		buf[MAX_STACKDUMP_SIZE];
	HANDLE		hFile = INVALID_HANDLE_VALUE;
	SYSTEMTIME	tm;
	CONTEXT		*context;
	DWORD		len;
	//DWORD		i, j;
	//CHAR		*stack;
	CHAR		*esp;
	CStringA	szMsg;
	CStringA	szLogFile;
	CStringA	szLogInfo;
	CStringA	szTitle;

	szTitle = _W2A(_exception.GetTitle());
	szLogFile = _W2A(_exception.GetLogFile());
	szLogInfo = _W2A(_exception.GetLogInfo());

	afxTRACE(_T("_exceptionFilter"));

	// Dump
	CreateMiniDump(info);

	// Create exception file
	hFile = ::CreateFileA(szLogFile, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//error
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	afxTRACE(_T("Exception log file: %s"), _AA2W(szLogFile));

	::SetFilePointer(hFile, 0, 0, FILE_END);
	::GetLocalTime(&tm);
	context = info->ContextRecord;

	CString szModule = (LPCTSTR)GetCurrentModuleName();
	//len = sprintf_s(buf, MAX_STACKDUMP_SIZE,
	szMsg.Format(
		"------ %s <%s> -----\r\n"
		" Date        : %d/%02d/%02d %02d:%02d:%02d\r\n"
		" Code/Addr   : 0x%08x / %p\r\n"
#ifdef _WIN64
		" AX/BX/CX/DX : %p / %p / %p / %p\r\n"
		" SI/DI/BP/SP : %p / %p / %p / %p\r\n"
		" 08/09/10/11 : %p / %p / %p / %p\r\n"
		" 12/13/14/15 : %p / %p / %p / %p\r\n"
#else
		" AX/BX/CX/DX : %08x / %08x / %08x / %08x\r\n"
		" SI/DI/BP/SP : %08x / %08x / %08x / %08x\r\n"
#endif
		"\r\n",
		szTitle.GetBuffer(),
		_W2A(szModule.GetBuffer()),
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond,
		info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress,
#ifdef _WIN64
		context->Rax, context->Rbx, context->Rcx, context->Rdx,
		context->Rsi, context->Rdi, context->Rbp, context->Rsp,
		context->R8, context->R9, context->R10, context->R11,
		context->R12, context->R13, context->R14, context->R15
#else
		context->Eax, context->Ebx, context->Ecx, context->Edx,
		context->Esi, context->Edi, context->Ebp, context->Esp
#endif
		);

	//	len = _stprintf_s(buf, MAX_STACKDUMP_SIZE,
	//		_T("------ %s -----\r\n")
	//		_T(" Date        : %d/%02d/%02d %02d:%02d:%02d\r\n")
	//		_T(" Code/Addr   : %X / %p\r\n")
	//#ifdef _WIN64
	//		_T(" AX/BX/CX/DX : %p / %p / %p / %p\r\n")
	//		_T(" SI/DI/BP/SP : %p / %p / %p / %p\r\n")
	//		_T(" 08/09/10/11 : %p / %p / %p / %p\r\n")
	//		_T(" 12/13/14/15 : %p / %p / %p / %p\r\n")
	//#else
	//		_T(" AX/BX/CX/DX : %08x / %08x / %08x / %08x\r\n")
	//		_T(" SI/DI/BP/SP : %08x / %08x / %08x / %08x\r\n")
	//#endif
	//		_T("------- stack info -----\r\n"),
	//		_Exception.GetTitle(),
	//		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond,
	//		info->ExceptionRecord->ExceptionCode, info->ExceptionRecord->ExceptionAddress,
	//#ifdef _WIN64
	//		context->Rax, context->Rbx, context->Rcx, context->Rdx,
	//		context->Rsi, context->Rdi, context->Rbp, context->Rsp,
	//		context->R8,  context->R9,  context->R10, context->R11,
	//		context->R12, context->R13, context->R14, context->R15
	//#else
	//		context->Eax, context->Ebx, context->Ecx, context->Edx,
	//		context->Esi, context->Edi, context->Ebp, context->Esp
	//#endif
	//		);

	::WriteFile(hFile, szMsg.GetBuffer(), (DWORD)szMsg.GetLength(), &len, 0);

	// Error Message
	szMsg = FormatExceptionMessage(info->ExceptionRecord);
	::WriteFile(hFile, szMsg.GetBuffer(), (DWORD)szMsg.GetLength(), &len, 0);

#ifdef _WIN64
	esp = (CHAR *)context->Rsp;
#else
	esp = (CHAR *)context->Esp;
#endif
	// Stack info
	szMsg = "\r\n------- stack info -----\r\n";
	::WriteFile(hFile, szMsg.GetBuffer(), (DWORD)szMsg.GetLength(), &len, 0);

	//for (i=0; i < MAX_STACKDUMP_SIZE / STACKDUMP_SIZE; i++)
	//{
	//	szMsg.Empty();
	//	stack = esp + (i * STACKDUMP_SIZE);

	//	if (::IsBadReadPtr(stack, STACKDUMP_SIZE))
	//		break;

	//	len = 0;
	//	for (j=0; j < STACKDUMP_SIZE / sizeof(DWORD_PTR); j++)
	//	{
	//		sprintf_s(buf, 
	//				MAX_STACKDUMP_SIZE,
	//				"%p%s",
	//				((DWORD_PTR *)stack)[j],
	//				((j+1)%(32/sizeof(DWORD_PTR))) ? " " : "\r\n");
	//		//_stprintf_s(buf, 
	//		//			MAX_STACKDUMP_SIZE,
	//		//			_T("%p%s"),
	//		//			((DWORD_PTR *)stack)[j],
	//		//			((j+1)%(32/sizeof(DWORD_PTR))) ? _T(" ") : _T("\r\n"));
	//		szMsg += buf;
	//	}

	//	::WriteFile(hFile, szMsg.GetBuffer(), (DWORD)szMsg.GetLength(), &len, 0);
	//}

	szMsg = "------------------------\r\n\r\n";
	::WriteFile(hFile, szMsg.GetBuffer(), (DWORD)szMsg.GetLength(), &len, 0);
	::CloseHandle(hFile);

	//	sprintf_s(buf, MAX_STACKDUMP_SIZE, szLogInfo.GetBuffer(), szLogFile.GetBuffer());
	//#ifdef _DEBUG
	//	::MessageBoxA(0, buf, szTitle, MB_OK | MB_ICONERROR);
	//#endif
	//_stprintf_s(buf, MAX_STACKDUMP_SIZE, _Exception.GetLogInfo(), _Exception.GetLogFile());
	//::MessageBoxA(0, buf, szTitle, MB_OK | MB_ICONERROR);

	return	EXCEPTION_EXECUTE_HANDLER;
}

CUnhandledException::CUnhandledException()
{
	m_szTitle.Empty();
	m_szLogFile.Empty();
	m_szLogInfo.Empty();
}

CUnhandledException::~CUnhandledException()
{
}

BOOL CUnhandledException::InstallExceptionFilter(LPCTSTR title, LPCTSTR info, LPCTSTR prefix)
{
	CString szPath = GetCurrentModulePath();
	if (!szPath.IsEmpty()) {
		if (!_IsEmpty(prefix))
			szPath += prefix;
		szPath += EXCEPTION_LOG;
	}

	m_szTitle = title;
	m_szLogFile = szPath;
	m_szLogInfo = info;

	::SetUnhandledExceptionFilter(&_exceptionFilter);

	return	TRUE;
}

// ----------------------------------------------------------------------------
// _CEXCEPTION_LINK
// ----------------------------------------------------------------------------
_CEXCEPTION_LINK::_CEXCEPTION_LINK() {
	m_pException = NULL;
}

_CEXCEPTION_LINK::~_CEXCEPTION_LINK() {
	if (m_pException != NULL) {
		m_pException->Delete();
	}
}

// ----------------------------------------------------------------------------
// CException class
// ----------------------------------------------------------------------------
CException::CException()
	: m_dwErrorCode(0)
	, m_bAutoDelete(TRUE)
{
#ifdef _DEBUG
	m_bReadyForDelete = FALSE;
#endif
}

CException::CException(BOOL bAutoDelete)
	: m_dwErrorCode(0)
	, m_bAutoDelete(bAutoDelete)
{
#ifdef _DEBUG
	m_bReadyForDelete = FALSE;
#endif
}

CException::CException(DWORD dwErrorCode)
	: m_dwErrorCode(dwErrorCode)
	, m_bAutoDelete(TRUE)
{
#ifdef _DEBUG
	m_bReadyForDelete = FALSE;
#endif
}

void CException::Delete()
{
	if (m_bAutoDelete > 0) {
#ifdef _DEBUG
		m_bReadyForDelete = TRUE;
#endif
		delete this;
	}
}

CException::~CException() {
}

DWORD CException::GetErrorCode() {
	return m_dwErrorCode;
}

LPCTSTR CException::GetErrorMessage() {
	if (m_dwErrorCode != ERROR_SUCCESS)
		m_szErrorMessage = GetWin32Message(m_dwErrorCode);
	return (LPCTSTR)m_szErrorMessage;
}

} // namespace UTL
