#ifndef __MEMFILE_H__
#define __MEMFILE_H__
#pragma once

#include <windows.h>
#include <stdio.h>

#define LOG_MAX_LENGTH2G		(2147483648L)		// 2G
#define BYTE_4K					4096				//(4 * 1024)
#define BYTE_16K				16384				//(16 * 1024)
#define BYTE_32K				32768				//(32 * 1024)
#define BYTE_64K				65536				//(64 * 1024)
#define BYTE_128K				131072				//(128 * 1024)
#define BYTE_256K				262144				//(256 * 1024)
#define BYTE_512K				524288				//(512 * 1024)
#define BYTE_1M					1048576				//(1024 * 1024)
#define BYTE_4M					4194304				//(4* 1024 * 1024)

class CMemoryFile
{
public:
	// Constructors
	explicit CMemoryFile();
	explicit CMemoryFile(UINT nGrowBytes);

	// Advanced Overridables
protected:
	virtual BYTE* Alloc(SIZE_T nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, SIZE_T nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, SIZE_T nBytes);
	virtual void Free(BYTE* lpMem);
	virtual BOOL GrowFile(SIZE_T dwNewLen);

	// Implementation
protected:
	SIZE_T	m_nGrowBytes;
	SIZE_T	m_nPosition;
	SIZE_T	m_nBufferSize;
	SIZE_T	m_nFileSize;
	SIZE_T	m_nUsedSize;

	BYTE*	m_lpBuffer;
	BOOL	m_bAutoDelete;

	TCHAR	m_szFileName[_MAX_PATH];

	USHORT			m_nRecLen;

public:
	virtual ~CMemoryFile();

	virtual LPCTSTR GetFileName() const;
	virtual void SetFileName(LPCTSTR pszFileName);
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom = FILE_BEGIN);
	virtual BOOL SetLength(ULONGLONG dwNewLen);
	virtual ULONGLONG GetLength() const;
	virtual BYTE* GetBuffer() const;
	virtual ULONGLONG GetUsedLength() const;

	virtual BOOL ReadFile(LPVOID lpBuf, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead = NULL);
	virtual BOOL WriteFile(LPCVOID lpBuf, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten = NULL);

	virtual void Close();
	virtual void Clear();
	virtual BOOL Save(HANDLE hHandle, LONGLONG llMaxFileSize = LOG_MAX_LENGTH2G);
	virtual BOOL Save(FILE* pf, LONGLONG llMaxFileSize = LOG_MAX_LENGTH2G);
};

#endif
