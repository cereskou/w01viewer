#include "stdafx.h"
#include "SwapFile.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

//-----------------------------------------------------------------------------
// 関数名：CSwapFile
// 説　明：スワップファイルコンストラクター
//
//
//
//-----------------------------------------------------------------------------
CSwapFile::CSwapFile()
	: m_pCachedData(NULL)
	, m_dwErrorCode(0)
{
}

//-----------------------------------------------------------------------------
// 関数名：~CSwapFile
// 説　明：スワップファイルデストラクター
//
//
//
//-----------------------------------------------------------------------------
CSwapFile::~CSwapFile(void)
{
	Close();
}

//-----------------------------------------------------------------------------
// 関数名：Open
// 説　明：スワップファイルを開く処理です。
// 引　数：
// 戻り値：関数が成功すると、0が返ります。
//         関数が失敗すると、-1が返ります。
//-----------------------------------------------------------------------------
bool CSwapFile::Open()
{
	Close();

	m_pCachedData = new CHashTblCachedData();
	if (m_pCachedData == NULL ||
		m_pCachedData->create(genHashKey) == NULL)
	{
		_DELETE(m_pCachedData);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// 関数名：Close
// 説　明：スワップファイルを閉じる
// 引　数：
// 戻り値：関数が成功すると、0が返ります。
//         
//-----------------------------------------------------------------------------
bool CSwapFile::Close()
{
	_DELETE(m_pCachedData);

	return true;
}

//-----------------------------------------------------------------------------
// 関数名：GetDataKey
// 説　明：新しいインデックスキーを取得する
// 引　数：[IN]  size         - データのサイズ
//
// 戻り値：関数が成功すると、インデックスキーが返ります。
//         関数が失敗すると、０が返ります。
//-----------------------------------------------------------------------------
long CSwapFile::GetDataKey(size_t size, size_t block)
{
	long dataKey = -1;

	if (m_pCachedData == NULL)
		return -1;

	dataKey = (long)m_pCachedData->size() + 1;
	while (m_pCachedData->find(dataKey)) {
		dataKey++;
	}

	LPCACHEFILEINDEX  lpCachedIndex = (LPCACHEFILEINDEX)malloc(sizeof(CACHEFILEINDEX));
	if (lpCachedIndex == NULL) {
		return -1;
	}

	lpCachedIndex->key = dataKey;
	lpCachedIndex->size = size;
	lpCachedIndex->block = block;
	lpCachedIndex->pLnkRec = new LinkedList<LPVOID>(true, _freeptr);
	if (lpCachedIndex->pLnkRec == NULL) {
		_FREE(lpCachedIndex);
		return -1;
	}

	m_pCachedData->insert(dataKey, lpCachedIndex);

	return dataKey;
}

//-----------------------------------------------------------------------------
// 関数名：Write
// 説　明：ファイルにデータを書込みます
// 引　数：[IN]  pData       - データバッファ
//         [IN]  size        - 書込み対象のサイズ
//         [IN]  count       - 書込み対象の数
//         [IN]  datakey     - インデックスキー
//
// 戻り値：関数が成功すると、0以外が返ります。
//         関数が失敗すると、0が返ります。
//-----------------------------------------------------------------------------
size_t CSwapFile::Write(void* pData, size_t size, int count, long datakey)
{
	if (m_pCachedData == NULL)
		return 0;

	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(datakey);
	if (lpCachedIndex == NULL)
		return 0;

	if (lpCachedIndex->pLnkRec == NULL)
		return 0;

	if (size != lpCachedIndex->size)
		return 0;

	size_t saved = 0;
	UCHAR* p = (UCHAR*)pData;
	for (int i = 0; i < count; i++) {
		LPVOID pBuf = (LPVOID)malloc(size);
		if (pBuf != NULL) {
			memcpy(pBuf, p, size);

			if (lpCachedIndex->pLnkRec->add(pBuf) > 0)
				saved++;

			p += size;
		}
	}

	return saved;
}

bool CSwapFile::MoveFirst(long dataKey)
{
	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(dataKey);
	if (lpCachedIndex == NULL)
		return false;

	if (lpCachedIndex->pLnkRec == NULL)
		return false;

	lpCachedIndex->position = 0;

	return true;
}

long CSwapFile::Read(long dataKey, size_t index, void** pData, LPDWORD pLength)
{
	if (m_pCachedData == NULL)
		return -1;

	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(dataKey);
	if (lpCachedIndex == NULL)
		return -1;

	if (lpCachedIndex->pLnkRec == NULL)
		return -1;

	if (pLength)
		*pLength = 0;

	if (lpCachedIndex->pLnkRec->move(index)) {
		*pData = (void*)calloc(1, lpCachedIndex->size);
		if (*pData == NULL)
			return -1;

		LPVOID pBuf = lpCachedIndex->pLnkRec->get();

		if (pBuf != NULL) {
			memcpy(*pData, pBuf, lpCachedIndex->size);
		}
	}

	if (pLength)
		*pLength = (DWORD)lpCachedIndex->size;

	return 1;
}

//-----------------------------------------------------------------------------
// Function     : Read
// Description  : Reads entire block data from cached data file.
// Parameters   : [IN ] dataKey      - id
//                [OUT] pData        - Pointer to data buffer
//                [OUT] pLength      - length of data
// Return Value : 
//                returns number of readed if successed.
//                returns -1 if failed.
//-----------------------------------------------------------------------------
long CSwapFile::Read(long dataKey, void** pData, LPDWORD pLength)
{
	if (m_pCachedData == NULL)
		return -1;

	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(dataKey);
	if (lpCachedIndex == NULL)
		return -1;

	if (lpCachedIndex->pLnkRec == NULL)
		return -1;

	long nRecCnt = (long)lpCachedIndex->pLnkRec->size() - (long)lpCachedIndex->position;
	nRecCnt = min(nRecCnt, (long)lpCachedIndex->block);

	DWORD dwLength = (DWORD)(nRecCnt * lpCachedIndex->size);
	if (dwLength == 0)
		return 0;

	if (pLength)
		*pLength = 0;

	*pData = (void*)calloc(nRecCnt, lpCachedIndex->size);
	if (*pData == NULL)
		return -1;

	UCHAR* p = (UCHAR*)(*pData);

	if (lpCachedIndex->pLnkRec->move(lpCachedIndex->position)) {
		long count = nRecCnt;
		do {
			LPVOID pBuf = lpCachedIndex->pLnkRec->get();
			if (pBuf != NULL) {
				memcpy(p, pBuf, lpCachedIndex->size);

				p += lpCachedIndex->size;
			}
		} while (lpCachedIndex->pLnkRec->next() && (--count > 0));
	}

	if (pLength)
		*pLength = dwLength;

	// move pointer
	lpCachedIndex->position += nRecCnt;

	return nRecCnt;
}

//-----------------------------------------------------------------------------
// Function     : ReadData
// Description  : Reads entire block data from cached data file.
// Parameters   : [IN ] dataKey      - id
//                [OUT] pData        - Pointer to data buffer
//                [OUT] pLength      - length of data
// Return Value : 
//                returns number of readed if successed.
//                returns -1 if failed.
//-----------------------------------------------------------------------------
long CSwapFile::ReadData(long dataKey, void**pData, LPDWORD pLength) {
	if (m_pCachedData == NULL)
		return -1;

	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(dataKey);
	if (lpCachedIndex == NULL)
		return -1;

	if (lpCachedIndex->pLnkRec == NULL)
		return -1;

	long nRecCnt = (long)lpCachedIndex->pLnkRec->size();

	DWORD dwLength = (DWORD)(nRecCnt * lpCachedIndex->size);
	if (dwLength == 0)
		return 0;

	if (pLength)
		*pLength = 0;

	*pData = (void*)calloc(nRecCnt, lpCachedIndex->size);
	if (*pData == NULL)
		return -1;

	UCHAR* p = (UCHAR*)(*pData);
	if (lpCachedIndex->pLnkRec->first()) {
		do {
			LPVOID pBuf = lpCachedIndex->pLnkRec->get();
			if (pBuf != NULL) {
				memcpy(p, pBuf, lpCachedIndex->size);

				p += lpCachedIndex->size;
			}
		} while (lpCachedIndex->pLnkRec->next());
	}

	if (pLength)
		*pLength = dwLength;

	return nRecCnt;
}

//-----------------------------------------------------------------------------
// Function     : GetDataCount
// Description  : get number of cached data from file
// Parameters   : [IN ] dataKey  - index key
//
// Return Value : 
//                returns number of cached data if successed.
//                returns -1 if failed.
//-----------------------------------------------------------------------------
long CSwapFile::GetDataCount(long dataKey) {
	if (m_pCachedData == NULL)
		return -1;

	LPCACHEFILEINDEX lpCachedIndex = m_pCachedData->get(dataKey);
	if (lpCachedIndex == NULL)
		return -1;

	if (lpCachedIndex->pLnkRec == NULL)
		return -1;

	return (long)lpCachedIndex->pLnkRec->size();
}
