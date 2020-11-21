#include "stdafx.h"
#include "Config.h"
#include "stringx.h"

#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef _INC_SHLWAPI
#include <shlwapi.h>
#endif
#pragma comment(lib, "shlwapi.lib")

#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

#define _Release(p)	if( p != NULL )	{ (p)->Release();	p = NULL;	}

void afxTRACE(const TCHAR* lpszFormat, ...)
{
#ifdef _DEBUG
	TCHAR msg[2048];
    va_list list;
    va_start(list, lpszFormat);

	_vstprintf_s(msg, 2048, lpszFormat, list);

	va_end(list);

	OutputDebugString(msg);
#endif
}

// ----------------------------------------------------------------------------
// Get dll module handle
HMODULE _GetCurrentModule()
{
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	HMODULE hModule = NULL;
	if( ::GetModuleHandleEx(flags, (LPCTSTR)_GetCurrentModule, &hModule) )
	{
		return hModule;
	}

	return NULL;
}

//void _freeItemLink(LPITEMLINK pLnk)
//{
//	if( pLnk )
//	{
//		_FREE(pLnk->pAttrName);
//
//		_FREE(pLnk);
//	}
//}

void _freeItemDataP(LPITEMDATA pItem)
{
	if( pItem )
	{
		// データ
		_FREE(pItem->pData);
		// キー
		_FREE(pItem->pKey);

		// 属性
		if( pItem->pMapAttrs )
		{
			PPOSITION pos = pItem->pMapAttrs->first();
			while(pos != NULL)
			{
				ULONG ulKey;
				LPITEMDATA pChild = NULL;
				pItem->pMapAttrs->next(pos, ulKey, pChild);
				if( pChild )
				{
					_freeItemDataP(pChild);
				}
			}
		}
		_DELETE(pItem->pMapAttrs);

		// 自体
		_FREE(pItem);
	}
}

void _freeConfigItems(LPITEMDATA pItem)
{
	if( pItem )
	{
		//_DELETE(pItem->pChildItems);
		_FREE(pItem->pData);
		_FREE(pItem->pKey);

		_FREE(pItem);
	}
}

Config::Config()
	: m_pSrch(NULL)
	, m_pList(NULL)
	, m_dwErrorCode(0)
	, m_bOverwrite(FALSE)
{
	Init();
}

Config::Config(LPCWSTR lpszFileName, BOOL bOverwrite)
	: m_pSrch(NULL)
	, m_pList(NULL)
	, m_pRoot(NULL)
	, m_dwErrorCode(0)
	, m_bOverwrite(bOverwrite)
{
	if( Init() )
	{
		if( Load(lpszFileName) )
		{
			// OK
		}
	}
}

Config::Config(LPCSTR lpszFileName, BOOL bOverwrite)
	: m_pSrch(NULL)
	, m_pList(NULL)
	, m_pRoot(NULL)
	, m_dwErrorCode(0)
	, m_bOverwrite(bOverwrite)
{
	if( Init() )
	{
		std::wstring wszFileName = _A2W(lpszFileName);
		if( Load(wszFileName.c_str()) )
		{
			// OK
		}
	}
}

Config::~Config()
{
	_DELETE(m_pList);
	_DELETE(m_pSrch);
}

BOOL Config::IsValid(LPWSTR pszRoot)
{
	if( m_pList == NULL ||
		m_pSrch == NULL )
		return FALSE;

	if( GetItem(pszRoot) == NULL )
		return FALSE;

	m_szRoot = pszRoot;

	return TRUE;
}

BOOL Config::Init()
{
	m_pRoot = NULL;

	_DELETE(m_pSrch);

	m_pList = new CLinkedListToItem(true, _freeItemDataP);
	if( m_pList == NULL )
	{
		goto ConfigInit_Err;
	}

	// Free しない
	m_pSrch = new CHashTblULongToItem();
	if( m_pSrch == NULL ||
		m_pSrch->create(genHashKey) == NULL )
	{
		goto ConfigInit_Err;
	}

	return TRUE;

ConfigInit_Err:

	// Clearn
	_DELETE(m_pList);
	_DELETE(m_pSrch);

	return FALSE;
}

#define CHKHR(stmt)	do { hr = (stmt); if(FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)	do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(p)	do { if (p) { p->Release(); } p = NULL; } while(0)

// Load from XML
BOOL Config::Load(LPCWSTR lpszFileName)
{
#ifdef _DEBUG
	afxTRACE(_T("Load xml %s\n"), lpszFileName);
#endif
	m_dwErrorCode = 0;

#ifdef _MSC_VER
	WCHAR szPath[_MAX_PATH] = {0};
	wcscpy_s(szPath, _MAX_PATH, lpszFileName);

	if( !::PathFileExistsW(lpszFileName) )
	{
		WCHAR szXml[_MAX_PATH] = {0};
		wcscpy_s(szXml, _MAX_PATH, lpszFileName);
		WCHAR szModule[_MAX_PATH] = {0};

		if( !::GetModuleFileNameW(_GetCurrentModule(), szModule, _MAX_PATH) )
			return FALSE;

		::PathRemoveFileSpecW(szModule);

		if( ::PathIsFileSpecW(szXml) )
		{
			::PathStripPathW(szXml);
		}
		wcscpy_s(szPath, _MAX_PATH, szModule);
		::PathAddBackslashW(szPath);
		::PathAppendW(szPath, szXml);
	}
#endif
	HRESULT hr = 0;
	IStream* pStream = NULL;

	hr = ::SHCreateStreamOnFileW(szPath, STGM_READ, &pStream);
	if( FAILED(hr) )
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("SHCreateStreamOnFile failed. error:%d\n"), m_dwErrorCode);
		throw m_dwErrorCode;

		return FALSE;
	}

	IXmlReader* pReader = NULL;
	hr = ::CreateXmlReader(IID_IXmlReader, (void**)&pReader, NULL);
	if( FAILED(hr) )
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("CreateXmlReader failed. error:%d\n"), m_dwErrorCode);
		_Release(pStream);

		return FALSE;
	}
	//// Encode
	//IXmlReaderInput* pReaderInput = NULL;
	//hr = ::CreateXmlReaderInputWithEncodingName(pStream, NULL, L"UTF-16BE", FALSE, NULL, &pReaderInput);
	//if( FAILED(hr) )
	//{
	//	m_dwErrorCode = ::GetLastError();
	//	afxTRACE(_T("CreateXmlReaderInputWithEncodingName failed. error:%d\n"), m_dwErrorCode);
	//	_Release(pReader);
	//	_Release(pStream);

	//	return FALSE;
	//}

	hr = pReader->SetInput(pStream);
	if( FAILED(hr) )
	{
		m_dwErrorCode = ::GetLastError();
		afxTRACE(_T("IXmlReader::SetInput failed. error:%d\n"), m_dwErrorCode);
		//_Release(pReaderInput);
		_Release(pReader);
		_Release(pStream);

		return FALSE;
	}

	LPCWSTR pwszPrefix = NULL;
	LPCWSTR pwszLocalName = NULL;
	LPCWSTR pwszValue = NULL;

	UINT cwchDepth = 0;
	UINT cwchPrefix = 0;

	XmlNodeType nodeType;

	LPWSTR pszEnd = NULL;
	LPITEMDATA pItem = NULL;
	LPITEMDATA pParentItem = NULL;

	WCHAR wszSubKey[512] = { 0 };
	WCHAR wszSubNo[32] = {0};
	std::wstring wszpKey;
	BOOL bBlock = FALSE;
	BOOL bBlank = FALSE;

	INT listcnt = 0;
	std::wstring wszKey = L"";
#ifdef _DEBUG
	int loop = 0;
#endif
	
	while( (hr = pReader->Read(&nodeType)) == S_OK )
	{
#ifdef _DEBUG
		//afxTRACE(_T("Readed: %d\n"), loop++);
#endif
		pszEnd = NULL;
		switch(nodeType)
		{
		case XmlNodeType_XmlDeclaration:
			//afxTRACE(_T("XmlNodeType_XmlDeclaration\n"));
			break;

		case XmlNodeType_Element:
			//afxTRACE(_T("XmlNodeType_Element\n"));
			if(FAILED(hr = pReader->GetDepth(&cwchDepth)))
			{
				afxTRACE(_T("Erro getting depth, error is %08.8lx"), hr);
				HR(hr);
			}
			//afxTRACE(_T("Depth: %d\n"), cwchDepth);
			//if( cwchDepth == 0 )
			//	break;
			bBlank = FALSE;

			if(FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				afxTRACE(_T("Erro getting local name, error is %08.8lx"), hr);
				HR(hr);
			}
			//afxTRACE(_T("Element: %s\n"), pwszLocalName);

			if( pReader->IsEmptyElement() )
			{
				bBlank = TRUE;
				//afxTRACE(_T("empty\n"));
			}

			wszKey = L"";
			// 親のキー
			GetKey(pParentItem, wszKey);
			if( pParentItem != NULL )
				wszKey.append(L".");

			wszKey.append(pwszLocalName);

			if( m_bOverwrite )
			{
				pItem = GetItem(wszKey.c_str());
			}
			else
			{
				listcnt = 0;
				do
				{
					WCHAR wszNum[32] = {0};
					std::wstring wszSubKey = wszKey;

					if( listcnt > 0 )
					{
						wszSubKey.append(L"-");
						wsprintf(wszNum, L"%d", listcnt);
						wszSubKey.append(wszNum);
					}

					pItem = GetItem(wszSubKey.c_str());
					listcnt++;

					if( pItem == NULL )
						wszKey = wszSubKey;

				} while( pItem );
			}
			if( pItem == NULL )
			{
				pItem = (LPITEMDATA)calloc(1, sizeof(ITEMDATA));
				if( pItem )
				{
					pItem->nKeyLen = wcslen(pwszLocalName);
					pItem->pKey = (wchar_t*)calloc(pItem->nKeyLen + 1, sizeof(wchar_t));
					if( pItem->pKey )
					{
						wcscpy_s(pItem->pKey, pItem->nKeyLen + 1, pwszLocalName);
					}

					// 親
					pItem->pParent = pParentItem;

					AddItem(wszKey.c_str(), pItem);
				}
			}

			// Attributes
			for( hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute() )
			{
				if( pItem->pMapAttrs == NULL )
				{
					pItem->pMapAttrs = new CHashTblStringToItem();
					if( pItem->pMapAttrs == NULL ||
						pItem->pMapAttrs->create(genHashKey) == NULL )
					{
						goto CleanUp;
					}
				}

				LPCWSTR pwszAttrName;
				LPCWSTR pwszAttrValue;
				if(FAILED(hr = pReader->GetLocalName(&pwszAttrName, NULL)))
				{
					afxTRACE(_T("Erro getting local name, error is %08.8lx"), hr);
					HR(hr);
				}
				if(FAILED(hr = pReader->GetValue(&pwszAttrValue, NULL)))
				{
					afxTRACE(_T("Erro getting local name, error is %08.8lx"), hr);
					HR(hr);
				}
				
				ULONG ulChildKey = genXmlKey(pwszAttrName);
				LPITEMDATA pAttr = pItem->pMapAttrs->get(ulChildKey);
				if( pAttr == NULL )
				{
					pAttr = (LPITEMDATA)calloc(1, sizeof(ITEMDATA));
					if( pAttr )
					{
						// 親
						pAttr->pParent = pItem;

						pItem->pMapAttrs->add(ulChildKey, pAttr);
					}
				}

				_FREE(pAttr->pKey);
				pAttr->nKeyLen = wcslen(pwszAttrName);
				pAttr->pKey = (wchar_t*)calloc(pAttr->nKeyLen + 1, sizeof(wchar_t));
				if( pAttr->pKey )
				{
					wcscpy_s(pAttr->pKey, pAttr->nKeyLen + 1, pwszAttrName);
				}

				_FREE(pAttr->pData);
				pAttr->nDataLen = wcslen(pwszAttrValue);
				pAttr->pData = (wchar_t*)calloc(pAttr->nDataLen + 1, sizeof(wchar_t));
				if( pAttr->pData )
				{
					wcscpy_s(pAttr->pData, pAttr->nDataLen + 1, pwszAttrValue);
				}
			}

			// First element is the root
			if( m_pRoot == NULL )
				m_pRoot = pItem;

			if( bBlank == TRUE )
			{
				break;
			}

			pParentItem = pItem;
			bBlock = TRUE;

			break;

		case XmlNodeType_EndElement:
			if(FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
			{
				afxTRACE(_T("Erro getting prefix, error is %08.8lx"), hr);
				HR(hr);
			}
			if(FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				afxTRACE(_T("Erro getting local name, error is %08.8lx"), hr);
				HR(hr);
			}

			//if( cwchPrefix > 0 )
			//	afxTRACE(_T("End Element: %s:%s\n"), pwszPrefix, pwszLocalName);
			//else
			//	afxTRACE(_T("End Element: %s\n"), pwszLocalName);
			
			pItem = NULL;

			if( pParentItem != NULL )
				pParentItem = pParentItem->pParent;
			bBlock = FALSE;

			break;

		case XmlNodeType_Text:
		//case XmlNodeType_Whitespace:
			if(FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
			{
				afxTRACE(_T("Erro getting value, error is %08.8lx"), hr);
				HR(hr);
			}
#ifdef _DEBUG
			//afxTRACE(_T("\t%s"), pwszValue);
#endif
			if( pItem != NULL )
			{
				_FREE(pItem->pData);

				pItem->nDataLen = wcslen(pwszValue);
				pItem->pData = (wchar_t*)calloc(pItem->nDataLen + 1, sizeof(wchar_t));
				if( pItem->pData )
				{
					wcscpy_s(pItem->pData, pItem->nDataLen + 1, pwszValue);
				}
			}

			break;
		}
	}
CleanUp:
	SAFE_RELEASE(pReader);
	SAFE_RELEASE(pStream);

	return TRUE;
}

// L"ROOT.KEY.??"
BOOL Config::Select(LPWSTR pszSection)
{
	std::wstring szKey = L"";
	if( !m_szRoot.empty() )
	{
		szKey.append(m_szRoot);
		szKey.append(L".");
	}
	szKey.append(pszSection);

	if( GetItem(szKey.c_str()) == NULL )
		return FALSE;

	m_szSection = pszSection;

	return TRUE;
}

LPWSTR Config::GetAttributeString(LPWSTR pszKey, LPWSTR pszAttr, LPWSTR pszDefault)
{
	return GetAttributeString(pszKey, 0, pszAttr, pszDefault);
}

LONG   Config::GetAttributeLong(LPWSTR pszKey, LPWSTR pszAttr, LONG lDefault)
{
	return GetAttributeLong(pszKey, 0, pszAttr, lDefault);
}

INT    Config::GetAttributeInt(LPWSTR pszKey, LPWSTR pszAttr, INT lDefault)
{
	return GetAttributeInt(pszKey, 0, pszAttr, lDefault);
}

double Config::GetAttributeDouble(LPWSTR pszKey, LPWSTR pszAttr, double dDefault)
{
	return GetAttributeDouble(pszKey, 0, pszAttr, dDefault);
}

BOOL   Config::GetAttributeBoolean(LPWSTR pszKey, LPWSTR pszAttr, BOOL bDefault)
{
	return GetAttributeBoolean(pszKey, 0, pszAttr, bDefault);
}


LPWSTR Config::GetAttributeString(LPWSTR pszKey, INT index, LPWSTR pszAttr, LPWSTR pszDefault)
{
	LPWSTR pszValue = pszDefault;

	std::wstring wszKey = pszKey;
	WCHAR szNum[32] = {0};
	if( index > 0 )
	{
		wsprintf(szNum, L"-%d", index);
	}
	if( szNum[0] != 0x00 )
		wszKey.append(szNum);

	LPITEMDATA pItem = GetItem(NULL, wszKey.c_str());
	if( pItem != NULL )
	{
		if( pItem->pMapAttrs != NULL )
		{
			unsigned long ulKey = genXmlKey(pszAttr);
			LPITEMDATA pAttr = pItem->pMapAttrs->get(ulKey);
			if( pAttr != NULL &&
				pAttr->pData != NULL )
				return pAttr->pData;
		}
	}

	return pszValue;
}

LONG   Config::GetAttributeLong(LPWSTR pszKey, INT index, LPWSTR pszAttr, LONG lDefault)
{
	LONG lValue = lDefault;

	std::wstring wszKey = pszKey;
	WCHAR szNum[32] = {0};
	if( index > 0 )
	{
		wsprintf(szNum, L"-%d", index);
	}
	if( szNum[0] != 0x00 )
		wszKey.append(szNum);

	LPITEMDATA pItem = GetItem(NULL, wszKey.c_str());
	if( pItem != NULL )
	{
		if( pItem->pMapAttrs != NULL )
		{
			LPWSTR pszEnd = NULL;
			unsigned long ulKey = genXmlKey(pszAttr);
			LPITEMDATA pAttr = pItem->pMapAttrs->get(ulKey);
			if( pAttr != NULL &&
				pAttr->pData != NULL )
				lValue = wcstol(pAttr->pData, &pszEnd, 10);
		}
	}

	return lValue;
}

INT    Config::GetAttributeInt(LPWSTR pszKey, INT index, LPWSTR pszAttr, INT lDefault)
{
	return (INT)GetAttributeLong(pszKey, index, pszAttr, lDefault);
}

double Config::GetAttributeDouble(LPWSTR pszKey, INT index, LPWSTR pszAttr, double dDefault)
{
	double dValue = dDefault;

	std::wstring wszKey = pszKey;
	WCHAR szNum[32] = {0};
	if( index > 0 )
	{
		wsprintf(szNum, L"-%d", index);
	}
	if( szNum[0] != 0x00 )
		wszKey.append(szNum);

	LPITEMDATA pItem = GetItem(NULL, wszKey.c_str());
	if( pItem != NULL )
	{
		if( pItem->pMapAttrs != NULL )
		{
			LPWSTR pszEnd = NULL;
			unsigned long ulKey = genXmlKey(pszAttr);
			LPITEMDATA pAttr = pItem->pMapAttrs->get(ulKey);
			if( pAttr != NULL &&
				pAttr->pData != NULL )
				dValue = wcstod(pAttr->pData, &pszEnd);
		}
	}

	return dValue;
}
BOOL   Config::GetAttributeBoolean(LPWSTR pszKey, INT index, LPWSTR pszAttr, BOOL bDefault)
{
	BOOL bValue = bDefault;

	std::wstring wszKey = pszKey;
	WCHAR szNum[32] = {0};
	if( index > 0 )
	{
		wsprintf(szNum, L"-%d", index);
	}
	if( szNum[0] != 0x00 )
		wszKey.append(szNum);

	LPITEMDATA pItem = GetItem(NULL, wszKey.c_str());
	if( pItem != NULL )
	{
		if( pItem->pMapAttrs != NULL )
		{
			LPWSTR pszEnd = NULL;
			unsigned long ulKey = genXmlKey(pszAttr);
			LPITEMDATA pAttr = pItem->pMapAttrs->get(ulKey);
			if( pAttr != NULL &&
				pAttr->pData != NULL )
			{
				if( _wcsicmp(pAttr->pData, L"False") == 0 ||
					_wcsicmp(pAttr->pData, L"0") == 0 )
					bValue = FALSE;
				else
					bValue = TRUE;
			}
		}
	}

	return bValue;
}


LPWSTR Config::GetStringEx(LPWSTR pszKey, LPWSTR pszDefaul)
{
	return GetString(NULL, pszKey, pszDefaul);
}

LONG   Config::GetLong(LPWSTR pszKey, LONG lDefault)
{
	return GetLong(NULL, pszKey, lDefault);
}

INT    Config::GetInt(LPWSTR pszKey, INT lDefault)
{
	return GetInt(NULL, pszKey, lDefault);
}

double Config::GetDouble(LPWSTR pszKey, double dDefault)
{
	return GetDouble(NULL, pszKey, dDefault);
}

BOOL   Config::GetBoolean(LPWSTR pszKey, BOOL bDefault)
{
	return GetBoolean(NULL, pszKey, bDefault);
}


LPWSTR Config::GetString(LPWSTR pszSection, LPWSTR pszKey, LPWSTR pszDefault)
{
	LPWSTR pszValue = pszDefault;

	LPITEMDATA pItem = GetItem(pszSection, pszKey);
	if( pItem != NULL &&
		pItem->pData != NULL )
		return pItem->pData;

	return pszValue;
}

LONG Config::GetLong(LPWSTR pszSection, LPWSTR pszKey, LONG lDefault)
{
	LONG lValue = lDefault;

	LPWSTR pszEnd = NULL;
	LPITEMDATA pItem = GetItem(pszSection, pszKey);
	if( pItem != NULL &&
		pItem->pData != NULL )
		lValue = wcstol(pItem->pData, &pszEnd, 10);

	return lValue;
}

INT    Config::GetInt(LPWSTR pszSection, LPWSTR pszKey, INT lDefault)
{
	INT iValue = lDefault;
	LPWSTR pszEnd = NULL;

	LPITEMDATA pItem = GetItem(pszSection, pszKey);
	if( pItem != NULL &&
		pItem->pData != NULL )
		iValue = (INT)wcstol(pItem->pData, &pszEnd, 10);

	return iValue;
}

double Config::GetDouble(LPWSTR pszSection, LPWSTR pszKey, double dDefault)
{
	double dValue = dDefault;

	LPWSTR pszEnd = NULL;
	LPITEMDATA pItem = GetItem(pszSection, pszKey);
	if( pItem != NULL &&
		pItem->pData != NULL )
		dValue = wcstod(pItem->pData, &pszEnd);

	return dValue;
}

BOOL   Config::GetBoolean(LPWSTR pszSection, LPWSTR pszKey, BOOL bDefault)
{
	BOOL bValue = bDefault;

	LPITEMDATA pItem = GetItem(pszSection, pszKey);
	if( pItem != NULL &&
		pItem->pData != NULL )
	{
		if( _wcsicmp(pItem->pData, L"False") == 0 ||
			_wcsicmp(pItem->pData, L"0") == 0 )
			bValue = FALSE;
		else
			bValue = TRUE;
	}

	return bValue;
}


//LPWSTR Config::GetString(LPWSTR pszSection, LPWSTR pszKey, LPWSTR pszDefault)
//{
//	if( IsValid() )
//	{
//		WCHAR wszKey[512] = { 0 };
//		swprintf_s(wszKey, 512, L"%s.%s", pszSection, pszKey);
//
//		LPITEMDATA pItem = GetItem(wszKey);
//		if( pItem != NULL &&
//			pItem->pData != NULL )
//			return pItem->pData;
//	}
//
//	return pszDefault;
//}
//
//LONG Config::GetLong(LPWSTR pszSection, LPWSTR pszKey, LONG lDefault)
//{
//	LONG lValue = lDefault;
//	if( IsValid() )
//	{
//		LPWSTR pszEnd = NULL;
//		WCHAR wszKey[512] = { 0 };
//		swprintf_s(wszKey, 512, L"%s.%s", pszSection, pszKey);
//
//		LPITEMDATA pItem = GetItem(wszKey);
//		if( pItem != NULL &&
//			pItem->pData != NULL )
//			lValue = wcstol(pItem->pData, &pszEnd, 10);
//	}
//
//	return lValue;
//}
//
//INT Config::GetInt(LPWSTR pszSection, LPWSTR pszKey, INT lDefault)
//{
//	INT iValue = lDefault;
//	if( IsValid() )
//	{
//		LPWSTR pszEnd = NULL;
//		WCHAR wszKey[512] = { 0 };
//		swprintf_s(wszKey, 512, L"%s.%s", pszSection, pszKey);
//
//		LPITEMDATA pItem = GetItem(wszKey);
//		if( pItem != NULL &&
//			pItem->pData != NULL )
//			iValue = (INT)wcstol(pItem->pData, &pszEnd, 10);
//	}
//
//	return iValue;
//}
//
//double Config::GetDouble(LPWSTR pszSection, LPWSTR pszKey, double dDefault)
//{
//	double dValue = dDefault;
//	if( IsValid() )
//	{
//		LPWSTR pszEnd = NULL;
//		WCHAR wszKey[512] = { 0 };
//		swprintf_s(wszKey, 512, L"%s.%s", pszSection, pszKey);
//
//		LPITEMDATA pItem = GetItem(wszKey);
//		if( pItem != NULL &&
//			pItem->pData != NULL )
//			dValue = wcstod(pItem->pData, &pszEnd);
//	}
//
//	return dValue;
//}
//
//BOOL Config::GetBoolean(LPWSTR pszSection, LPWSTR pszKey, BOOL bDefault)
//{
//	BOOL bValue = bDefault;
//	if( IsValid() )
//	{
//		LPWSTR pszEnd = NULL;
//		WCHAR wszKey[512] = { 0 };
//		swprintf_s(wszKey, 512, L"%s.%s", pszSection, pszKey);
//
//		LPITEMDATA pItem = GetItem(wszKey);
//		if( pItem != NULL &&
//			pItem->pData != NULL )
//		{
//			if( _wcsicmp(pItem->pData, L"False") == 0 ||
//				_wcsicmp(pItem->pData, L"0") == 0 )
//				bValue = FALSE;
//			else
//				bValue = TRUE;
//
//		}
//	}
//
//	return bValue;
//}
//
//
LPITEMDATA Config::GetItem(LPCWSTR key)
{
	if( m_pSrch == NULL ||
		m_pSrch->size() == 0 )
		return NULL;

	return m_pSrch->get(genXmlKey(key));
}

LPITEMDATA Config::GetItem(LPCWSTR section, LPCWSTR key)
{
	std::wstring wszKey = L"";
	if( !m_szRoot.empty() )
		wszKey.append(m_szRoot);

	if( !wszKey.empty() )
		wszKey.append(L".");

	if( section == NULL )
	{
		if( !m_szSection.empty() )
			wszKey.append(m_szSection);
	}
	else
		wszKey.append(section);


	if( !wszKey.empty() )
		wszKey.append(L".");

	wszKey.append(key);

	return GetItem(wszKey.c_str());
}

BOOL Config::AddItem(LPCWSTR key, LPITEMDATA pItem, BOOL bAddToList)
{
	if( m_pList == NULL ||
		m_pSrch == NULL )
		return FALSE;

	if( GetItem(key) != NULL )
		return TRUE;

#ifdef _DEBUG
	//afxTRACE(_T("AddItem[%s]\n"), key);
#endif

	m_pSrch->add(genXmlKey(key), pItem);

	if( bAddToList )
		m_pList->add(pItem);

	return TRUE;
}

unsigned long Config::genXmlKey(LPCWSTR key)
{
	std::wstring wkey = key;
	transform(wkey.begin(), wkey.end(), wkey.begin(), tolower);

    std::hash<std::wstring> hash_fn;
 
    size_t hash = hash_fn(wkey);

	return hash;
};

//bool Config::getParentKey(LPITEMDATA pItem, std::wstring& wszpKey)
//{
//	if( pItem == NULL )
//		return false;
//
//	LPITEMDATA p = pItem->pParent;
//	while( p != NULL )
//	{
//		std::wstring wszpTmp = p->pKey;
//		wszpTmp += L".";
//
//		wszpKey.insert(0, wszpTmp);
//
//		p = p->pParent;
//	}
//	wszpKey += pItem->pKey;
//
//	//LPITEMDATA pParent = pItem->pParent;
//	//while(pParent != NULL)
//	//{
//	//	if( !getParentKey(pItem->pParent, wszpKey) )
//	//		break;
//
//	//	wszpKey += L".";
//	//	pParent = pParent->pParent;
//	//}
//	//wszpKey += pItem->pKey;
//
//	return true;
//}
//
//#ifdef _DEBUG
//void Config::Dump()
//{
//	if( m_pList == NULL ||
//		m_pList->size() == 0 )
//		return;
//
//	afxTRACE(_T("Dump Config\n"));
//	m_pList->first();
//	do {
//		LPITEMDATA pItem = m_pList->get();
//		if( pItem )
//		{
//			afxTRACE(_T("<%s> -> %s\n"), pItem->pKey, pItem->pData);
//
//		}
//	} while( m_pList->next() );
//}
//#endif

bool Config::GetKey(LPITEMDATA pItem, std::wstring& wszpKey)
{
	wszpKey = L"";
	if( pItem == NULL )
		return false;

	LPITEMDATA p = pItem->pParent;
	while( p != NULL )
	{
		std::wstring wszpTmp = p->pKey;
		wszpTmp += L".";

		wszpKey.insert(0, wszpTmp);

		p = p->pParent;
	}
	wszpKey.append(pItem->pKey);

	return true;
}
