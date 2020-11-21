#include "stdafx.h"
#include "Config.h"
#include "cstring.h"

#include <tchar.h>
#include <shlobj.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <atlbase.h>
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

#include "trace.h"

namespace UTL {

#ifdef _UNICODE
#define _EQU(x)		(x)
#else
#define _EQU(x)		_CW2A(x)
#endif

void _freeItemLink(LPITEMLINK pLnk)
{
	if (pLnk)
	{
		_FREE(pLnk->pAttrName);

		_FREE(pLnk);
	}
}

void _freeConfigItems(LPITEMDATA pItem)
{
	if (pItem)
	{
		_DELETE(pItem->pChildItems);
		_FREE(pItem->pData);
		_FREE(pItem->pKey);

		_FREE(pItem);
	}
}

Config::Config()
	: m_pSrch(NULL)
	, m_pList(NULL)
	, m_pOff(NULL)
	, m_dwErrorCode(0)
{
	Init();
}

Config::Config(LPCTSTR lpszFileName)
	: m_pSrch(NULL)
	, m_pList(NULL)
	, m_pOff(NULL)
	, m_dwErrorCode(0)
{
	if (Init())
	{
		if (Load(lpszFileName))
		{
			// OK
		}
	}
}

Config::~Config()
{
	_DELETE(m_pSrch);
	_DELETE(m_pList);
	_DELETE(m_pOff);
}

BOOL Config::IsValid()
{
	if (m_pList == NULL ||
		m_pSrch == NULL)
		return FALSE;

	if (m_pList->size() == 0)
		return FALSE;

	if (m_pSrch->size() == 0)
		return FALSE;

	return TRUE;
}

BOOL Config::Init()
{
	_DELETE(m_pList);
	_DELETE(m_pSrch);
	_DELETE(m_pOff);

	m_pList = new CLinkedListItem(true, _freeConfigItems);
	if (m_pList == NULL)
	{
		goto ConfigInit_Err;
	}

	m_pSrch = new CHashTblULongToItem();
	if (m_pSrch == NULL ||
		m_pSrch->create(genHashKey) == NULL)
	{
		goto ConfigInit_Err;
	}

	m_pOff = new CHashTblUlongToInt();
	if (m_pOff == NULL ||
		m_pOff->create(genHashKey) == NULL)
	{
		goto ConfigInit_Err;
	}

	return TRUE;

ConfigInit_Err:

	// Clearn
	_DELETE(m_pList);
	_DELETE(m_pSrch);
	_DELETE(m_pOff);

	return FALSE;
}

// Load from XML
BOOL Config::Load(LPCTSTR lpszFileName)
{
	afxTRACE(_T("Load xml %s"), lpszFileName);
	m_dwErrorCode = 0;

#ifdef _MSC_VER
	CString szPath = lpszFileName;

	if (!::PathFileExists(lpszFileName))
	{
		TCHAR szXml[_MAX_PATH] = { 0 };
		_tcscpy_s(szXml, _MAX_PATH, lpszFileName);

		szPath = GetCurrentModulePath();
		if (szPath.IsEmpty()) {
			return FALSE;
		}

		if (::PathIsFileSpec(szXml))
		{
			::PathStripPath(szXml);
		}

		szPath += szXml;
	}
#endif
	HRESULT hr = 0;
	IStream* pStream = NULL;

	hr = ::SHCreateStreamOnFile(szPath, STGM_READ, &pStream);
	if (FAILED(hr))
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("SHCreateStreamOnFile failed. error:%d"), m_dwErrorCode);
		// throw m_dwErrorCode;

		return FALSE;
	}

	IXmlReader* pReader = NULL;
	hr = ::CreateXmlReader(IID_IXmlReader, (void**)&pReader, NULL);
	if (FAILED(hr))
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("CreateXmlReader failed. error:%d"), m_dwErrorCode);
		_RELEASEP(pStream);

		return FALSE;
	}
	//// Encode
	//IXmlReaderInput* pReaderInput = NULL;
	//hr = ::CreateXmlReaderInputWithEncodingName(pStream, NULL, L"UTF-16BE", FALSE, NULL, &pReaderInput);
	//if( FAILED(hr) )
	//{
	//	m_dwErrorCode = ::GetLastError();
	//	afxTRACE(_T("CreateXmlReaderInputWithEncodingName failed. error:%d"), m_dwErrorCode);
	//	_RELEASEP(pReader);
	//	_RELEASEP(pStream);

	//	return FALSE;
	//}

	hr = pReader->SetInput(pStream);
	if (FAILED(hr))
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("IXmlReader::SetInput failed. error:%d"), m_dwErrorCode);
		//_RELEASEP(pReaderInput);
		_RELEASEP(pReader);
		_RELEASEP(pStream);

		return FALSE;
	}

	LPCWSTR pwszLocalName = NULL;
	LPCWSTR pwszValue = NULL;
	CString szLocalName;
	CString szValue;
	CString szAttrName;
	CString szAttrValue;

	XmlNodeType nodeType;

	LPWSTR pszEnd = NULL;
	LPITEMDATA pItem = NULL;
	INT nCount = 0;
	INT nSubNo = 0;

	CString szSubKey;
	CString szIdValue;

	CLinkedListItemLink* pItemLink = new CLinkedListItemLink(true, _freeItemLink);

	while ((hr = pReader->Read(&nodeType)) == S_OK)
	{
		pszEnd = NULL;
		switch (nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			if (FAILED(hr))
			{
				m_dwErrorCode = ::GetLastError();
				afxTRACE(_T("IXmlReader::GetLocalName failed. error:%d"), m_dwErrorCode);
				//_RELEASEP(pReaderInput);
				_RELEASEP(pReader);
				_RELEASEP(pStream);

				_DELETE(pItemLink);

				return FALSE;
			}

			// Set Local Name
			szLocalName = _EQU(pwszLocalName);

			pItem = GetItem(szLocalName);
			if (pItem == NULL)
			{
				pItem = (LPITEMDATA)calloc(1, sizeof(ITEMDATA));
				if (pItem)
				{
					pItem->nKeyLen = szLocalName.GetLength();
					pItem->pKey = (LPTSTR)calloc(pItem->nKeyLen + 1, sizeof(TCHAR));
					if (pItem->pKey)
					{
						_tcscpy_s(pItem->pKey, pItem->nKeyLen + 1, szLocalName);
					}

					AddItem(szLocalName, pItem);
				}
			}

			nCount = 0;
			nSubNo = GetMultiKeyCount(szLocalName);

			szIdValue.Empty();
			pItemLink->clear();

			// Attributes
			for (hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				LPCWSTR pwszAttrName;
				LPCWSTR pwszAttrValue;
				hr = pReader->GetLocalName(&pwszAttrName, NULL);
				if (FAILED(hr))
					break;
				hr = pReader->GetValue(&pwszAttrValue, NULL);
				if (FAILED(hr))
					break;

				if (pItem->pChildItems == NULL)
				{
					pItem->pChildItems = new CLinkedListItem(true, _freeConfigItems);
					if (pItem->pChildItems == NULL)
					{
						m_dwErrorCode = ::GetLastError();
						afxERROR(_T("new CLinkedListItem failed. error:%d"), m_dwErrorCode);

						//_RELEASEP(pReaderInput);
						_RELEASEP(pReader);
						_RELEASEP(pStream);

						_DELETE(pItemLink);

						return FALSE;
					}
				}

				szAttrName = _EQU(pwszAttrName);
				szAttrValue = _EQU(pwszAttrValue);

				// SubKey
				szSubKey.Format(_T("%s.%s#%lu"), szLocalName.GetBuffer(), szAttrName.GetBuffer(), nSubNo);

				LPITEMDATA pSubItem = GetItem(szSubKey);
				if (pSubItem == NULL)
				{
					pSubItem = (LPITEMDATA)calloc(1, sizeof(ITEMDATA));
					if (pSubItem)
					{
						// Set Key
						pSubItem->nKeyLen = szAttrName.GetLength();
						pSubItem->pKey = (LPTSTR)calloc(pSubItem->nKeyLen + 1, sizeof(TCHAR));
						if (pSubItem->pKey)
						{
							_tcscpy_s(pSubItem->pKey, pSubItem->nKeyLen + 1, szAttrName);
						}

						AddItem(szSubKey, pSubItem);
					}

					szIdValue = _T("0");
				}

				if (pItemLink)
				{
					LPITEMLINK pItemLnk = (LPITEMLINK)calloc(1, sizeof(ITEMLINK));
					if (pItemLnk)
					{
						pItemLnk->index = nSubNo;
						pItemLnk->pAttrName = _tcsdup(szAttrName);

						pItemLink->add(pItemLnk);
					}
				}

				if (szAttrName.Compare(_T("ID")) == 0) {
					szIdValue = _EQU(pwszAttrValue);
				}

				// Set Value
				if (pSubItem)
				{
					pSubItem->nDataLen = szAttrValue.GetLength();
					pSubItem->pData = (LPTSTR)calloc(pSubItem->nDataLen + 1, sizeof(TCHAR));
					if (pSubItem->pData)
					{
						_tcscpy_s(pSubItem->pData, pSubItem->nDataLen + 1, szAttrValue);
					}
				}
				nCount++;
			}
			if (nCount > 0)
			{
				nSubNo++;

				SetMultiKeyCount(szLocalName, nSubNo);

				if (!szIdValue.IsEmpty() &&
					pItemLink != NULL &&
					pItemLink->size() > 0)
				{
					pItemLink->first();
					do {
						LPITEMLINK pItemLnk = pItemLink->get();
						if (pItemLnk)
						{
							CString szIdValTmp = szIdValue;
							// Restore Key
							szSubKey.Format(_T("%s.%s#%lu"), szLocalName.GetBuffer(), pItemLnk->pAttrName, pItemLnk->index);
							LPITEMDATA pOrgItemData = GetItem(szSubKey);
							if (pOrgItemData)
							{
								if (!szIdValTmp.startsWith(_T("0x"))) {
									long nVal = _tcstol(szIdValTmp.GetBuffer(), NULL, 10);

									szIdValTmp.Format(_T("0x%X"), nVal);
								}
								//szIdValTmp.MakeUpper();

								CString szSubKeyN;
								// Generator Key
								szSubKeyN = szLocalName;
								szSubKeyN += _T(".");
								szSubKeyN += pItemLnk->pAttrName;
								szSubKeyN += _T("#");
								szSubKeyN += szIdValTmp;

								AddItem(szSubKeyN.GetBuffer(), pOrgItemData, FALSE);
							}
						}
					} while (pItemLink->next());
				}
				pItemLink->clear();
				szIdValue.Empty();
			}

			break;
		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			if (FAILED(hr))
			{
				m_dwErrorCode = ::GetLastError();
				afxTRACE(_T("IXmlReader::GetLocalName failed. error:%d"), m_dwErrorCode);
				//_RELEASEP(pReaderInput);
				_RELEASEP(pReader);
				_RELEASEP(pStream);

				_DELETE(pItemLink);

				return FALSE;
			}

			//afxTRACE(_T("</%s>"), pwszLocalName);

			// Clear
			pItem = NULL;

			break;
		case XmlNodeType_Text:
			hr = pReader->GetValue(&pwszValue, NULL);
			if (FAILED(hr))
			{
				break;
			}

			szValue = _EQU(pwszValue);

			pItem = GetItem(szLocalName);
			if (pItem != NULL)
			{
				_FREE(pItem->pData);

				pItem->nDataLen = szValue.GetLength();
				pItem->pData = (LPTSTR)calloc(pItem->nDataLen + 1, sizeof(TCHAR));
				if (pItem->pData) {
					_tcscpy_s(pItem->pData, pItem->nDataLen + 1, szValue);
				}
			}

			//afxTRACE(_T("\t%s"), pwszValue);

			break;
		}
	}
	_DELETE(pItemLink);

	//_RELEASEP(pReaderInput);
	_RELEASEP(pReader);
	_RELEASEP(pStream);

	return TRUE;
}

LPCTSTR Config::GetStringById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, LPCTSTR pszDefault)
{
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nId);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL)
			return pItem->pData;
	}

	return pszDefault;
}

LONG Config::GetLongById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, LONG lDefault)
{
	LONG lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nId);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstol(pItem->pData, &pszEnd, 10);
	}

	return lValue;
}

INT Config::GetIntegerById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, INT lDefault)
{
	LONG lValue = lDefault;
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nId);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _ttoi(pItem->pData);
	}

	return lValue;
}

double Config::GetDoubleById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, double lDefault)
{
	double lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nId);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstod(pItem->pData, &pszEnd);
		if (lValue == 0.0 && pszEnd != NULL) {
			lValue = lDefault;
		}
	}

	return lValue;
}

BOOL Config::GetBooleanById(LPCTSTR pszSection, LPCTSTR pszKey, LONG nId, BOOL bDefault)
{
	BOOL bValue = bDefault;
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nId);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
		{
			if (_tcsicmp(pItem->pData, _T("True")) == 0 ||
				_tcsicmp(pItem->pData, _T("1")) == 0)
				bValue = TRUE;
			else
				bValue = FALSE;

		}
	}

	return bValue;
}

LPCTSTR Config::GetString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	return GetString(pszSection, pszKey, 0, pszDefault);
}

LONG Config::GetLong(LPCTSTR pszSection, LPCTSTR pszKey, LONG lDefault)
{
	return GetLong(pszSection, pszKey, 0, lDefault);
}

INT Config::GetInteger(LPCTSTR pszSection, LPCTSTR pszKey, INT lDefault)
{
	return GetInteger(pszSection, pszKey, 0, lDefault);
}

double Config::GetDouble(LPCTSTR pszSection, LPCTSTR pszKey, double lDefault)
{
	return GetDouble(pszSection, pszKey, 0, lDefault);
}

BOOL Config::GetBoolean(LPCTSTR pszSection, LPCTSTR pszKey, BOOL bDefault)
{
	return GetBoolean(pszSection, pszKey, 0, bDefault);
}

LPCTSTR Config::GetString(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, LPCTSTR pszDefault)
{
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nNo);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			return pItem->pData;
	}

	return pszDefault;
}

LONG Config::GetLong(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, LONG lDefault)
{
	LONG lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nNo);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstol(pItem->pData, &pszEnd, 10);
	}

	return lValue;
}

INT Config::GetInteger(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, INT lDefault)
{
	INT lValue = lDefault;
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nNo);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _ttoi(pItem->pData);
	}

	return lValue;
}

double Config::GetDouble(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, double lDefault)
{
	double lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nNo);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstod(pItem->pData, &pszEnd);
		if (lValue == 0.0 && pszEnd != NULL) {
			lValue = lDefault;
		}
	}

	return lValue;
}

BOOL Config::GetBoolean(LPCTSTR pszSection, LPCTSTR pszKey, ULONG nNo, BOOL bDefault)
{
	BOOL bValue = bDefault;
	if (IsValid())
	{
		CString szKey;
		szKey.Format(_T("%s.%s#0x%X"), pszSection, pszKey, nNo);

		LPITEMDATA pItem = GetItem(szKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
		{
			if (_tcsicmp(pItem->pData, _T("True")) == 0 ||
				_tcsicmp(pItem->pData, _T("1")) == 0)
				bValue = TRUE;
			else
				bValue = FALSE;

		}
	}

	return bValue;
}


LPCTSTR Config::GetString(LPCTSTR pszKey, LPCTSTR pszDefault)
{
	if (IsValid())
	{
		LPITEMDATA pItem = GetItem(pszKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			return pItem->pData;
	}

	return pszDefault;
}

LONG Config::GetLong(LPCTSTR pszKey, LONG lDefault)
{
	LONG lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		LPITEMDATA pItem = GetItem(pszKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstol(pItem->pData, &pszEnd, 10);
	}

	return lValue;
}

INT Config::GetInteger(LPCTSTR pszKey, INT lDefault)
{
	INT lValue = lDefault;
	if (IsValid())
	{
		LPITEMDATA pItem = GetItem(pszKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _ttoi(pItem->pData);
	}

	return lValue;
}

double Config::GetDouble(LPCTSTR pszKey, double lDefault)
{
	double lValue = lDefault;
	if (IsValid())
	{
		LPTSTR pszEnd = NULL;
		LPITEMDATA pItem = GetItem(pszKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
			lValue = _tcstod(pItem->pData, &pszEnd);

		if (lValue == 0.0 && pszEnd != NULL) {
			lValue = lDefault;
		}
	}

	return lValue;
}

BOOL Config::GetBoolean(LPCTSTR pszKey, BOOL bDefault)
{
	BOOL bValue = bDefault;
	if (IsValid())
	{
		LPWSTR pszEnd = NULL;

		LPITEMDATA pItem = GetItem(pszKey);
		if (pItem != NULL &&
			pItem->pData != NULL)
		{
			if (_tcsicmp(pItem->pData, _T("True")) == 0 ||
				_tcsicmp(pItem->pData, _T("1")) == 0)
				bValue = TRUE;
			else
				bValue = FALSE;

		}
	}

	return bValue;
}

INT Config::GetMultiKeyCount(LPCTSTR key)
{
	if (m_pOff == NULL ||
		m_pOff->size() == 0)
		return 0;

	return m_pOff->get(genXmlKey(key));
}

void Config::SetMultiKeyCount(LPCTSTR key, INT count)
{
	if (m_pOff == NULL)
		return;

	m_pOff->add(genXmlKey(key), count);
}


LPITEMDATA Config::GetItem(LPCTSTR key)
{
	if (m_pSrch == NULL ||
		m_pSrch->size() == 0)
		return NULL;

	return m_pSrch->get(genXmlKey(key));
}

BOOL Config::AddItem(LPCTSTR key, LPITEMDATA pItem, BOOL bAddToList)
{
	if (m_pSrch == NULL ||
		m_pList == NULL)
		return FALSE;

	if (GetItem(key) != NULL)
		return TRUE;

	m_pSrch->add(genXmlKey(key), pItem);

	if (bAddToList)
		m_pList->add(pItem);

	return TRUE;
}

unsigned long Config::genXmlKey(LPCTSTR key)
{
	unsigned long hash = 0;
	const TCHAR* p = (TCHAR*)key;
	while (*p != 0)
	{
		hash = (hash << 5) + hash + (*p);
		p++;
	}

	return hash;
};

#ifdef _DEBUG
void Config::Dump()
{
	if (m_pList == NULL ||
		m_pList->size() == 0)
		return;

	afxTRACE(_T("MAP >>"));
	PPOSITION pos = m_pSrch->first();
	while (pos != NULL) {
		ULONG ulKey = 0;
		LPITEMDATA pItem = NULL;
		m_pSrch->next(pos, ulKey, pItem);
		if (pItem)
		{
			afxTRACE(_T("KEY:%u <%s> -> %s"), ulKey, pItem->pKey, pItem->pData);
			if (pItem->pChildItems != NULL &&
				pItem->pChildItems->first() ) {
				do {
					LPITEMDATA pValue = pItem->pChildItems->get();
					if (pValue) {
						afxTRACE(_T("\t<%s> -> %s"), pValue->pKey, pValue->pData);
					}
				} while (pItem->pChildItems->next());
					
			}
		}
	}
	afxTRACE(_T("MAP <<"));

	afxTRACE(_T("LIST >>"));
	m_pList->first();
	do {
		LPITEMDATA pItem = m_pList->get();
		if (pItem)
		{
			afxTRACE(_T("<%s> -> %s"), pItem->pKey, pItem->pData);

		}
	} while (m_pList->next());
	afxTRACE(_T("LIST <<"));
}
#endif

} // namespace UTL
