#ifndef __SWAPFILE_H__
#define __SWAPFILE_H__
#pragma once

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif
#include <stdlib.h>

#define KEY_BUFFSIZE		64

// ----------------------------------------------------------------------------
// File Format
// index struct
typedef struct __cacheFileIndex
{
	long		key;					// key identifier 
	size_t		size;					// cached data's size
	size_t		position;				// pointer position
	size_t		block;					// numbers per block
	LinkedList<LPVOID>*		pLnkRec;	// cached data
} CACHEFILEINDEX, *LPCACHEFILEINDEX;

// ------------------------------------------------------------------
// function for HashTbl<ULONG, LPRECORD>
// ------------------------------------------------------------------
struct _freeCFIP
{
	void operator()(LONG key, LPCACHEFILEINDEX lpRec) const
	{
		if (lpRec != NULL) {
			_DELETE(lpRec->pLnkRec);
		}
		_FREE(lpRec);
	}
};
typedef HashTbl<LONG, LPCACHEFILEINDEX, _freeCFIP>	CHashTblCachedData;

class CSwapFile
{
public:
	CSwapFile(void);
	~CSwapFile(void);

public:
	bool Open();
	bool Close();

	long GetDataKey(size_t size, size_t block = 32);
	
	// get Data Count
	long GetDataCount(long datakey);

	// Write Data
	size_t Write(void* pData, size_t size, int count, long dataKey);

	long ReadData(long dataKey, void** pData, LPDWORD pLength = NULL);
	long Read(long dataKey, void** pData, LPDWORD pLength = NULL);
	long Read(long dataKey, size_t index, void** pData, LPDWORD pLength = NULL);
	bool MoveFirst(long dataKey);

	DWORD GetErrorCode() { return m_dwErrorCode; }

private:
	// error
	DWORD	m_dwErrorCode;

	CHashTblCachedData*		m_pCachedData;
};

#endif
