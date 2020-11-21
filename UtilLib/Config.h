#ifndef __CONFIG_H__
#define __CONFIG_H__
#pragma once

#include "hashtbl.h"

#define KEY_NONE				0x0000
#define KEY_MESSAGE				0xF000

namespace UTL {

// XML item
typedef struct _ITEMDATA
{
	unsigned long	type;
	size_t			nKeyLen;
	LPTSTR			pKey;
	size_t			nDataLen;
	LPTSTR			pData;

	LinkedList<_ITEMDATA*>*	pChildItems;
} ITEMDATA, *LPITEMDATA;
typedef LinkedList<LPITEMDATA>	CLinkedListItem;


//<MSG ID="0001" CODE="ERROR_EXCEPTION" MSG="Error message"/>
typedef struct _ITEMLNK
{
	int			index;
	LPTSTR		pAttrName;
} ITEMLINK, *LPITEMLINK;
typedef LinkedList<LPITEMLINK>	CLinkedListItemLink;

// for Search
struct _freeItemULongP
{
	void operator()(ULONG key, LPITEMDATA lpItem) const
	{
	}
};
typedef HashTbl<ULONG, LPITEMDATA, _freeItemULongP>	CHashTblULongToItem;

typedef HashTbl<ULONG, INT> CHashTblUlongToInt;

class Config
{
public:
	Config();
	Config(LPCTSTR lpszFileName);
	~Config();

public:
	unsigned long genXmlKey(LPCTSTR key);

	BOOL Load(LPCTSTR lpszFileName);

	LPCTSTR GetString(LPCTSTR pszKey, LPCTSTR pszDefault = _T(""));
	LONG   GetLong(LPCTSTR pszKey, LONG lDefault = -1);
	INT GetInteger(LPCTSTR pszKey, INT lDefault = -1);
	double GetDouble(LPCTSTR pszKey, double lDefault = 0.0);
	BOOL   GetBoolean(LPCTSTR pszKey, BOOL bDefault = FALSE);

	LPCTSTR GetString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	LONG   GetLong(LPCTSTR pszSection, LPCTSTR pszKey, LONG lDefault);
	INT GetInteger(LPCTSTR pszSection, LPCTSTR pszKey, INT lDefault);
	double GetDouble(LPCTSTR pszSection, LPCTSTR pszKey, double lDefault);
	BOOL   GetBoolean(LPCTSTR pszSection, LPCTSTR pszKey, BOOL bDefault);

	LPCTSTR GetString(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, LPCTSTR pszDefault = _T(""));
	LONG   GetLong(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, LONG lDefault = -1);
	INT	   GetInteger(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, INT lDefault = -1);
	double GetDouble(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, double lDefault = 0.0);
	BOOL   GetBoolean(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, BOOL bDefault = FALSE);

	LPCTSTR GetStringById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, LPCTSTR pszDefault = _T(""));
	LONG   GetLongById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, LONG lDefault = -1);
	INT   GetIntegerById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, INT lDefault = -1);
	double   GetDoubleById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, double lDefault = 0.0);
	BOOL   GetBooleanById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, BOOL bDefault = FALSE);

	DWORD GetLastError() { return m_dwErrorCode; }
	BOOL IsValid();

#ifdef _DEBUG
	void Dump();
#endif
protected:
	BOOL Init();
	LPITEMDATA GetItem(LPCTSTR key);
	BOOL AddItem(LPCTSTR key, LPITEMDATA pItem, BOOL bAddToList = TRUE);

	INT GetMultiKeyCount(LPCTSTR key);
	void SetMultiKeyCount(LPCTSTR key, INT count);

	//LPITEMDATA GetItem(LPCWSTR key);
	//BOOL AddItem(LPCWSTR key, LPITEMDATA pItem, BOOL bAddToList = TRUE);

	//INT GetMultiKeyCount(LPCWSTR key);
	//void SetMultiKeyCount(LPCWSTR key, INT count);
private:
	DWORD m_dwErrorCode;

	CLinkedListItem*		m_pList;
	CHashTblULongToItem*	m_pSrch;
	CHashTblUlongToInt*		m_pOff;
};

} // namespace UTL

#endif
