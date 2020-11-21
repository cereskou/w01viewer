
#ifndef __WSIFILE_H__
#define __WSIFILE_H__
#pragma once

// ----------------------------------------------------------------------------
// MACRO
// ----------------------------------------------------------------------------
#ifndef _FREE
#define _FREE(p) if( p != NULL ) { free(p); p = NULL; }
#endif
#ifndef _DELETE
#define _DELETE(p) if( (p) != NULL ) { delete (p); (p) = NULL; }
#endif
#ifndef _DELETEM
#define _DELETEM(p) if( (p) ) { delete[] (p); (p) = NULL; }
#endif
#ifndef _RELEASE
#define _RELEASE(p) if( (p) ) { (p).Release(); (p) = NULL; }
#endif
#ifndef _RELEASEP
#define _RELEASEP(p) if( (p) ) { (p)->Release(); (p) = NULL; }
#endif
#ifndef _CLOSE
#define _CLOSE(p) if( (p) != NULL ) { ::CloseHandle((p)); (p) = NULL; }
#endif

#define _MAX_LINE		1024

#define MAX_WSISTR			10			// 文字変数数最大値
#define MAX_WSIKEY			1024		// 仕分けキー数最大値
#define MAX_WSIATTRIBUTE	1024		// 属性情報数最大値

#define WPIKEY_DATAKEY		_T(":")
#define WPIKEY_PATKEY		_T(">")
#define WPIKEY_COMMENT		_T("#")
#define WPIKEY_STR			_T("&STR")
#define WPIKEY_TAG			_T("&TAG")

#define WPIKEY_GLOBAL		_T(":$$")
#define WPIKEY_COMMON		_T("未設定時")

#define WPIKEY_SETTING      _T("ATTRNAME")

#define WPIKEY_SEPS			_T(",")

// Max data length
#define MAX_CHAR_LENGTH		127

// DPI
#define W01_DPI				240

// Key Flag
#define WPIKEY_STRKEY		1

// &STR
#define WPISTR_NO			1
#define WPISTR_PAGE			2
#define WPISTR_SX			3
#define WPISTR_SY			4
#define WPISTR_EX			5
#define WPISTR_EY			6
#define WPISTR_MAXCHAR		7
#define WPISTR_TYPE			8

// &TAG
#define WPITAG_KEY			1
#define WPITAG_VALUE		2

// Type
#define WPITYPE_KEY         0
#define WPITYPE_SET         1


/*-------------------------------------------*/
/* ユーザー情報タグ                          */
/*-------------------------------------------*/
#define  WSI_DEF_TAG_FORMNO                "FORMNO"
#define  WSI_DEF_TAG_FORMNAME              "FORMNAME"
#define  WSI_DEF_TAG_DATE                  "DATE"
#define  WSI_DEF_TAG_FORMDATE              "FORMDATE"
#define  WSI_DEF_TAG_GROUP                 "GROUP"
#define  WSI_DEF_TAG_FORMTYPE              "FORMTYPE"
#define  WSI_DEF_TAG_SENDDATE              "SENDDATE"
#define  WSI_DEF_TAG_DELDATE               "DELDATE"
#define  WSI_DEF_TAG_SECCLASS              "SECCLASS"				// 帳票管理クラス
#define  WSI_DEF_TAG_USER_HEADER           "USER_HEADER"
#define  WSI_DEF_TAG_FORMID                "FORMID"
#define  WSI_DEF_TAG_CELL_DEF_FILE         "CELL_DEF_FILE"
#define  WSI_DEF_TAG_MOVEDATE              "MOVE_DATE"
#define  WSI_DEF_TAG_MOVE_DIR              "MOVE_DIR"
#define  WSI_DEF_TAG_INFOID					"INFOID"
#define  WSI_DEF_TAG_W01DIR					"OUTPUT_DIR"
#define	 WSI_DEF_TAG_SYSTEM_FLAG			"SYSTEM_FLAG"

// Sample
// &TAG,USER_HEADER,資金繰実績表_%S01年%S02月
// &STR,1,1,288,322,374,372,0,0
// &STR,2,1,388,320,428,374,0,0
//
// Foramt
// &STR, xx, pp, [startX],[startY],[endX],[endY],[maxChar],[type]
// xx		割り当て番号（１～１０）
// pp       ページ番号（常に１）
// startX	矩形開始座標X（ピクセル）
// startY	矩形開始座標Y（ピクセル）
// endX		矩形終了座標X（ピクセル）
// endY		矩形終了座標Y（ピクセル）
// maxChar	含まれる最大文字数（０～１２７）
// type		仕分けフラグ（0: しない	1: する）

#pragma pack(push, 1)
typedef struct _itemStr
{
	int				no;
	int				page;
	unsigned short	sx;
	unsigned short	sy;
	unsigned short	ex;
	unsigned short	ey;
	short			maxchar;
	short			type;
	int				dpi;
} ITEMSTR, *LPITEMSTR;

typedef struct _itemTag
{
	LPTSTR	pKey;
	size_t	nKeyLen;
	LPTSTR	pData;
	size_t	nDataLen;
} ITEMTAG, *LPITEMTAG;

typedef struct _itemNode
{
	TCHAR   key[MAX_CHAR_LENGTH+1];	// キーワード
	TCHAR	attr[33];	// 属性情報ID
	BYTE    global;
	BYTE    type;
	BYTE    flag;       // 0: not output 1: outputed
	INT		order;		// order

	// &STR,
	CTypedPtrList<CPtrList, LPITEMSTR>*	pStrs;
	// &TAG,
	CTypedPtrList<CPtrList, LPITEMTAG>*	pTags;

	_itemNode*	pLikeNode;
} ITEMNODE, *LPITEMNODE;

typedef CTypedPtrMap<CMapStringToPtr, CString, LPITEMNODE>	CHashTblWSINode;

// ----------------------------------------------------------------------------
// Free function
inline void _DELETETAG(LPITEMTAG& pTag) {
	if (pTag != NULL) {
		_FREE(pTag->pKey);
		_FREE(pTag->pData);

		_FREE(pTag);
	}
}

inline void _DELETESTRS(CTypedPtrList<CPtrList, LPITEMSTR>*& pStrs) {
	POSITION pos = pStrs->GetHeadPosition();
	while (pos != NULL) {
		LPITEMSTR pStr = pStrs->GetNext(pos);
		if (pStr != NULL) {
			_FREE(pStr);
		}
	}
	pStrs->RemoveAll();

	_DELETE(pStrs);
}

inline void _DELETETAGS(CTypedPtrList<CPtrList, LPITEMTAG>*& pTags) {
	POSITION pos = pTags->GetHeadPosition();
	while (pos != NULL) {
		LPITEMTAG pTag = pTags->GetNext(pos);
		if (pTag != NULL) {
			_DELETETAG(pTag);
		}
	}
	pTags->RemoveAll();

	_DELETE(pTags);
}

inline void _DELETENODE(LPITEMNODE& pNode)
{
	if (pNode != NULL) {
		_DELETETAGS(pNode->pTags);
		_DELETESTRS(pNode->pStrs);

		_FREE(pNode);
	}
}
#pragma pack(pop)

// ----------------------------------------------------------------------------
// class WSI file
// ----------------------------------------------------------------------------
class CWsiFile {
public:
	CWsiFile();
	CWsiFile(LPCTSTR pszFileName);
	virtual ~CWsiFile();

#ifdef _DEBUG
	void Dump();
	void Dump(LPITEMNODE pNode);
    void Dump(LPITEMSTR pItem);
	void Dump(LPITEMTAG pItem);
	void Dump(CTypedPtrList<CPtrList, LPITEMSTR>* pList);
	void Dump(CTypedPtrList<CPtrList, LPITEMTAG>* pList);
#endif
	INT GetKeyList(CTypedPtrList<CPtrList, LPITEMNODE>* pList);
	INT GetAttrList(CTypedPtrList<CPtrList, LPITEMNODE>* pList);

	INT GetKeyList(CTypedPtrList<CPtrList, LPITEMNODE>* pList, LPCTSTR pTagName, LPCTSTR pData);

	LPITEMNODE GetFull(LPCTSTR pKey);

	LPITEMNODE GetKey(LPCTSTR pKey, BOOL bAttr = FALSE);
	LPITEMNODE AddKey(LPCTSTR pKey, BOOL bAttr = FALSE);
	BOOL RemoveKey(LPCTSTR pKey, BOOL bAttr = FALSE);

	LPITEMNODE GetAttr(LPCTSTR pKey);
	LPITEMNODE AddAttr(LPCTSTR pKey);
	BOOL RemoveAttr(LPCTSTR pKey);

	BOOL AddTag(LPITEMNODE pItemNode, LPCTSTR pKey, LPCTSTR pData, BOOL bOverwrite = TRUE);
	LPITEMTAG GetTag(LPITEMNODE pItemNode, LPCTSTR pKey);
	BOOL RemoveTag(LPITEMNODE pItemNode, LPCTSTR pKey);

	BOOL AddStr(LPITEMNODE pItemNode, LPITEMSTR pItemStr, BOOL bOverwrite = TRUE);
	LPITEMSTR GetStr(LPITEMNODE pItemNode, INT nIndex);
	BOOL RemoveStr(LPITEMNODE pItemNode, INT nIndex);

	LPITEMNODE GetGlobalNode();
	LPITEMNODE GetDefaultNode();

	BOOL Load(LPCTSTR pszFileName = NULL);
	BOOL Save(LPCTSTR pszFileName = NULL);
	
	void Close();

	BOOL IsValide() { return m_bLoaded; }
	DWORD GetErrorCode() { return m_dwErrorCode; }

	void SetFileName(LPCTSTR pFileName);

	CHashTblWSINode* GetRoot() { return m_pNode; }

	//@QHGOU 20160818 
	void SetKeyMaxCount(INT nCount = 1024) { m_nKeyMax = nCount; }
	void SetAttMaxCount(INT nCount = 1024) { m_nAttMax = nCount; }

	void SetKeyNameMaxLength(INT nLen = 127) { m_nKeyLength = nLen; }
	void SetAttNameMaxLength(INT nLen = 32) { m_nAttLength = nLen; }
	//@QHGOU 20160818 <<

protected:
	
	LPITEMNODE CreateNode();
	LPITEMNODE GetNode(LPCTSTR pKey, BOOL bCreate = FALSE, BOOL bAttr = FALSE);

	LPITEMSTR GetItemStr(LPITEMNODE pNode, LPTSTR pLine);
	LPITEMTAG GetItemTag(LPITEMNODE pNode, LPTSTR pLine);

	BOOL CopyItemNode(LPITEMNODE pSrc, LPITEMNODE pTar);

	void ResetFlag(CHashTblWSINode* pNodes);

	// Sort
	void Sort(CHashTblWSINode* pMap, CTypedPtrList<CPtrList, LPITEMNODE>* pOrder);
	void Sort(CTypedPtrList<CPtrList, LPITEMNODE>* pList);
private:
	DWORD		        m_dwErrorCode;
	BOOL		        m_bLoaded;
	CString		        m_szFileName;
	SHORT		        m_nDPI;
	CHashTblWSINode*	m_pNode;
	CHashTblWSINode*	m_pNodeAttr;

	//@QHGOU 20160818 >>
	INT		m_nKeyMax;
	INT		m_nAttMax;

	INT		m_nKeyLength;
	INT		m_nAttLength;
	//@QHGOU 20160818 <<

	INT	m_nCount;
	INT m_nCountAttr;
};

#endif
