#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#pragma once

#include "cstring.h"

#define STACKDUMP_SIZE		256
#define MAX_STACKDUMP_SIZE	8192

namespace UTL {

class CUnhandledException {
public:
	CUnhandledException();
	virtual ~CUnhandledException();

	BOOL InstallExceptionFilter(LPCTSTR title, LPCTSTR info, LPCTSTR prefix = NULL);

	CString& GetTitle()   { return m_szTitle; }
	CString& GetLogFile() { return m_szLogFile; }
	CString& GetLogInfo() { return m_szLogInfo; }

private:
	CString m_szTitle;
	CString m_szLogFile;
	CString m_szLogInfo;
};

extern CUnhandledException _exception;


class CException {
private:
	DWORD	m_dwErrorCode;
	CString	m_szErrorMessage;
	BOOL	m_bAutoDelete;
#ifdef _DEBUG
	BOOL	m_bReadyForDelete;
#endif
public:
	CException();
	explicit CException(BOOL bAutoDelete);
	explicit CException(DWORD dwErrorCode);
	virtual ~CException();

	const CException& operator=(DWORD dwCode) {
		m_dwErrorCode = dwCode;
		return *this;
	}

	const CException& operator=(LPCTSTR pszMsg) {
		m_szErrorMessage = pszMsg;
		m_dwErrorCode = 0;

		return *this;
	}

	// return pointer to const string
	operator const DWORD() {
		return this->GetErrorCode();
	}

	// Operations
	void Delete();  // use to delete exception in 'catch' block

public:
	virtual DWORD GetErrorCode();
	virtual LPCTSTR GetErrorMessage();
};

// Placed on frame for EXCEPTION linkage, or CException cleanup
struct _CEXCEPTION_LINK
{
	CException* m_pException;   // current exception (NULL in TRY block)

	_CEXCEPTION_LINK();
	~_CEXCEPTION_LINK();
};

#define _afxTRY { _CEXCEPTION_LINK _afxLink; \
	CException _afxExcept(FALSE); \
	try {
#define _afxCATCH(class, e) } catch (class* e) \
	{ _afxLink.m_pException = e;
#define _afxCATCH_ALL } catch (...) \
	{ _afxExcept = ::GetLastError(); \
	if(_afxExcept == 0) _afxExcept = errno; \
	_afxLink.m_pException = &_afxExcept;
#define _afxEND } }
#define _afxTHROW(e) { _afxExcept = e; throw &_afxExcept; }
#define _afxThrowException(e) { throw new CException(e); }
#define _afxExceptionCode _afxExcept

} // namespace UTL

#endif
