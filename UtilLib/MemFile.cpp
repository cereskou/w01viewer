#include "stdafx.h"
#include <cassert>
#include "MemFile.h"

////////////////////////////////////////////////////////////////////////////
// Function
BOOL IsValidAddress(const void* p, size_t nBytes, BOOL bReadWrite = TRUE) {
	(bReadWrite);
	(nBytes);
	return (p != NULL);
}

////////////////////////////////////////////////////////////////////////////
// CMemoryFile implementation
CMemoryFile::CMemoryFile()
{
	m_nGrowBytes = BYTE_64K;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_nUsedSize = 0;
	m_lpBuffer = NULL;
	m_bAutoDelete = TRUE;

	m_szFileName[0] = 0x0;
	// ディフォルトは64kバッファー
	GrowFile(m_nGrowBytes);
}

CMemoryFile::CMemoryFile(UINT nGrowBytes)
{
	assert(nGrowBytes <= UINT_MAX);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_nUsedSize = 0;
	m_lpBuffer = NULL;
	m_bAutoDelete = TRUE;

	m_szFileName[0] = 0x0;
	// ディフォルトは64kバッファー
	GrowFile(m_nGrowBytes);
}

CMemoryFile::~CMemoryFile()
{
	// Close should have already been called, but we check anyway
	if (m_lpBuffer)
		Close();
	assert(m_lpBuffer == NULL);

	m_nGrowBytes = 0;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_nUsedSize = 0;
}

// ----------------------------------------------------------------------------
// 関　数：Close
// 説　明：クローズ処理（メモリ解放）
//         
// 引　数：
// 
// 戻り値：
//         
// 備　考：
// ----------------------------------------------------------------------------
void CMemoryFile::Close()
{
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_nUsedSize = 0;
	if (m_lpBuffer && m_bAutoDelete)
		Free(m_lpBuffer);
	m_lpBuffer = NULL;
}

// ----------------------------------------------------------------------------
// 関　数：Clear
// 説　明：クローズ処理（メモリクリアする）
//         
// 引　数：
// 
// 戻り値：
//         
// 備　考：
// ----------------------------------------------------------------------------
void CMemoryFile::Clear()
{
	m_nPosition = 0;
	m_nUsedSize = 0;
	ZeroMemory(m_lpBuffer, m_nBufferSize);
}

// ----------------------------------------------------------------------------
// 関　数：Save
// 説　明：指定された出力先ファイルに書込み
//         
// 引　数：
// 
// 戻り値：関数が成功すると、trueが返ります。
//         関数が失敗すると、falseが返ります。
// 備　考：
// ----------------------------------------------------------------------------
BOOL CMemoryFile::Save(FILE* pf, LONGLONG llMaxFileSize)
{
	BOOL bSaved = FALSE;

	DWORD dwWritten = 0;
	if (pf != NULL)
	{
		if (m_lpBuffer && m_nUsedSize > 0)
		{
			int result = fseek(pf, 0, SEEK_END);
			if (result != 0 ||
				// 2G超える場合、Clearする
				(LONGLONG)(ftell(pf) + m_nUsedSize) > llMaxFileSize)
			{
				result = fseek(pf, 0, SEEK_SET);
			}

			size_t ws = fwrite(m_lpBuffer, sizeof(BYTE), m_nUsedSize, pf);
			if (ws == 0)
			{
				bSaved = FALSE;
			}
			else
			{
				fflush(pf);

				Clear();
			}
		}
	}

	return bSaved;
}

BOOL CMemoryFile::Save(HANDLE hHandle, LONGLONG llMaxFileSize)
{
	BOOL bSaved = FALSE;

	DWORD dwWritten = 0;
	if (hHandle != INVALID_HANDLE_VALUE)
	{
		if (m_lpBuffer && m_nUsedSize > 0)
		{
			DWORD dwNewPointer = ::SetFilePointer(hHandle, 0, NULL, FILE_END);
			// 2G超える場合、Clearする
			if (dwNewPointer == INVALID_SET_FILE_POINTER ||
				(LONGLONG)(dwNewPointer + m_nUsedSize) > llMaxFileSize)
			{
				dwNewPointer = ::SetFilePointer(hHandle, 0, NULL, FILE_BEGIN);
				::SetEndOfFile(hHandle);
			}

			bSaved = ::WriteFile(hHandle, m_lpBuffer, (DWORD)m_nUsedSize, &dwWritten, NULL);
			if (bSaved)
			{
				bSaved = ::FlushFileBuffers(hHandle);

				Clear();
			}
		}
	}

	return bSaved;
}

// ----------------------------------------------------------------------------
// 関　数：Alloc
// 説　明：指定したサイズのメモリを割り当てます。
//         
// 引　数：nBytes		割り当てるバイト数
// 
// 戻り値：関数が成功すると、割り当てられた領域への void ポインタを返します。
//         関数が失敗すると、NULL を返します。
// 備　考：
// ----------------------------------------------------------------------------
BYTE* CMemoryFile::Alloc(SIZE_T nBytes)
{
	return (BYTE*)malloc(nBytes);
}

// ----------------------------------------------------------------------------
// 関　数：Realloc
// 説　明：メモリ ブロックの再割り当てを行います。
//         
// 引　数：lpMem	事前に割り当てられているメモリへのポインタ	
//         nBytes	新しいサイズ
// 
// 戻り値：関数が成功すると、割り当てられた領域への void ポインタを返します。
//         関数が失敗すると、NULL を返します。
// 備　考：
// ----------------------------------------------------------------------------
BYTE* CMemoryFile::Realloc(BYTE* lpMem, SIZE_T nBytes)
{
	assert(nBytes > 0);
	return (BYTE*)realloc(lpMem, nBytes);
}

// ----------------------------------------------------------------------------
// 関　数：Memcpy
// 説　明：バッファ間でバイト数をコピーします。
//         
// 引　数：lpMemTarget	コピー先のバッファ
//         lpMemSource	コピー元のバッファ
//         nBytes		コピーする文字数
// 
// 戻り値：lpMemTarget の値
//
// 備　考：
// ----------------------------------------------------------------------------
BYTE* CMemoryFile::Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource,
	SIZE_T nBytes)
{
	assert(lpMemTarget != NULL);
	assert(lpMemSource != NULL);

	assert(IsValidAddress(lpMemTarget, nBytes));
	assert(IsValidAddress(lpMemSource, nBytes, FALSE));

	memcpy_s(lpMemTarget, nBytes, lpMemSource, nBytes);
	return lpMemTarget;
}

// ----------------------------------------------------------------------------
// 関　数：Free
// 説　明：メモリを解放します。
//         
// 引　数：lpMem		解放する割り当て済みメモリ
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
void CMemoryFile::Free(BYTE* lpMem)
{
	assert(lpMem != NULL);

	free(lpMem);
}

// ----------------------------------------------------------------------------
// 関　数：GetPosition
// 説　明：ファイル ポインタの現在値を取得します
//         
// 引　数：
// 
// 戻り値：ファイル ポインタを返します
//
// 備　考：
// ----------------------------------------------------------------------------
ULONGLONG CMemoryFile::GetPosition() const
{
	return m_nPosition;
}

// ----------------------------------------------------------------------------
// 関　数：Seek
// 説　明：メモリファイルのポインタを移動します
//         
// 引　数：lOff		ポインタを移動するバイト数
//         nFrom	ポインタの移動モード
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
ULONGLONG CMemoryFile::Seek(LONGLONG lOff, UINT nFrom)
{
	::SetLastError(NO_ERROR);

	if (nFrom != FILE_BEGIN &&
		nFrom != FILE_END &&
		nFrom != FILE_CURRENT)
	{
		return 0;
	}

	LONGLONG lNewPos = m_nPosition;

	if (nFrom == FILE_BEGIN)
		lNewPos = lOff;
	else if (nFrom == FILE_CURRENT)
		lNewPos += lOff;
	else if (nFrom == FILE_END) {
		if (lOff > 0) {
			return 0;
		}
		lNewPos = m_nFileSize + lOff;
	}
	else
		return m_nPosition;

	if (lNewPos < 0)
	{
		return 0;
	}

	if (static_cast<DWORD>(lNewPos) > m_nFileSize)
		GrowFile((SIZE_T)lNewPos);

	m_nPosition = (SIZE_T)lNewPos;

	return m_nPosition;
}

// ----------------------------------------------------------------------------
// 関　数：GrowFile
// 説　明：メモリファイルを拡張する
//         
// 引　数：dwNewLen		メモリ ファイルの新しいサイズ
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
BOOL CMemoryFile::GrowFile(SIZE_T dwNewLen)
{
	::SetLastError(NO_ERROR);

	if (dwNewLen > m_nBufferSize)
	{
		// grow the buffer
		SIZE_T dwNewBufferSize = m_nBufferSize;

		// watch out for buffers which cannot be grown!
		assert(m_nGrowBytes != 0);
		if (m_nGrowBytes == 0) {
			//throw(_T("MemoryException"));
			return FALSE;
		}

		// determine new buffer size
		while (dwNewBufferSize < dwNewLen)
			dwNewBufferSize += m_nGrowBytes;

		// allocate new buffer
		BYTE* lpNew;
		if (m_lpBuffer == NULL)
			lpNew = Alloc(dwNewBufferSize);
		else
			lpNew = Realloc(m_lpBuffer, dwNewBufferSize);

		if (lpNew == NULL) {
			//throw(_T("MemoryException"));
			return FALSE;
		}

		m_lpBuffer = lpNew;
		m_nBufferSize = dwNewBufferSize;

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// 関　数：GetLength
// 説　明：ファイルの現在の論理長をバイト単位で取得します。
//         
// 引　数：
// 
// 戻り値：ファイルの長さを返します。
//
// 備　考：
// ----------------------------------------------------------------------------
ULONGLONG CMemoryFile::GetLength() const
{
	return m_nFileSize;
}

ULONGLONG CMemoryFile::GetUsedLength() const
{
	return m_nUsedSize;
}

BYTE* CMemoryFile::GetBuffer() const
{
	return m_lpBuffer;
}

// ----------------------------------------------------------------------------
// 関　数：SetLength
// 説　明：ファイルの長さを変更します
//         
// 引　数：dwNewLen		変更後のファイルのバイト単位の長さ
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
BOOL CMemoryFile::SetLength(ULONGLONG dwNewLen)
{
#ifdef WIN32
	if (dwNewLen > ULONG_MAX)
		return FALSE;
#endif  // WIN32
	if (dwNewLen > m_nBufferSize)
		GrowFile((SIZE_T)dwNewLen);

	if (dwNewLen < m_nPosition)
		m_nPosition = (SIZE_T)dwNewLen;

	m_nFileSize = (SIZE_T)dwNewLen;

	return TRUE;
}

// ----------------------------------------------------------------------------
// 関　数：Read
// 説　明：メモリファイルからバッファにデータを読み出します
//         
// 引　数：lpBuf		ユーザーが指定したバッファへのポインタ
//         nCount		メモリファイルから読み取る最大バイト数
// 
// 戻り値：バッファに転送されたバイト数を返します
//
// 備　考：
// ----------------------------------------------------------------------------
BOOL CMemoryFile::ReadFile(LPVOID lpBuf, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
	::SetLastError(NO_ERROR);

	if (nNumberOfBytesToRead == 0)
		return FALSE;

	if (lpBuf == NULL)
	{
		return FALSE;
		//		AfxThrowInvalidArgException();
	}

	if (!IsValidAddress(lpBuf, nNumberOfBytesToRead))
	{
		return FALSE;
	}

	if (m_nPosition > m_nFileSize)
		return FALSE;

	UINT nRead;
	if (m_nPosition + nNumberOfBytesToRead > m_nFileSize || m_nPosition + nNumberOfBytesToRead < m_nPosition)
		nRead = (UINT)(m_nFileSize - m_nPosition);
	else
		nRead = nNumberOfBytesToRead;

	Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
	m_nPosition += nRead;

	if (lpNumberOfBytesRead != NULL)
		*lpNumberOfBytesRead = nRead;

	return TRUE;
}

//UINT CMemoryFile::Read(void* lpBuf, UINT nCount)
//{
//	if (nCount == 0)
//		return 0;

//	assert(lpBuf != NULL);

//	if (lpBuf == NULL)
//	{
//		return 0;
//		//		AfxThrowInvalidArgException();
//	}

//	assert(IsValidAddress(lpBuf, nCount));

//	if (m_nPosition > m_nFileSize)
//		return 0;

//	UINT nRead;
//	if (m_nPosition + nCount > m_nFileSize || m_nPosition + nCount < m_nPosition)
//		nRead = (UINT)(m_nFileSize - m_nPosition);
//	else
//		nRead = nCount;

//	Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
//	m_nPosition += nRead;

//	return nRead;
//}

// ----------------------------------------------------------------------------
// 関　数：Write
// 説　明：バッファからデータをメモリファイルに書き込みます
//         
// 引　数：lpBuf		ユーザーが指定したバッファへのポインタ
//         nCount		メモリファイルから読み取る最大バイト数
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
BOOL CMemoryFile::WriteFile(LPCVOID lpBuf, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
	::SetLastError(NO_ERROR);

	if (nNumberOfBytesToWrite == 0)
		return FALSE;

	assert(lpBuf != NULL);
	assert(IsValidAddress(lpBuf, nNumberOfBytesToWrite, FALSE));

	if (lpBuf == NULL)
	{
		return FALSE;
	}
	//If we have no room for nCount, it must be an overflow
	if (m_nPosition + nNumberOfBytesToWrite < m_nPosition)
	{
		return 0;
	}

	if (m_nPosition + nNumberOfBytesToWrite > m_nBufferSize) {
		if (!GrowFile(m_nPosition + nNumberOfBytesToWrite)) {
			return FALSE;
		}
	}

	if (m_nPosition + nNumberOfBytesToWrite > m_nBufferSize)
		return FALSE;

	Memcpy((BYTE*)m_lpBuffer + m_nPosition, (BYTE*)lpBuf, nNumberOfBytesToWrite);

	m_nPosition += nNumberOfBytesToWrite;

	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;

	if (lpNumberOfBytesWritten != NULL)
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;

	// used buffer size
	m_nUsedSize += nNumberOfBytesToWrite;

	return TRUE;

}

void CMemoryFile::SetFileName(LPCTSTR pszFileName) {
	_tcscpy_s(m_szFileName, _MAX_PATH, pszFileName);
}

LPCTSTR CMemoryFile::GetFileName() const {
	return (LPCTSTR)m_szFileName;
}
