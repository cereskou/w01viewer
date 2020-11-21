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
// 関　数：CLogMap::getInstance
// 説　明：インスタンスの取得
//
// 引　数：
//
// 戻り値：インスタンス
//         
// 備　考：
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
	// クリティカルセクションの初期化
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

	// クリティカルセクションの終了
	::DeleteCriticalSection(&pCriticalSection);

	Clear();
}

// ----------------------------------------------------------------------------
// 関　数：CLogMap::destroy
// 説　明：インスタンスの解放
//
// 引　数：
//
// 戻り値：このクラスの唯一のインスタンスを返す
//         
// 備　考：
// ----------------------------------------------------------------------------
#ifdef _USRDLL
void CLogMap::destroy()
{
	_DELETE(pInstance);
}
#endif

// ----------------------------------------------------------------------------
// 関　数：CLogMap::getGlobalIndex
// 説　明：インデックス番号取得
//
// 引　数：
//
// 戻り値：インデックス番号
//         
// 備　考：static
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
// 関　数：Clear
// 説　明：メモリファイルの削除を行います
//         
// 引　数：
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
void CLogMap::Clear()
{
	_DELETE(pMemFile);
}

// ----------------------------------------------------------------------------
// 関　数：GetMemFile
// 説　明：指定したスレッドIDのメモリファイルを取得する
//         
// 引　数：dwID		スレッドID
//         bCreate	新規作成フラグ
// 
// 戻り値：メモリファイルのポイント
//
// 備　考：bCreate = TRUEの場合、指定したdwIDのメモリファイルが存在しない時に
//         新規作成する。bCreate = FALSEの場合、NULLを返します。
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
// 関　数：ClearMemFile
// 説　明：指定したスレッドIDのメモリファイルをクローズし、メモリを解放する。
//         
// 引　数：dwID		スレッドID
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
void CLogMap::ClearMemFile()
{
	if (pMemFile)
	{
		pMemFile->Close();
	}
}

// ----------------------------------------------------------------------------
// 関　数：SaveMemFile
// 説　明：指定したスレッドIDのメモリファイルから、指定した出力先ファイルへ
//         書き込みます。
//
// 引　数：hHandle		指定した出力先ファイルのハンドル
//         dwID			スレッドID
// 
// 戻り値：
//
// 備　考：
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
// 関　数：GetLastErrorCode
// 説　明：指定したスレッドIDのメモリファイルのラストエラーコードを取得
//         
// 引　数：dwID		スレッドID
// 
// 戻り値：エラーコード
//
// 備　考：
// ----------------------------------------------------------------------------
DWORD CLogMap::GetLastErrorCode()
{
	return m_dwErrorCode;
}


// ----------------------------------------------------------------------------
// 関　数：SetAllocMemory
// 説　明：指定したスレッドのメモリー使用量を加算する（alloc/realloc）
//         
// 引　数：[IN ] stSize		メモリー使用量
//         [IN ] dwID		スレッドID
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
void CLogMap::SetAllocMemory(SIZE_T stSize)
{
	m_WorkingSetSize += stSize;
}

// ----------------------------------------------------------------------------
// 関　数：SetFreeMemory
// 説　明：指定したスレッドのメモリー使用量を減算する(free)
//         
// 引　数：[IN ] stSize		メモリー使用量
//         [IN ] dwID		スレッドID
// 
// 戻り値：
//
// 備　考：
// ----------------------------------------------------------------------------
void CLogMap::SetFreeMemory(SIZE_T stSize)
{
	m_WorkingSetSize -= stSize;
}

// ----------------------------------------------------------------------------
// 関　数：SetFreeMemory
// 説　明：指定したスレッドのメモリー使用量を取得する
//         
// 引　数：[IN ] dwID		スレッドID
// 
// 戻り値：メモリー使用量
//
// 備　考：
// ----------------------------------------------------------------------------
SIZE_T CLogMap::GetAllocedMemory()
{
	return m_WorkingSetSize;
}

// ----------------------------------------------------------------------------
// 関　数：ClearMemoryCount
// 説　明：指定したスレッドのメモリー使用量を０にする
//         
// 引　数：[IN ] dwID		スレッドID
// 
// 戻り値：
//
// 備　考：
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
