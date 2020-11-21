#include "stdafx.h"
#include "logmap.h"

namespace UTL {

// ----------------------------------------------------------------------------
// static
LONG volatile CLogMap::nGlobalIndex = 0;
#ifdef _USRDLL
CLogMap* volatile CLogMap::pInstance = 0;
#endif
//CRITICAL_SECTION CLogMap::pCriticalSection = { 0 };

// ----------------------------------------------------------------------------
// �ց@���FCLogMap::getInstance
// ���@���F�C���X�^���X�̎擾
//
// ���@���F
//
// �߂�l�F�C���X�^���X
//         
// ���@�l�F
// ----------------------------------------------------------------------------
CLogMap* CLogMap::getInstance() {
#ifdef _USRDLL
	if (pInstance == 0)
	{
		CLogMap* volatile tmp = new CLogMap;
		if (tmp)
		{
			tmp->pMemFile = new CMemoryFile();
			if (tmp->pMemFile == NULL)
			{
				_DELETE(tmp);

				return NULL;
			}
			tmp->m_WorkingSetSize = 0;
			tmp->m_dwErrorCode = 0;
			tmp->m_nLogLevel = 0;
		}

		pInstance = tmp;
	}

	return pInstance;
#else
	static CLogMap logMap;
	if (logMap.pMemFile == NULL) {
		logMap.pMemFile = new CMemoryFile();
		if (logMap.pMemFile == NULL)
		{
		}
		logMap.m_WorkingSetSize = 0;
		logMap.m_dwErrorCode = 0;
		logMap.m_nLogLevel = 0;
	}
	return &logMap;
#endif
}

CLogMap::CLogMap() {
	// �N���e�B�J���Z�N�V�����̏�����
	::InitializeCriticalSection(&pCriticalSection);

	m_ulMemSize = BYTE_4K;
	// Memory Buffer Size = 256k

	// Clear
	pMemFile = NULL;

	m_WorkingSetSize = 0;
	m_dwErrorCode = 0;
	m_nLogLevel = 0;

	m_szLogPrefix[0] = 0;
	m_szLogPath[0] = 0;
	m_szLogExt[0] = 0;
	m_bLogDate = FALSE;
}

CLogMap::~CLogMap()
{
	afxTRACE(_T("CLogMap::~CLogMap()"));
	// Write Last Text
	WriteToFile();

	// �N���e�B�J���Z�N�V�����̏I��
	::DeleteCriticalSection(&pCriticalSection);

	Clear();
}

// ----------------------------------------------------------------------------
// �ց@���FCLogMap::destroy
// ���@���F�C���X�^���X�̉��
//
// ���@���F
//
// �߂�l�F���̃N���X�̗B��̃C���X�^���X��Ԃ�
//         
// ���@�l�F
// ----------------------------------------------------------------------------
#ifdef _USRDLL
void CLogMap::destroy()
{
	_DELETE(pInstance);
}
#endif

// ----------------------------------------------------------------------------
// �ց@���FCLogMap::getGlobalIndex
// ���@���F�C���f�b�N�X�ԍ��擾
//
// ���@���F
//
// �߂�l�F�C���f�b�N�X�ԍ�
//         
// ���@�l�Fstatic
// ----------------------------------------------------------------------------
LONG volatile CLogMap::getGlobalIndex() {
	nGlobalIndex++;
	if (nGlobalIndex >= 100000000)
		nGlobalIndex = 0;
	return (nGlobalIndex);
}


void CLogMap::Lock()
{
	::EnterCriticalSection(&pCriticalSection);
}

void CLogMap::Unlock()
{
	::LeaveCriticalSection(&pCriticalSection);
}

// ----------------------------------------------------------------------------
// �ց@���FClear
// ���@���F�������t�@�C���̍폜���s���܂�
//         
// ���@���F
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::Clear()
{
	_DELETE(pMemFile);
}

// ----------------------------------------------------------------------------
// �ց@���FGetMemFile
// ���@���F�w�肵���X���b�hID�̃������t�@�C�����擾����
//         
// ���@���FdwID		�X���b�hID
//         bCreate	�V�K�쐬�t���O
// 
// �߂�l�F�������t�@�C���̃|�C���g
//
// ���@�l�FbCreate = TRUE�̏ꍇ�A�w�肵��dwID�̃������t�@�C�������݂��Ȃ�����
//         �V�K�쐬����BbCreate = FALSE�̏ꍇ�ANULL��Ԃ��܂��B
// ----------------------------------------------------------------------------
CMemoryFile* CLogMap::GetMemFile(BOOL bCreate)
{
	if (pMemFile == NULL)
	{
		if (bCreate)
			pMemFile = new CMemoryFile();
	}

	return pMemFile;
}

// ----------------------------------------------------------------------------
// �ց@���FClearMemFile
// ���@���F�w�肵���X���b�hID�̃������t�@�C�����N���[�Y���A���������������B
//         
// ���@���FdwID		�X���b�hID
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::ClearMemFile()
{
	if (pMemFile)
	{
		pMemFile->Close();
	}
}

// ----------------------------------------------------------------------------
// �ց@���FSaveMemFile
// ���@���F�w�肵���X���b�hID�̃������t�@�C������A�w�肵���o�͐�t�@�C����
//         �������݂܂��B
//
// ���@���FhHandle		�w�肵���o�͐�t�@�C���̃n���h��
//         dwID			�X���b�hID
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::SaveMemFile(FILE* pf, BOOL bClose)
{
	CMemoryFile* pFile = GetMemFile();
	if (pFile) {
		pFile->Save(pf);

		if (bClose) {
			pFile->Close();
		}
	}
}

void CLogMap::SaveMemFile(HANDLE hHandle, BOOL bClose)
{
	CMemoryFile* pFile = GetMemFile();
	if (pFile) {
		pFile->Save(hHandle);

		if (bClose) {
			pFile->Close();
		}
	}
}

// ----------------------------------------------------------------------------
// �ց@���FGetLastErrorCode
// ���@���F�w�肵���X���b�hID�̃������t�@�C���̃��X�g�G���[�R�[�h���擾
//         
// ���@���FdwID		�X���b�hID
// 
// �߂�l�F�G���[�R�[�h
//
// ���@�l�F
// ----------------------------------------------------------------------------
DWORD CLogMap::GetLastErrorCode()
{
	return m_dwErrorCode;
}


// ----------------------------------------------------------------------------
// �ց@���FSetAllocMemory
// ���@���F�w�肵���X���b�h�̃������[�g�p�ʂ����Z����ialloc/realloc�j
//         
// ���@���F[IN ] stSize		�������[�g�p��
//         [IN ] dwID		�X���b�hID
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::SetAllocMemory(SIZE_T stSize)
{
	m_WorkingSetSize += stSize;
}

// ----------------------------------------------------------------------------
// �ց@���FSetFreeMemory
// ���@���F�w�肵���X���b�h�̃������[�g�p�ʂ����Z����(free)
//         
// ���@���F[IN ] stSize		�������[�g�p��
//         [IN ] dwID		�X���b�hID
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::SetFreeMemory(SIZE_T stSize)
{
	m_WorkingSetSize -= stSize;
}

// ----------------------------------------------------------------------------
// �ց@���FSetFreeMemory
// ���@���F�w�肵���X���b�h�̃������[�g�p�ʂ��擾����
//         
// ���@���F[IN ] dwID		�X���b�hID
// 
// �߂�l�F�������[�g�p��
//
// ���@�l�F
// ----------------------------------------------------------------------------
SIZE_T CLogMap::GetAllocedMemory()
{
	return m_WorkingSetSize;
}

// ----------------------------------------------------------------------------
// �ց@���FClearMemoryCount
// ���@���F�w�肵���X���b�h�̃������[�g�p�ʂ��O�ɂ���
//         
// ���@���F[IN ] dwID		�X���b�hID
// 
// �߂�l�F
//
// ���@�l�F
// ----------------------------------------------------------------------------
void CLogMap::ClearMemoryCount()
{
	m_WorkingSetSize = 0;
}

void CLogMap::SetLogLevel(int newValue)
{
	m_nLogLevel = newValue;
}

int CLogMap::GetLogLevel()
{
	return m_nLogLevel;
}

void CLogMap::SetMemorySize(ULONG ulSize) {
	m_ulMemSize = max(BYTE_4K, ulSize);

	CMemoryFile* pMemFile = GetMemFile();
	if (pMemFile != NULL) {
		pMemFile->SetLength(m_ulMemSize);
	}
}

void CLogMap::SetLogFile(LPCTSTR pszPath, LPCTSTR pszPrefix, LPCTSTR pszExt, BOOL bDate)
{
	if (!_IsEmpty(pszPath)) {
		_tcscpy_s(m_szLogPath, _MAX_PATH, pszPath);
	}
	if (!_IsEmpty(pszPrefix)) {
		_tcscpy_s(m_szLogPrefix, _MAX_PATH, pszPrefix);
	}
	if (!_IsEmpty(pszExt)) {
		_tcscpy_s(m_szLogExt, _MAX_EXT, pszExt);
	}
	else {
		_tcscpy_s(m_szLogExt, _MAX_EXT, LOG_EXT);
	}

	m_bLogDate = bDate;
}

CString CLogMap::GetLogFile() {
	if (_IsEmpty(m_szLogPath) &&
		_IsEmpty(m_szLogPrefix) &&
		!m_bLogDate)
		return _T("");

	CString szLog(m_szLogPath);
	if (!szLog.endsWith(_T("\\"))) {
		szLog += _T('\\');
	}
	szLog += m_szLogPrefix;

	if (m_bLogDate) {
		CString tm;
		time_t ltime;
		struct tm createtime;
		time(&ltime);

		// Convert to local time
		errno_t terr = localtime_s(&createtime, &ltime);
		if (terr == 0)
		{
			// ext. 20130809
			tm.Format(_T("%04d%02d%02d"),
				createtime.tm_year + 1900,
				createtime.tm_mon + 1,
				createtime.tm_mday
				);
		}
		szLog += tm;
	}
	szLog += _T(".");
	szLog += m_szLogExt;

	return szLog;
}

BOOL CLogMap::MakeLogFile() {
	BOOL bMaked = FALSE;
	CString pszLog = GetLogFile();
	if (!pszLog.IsEmpty()) {
		Lock();

		HANDLE hFile = ::CreateFile(pszLog.GetBuffer(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE) {
			::CloseHandle(hFile);

			bMaked = TRUE;
		}

		Unlock();
	}

	return bMaked;
}

void CLogMap::WriteToFile(BOOL bClose) {
	if (pMemFile != NULL) {
		// Has Data
		if (pMemFile->GetUsedLength() > 0) {
			CString pszLog = GetLogFile();
			if (!pszLog.IsEmpty()) {
				Lock();

				HANDLE hFile = ::CreateFile(pszLog.GetBuffer(),
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
					NULL);

				if (hFile != INVALID_HANDLE_VALUE) {

					SaveMemFile(hFile, FALSE);

					::CloseHandle(hFile);
				}

				Unlock();
			}
		}

		if (bClose) {
			pMemFile->Close();
		}
	}
}

} // namespace UTL
