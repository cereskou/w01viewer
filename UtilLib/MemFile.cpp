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
	// �f�B�t�H���g��64k�o�b�t�@�[
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
	// �f�B�t�H���g��64k�o�b�t�@�[
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
// �ց@���FClose
// ���@���F�N���[�Y�����i����������j
//         
// ���@���F
// 
// �߂�l�F
//         
// ���@�l�F
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
// �ց@���FClear
// ���@���F�N���[�Y�����i�������N���A����j
//         
// ���@���F
// 
// �߂�l�F
//         
// ���@�l�F
// ----------------------------------------------------------------------------
void CMemoryFile::Clear()
{
	m_nPosition = 0;
	m_nUsedSize = 0;
	ZeroMemory(m_lpBuffer, m_nBufferSize);
}

// ----------------------------------------------------------------------------
// �ց@���FSave
// ���@���F�w�肳�ꂽ�o�͐�t�@�C���ɏ�����
//         
// ���@���F
// 
// �߂�l�F�֐�����������ƁAtrue���Ԃ�܂��B
//         �֐������s����ƁAfalse���Ԃ�܂��B
// ���@�l�F
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
				// 2G������ꍇ�AClear����
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
			// 2G������ꍇ�AClear����
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
// �ց@���FAlloc
// ���@���F�w�肵���T�C�Y�̃����������蓖�Ă܂��B
//         
// ���@���FnBytes		���蓖�Ă�o�C�g��
// 
// �߂�l�F�֐�����������ƁA���蓖�Ă�ꂽ�̈�ւ� void �|�C���^��Ԃ��܂��B
//         �֐������s����ƁANULL ��Ԃ��܂��B
// ���@�l�F
// ----------------------------------------------------------------------------
BYTE* CMemoryFile::Alloc(SIZE_T nBytes)
{
	return (BYTE*)malloc(nBytes);
}

// ----------------------------------------------------------------------------
// �ց@���FRealloc
// ���@���F������ �u���b�N�̍Ċ��蓖�Ă��s���܂��B
//         
// ���@���FlpMem	���O�Ɋ��蓖�Ă��Ă��郁�����ւ̃|�C���^	
//         nBytes	�V�����T�C�Y
// 
// �߂�l�F�֐�����������ƁA���蓖�Ă�ꂽ�̈�ւ� void �|�C���^��Ԃ��܂��B
//         �֐������s����ƁANULL ��Ԃ��܂��B
// ���@�l�F
// ----------------------------------------------------------------------------
BYTE* CMemoryFile::Realloc(BYTE* lpMem, SIZE_T nBytes)
{
	assert(nBytes > 0);
	return (BYTE*)realloc(lpMem, nBytes);
}

// ----------------------------------------------------------------------------
// �ց@���FMemcpy
// ���@���F�o�b�t�@�ԂŃo�C�g�����R�s�[���܂��B
//         
// ���@���FlpMemTarget	�R�s�[��̃o�b�t�@
//         lpMemSource	�R�s�[���̃o�b�t�@
//         nBytes		�R�s�[���镶����
// 
// �߂�l�FlpMemTarget �̒l
//
// ���@�l�F
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
// �ց@���FFree
// ���@���F��������������܂��B
//         
// ���@���FlpMem		������銄�蓖�čς݃�����
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CMemoryFile::Free(BYTE* lpMem)
{
	assert(lpMem != NULL);

	free(lpMem);
}

// ----------------------------------------------------------------------------
// �ց@���FGetPosition
// ���@���F�t�@�C�� �|�C���^�̌��ݒl���擾���܂�
//         
// ���@���F
// 
// �߂�l�F�t�@�C�� �|�C���^��Ԃ��܂�
//
// ���@�l�F
// ----------------------------------------------------------------------------
ULONGLONG CMemoryFile::GetPosition() const
{
	return m_nPosition;
}

// ----------------------------------------------------------------------------
// �ց@���FSeek
// ���@���F�������t�@�C���̃|�C���^���ړ����܂�
//         
// ���@���FlOff		�|�C���^���ړ�����o�C�g��
//         nFrom	�|�C���^�̈ړ����[�h
// 
// �߂�l�F
//
// ���@�l�F
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
// �ց@���FGrowFile
// ���@���F�������t�@�C�����g������
//         
// ���@���FdwNewLen		������ �t�@�C���̐V�����T�C�Y
// 
// �߂�l�F
//
// ���@�l�F
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
// �ց@���FGetLength
// ���@���F�t�@�C���̌��݂̘_�������o�C�g�P�ʂŎ擾���܂��B
//         
// ���@���F
// 
// �߂�l�F�t�@�C���̒�����Ԃ��܂��B
//
// ���@�l�F
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
// �ց@���FSetLength
// ���@���F�t�@�C���̒�����ύX���܂�
//         
// ���@���FdwNewLen		�ύX��̃t�@�C���̃o�C�g�P�ʂ̒���
// 
// �߂�l�F
//
// ���@�l�F
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
// �ց@���FRead
// ���@���F�������t�@�C������o�b�t�@�Ƀf�[�^��ǂݏo���܂�
//         
// ���@���FlpBuf		���[�U�[���w�肵���o�b�t�@�ւ̃|�C���^
//         nCount		�������t�@�C������ǂݎ��ő�o�C�g��
// 
// �߂�l�F�o�b�t�@�ɓ]�����ꂽ�o�C�g����Ԃ��܂�
//
// ���@�l�F
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
// �ց@���FWrite
// ���@���F�o�b�t�@����f�[�^���������t�@�C���ɏ������݂܂�
//         
// ���@���FlpBuf		���[�U�[���w�肵���o�b�t�@�ւ̃|�C���^
//         nCount		�������t�@�C������ǂݎ��ő�o�C�g��
// 
// �߂�l�F
//
// ���@�l�F
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
