#ifndef __CONFIG_H__
#define __CONFIG_H__
#pragma once

#include "hashtbl.h"
#include <string>
using namespace std;

#pragma pack (1)

// XML item
struct _ITEMDATA;
typedef HashTbl<ULONG, _ITEMDATA*>	CHashTblStringToItem;
typedef struct _ITEMDATA
{
	unsigned long	type;
	size_t			nKeyLen;
	wchar_t*		pKey;
	size_t			nDataLen;
	wchar_t*		pData;

	_ITEMDATA*				pParent;		// êe
	CHashTblStringToItem*	pMapAttrs;		// ëÆê´
} ITEMDATA, *LPITEMDATA;

typedef LinkedList<LPITEMDATA>	CLinkedListToItem;

struct _freeMapItemDataP2
{
	void operator()(ULONG key, LPITEMDATA lpItem) const
	{
	}
};
typedef HashTbl<ULONG, LPITEMDATA, _freeMapItemDataP2> CHashTblULongToItem;
#pragma pack ()

class Config
{
public:
	Config();
	Config(LPCWSTR lpszFileName, BOOL bOverwrite = FALSE);
	Config(LPCSTR lpszFileName, BOOL bOverwrite = FALSE);

	~Config();

public:
	unsigned long genXmlKey(LPCWSTR key);

	BOOL Load(LPCWSTR lpszFileName);

	LPWSTR GetString(LPWSTR pszSection, LPWSTR pszKey, LPWSTR pszDefault = L"");
	LONG   GetLong(LPWSTR pszSection, LPWSTR pszKey, LONG lDefault=-1);
	INT    GetInt(LPWSTR pszSection, LPWSTR pszKey, INT lDefault=-1);
	double GetDouble(LPWSTR pszSection, LPWSTR pszKey, double dDefault=0.0);
	BOOL   GetBoolean(LPWSTR pszSection, LPWSTR pszKey, BOOL bDefault=FALSE);

	BOOL Select(LPWSTR pszSection);
	LPWSTR GetStringEx(LPWSTR pszKey, LPWSTR pszDefault = L"");
	LONG   GetLong(LPWSTR pszKey, LONG lDefault=-1);
	INT    GetInt(LPWSTR pszKey, INT lDefault=-1);
	double GetDouble(LPWSTR pszKey, double dDefault=0.0);
	BOOL   GetBoolean(LPWSTR pszKey, BOOL bDefault=FALSE);

	LPWSTR GetAttributeString(LPWSTR pszKey, INT index, LPWSTR pszAttr, LPWSTR pszDefault = L"");
	LONG   GetAttributeLong(LPWSTR pszKey, INT index, LPWSTR pszAttr, LONG lDefault=-1);
	INT    GetAttributeInt(LPWSTR pszKey, INT index, LPWSTR pszAttr, INT lDefault=-1);
	double GetAttributeDouble(LPWSTR pszKey, INT index, LPWSTR pszAttr, double dDefault=0.0);
	BOOL   GetAttributeBoolean(LPWSTR pszKey, INT index, LPWSTR pszAttr, BOOL bDefault=FALSE);

	LPWSTR GetAttributeString(LPWSTR pszKey, LPWSTR pszAttr, LPWSTR pszDefault = L"");
	LONG   GetAttributeLong(LPWSTR pszKey, LPWSTR pszAttr, LONG lDefault=-1);
	INT    GetAttributeInt(LPWSTR pszKey, LPWSTR pszAttr, INT lDefault=-1);
	double GetAttributeDouble(LPWSTR pszKey, LPWSTR pszAttr, double dDefault=0.0);
	BOOL   GetAttributeBoolean(LPWSTR pszKey, LPWSTR pszAttr, BOOL bDefault=FALSE);

	DWORD GetLastError() { return m_dwErrorCode; }
	BOOL IsValid(LPWSTR pszRoot);

#ifdef _DEBUG
	//void Dump();
#endif
protected:
	BOOL Init();
	LPITEMDATA GetItem(LPCWSTR key);
	LPITEMDATA GetItem(LPCWSTR section, LPCWSTR key);

	BOOL AddItem(LPCWSTR key, LPITEMDATA pItem, BOOL bAddToList = TRUE);

	bool GetKey(LPITEMDATA pItem, std::wstring& wszpKey);

private:
	DWORD					m_dwErrorCode;
	LPITEMDATA				m_pRoot;

	std::wstring			m_szSection;
	std::wstring            m_szRoot;

	BOOL                    m_bOverwrite;
	CLinkedListToItem*		m_pList;
	CHashTblULongToItem*	m_pSrch;
};

#endif
