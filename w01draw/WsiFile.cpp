#include "stdafx.h"
#include "WsiFile.h"

#ifndef __CSTRING_H__
inline BOOL _IsEmptyA(LPCSTR lpszA) {
	if (lpszA == NULL)
		return TRUE;
	if (strlen(lpszA) == 0)
		return TRUE;

	return FALSE;
}

inline BOOL _IsEmptyW(LPCWSTR lpszW) {
	if (lpszW == NULL)
		return TRUE;
	if (wcslen(lpszW) == 0)
		return TRUE;

	return FALSE;
}

// ----------------------------------------------------------------------------
// inline functions
// ----------------------------------------------------------------------------
inline CStringA _CW2A(CStringW pszW, UINT acp = CP_ACP) {
	int len = ::WideCharToMultiByte(acp,
		0,
		pszW.GetBuffer(),
		-1,
		NULL,
		0,
		NULL,
		NULL);

	CStringA destA;
	int ret = ::WideCharToMultiByte(acp,
		0,
		pszW.GetBuffer(),
		-1,
		destA.GetBufferSetLength(len),
		len,
		NULL,
		NULL);

	if (ret == 0)
		return "";
	destA.ReleaseBuffer();

	return destA;
}

inline CStringW _CA2W(CStringA pszA, UINT acp = CP_ACP)
{
	int len = ::MultiByteToWideChar(acp,
		0,
		pszA.GetBuffer(),
		-1,
		NULL,
		0);
	CStringW destW;

	int ret = ::MultiByteToWideChar(acp,
		0,
		pszA.GetBuffer(),
		-1,
		destW.GetBufferSetLength(len),
		len);

	if (ret == 0)
		return L"";
	destW.ReleaseBuffer();

	return destW;
}

//@QHGOU 20160818 Added
size_t GetBytesLength(LPCTSTR pszStr, INT& nMaxLen)
{
	// 文字列の文字数
	size_t length = _tcslen(pszStr);

	// 文字情報の配列
	WORD *type = new WORD[length];

	BOOL bMaxSet = FALSE;
	size_t count = 0;
	// 文字情報を取得
	if (GetStringTypeEx(0, CT_CTYPE3, pszStr, (int)length, type))
	{
		INT nDbcs = 0;
		// 取得成功
		for (size_t i = 0; i < length; i++) {
			nDbcs = 0;
			// 1文字毎に判定
			if ((type[i] & C3_FULLWIDTH) == C3_FULLWIDTH ||
				(type[i] & C3_IDEOGRAPH) == C3_IDEOGRAPH ||
				(type[i] & C3_KATAKANA) == C3_KATAKANA ||
				(type[i] & C3_HIRAGANA) == C3_HIRAGANA)
			{
				// 全角英数、全角カタカナ、全角スペース
				// 全角漢字
				// 全角ひらがな
				count++;
				nDbcs = 1;
			}

			if (!bMaxSet &&
				count >= nMaxLen) {
				nMaxLen = count - nDbcs;
				bMaxSet = TRUE;
			}

			count++;
		}
	}

	// 文字情報の解放
	delete[] type;

	return count;
}
#ifdef _UNICODE
#define _IsEmpty	_IsEmptyW
#define _W2A(x)		(LPCSTR)_CW2A((LPCTSTR)x)
#define _WW2A(x)	(LPCTSTR)(x)
#define _A2W(x)		(LPCWSTR)(x)
#define _AA2W(x)	(LPCTSTR)_CA2W((LPCSTR)x)
#define _AA2W2(x,y)	(LPCTSTR)_CA2W((LPCSTR)x,y)
#else
#define _IsEmpty	_IsEmptyA
#define _W2A(x)		(LPCSTR)(x)
#define _WW2A(x)	(LPCSTR)_CW2A((LPCWSTR)x)
#define _WW2A2(x,y)	(LPCSTR)_CW2A((LPCWSTR)x,y)
#define _A2W(x)		(LPCWSTR)_CA2W((LPCSTR)x)
#define _A2W2(x,y)	(LPCWSTR)_CA2W((LPCSTR)x,y)
#define _AA2W(x)	(LPCTSTR)(x)
#endif
#else
size_t GetBytesLength(LPCWSTR pszStr, INT& nMaxLen, INT& nCharCnt)
{
	// 文字列の文字数
	size_t length = wcslen(pszStr);

	// 文字情報の配列
	WORD *type = new WORD[length];

	BOOL bMaxSet = FALSE;
	INT count = 0;
	// 文字情報を取得
	if (GetStringTypeExW(0, CT_CTYPE3, pszStr, (int)length, type))
	{
		INT nDbcs = 0;
		// 取得成功
		for (size_t i = 0; i < length; i++) {
			nDbcs = 0;
			//#ifdef _DEBUG
			//			PrintType(pszStr[i], type[i]);
			//#endif
			// 1文字毎に判定
			if ((type[i] & C3_FULLWIDTH) == C3_FULLWIDTH ||
				(type[i] & C3_IDEOGRAPH) == C3_IDEOGRAPH ||
				(type[i] & C3_KATAKANA) == C3_KATAKANA ||
				(type[i] & C3_HIRAGANA) == C3_HIRAGANA ||
				type[i] == 0x0000)
			{
				// 全角英数、全角カタカナ、全角スペース
				// 全角漢字
				// 全角ひらがな
				count++;
				nDbcs = 1;
			}

			if (!bMaxSet &&
				count >= nMaxLen) {
				nMaxLen = count - nDbcs;
				bMaxSet = TRUE;
				nCharCnt = i;
			}

			count++;
		}
	}

	// 文字情報の解放
	delete[] type;

	return count;
}
#endif

//-----------------------------------------------------------------------------
// 関数名：CWsiFile
// 説　明：コンストラクター
//
//
//
//-----------------------------------------------------------------------------
CWsiFile::CWsiFile()
	: m_dwErrorCode(0)
	, m_bLoaded(FALSE)
	, m_pNode(NULL)
	, m_pNodeAttr(NULL)
	, m_szFileName(_T(""))
	, m_nDPI(W01_DPI)
	, m_nCount(0)
	, m_nCountAttr(0)
	, m_nKeyMax(1024)
	, m_nAttMax(1024)
	, m_nKeyLength(127)
	, m_nAttLength(32)
{
}

//-----------------------------------------------------------------------------
// 関数名：CWsiFile
// 説　明：コンストラクター
//
//
//
//-----------------------------------------------------------------------------
CWsiFile::CWsiFile(LPCTSTR pszFileName)
	: m_szFileName(pszFileName)
	, m_dwErrorCode(0)
	, m_bLoaded(FALSE)
	, m_pNode(NULL)
	, m_pNodeAttr(NULL)
	, m_nDPI(W01_DPI)
	, m_nCount(0)
	, m_nCountAttr(0)
	, m_nKeyMax(1024)
	, m_nAttMax(1024)
	, m_nKeyLength(127)
	, m_nAttLength(32)
{
	Load();
}

//-----------------------------------------------------------------------------
// 関数名：~CWsiFile
// 説　明：デストラクター
//
//
//
//-----------------------------------------------------------------------------
CWsiFile::~CWsiFile()
{
	Close();
}

//-----------------------------------------------------------------------------
// 関数名：Load
// 説　明：WSIファイルの読込処理を行う
// 引　数：[IN]  pszFilaNem   - WSIファイル名
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::Load(LPCTSTR pszFilaNem)
{
	m_dwErrorCode = 0;
	_DELETE(m_pNode);
	_DELETE(m_pNodeAttr);

	if (!_IsEmpty(pszFilaNem)) {
		m_szFileName = pszFilaNem;
	}

	m_pNode = new CHashTblWSINode();
	if(m_pNode == NULL) {
		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	m_pNodeAttr = new CHashTblWSINode();
	if (m_pNodeAttr == NULL) {
		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	// --------------------------------------
	// Global
	LPITEMNODE lpNode = GetNode(WPIKEY_GLOBAL, TRUE);
	if (lpNode == NULL) {
		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	lpNode->global = 1;
	lpNode->type = WPITYPE_KEY;

	// --------------------------------------
	// 未設定時
	LPITEMNODE lpNodeS = GetNode(WPIKEY_COMMON, TRUE);
	if (lpNodeS == NULL) {
		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	lpNodeS->global = 0;
	lpNodeS->type = WPITYPE_KEY;

	if (m_szFileName.IsEmpty()) {
		return TRUE;
	}

	m_bLoaded = FALSE;
	FILE* fp = NULL;
	int lineNo = 0;

	TRY
	{
		errno_t error = _tfopen_s(&fp, m_szFileName, _T("r"));
		if (fp == NULL) {
			m_dwErrorCode = errno;
			return FALSE;
		}

		int dataNo = 0;
		// readline
		CHAR buff[_MAX_LINE];
		while (fgets(buff, _MAX_LINE, fp) != NULL)
		{
			CString line(_AA2W(buff));
			line.Replace(_T("\n"), _T(""));
			line.Trim();
			// 行番号
			lineNo++;
			// 空行
			if (line.IsEmpty()) {
				continue;
			}
#ifndef UNICODE
			// 先頭にある空白を削除
			int pos = 0;
			for (pos = 0; pos < line.GetLength(); pos++) {
				UCHAR ch0 = (UCHAR)line[pos];
				if (ch0 == 0x20 || ch0 == 0x09)
					continue;
				UCHAR ch1 = (UCHAR)line[pos + 1];
				if (ch0 != 0x81 && ch1 != 0x40)
					break;

				// 全角スペース
				if (ch0 == 0x81 && ch1 == 0x40) {
					pos++;
				}
			}
			// 末尾にある空白を削除
			int tpos = line.GetLength();
			for (tpos--; tpos > pos; tpos--) {
				UCHAR ch1 = (UCHAR)line[tpos];
				if (ch1 == 0x20 || ch1 == 0x09)
					continue;
				UCHAR ch0 = (UCHAR)line[tpos - 1];
				// 全角スペース
				if (ch0 == 0x81 && ch1 == 0x40) {
					tpos--;
				}
				else
					break;
			}
			// 文字数
			tpos -= pos;
			tpos++;

			if (tpos > 0) {
				line = line.Mid(pos, tpos);
			}
#endif

			// &STR
			if (line.Find(WPIKEY_STR) == 0)
			{
				LPITEMSTR pStr = GetItemStr(lpNode, line.GetBuffer());
				if (pStr == NULL) {
					m_dwErrorCode = ERROR_INVALID_DATA;
					AfxThrowUserException();
				}
			}
			// &TAG
			else if (line.Find(WPIKEY_TAG) == 0)
			{
				// arakaki 20160819
				// MAXを越えた後のNULLはエラー扱いせず次の行へ
				if (lpNode == NULL && (m_nCount-2 >= m_nKeyMax || m_nCountAttr >= m_nAttMax))
					continue;

				LPITEMTAG pTag = GetItemTag(lpNode, line.GetBuffer());
				if (pTag == NULL) {
					m_dwErrorCode = ERROR_INVALID_DATA;
					AfxThrowUserException();
				}

                // ATTRNAME
				if (_tcsicmp(pTag->pKey, WPIKEY_SETTING) == 0) {
					_tcscpy_s(lpNode->attr, 32, pTag->pData);
				}
			}
			// :keyorwd
			else if (line.Find(WPIKEY_DATAKEY) == 0) 
			{
				size_t len = _tcslen(WPIKEY_DATAKEY);
				line = line.Mid((int)len);
				line.Trim();

				// Find new one
				lpNode = GetNode(line.GetBuffer(), TRUE);
                // 
				if (lpNode != NULL) {
					lpNode->global = 0;
					lpNode->type = WPITYPE_KEY;
				}
			}
            // : Pattern
			else if (line.Find(WPIKEY_PATKEY) == 0)
			{
				size_t len = _tcslen(WPIKEY_PATKEY);
				line = line.Mid((int)len);
				line.Trim();

				// Find new one
				lpNode = GetNode(line.GetBuffer(), TRUE, TRUE);
				if (lpNode != NULL) {
					lpNode->global = 0;
					lpNode->type = WPITYPE_SET;
				}
			}
			// #comment
			else if (line.Find(WPIKEY_COMMENT) == 0)
			{
				continue;
			}
            // データ（有効）の行番号
			dataNo = lineNo;
		}

		if (fp)
			fclose(fp);
		m_bLoaded = TRUE;
	}
	CATCH(CException, e)
	{
		TRACE(_T("line no = %d.\n"), lineNo);
		if (fp)
			fclose(fp);
		m_bLoaded = FALSE;
	}
	END_CATCH

	return m_bLoaded;
}

//-----------------------------------------------------------------------------
// 関数名：WriteEnter
// 説　明：改行コードをファイルに出力する
// 引　数：[IN]  fp   - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteEnter(FILE* fp)
{
	CStringA line("\n");
	size_t len = line.GetLength();

	size_t written = fwrite(line.GetBuffer(), sizeof(CHAR), len, fp);
	if (written != len) {
		return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：WriteStr
// 説　明：仕分け変数情報をファイルに出力する
// 引　数：[IN]  pStr  - 仕分け変数構造体へのポインター
//        [IN]  fp     - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteStr(LPITEMSTR pStr, FILE* fp)
{
	if (fp == NULL)
		return FALSE;

	CHAR line[1024] = { 0 };
	sprintf_s(line, 1024, "&STR,%d,%d,%d,%d,%d,%d,%d,%d\n",
		pStr->no,
		pStr->page,
		pStr->sx,
		pStr->sy,
		pStr->ex,
		pStr->ey,
		(pStr->maxchar == 127) ? 0 : pStr->maxchar,
		pStr->type
		);
	size_t len = strlen(line);
	size_t written = fwrite(line, sizeof(CHAR), len, fp);
	if (written != len) {
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：WriteTag
// 説　明：タグ情報をファイルに出力する
// 引　数：[IN]  pTag  - タグ情報構造体へのポインター
//        [IN]  fp     - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteTag(LPITEMTAG pTag, FILE* fp)
{
	if (fp == NULL)
		return FALSE;

	CHAR* pBuf = NULL;
	size_t len = 7 + (pTag->nKeyLen + pTag->nDataLen) * sizeof(WCHAR);
	pBuf = (CHAR*)calloc(1, len);

	strcpy_s(pBuf, len, "&TAG,");

    // Key
	CStringA key(_W2A(pTag->pKey));
	strcat_s(pBuf, len, key.GetBuffer());

    // conmma
	strcat_s(pBuf, len, ",");

    // Data
	CStringA data(_W2A(pTag->pData));
	strcat_s(pBuf, len, data.GetBuffer());

    // Enter Line
	strcat_s(pBuf, len, "\n");

	len = strlen(pBuf);
	size_t written = fwrite(pBuf, sizeof(CHAR), len, fp);
	if (written != len) {
		_FREE(pBuf);
		return FALSE;
	}
	_FREE(pBuf);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：WriteStrs
// 説　明：仕分け変数情報一覧をファイルに出力する
// 引　数：[IN]  pStrs  - 仕分け変数情報一覧
//        [IN]  fp     - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteStrs(CTypedPtrList<CPtrList, LPITEMSTR>* pStrs, FILE* fp)
{
	if (pStrs == NULL || pStrs->GetCount() == 0 )
		return TRUE;

	if (fp == NULL)
		return FALSE;

	POSITION pos = pStrs->GetHeadPosition();
	while (pos != NULL)
	{
		LPITEMSTR pStr = pStrs->GetNext(pos);
		if (pStr != NULL) {
			if (!WriteStr(pStr, fp)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：WriteTags
// 説　明：タグ情報一覧をファイルに出力する
// 引　数：[IN]  pTags  - タグ情報一覧
//        [IN]  fp     - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteTags(CTypedPtrList<CPtrList, LPITEMTAG>* pTags, FILE* fp)
{
	if (pTags == NULL || pTags->GetCount() == 0)
		return TRUE;

	if (fp == NULL)
		return FALSE;

	POSITION pos = pTags->GetHeadPosition();
	while (pos != NULL)
	{
		LPITEMTAG pTag = pTags->GetNext(pos);
		if (pTag != NULL) {
			if (!WriteTag(pTag, fp)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：WriteNode
// 説　明：設定（ノート）情報をファイルに出力する
// 引　数：[IN]  pNode  - 設定情報へのポインター
//        [IN]  fp     - FILE構造体へのポインター
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL WriteNode(LPITEMNODE pNode, FILE* fp)
{
	if (pNode == NULL || pNode->flag == 1)
		return TRUE;

	if (fp == NULL)
		return FALSE;

	size_t len = 0;
	size_t written = 0;

	TRY
	{
		if (pNode->type == WPITYPE_KEY) {
			// Write Key
			CStringA key;
			key = ":";
			key += _W2A(pNode->key);
			key += "\n";
			len = key.GetLength();
			written = fwrite(key.GetBuffer(), sizeof(CHAR), len, fp);
			if (written != len) {
				//W01ERR_WRITE
				AfxThrowUserException();
			}
		}
		else if (pNode->type == WPITYPE_SET) {
			// Write Key
			CStringA key;
			key = ">";
			key += _W2A(pNode->key);
			key += "\n";
			len = key.GetLength();
			written = fwrite(key.GetBuffer(), sizeof(CHAR), len, fp);
			if (written != len) {
				//W01ERR_WRITE
				AfxThrowUserException();
			}

		}

		// Write Str define
		if (!WriteStrs(pNode->pStrs, fp)) {
			// W01ERR_WRITE
			AfxThrowUserException();
		}
		// Write Tag 
		if (!WriteTags(pNode->pTags, fp)) {
			// W01ERR_WRITE
			AfxThrowUserException();
		}

		if (pNode->pLikeNode != NULL) {
			if (!WriteNode(pNode->pLikeNode, fp)) {
				// W01ERR_WRITE
				AfxThrowUserException();
			}
		}
		else {
			// Enter 
			WriteEnter(fp);
		}
		pNode->flag = 1;

		return TRUE;
	}
	CATCH(CException, e)
	{
		TRACE(_T("Exception occurred during write to file. error:%d\n"), ::GetLastError());
	}
	END_CATCH

	return FALSE;
}

//-----------------------------------------------------------------------------
// 関数名：ResetFlag
// 説　明：ノートの出力済みフラグをリセットする
// 引　数：
//
// 戻り値：
//        
//-----------------------------------------------------------------------------
void CWsiFile::ResetFlag(CHashTblWSINode* pNodes)
{
	if (pNodes == NULL)
		return;

	POSITION pos = pNodes->GetStartPosition();
	while (pos != NULL) {
		LPITEMNODE pNode = NULL;
		CString pKey;
		pNodes->GetNextAssoc(pos, pKey, pNode);
		if (pNode != NULL) {
			pNode->flag = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// 関数名：Save
// 説　明：WSIファイルへ出力する
// 引　数：[IN]  pszFileName  - WSIファイル名
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::Save(LPCTSTR pszFileName)
{
	CString szPath;
	if (pszFileName == NULL) {
        if(m_szFileName.IsEmpty())
		    return FALSE;
	    szPath = m_szFileName;
	}
	else {
		szPath = pszFileName;
	}
	if (m_pNode == NULL ||
		m_pNode->GetCount() == 0)
	{
        // No Data
		return FALSE;
	}

	BOOL bSaved = FALSE;
	FILE* fp = NULL;
	TRY
	{
		errno_t error = _tfopen_s(&fp, szPath, _T("w"));
	    if (fp == NULL) {
		    m_dwErrorCode = errno;
		    return FALSE;
	    }
		size_t written = 0;
		size_t len = 0;
		CString pKey;
		LPITEMNODE pNode = NULL;

        // Reset Flags
		ResetFlag(m_pNode);

        // Write Global
		m_pNode->Lookup(WPIKEY_GLOBAL, pNode);
		if (pNode != NULL) {
			// Write Str define
			if (!WriteStrs(pNode->pStrs, fp)) {
				// W01ERR_WRITE
				AfxThrowUserException();
			}
			// Write Tag 
			if (!WriteTags(pNode->pTags, fp)) {
				// W01ERR_WRITE
				AfxThrowUserException();
			}
            // Enter 
			WriteEnter(fp);

			pNode->flag = 1;
		}

		// Write keys
		CTypedPtrList<CPtrList, LPITEMNODE> order;
		Sort(m_pNode, &order);
		if (order.GetCount() > 0) {
			POSITION pos = order.GetHeadPosition();
			while (pos != NULL) {
				pNode = order.GetNext(pos);
				if (pNode->flag == 1)
					continue;

				// Write Node
				if (!WriteNode(pNode, fp)) {
					// W01ERR_WRITE
					AfxThrowUserException();
				}
			}
		}

		// Write attributes
		// Reset written flag = 0
		ResetFlag(m_pNodeAttr);
		// set map elements to list in order
		order.RemoveAll();
		Sort(m_pNodeAttr, &order);
		if (order.GetCount() > 0) {
			POSITION pos = order.GetHeadPosition();
			while (pos != NULL) {
				pNode = order.GetNext(pos);
				if (pNode->flag == 1)
					continue;

				// Write Node
				if (!WriteNode(pNode, fp)) {
					// W01ERR_WRITE
					AfxThrowUserException();
				}
			}
		}

		fclose(fp);
		bSaved = TRUE;
	}
	CATCH(CException, e)
	{
		if (fp)
			fclose(fp);
	}
	END_CATCH

	return bSaved;
}

//-----------------------------------------------------------------------------
// 関数名：Close
// 説　明：メモリーを開放する。
// 引　数：
//
// 戻り値：
//        
//-----------------------------------------------------------------------------
void CWsiFile::Close()
{
	// Clear Memory
	if (m_pNode != NULL) {
		POSITION pos = m_pNode->GetStartPosition();
		while (pos != NULL) {
			CString pKey;
			LPITEMNODE pNode;
			m_pNode->GetNextAssoc(pos, pKey, pNode);

			_DELETENODE(pNode);
		}
		m_pNode->RemoveAll();
	}

	if (m_pNodeAttr != NULL) {
		POSITION pos = m_pNodeAttr->GetStartPosition();
		while (pos != NULL) {
			CString pKey;
			LPITEMNODE pNode;
			m_pNodeAttr->GetNextAssoc(pos, pKey, pNode);

			_DELETENODE(pNode);
		}
		m_pNodeAttr->RemoveAll();
	}

	_DELETE(m_pNode);
	_DELETE(m_pNodeAttr);

	m_szFileName.Empty();
	m_bLoaded = FALSE;

	m_nCount = 0;
	m_nCountAttr = 0;
}

//-----------------------------------------------------------------------------
// 関数名：CloneStr
// 説　明：仕分け変数のクローンを作成する
// 引　数：[IN]  pStr  - 仕分け変数
//
// 戻り値：関数が成功すると、クローンした仕分け変数が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMSTR CloneStr(LPITEMSTR pStr)
{
	LPITEMSTR ps = (LPITEMSTR)calloc(1, sizeof(ITEMSTR));
	if (ps != NULL) {
		memcpy(ps, pStr, sizeof(ITEMSTR));

		return ps;
    }

	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：CloneTag
// 説　明：タグ情報のクローンを作成する
// 引　数：[IN]  pTag  - タグ情報
//
// 戻り値：関数が成功すると、クローンしたタグ情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMTAG CloneTag(LPITEMTAG pTag)
{
	LPITEMTAG pt = (LPITEMTAG)calloc(1, sizeof(ITEMTAG));
	if (pt != NULL) {
		pt->pData = _tcsdup(pTag->pData);
		pt->pKey = _tcsdup(pTag->pKey);

		pt->nKeyLen = pTag->nKeyLen;
		pt->nDataLen = pTag->nDataLen;

		return pt;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：FindTag
// 説　明：タグ情報を検索する
// 引　数：[IN]  pTags  - タグ情報の格納リスト
//        [IN]  pKey   - タグの名前
//        [IN]  bRemove  - 削除フラグ
//
// 戻り値：関数が成功すると、見つけたタグ情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMTAG FindTag(CTypedPtrList<CPtrList, LPITEMTAG>* pTags, LPCTSTR pKey, BOOL bRemove = FALSE)
{
	// &STR,
	POSITION pos = pTags->GetHeadPosition();
	while (pos != NULL)
	{
		POSITION posv = pos;
		LPITEMTAG pTag = pTags->GetNext(pos);
		if (pTag != NULL) {
			if (_tcscmp(pTag->pKey, pKey) == 0)
			{
				if (bRemove)
					pTags->RemoveAt(posv);

				return pTag;
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：CopyItemNode
// 説　明：設定（ノート）情報をコピーする。
// 引　数：[IN]  pSrc  - コピー元
//        [IN]  pTar   - コピー先
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::CopyItemNode(LPITEMNODE pSrc, LPITEMNODE pTar)
{
	if (pSrc == NULL || pTar == NULL)
		return FALSE;

	pTar->global = pSrc->global;
	pTar->type = pSrc->type;
    
    // Key
	_tcscpy_s(pTar->key, 128, pSrc->key);

    // LIKE
	pTar->pLikeNode = pSrc->pLikeNode;

	// &STR,
	POSITION pos = pSrc->pStrs->GetHeadPosition();
	while(pos != NULL)
	{
		LPITEMSTR pstr = pSrc->pStrs->GetNext(pos);
		if (pstr != NULL) {
			LPITEMSTR ps = CloneStr(pstr);
			if (ps != NULL) {
				pTar->pStrs->AddTail(ps);
			}
		}
	}

	// &TAG,
	pos = pSrc->pTags->GetHeadPosition();
	while (pos != NULL) {
		LPITEMTAG pTag = pSrc->pTags->GetNext(pos);
		if (pTag != NULL) {
			if (_tcsicmp(pTag->pKey, WPIKEY_SETTING) == 0) {
				LPITEMNODE pNode = NULL;
				if (m_pNodeAttr->Lookup(pTag->pData, pNode)) {
					if (pNode != NULL) {
						CopyItemNode(pNode, pTar);
					}
				}
			}
			else {
				LPITEMTAG pt = CloneTag(pTag);
				if (pt != NULL) {
					LPITEMTAG pto= FindTag(pTar->pTags, pTag->pKey);
					if (pto != NULL) {
						_FREE(pto->pData);
						_FREE(pto->pKey);
						// Set Data
						pto->pData = _tcsdup(pt->pData);
						pto->pKey = _tcsdup(pt->pKey);

						pto->nKeyLen = pt->nKeyLen;
						pto->nDataLen = pt->nDataLen;

						_DELETETAG(pt);
					}
					else {
						pTar->pTags->AddTail(pt);
					}
					//pTar->pTags->SetAt(pKey, pt);
				}
			}
		}
	}

	// LIKE
	if (pSrc->pLikeNode != NULL) {
		CopyItemNode(pSrc->pLikeNode, pTar);
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
// 関数名：GetNode
// 説　明：設定（ノート）情報を検索する
// 引　数：[IN]  pKey  - キーワード
//        [IN]  bCreate   - 作成フラグ（TRUE：見つからない場合、新規作成）
//        [IN]  bAttr     - 属性情報フラグ
//
// 戻り値：関数が成功すると、設定（ノート）情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetNode(LPCTSTR pKeyName, BOOL bCreate, BOOL bAttr)
{
	m_dwErrorCode = 0;

	CHashTblWSINode* pNodes = bAttr ? m_pNodeAttr : m_pNode;
	if (pNodes == NULL) {
		return NULL;
	}

	//@QHGOU 20160818 Length Check >>
	CString pKey(pKeyName);
	INT nChrCnt = 0;
	INT nMaxLen = bAttr ? m_nAttLength : m_nKeyLength;
	size_t nLength = GetBytesLength(pKey, nMaxLen, nChrCnt);
	if (nLength > nMaxLen)
		nLength = nMaxLen;
	pKey.GetBufferSetLength(nLength);
	//@QHGOU 20160818 Length Check <<

	LPITEMNODE pNode = NULL;
	if(pNodes->Lookup(pKey, pNode)) {
		return pNode;
	}
	if (pNode != NULL) {
		return pNode;
	}

	if (bCreate) {
		//@QHGOU 20160818 Max Count Check >>
		// Global, Defaultを除く
		INT nNodeCount = bAttr ? m_nCountAttr : m_nCount - 2;
		INT nMaxCount = bAttr ? m_nAttMax : m_nKeyMax;
		if (nMaxCount <= nNodeCount) {
			// 最大数を超えた
			return NULL;
		}
		//@QHGOU 20160818 Max Count Check <<
		// Create a new node
		pNode = CreateNode();
		if (pNode != NULL) {
			// set order
			pNode->order = bAttr ? m_nCountAttr++ : m_nCount++;
			// Type
			pNode->type = bAttr ? WPITYPE_SET : WPITYPE_KEY;

			_tcsncpy_s(pNode->key, pKey, MAX_CHAR_LENGTH );

			pNodes->SetAt(pKey, pNode);
		}
	}

	return pNode;
}

//-----------------------------------------------------------------------------
// 関数名：CreateNode
// 説　明：設定（ノート）情報を新規作成する
// 引　数：[IN]  pKey  - キーワード
//
// 戻り値：関数が成功すると、設定（ノート）情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::CreateNode()
{
	m_dwErrorCode = 0;
	// Create Node
	LPITEMNODE pNode = (LPITEMNODE)calloc(1, sizeof(ITEMNODE));
	if (pNode == NULL) {
		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return NULL;
	}

	pNode->pLikeNode = NULL;
	pNode->pStrs = new CTypedPtrList<CPtrList, LPITEMSTR>();
	pNode->pTags = new CTypedPtrList<CPtrList, LPITEMTAG>();
	if (pNode->pStrs == NULL ||
		pNode->pTags == NULL )
	{
		_DELETE(pNode->pStrs);
		_DELETE(pNode->pTags);
		_FREE(pNode);

		m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
		return NULL;
	}
	pNode->flag = 0;

	return pNode;
}

//-----------------------------------------------------------------------------
// 関数名：GetItemStr
// 説　明：ファイルの行データより、仕分け変数を作成する
// 引　数：[IN]  pNode  - 仕分け変数の格納バッファー
//        [IN]  pLine  - WSIファイルの行データ
//
// 戻り値：関数が成功すると、仕分け変数が返ります。
//        関数が失敗すると、NULLが返ります。
// pLine : &STR,1,1,171,258,359,330,0,1
//-----------------------------------------------------------------------------
LPITEMSTR CWsiFile::GetItemStr(LPITEMNODE pNode, LPTSTR pLine)
{
	if (pNode == NULL ||
		_IsEmpty(pLine))
		return NULL;

	LPITEMSTR pItem = NULL;

	LPTSTR token = NULL;
	LPTSTR next = NULL;
	LPTSTR pEnd = NULL;
	TRY
	{
		pItem = (LPITEMSTR)calloc(1, sizeof(ITEMSTR));
		if (pItem == NULL) {
			// W01ERR_NOTENOUGHMEMORY
			AfxThrowMemoryException();
		}

		int valcnt = 0;
		int index = 0;
		token = _tcstok_s(pLine, WPIKEY_SEPS, &next);
		while (token != NULL)
		{
			pEnd = NULL;
			switch (index)
			{
			case WPISTR_NO:
				pItem->no = (int)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_PAGE:
				pItem->page = (int)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_SX:
				pItem->sx = (unsigned short)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_SY:
				pItem->sy = (unsigned short)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_EX:
				pItem->ex = (unsigned short)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_EY:
				pItem->ey = (unsigned short)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			case WPISTR_MAXCHAR:
				pItem->maxchar = (unsigned short)_tcstol(token, &pEnd, 10);
				if (pItem->maxchar == 0)
					pItem->maxchar = MAX_CHAR_LENGTH;
				valcnt++;
				break;
			case WPISTR_TYPE:
				pItem->type = (unsigned short)_tcstol(token, &pEnd, 10);
				valcnt++;
				break;
			default:
				break;
			}

			// next
			index++;
			token = _tcstok_s(NULL, WPIKEY_SEPS, &next);
		}

		// valid data
		if (valcnt > 0) {
			// Convert W01 dpi to PDF dpi
			//pItem->sx = (unsigned short)(pItem->sx * 72.0 / m_nDPI);
			//pItem->sy = (unsigned short)(pItem->sy * 72.0 / m_nDPI);
			//pItem->ex = (unsigned short)(pItem->ex * 72.0 / m_nDPI);
			//pItem->ey = (unsigned short)(pItem->ey * 72.0 / m_nDPI);

			pNode->pStrs->AddTail(pItem);

			return pItem;
		}
	}
	CATCH(CException, e)
	{

	}
	END_CATCH

	// free
	_FREE(pItem);
	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：GetItemTag
// 説　明：ファイルの行データより、タグ情報を作成する
// 引　数：[IN]  pNode  - タグ情報の格納バッファー
//        [IN]  pLine  - WSIファイルの行データ
//
// 戻り値：関数が成功すると、タグ情報が返ります。
//        関数が失敗すると、NULLが返ります。
// pLine : &TAG,OUTPUT_DIR,C:\PDF\TEST
//-----------------------------------------------------------------------------
LPITEMTAG CWsiFile::GetItemTag(LPITEMNODE pNode, LPTSTR pLine)
{
	if (pNode == NULL ||
		_IsEmpty(pLine))
		return NULL;

	LPITEMTAG pItem = NULL;
	LPTSTR token = NULL;
	LPTSTR next = NULL;
	LPTSTR pEnd = NULL;

	TRY
	{
		CString key;
		CString value;
		pItem = (LPITEMTAG)calloc(1, sizeof(ITEMTAG));
		int valcnt = 0;
		int index = 0;
		token = _tcstok_s(pLine, WPIKEY_SEPS, &next);
		while (token != NULL)
		{
			pEnd = NULL;
			switch (index)
			{
			case WPITAG_KEY:
				{
					key = token;
					key.Trim();

					pItem->nKeyLen = key.GetLength();
					pItem->pKey = (LPTSTR)calloc(pItem->nKeyLen + 1, sizeof(TCHAR));
					if (pItem->pKey == NULL) {
						m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
						_DELETE(pItem);

						return NULL;
					}
					_tcscpy_s(pItem->pKey, pItem->nKeyLen + 1, key.GetBuffer());
					valcnt++;
				}
				break;
			case WPITAG_VALUE:
				{
					value = token;
					value.Trim();
					//value.Replace(L"%", L"%%");

					pItem->nDataLen = value.GetLength();
					pItem->pData = (LPTSTR)calloc(pItem->nDataLen + 1, sizeof(TCHAR));
					if (pItem->pData == NULL) {
						m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
						_FREE(pItem->pKey);
						_DELETE(pItem);

						return NULL;
					}

					_tcscpy_s(pItem->pData, pItem->nDataLen + 1, value.GetBuffer());
					valcnt++;
				}
				break;
			default:
				break;
			}

			// next
			index++;
			token = _tcstok_s(NULL, WPIKEY_SEPS, &next);
		}

		// valid data
		if (valcnt > 0) {
			// Clear Old One
			LPITEMTAG pto = FindTag(pNode->pTags, key.GetBuffer());
			if (pto != NULL) {
				_FREE(pto->pData);
				_FREE(pto->pKey);
				// Set Data
				pto->pData = _tcsdup(pItem->pData);
				pto->pKey = _tcsdup(pItem->pKey);

				pto->nKeyLen = pItem->nKeyLen;
				pto->nDataLen = pItem->nDataLen;

				_DELETETAG(pItem);

				pItem = pto;
			}
			else {
				pNode->pTags->AddTail(pItem);
			}

			return pItem;
		}
		else
		{
			_DELETETAG(pItem);
		}
	}
	CATCH(CException, e)
	{
	}
	END_CATCH

	// free
	_FREE(pItem);
	return NULL;
}


//-----------------------------------------------------------------------------
// 関数名：GetDefaultNode
// 説　明：未設定時データを取得する
// 引　数：
//        
//
// 戻り値：関数が成功すると、未設定時データが返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetDefaultNode()
{
	if (m_pNode == NULL)
		return NULL;

	LPITEMNODE pNode = NULL;
	if(m_pNode->Lookup(WPIKEY_COMMON, pNode))
	{
		return pNode;
	}
	if (pNode != NULL) {
		return pNode;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：GetGlobalNode
// 説　明：共通データを取得する
// 引　数：
//        
//
// 戻り値：関数が成功すると、共通データが返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetGlobalNode()
{
	if (m_pNode == NULL)
		return NULL;

	LPITEMNODE pNode = NULL;
	if (m_pNode->Lookup(WPIKEY_GLOBAL, pNode))
	{
		return pNode;
	}
	if (pNode != NULL) {
		return pNode;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：AddStr
// 説　明：指定した仕分け情報に仕分け変数情報を追加する
// 引　数：[IN]  pItemNode  - 仕分け設定情報 (NULLの場合、「共通」GLOBALに追加する)
//        [IN]  pItemStr  - 仕分け変数情報
//        [IN]  bOverwrite  - 上書き府gラグ
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::AddStr(LPITEMNODE pItemNode, LPITEMSTR pItemStr, BOOL bOverwrite)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return FALSE;
	}

	BOOL bAdd = TRUE;
	POSITION pos = pNode->pStrs->GetHeadPosition();
	while(pos != NULL)
	{
		LPITEMSTR pStr = pNode->pStrs->GetNext(pos);
		if (pStr != NULL) {
			if (pStr->no == pItemStr->no) {
				if (bOverwrite) {
					memcpy(pStr, pItemStr, sizeof(ITEMSTR));

					bAdd = FALSE;
					break;
				}
				return FALSE;
			}
		}
	}

	// 新規追加
	if (bAdd){
		LPITEMSTR pStr = (LPITEMSTR)calloc(1, sizeof(ITEMSTR));
		if (pStr == NULL) {
			return FALSE;
		}
		memcpy(pStr, pItemStr, sizeof(ITEMSTR));

		pNode->pStrs->AddTail(pStr);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：GetStr
// 説　明：指定した仕分け情報から仕分け変数情報を取得する
// 引　数：[IN]  pItemNode  - 仕分け情報 (NULLの場合＝「共通」GLOBAL)
//        [IN]  nIndex  - 仕分け変数の順番(1～10）
//
// 戻り値：関数が成功すると、仕分け変数情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMSTR CWsiFile::GetStr(LPITEMNODE pItemNode, INT nIndex)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return NULL;
	}

	POSITION pos = pNode->pStrs->GetHeadPosition();
	while (pos != NULL)
	{
		LPITEMSTR pStr = pNode->pStrs->GetNext(pos);
		if (pStr != NULL) {
			if (pStr->no == nIndex) {
				return pStr;
			}
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：GetStr
// 説　明：指定した仕分け情報から仕分け変数情報を削除する
// 引　数：[IN]  pItemNode  - 仕分け情報 (NULLの場合＝「共通」GLOBAL)
//        [IN]  nIndex  - 仕分け変数の順番(1～10）
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::RemoveStr(LPITEMNODE pItemNode, INT nIndex)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return TRUE;
	}

	POSITION pos = pNode->pStrs->GetHeadPosition();
	while (pos != NULL)
	{
		POSITION posv = pos;
		LPITEMSTR pStr = pNode->pStrs->GetNext(pos);
		if (pStr != NULL) {
			if (pStr->no == nIndex) {
				_FREE(pStr);
				pNode->pStrs->RemoveAt(posv);

				return TRUE;
			}
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// 関数名：AddTag
// 説　明：指定した仕分け情報からタグ情報を追加する
// 引　数：[IN]  pItemNode  - 仕分け情報 (NULLの場合、「共通」GLOBALに追加する)
//        [IN]  pKey  - タグの名前
//        [IN]  pData  - タグの設定データ
//        [IN]  bOverwrite  - 上書きフラグ名前
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::AddTag(LPITEMNODE pItemNode, LPCTSTR pKey, LPCTSTR pData, BOOL bOverwrite)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return FALSE;
	}

	BOOL bFree = TRUE;
	LPITEMTAG pItem = FindTag(pNode->pTags, pKey);
	if (pItem == NULL) {
		pItem = (LPITEMTAG)calloc(1, sizeof(ITEMTAG));
		bFree = FALSE;
	}

	if (pItem != NULL) {
		// Clear
		if (bOverwrite && bFree) {
			_FREE(pItem->pData);
			pItem->nDataLen = 0;
			_FREE(pItem->pKey);
			pItem->nKeyLen = 0;
		}

		// Key
		pItem->nKeyLen = _tcslen(pKey);
		pItem->pKey = (LPTSTR)calloc(pItem->nKeyLen + 1, sizeof(TCHAR));
		if (pItem->pKey == NULL) {
			m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
			_DELETE(pItem);

			return FALSE;
		}
		_tcscpy_s(pItem->pKey, pItem->nKeyLen + 1, pKey);

		// Value
		pItem->nDataLen = _tcslen(pData);
		pItem->pData = (LPTSTR)calloc(pItem->nDataLen + 1, sizeof(TCHAR));
		if (pItem->pData == NULL) {
			m_dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
			_FREE(pItem->pKey);
			_DELETE(pItem);

			return FALSE;
		}
		_tcscpy_s(pItem->pData, pItem->nDataLen + 1, pData);

		// Add to Node
		if (!bFree) {
			pNode->pTags->AddTail(pItem);
		}

		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// 関数名：GetTag
// 説　明：指定した仕分け情報からタグ情報を取得する
// 引　数：[IN]  pItemNode  - 仕分け情報 (NULLの場合＝「共通」GLOBAL)
//        [IN]  pKey  - タグの名前
//
// 戻り値：関数が成功すると、グ情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMTAG CWsiFile::GetTag(LPITEMNODE pItemNode, LPCTSTR pKey)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return FALSE;
	}

	LPITEMTAG pItem = FindTag(pNode->pTags, pKey);
	if (pItem != NULL) {
		return pItem;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// 関数名：RemoveTag
// 説　明：指定した仕分け情報からタグ情報を削除する
// 引　数：[IN]  pItemNode  - 仕分け情報 (NULLの場合＝「共通」GLOBAL)
//        [IN]  pKey  - タグの名前
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::RemoveTag(LPITEMNODE pItemNode, LPCTSTR pKey)
{
	LPITEMNODE pNode = pItemNode;
	if (pNode == NULL) {
		pNode = GetNode(WPIKEY_GLOBAL);
	}

	if (pNode == NULL) {
		return FALSE;
	}

	LPITEMTAG pItem = FindTag(pNode->pTags, pKey, TRUE);
	if (pItem != NULL) {
		_DELETETAG(pItem);
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// 関数名：GetAttr
// 説　明：属性情報を取得する
// 引　数：[IN]  pKey  - 属性情報の名前
//
// 戻り値：関数が成功すると、属性情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetAttr(LPCTSTR pKey)
{
	return GetKey(pKey, TRUE);
}

//-----------------------------------------------------------------------------
// 関数名：AddAttr
// 説　明：属性情報を新規作成する
// 引　数：[IN]  pKey  - 属性情報の名前
//
// 戻り値：関数が成功すると、属性情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::AddAttr(LPCTSTR pKey)
{
	return AddKey(pKey, TRUE);
}

//-----------------------------------------------------------------------------
// 関数名：RemoveAttr
// 説　明：属性情報を削除する
// 引　数：[IN]  pKey  - 属性情報の名前
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::RemoveAttr(LPCTSTR pKey)
{
	return RemoveKey(pKey, TRUE);
}

//-----------------------------------------------------------------------------
// 関数名：GetFull
// 説　明：指定した仕分け情報の全データをクローンして返ります。
// 引　数：[IN]  pKey  - 仕分け情報の名前
//
// 戻り値：関数が成功すると、仕分け情報が返ります。
//        関数が失敗すると、NULLが返ります。
// 注意  ：GetFullより作成したポインターは必ず_DELETENODEで開放してください。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetFull(LPCTSTR pKey)
{
	CHashTblWSINode* pNodes = m_pNode;
	if (pNodes == NULL) {
		return NULL;
	}

	LPITEMNODE pNodeF = NULL;

	// Create Node
	LPITEMNODE pNode = CreateNode();
	if (pNode == NULL)
		return NULL;

	// Copy common
	pNodes->Lookup(WPIKEY_GLOBAL, pNodeF);
	if (pNodeF != NULL) {
		CopyItemNode(pNodeF, pNode);
	}

	// Copy data to new Object
	if(!pNodes->Lookup(pKey, pNodeF))
	{
		if(!pNodes->Lookup(WPIKEY_COMMON, pNodeF)) {
			pNodeF = NULL;
		}
	}

	// 
	if (pNodeF != NULL) {
		CopyItemNode(pNodeF, pNode);
	}

	return pNode;
}

//-----------------------------------------------------------------------------
// 関数名：GetKey
// 説　明：仕分け情報を取得する
// 引　数：[IN]  pKey  - 仕分け情報の名前
//         [IN]  bAttr  - オブジェクトフラグ (TRUE: 属性情報から FALSE:　仕分け情報から　）
//
// 戻り値：関数が成功すると、仕分け情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::GetKey(LPCTSTR pKey, BOOL bAttr)
{
	CHashTblWSINode* pNodes = bAttr ? m_pNodeAttr : m_pNode;
	if (pNodes == NULL) {
		return NULL;
	}

	LPITEMNODE pNode = NULL;
	if (!pNodes->Lookup(pKey, pNode))
		return NULL;

	return pNode;
}

//-----------------------------------------------------------------------------
// 関数名：AddKey
// 説　明：仕分け情報を新規作成する
// 引　数：[IN]  pKey  - 仕分け情報の名前
//         [IN]  bAttr  - オブジェクトフラグ (TRUE: 属性情報から FALSE:　仕分け情報から　）
//
// 戻り値：関数が成功すると、仕分け情報が返ります。
//        関数が失敗すると、NULLが返ります。
//-----------------------------------------------------------------------------
LPITEMNODE CWsiFile::AddKey(LPCTSTR pKey, BOOL bAttr)
{
	CHashTblWSINode* pNodes = bAttr ? m_pNodeAttr : m_pNode;
	if (pNodes == NULL) {
		return NULL;
	}

	LPITEMNODE pNode = NULL;
	if (!pNodes->Lookup(pKey, pNode)) {
		pNode = GetNode(pKey, TRUE, bAttr);
	}

	// arakaki 20160819
	if (bAttr)
		m_nCountAttr++;
	else
		m_nCount++;

	return pNode;
}

//-----------------------------------------------------------------------------
// 関数名：RemoveKey
// 説　明：仕分け情報を削除する
// 引　数：[IN]  pKey  - 仕分け情報の名前
//         [IN]  bAttr  - オブジェクトフラグ (TRUE: 属性情報から FALSE:　仕分け情報から　）
//
// 戻り値：関数が成功すると、TRUEが返ります。
//        関数が失敗すると、FALSEが返ります。
//-----------------------------------------------------------------------------
BOOL CWsiFile::RemoveKey(LPCTSTR pKey, BOOL bAttr)
{
	CHashTblWSINode* pNodes = bAttr ? m_pNodeAttr : m_pNode;
	if (pNodes == NULL) {
		return TRUE;
	}

	CString szKey(pKey);
	// Remove
	LPITEMNODE pNode = NULL;
	if (pNodes->Lookup(szKey, pNode))
	{
		_DELETENODE(pNode);

		pNodes->RemoveKey(szKey);
	}

	// arakaki 20160819
	if (bAttr)
		m_nCountAttr--;
	else
		m_nCount--;

	return TRUE;
}

//-----------------------------------------------------------------------------
// 関数名：GetKeyList
// 説　明：指定した属性を使用中の仕分け情報一覧を取得する。
// 引　数：[OUT]  pList  - 仕分け情報の格納リスト
//        [IN ] pTagName - タグ名
//        [IN ] pData    - 属性値
//
// 戻り値：関数が成功すると、仕分け情報数が返ります。
//        関数が失敗すると、０が返ります。
//-----------------------------------------------------------------------------
INT CWsiFile::GetKeyList(CTypedPtrList<CPtrList, LPITEMNODE>* pList, LPCTSTR pTagName, LPCTSTR pData)
{
	if (m_pNode == NULL || pList == NULL)
		return 0;

	// Add to list
	POSITION pos = m_pNode->GetStartPosition();
	while (pos != NULL) {
		CString pKey;
		LPITEMNODE pNode = NULL;
		m_pNode->GetNextAssoc(pos, pKey, pNode);
		if (pNode != NULL) {
			LPITEMTAG pTag = GetTag(pNode, pTagName);
			if (pTag != NULL) {
				if (_tcsicmp(pTag->pData, pData) == 0) {
					pList->AddTail(pNode);
				}
			}
		}
	}

	// Sort
	Sort(pList);

	return pList->GetCount();
}

//-----------------------------------------------------------------------------
// 関数名：GetKeyList
// 説　明：仕分け情報一覧を取得する。
// 引　数：[OUT]  pList  - 仕分け情報の格納リスト
//
// 戻り値：関数が成功すると、仕分け情報数が返ります。
//        関数が失敗すると、０が返ります。
//-----------------------------------------------------------------------------
INT CWsiFile::GetKeyList(CTypedPtrList<CPtrList, LPITEMNODE>* pList)
{
	if (m_pNode == NULL || pList == NULL)
		return 0;

	Sort(m_pNode, pList);

	return pList->GetCount();
}

//-----------------------------------------------------------------------------
// 関数名：GetAttrList
// 説　明：属性情報一覧を取得する。
// 引　数：[OUT]  pList  - 属性情報の格納リスト
//
// 戻り値：関数が成功すると、属性情報数が返ります。
//        関数が失敗すると、０が返ります。
//-----------------------------------------------------------------------------
INT CWsiFile::GetAttrList(CTypedPtrList<CPtrList, LPITEMNODE>* pList)
{
	if (m_pNodeAttr == NULL || pList == NULL)
		return 0;

	Sort(m_pNodeAttr, pList);

	return pList->GetCount();
}

//-----------------------------------------------------------------------------
// 関数名：Sort
// 説　明：仕分け情報／属性情報のソートを行う
// 引　数：[IN/OUT ]  pOrder - ソート結果 
//
// 戻り値：
//        
//-----------------------------------------------------------------------------
void CWsiFile::Sort(CTypedPtrList<CPtrList, LPITEMNODE>* pList)
{
	if (pList == NULL ||
		pList->GetCount() < 2)
		return;

	LPITEMNODE p0 = NULL;
	LPITEMNODE p1 = NULL;
	// Sort
	BOOL bChanged = TRUE;
	while (bChanged) {
		bChanged = FALSE;

		POSITION posv = NULL;
		POSITION pos = pList->GetHeadPosition();
		while (pos != NULL) {
			posv = pos;

			p0 = pList->GetNext(pos);

			if (pos != NULL) {
				p1 = pList->GetAt(pos);

				if (p0->order > p1->order) {
					pList->SetAt(posv, p1);
					pList->SetAt(pos, p0);

					bChanged = TRUE;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// 関数名：Sort
// 説　明：仕分け情報／属性情報のソートを行う
// 引　数：[IN ]  pMap  - 仕分け情報のマップ
//        [OUT]  pOrder - ソート結果 
//
// 戻り値：
//        
//-----------------------------------------------------------------------------
void CWsiFile::Sort(CHashTblWSINode* pMap, CTypedPtrList<CPtrList, LPITEMNODE>* pOrder)
{
	if (pMap == NULL || pOrder == NULL)
		return;

	// Add to list
	POSITION pos = pMap->GetStartPosition();
	while (pos != NULL) {
		CString pKey;
		LPITEMNODE pNode = NULL;
		pMap->GetNextAssoc(pos, pKey, pNode);
		if (pNode != NULL) {
			pOrder->AddTail(pNode);
		}
	}

	if (pMap->GetCount() <= 1) {
		return;
	}

	// Sort
	Sort(pOrder);
}

void CWsiFile::SetFileName(LPCTSTR pFileName)
{
	m_szFileName = pFileName;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
// 関数名：Dump
// 説　明：DEBUG用
// 引　数：
//
// 戻り値：
//        
//-----------------------------------------------------------------------------
void CWsiFile::Dump(LPITEMSTR pItem)
{
	if (pItem == NULL)
		return;

	TRACE(_T("\t\tno     : %d\n"), pItem->no);
	TRACE(_T("\t\tpage   : %d\n"), pItem->page);
	TRACE(_T("\t\tsx     : %d\n"), pItem->sx);
	TRACE(_T("\t\tsy     : %d\n"), pItem->sy);
	TRACE(_T("\t\tex     : %d\n"), pItem->ex);
	TRACE(_T("\t\tey     : %d\n"), pItem->ey);
	TRACE(_T("\t\tmaxchar: %d\n"), pItem->maxchar);
	TRACE(_T("\t\ttype   : %d\n"), pItem->type);
	TRACE(_T("\t\tdpi    : %d\n"), pItem->dpi);
}

void CWsiFile::Dump(LPITEMTAG pItem)
{
	if (pItem == NULL)
		return;

	TRACE(_T("\t\tkey       : %s\n"), pItem->pKey);
	TRACE(_T("\t\tkey size  : %d\n"), pItem->nKeyLen);
	CString value(pItem->pData);
	value.Replace(_T("%"), _T("%%"));
	TRACE(_T("\t\tdata      : %s\n"), value);
	TRACE(_T("\t\tdata size : %d\n"), pItem->nDataLen);
}

void CWsiFile::Dump(CTypedPtrList<CPtrList, LPITEMSTR>* pList)
{
	if (pList == NULL)
		return;

	int stridx = 0;
	POSITION pos = pList->GetHeadPosition();
	while( pos != NULL) {
		LPITEMSTR pStr = pList->GetNext(pos);
		if (pStr != NULL) {
			TRACE(_T("\t&STR #%d\n"), stridx++);
			Dump(pStr);
		}
	}
}

void CWsiFile::Dump(CTypedPtrList<CPtrList, LPITEMTAG>* pList)
{
	if (pList == NULL)
		return;

	int tag = 0;
	POSITION pos = pList->GetHeadPosition();
	while (pos != NULL)
	{
		LPITEMTAG pTag = pList->GetNext(pos);
		if (pTag != NULL) {
			TRACE(_T("\t&TAG #%d, %s\n"), tag++, pTag->pKey);
			Dump(pTag);
		}
	}
}

void CWsiFile::Dump(LPITEMNODE pNode)
{
	if (pNode != NULL) {
		// Flag
		TRACE(_T("global   : %d\n"), pNode->global);
		TRACE(_T("type     : %d\n"), pNode->type);
		// Dump Str
		TRACE(_T("&STR size: %d\n"), pNode->pStrs->GetCount());
		Dump(pNode->pStrs);
		// Dump Tag
		TRACE(_T("&TAG size: %d\n"), pNode->pTags->GetCount());
		Dump(pNode->pTags);
		// Like
		TRACE(_T("LikeNode : %p\n"), pNode->pLikeNode);
		Dump(pNode->pLikeNode);
	}
}

void CWsiFile::Dump()
{
	if (m_pNode != NULL) {
		POSITION pos = m_pNode->GetStartPosition();
		while (pos != NULL)
		{
			LPITEMNODE pNode = NULL;
			CString pKey;
			m_pNode->GetNextAssoc(pos, pKey, pNode);
			if (pNode != NULL) {
				TRACE(_T("-- <%s> --\n"), pKey);
                // Flag
				TRACE(_T("global   : %d\n"), pNode->global);
				TRACE(_T("type     : %d\n"), pNode->type);
				TRACE(_T("order    : %d\n"), pNode->order);
				// Dump Str
				TRACE(_T("&STR size: %d\n"), pNode->pStrs->GetCount());
				Dump(pNode->pStrs);
				// Dump Tag
				TRACE(_T("&TAG size: %d\n"), pNode->pTags->GetCount());
				Dump(pNode->pTags);
			}
		}
	}
}
#endif
