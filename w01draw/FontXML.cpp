#include "stdafx.h"
#include "FontXML.h"

#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define XMLKEY_NONE			0x00	
#define XMLKEY_FONT			0x05			// FONT
#define XMLKEY_DEFAULTFONTNAME	0x1e			// DefaultFontName

//    time_t -> FILETIME 変換
FILETIME
UnixTimeToFileTime(time_t t){
    FILETIME ft;
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    return ft;
}

BOOL WriteFileInUTF8(HANDLE hFile, LPWSTR lpData)
{
	BOOL bWritten = TRUE;
	DWORD nBytesWritten = 0;
	LPCSTR lpText = _WW2A2(lpData, CP_UTF8);
	DWORD nLen = strlen(lpText);
	if( !::WriteFile(hFile, lpText, nLen, &nBytesWritten, NULL) ||
		(nBytesWritten != nLen) )
	{
		bWritten = FALSE;
	}
	delete[] lpText;

	return bWritten;
}

CFontXML::CFontXML(void)
{
	m_pFontTbl = NULL;
	m_pDataTbl = NULL;
}


CFontXML::~CFontXML(void)
{
	ClearMemory();
}

BOOL CFontXML::InitMemory()
{
	if( m_pFontTbl != NULL )
		return TRUE;

	m_pFontTbl = new CHashTblStringToOutFont();
	if( m_pFontTbl == NULL ||
		m_pFontTbl->create(genHashKeyStrW, true) == NULL )
	{
		return FALSE;
	}
	m_pDataTbl = new CHashTblIntToXmlData();
	if( m_pDataTbl == NULL ||
		m_pDataTbl->create(genHashKey) == NULL )
	{
		return FALSE;
	}

	return TRUE;
}

void CFontXML::ClearMemory()
{
	if( m_pFontTbl != NULL )
	{
		delete m_pFontTbl;
		m_pFontTbl = NULL;
	}
	if( m_pDataTbl != NULL )
	{
		delete m_pDataTbl;
		m_pDataTbl = NULL;
	}
}


BOOL CFontXML::Open(LPCTSTR lpszName)
{
	if( !InitMemory() )
		return FALSE;

	m_szFileName = lpszName;

	// Clear
	m_pFontTbl->removeall();
	m_pDataTbl->removeall();

	if( readFontList(m_pFontTbl, m_pDataTbl) < 0 )
		return FALSE;

	return TRUE;
}

/*------------------------------------------------------------------*/
// FontList.xml common section
/*------------------------------------------------------------------*/
struct _xmlFontList
{
	UINT id;
	wchar_t	name[256];
	wchar_t	value[256];
} FONTLISTXML[] = 
{
	{ID_DEFAULTFONTNAME, L"DefaultFontName", L""},
	{ID_NODATA, L"", L""}
};

int PreSetFontList(CHashTblIntToXmlData* pxmlData)
{
	if( !pxmlData )
		return 0;
	int count = 0;
	
	while(FONTLISTXML[count].id != ID_NODATA)
	{
		LPXMLDATA pData = pxmlData->get(FONTLISTXML[count].id);
		if( !pData )
		{
				pData = (LPXMLDATA)malloc(sizeof(XMLDATA));
			if( pData )
			{
				memset(pData, 0x00, sizeof(XMLDATA));
				pData->id = FONTLISTXML[count].id;
				wcscpy_s(pData->name, 256, FONTLISTXML[count].name);
							
				pxmlData->add(FONTLISTXML[count].id, pData);
			}
		}

		count++;
	}

	return count;
}

int CFontXML::readFontList(CHashTblStringToOutFont* pfontOut, CHashTblIntToXmlData* pxmlData)
{
	if( !pfontOut )
		return 0;

	if( !::PathFileExists(m_szFileName) )
		return 0;

	int ret = 0;
	HRESULT hr = 0;
	CComPtr<IStream> pStream = NULL;

	try
	{
		hr = ::SHCreateStreamOnFile(m_szFileName, STGM_READ, &pStream);
		if( FAILED(hr) )
		{
			return -2;
		}

		CComPtr<IXmlReader> pReader = NULL;
		hr = CreateXmlReader(IID_IXmlReader, (void**)&pReader, NULL);
		if( FAILED(hr) )
		{
			return -1;
		}
	
		//// Encode
		//CComPtr<IXmlReaderInput> pReaderInput;
		//hr = CreateXmlReaderInputWithEncodingName(pStream, NULL, L"UTF-16BE", FALSE, NULL, &pReaderInput);
		//if( FAILED(hr) )
		//{
		//	afxTRACE(_T("Failed to CreateXmlReaderInputWithEncodingName.\n"));
		//	return -2;
		//}
		//hr = pReader->SetInput(pReaderInput);

		hr = pReader->SetInput(pStream);
		if( FAILED(hr) )
		{
			return -3;
		}

		LPCWSTR pwszLocalName;
		LPCWSTR pwszValue;
		XmlNodeType nodeType;

		LPWSTR pszEnd = NULL;
		int nNodeKey = XMLKEY_NONE;

		LPOUTFONT lpFont = NULL;
		// Read Node Data
		while( (hr = pReader->Read(&nodeType)) == S_OK )
		{
			pszEnd = NULL;

			switch(nodeType)
			{
			case XmlNodeType_Element:
				hr = pReader->GetLocalName(&pwszLocalName, NULL);
				if( FAILED(hr) )
				{
					return -4;
				}

				// Keyword: Font
				if( _wcsicmp(pwszLocalName, L"FONT") == 0 )
				{
					nNodeKey = XMLKEY_FONT;
				}
				// Keywrod: DefaultFontName
				else if( _wcsicmp(pwszLocalName, L"DefaultFontName") == 0 )
				{
					if( pxmlData != NULL )
					{
						LPXMLDATA pData = pxmlData->get(ID_DEFAULTFONTNAME);
						if( !pData )
						{
							 pData = (LPXMLDATA)malloc(sizeof(XMLDATA));
							if( pData )
							{
								memset(pData, 0x00, sizeof(XMLDATA));
								pData->id = ID_DEFAULTFONTNAME;
								wcscpy_s(pData->name, 256, L"defaultfontname");
							
								pxmlData->add(ID_DEFAULTFONTNAME, pData);
							}
						}
					}
					nNodeKey = XMLKEY_DEFAULTFONTNAME;
				}
				else
				{
					nNodeKey = XMLKEY_NONE;
				}
				// Attributes
				for( hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
				{
					LPCWSTR pwszAttrName;
					LPCWSTR pwszAttrValue;
					hr = pReader->GetLocalName(&pwszAttrName, NULL);
					if( FAILED(hr) )
						break;
					hr = pReader->GetValue(&pwszAttrValue, NULL);
					if( FAILED(hr) )
						break;

					// SetValue
					if (nNodeKey == XMLKEY_FONT)
					{
						if( lpFont == NULL )
						{
							lpFont = (LPOUTFONT)calloc(1, sizeof(OUTFONT));
							if( lpFont == NULL )
							{
								return -5;
							}
						}
						// FontName
						if( _wcsicmp(pwszAttrName, L"FontName") == 0 )
						{
							wcscpy_s(lpFont->fontName, 63, pwszAttrValue);
							wcscpy_s(lpFont->baseName, 63, pwszAttrValue);
						}
						// DisplayFontName
						else if( _wcsicmp(pwszAttrName, L"DisplayFontName") == 0 )
						{
							wcscpy_s(lpFont->displayName, 63, pwszAttrValue);
						}
						// LastUpdateDateTime
						else if( _wcsicmp(pwszAttrName, L"UpdateTime") == 0 )
						{
							wcscpy_s(lpFont->updtime, 20, pwszAttrValue);
						}
						// PrintFontName
						else if( _wcsicmp(pwszAttrName, L"PrintFontName") == 0 )
						{
							wcscpy_s(lpFont->printName, 63, pwszAttrValue);
						}
						// Mode
						else if( _wcsicmp(pwszAttrName, L"Mode") == 0 )
						{
							if( pwszAttrValue && 
								wcslen(pwszAttrValue) > 0 )
							{
								LPWSTR lpEndPtr = NULL;
								lpFont->mode = (short)wcstol(pwszAttrValue, &lpEndPtr, 10);
							}
						}
						// Like
						else if( _wcsicmp(pwszAttrName, L"Like") == 0 )
						{
							if( _wcsicmp(pwszAttrValue, L"true") == 0 )
								lpFont->like = true;
							else
								lpFont->like = false;
						}
					}
				}
				// Next Font Info
				if (nNodeKey == XMLKEY_FONT)
				{
					if( lpFont )
					{
						if( wcslen(lpFont->fontName) > 0 )
						{
							// Add to String To Font
							LPOUTFONT lpOldF = pfontOut->get(lpFont->fontName);
							if( !lpOldF )
							{
								pfontOut->insert(lpFont->fontName, lpFont);
							}
						}
						else
						{
							free(lpFont);
							lpFont = NULL;
						}
					}

					// Next One
					lpFont = NULL;
				}

				break;
			case XmlNodeType_EndElement:
				hr = pReader->GetLocalName(&pwszLocalName, NULL);
				if( FAILED(hr) )
				{
					return -4;
				}

				nNodeKey = XMLKEY_NONE;
				break;
			case XmlNodeType_Text:
				hr = pReader->GetValue(&pwszValue, NULL);
				if( FAILED(hr) )
				{
					break;
				}
				// Keyword: Font
				if (nNodeKey == XMLKEY_FONT)
				{
				}
				// Keyword: DefaultFontName
				else if (nNodeKey == XMLKEY_DEFAULTFONTNAME)
				{
					if( pxmlData != NULL )
					{
						LPXMLDATA pData = pxmlData->get(ID_DEFAULTFONTNAME);
						if( !pData )
						{
							 pData = (LPXMLDATA)malloc(sizeof(XMLDATA));
							if( pData )
							{
								memset(pData, 0x00, sizeof(XMLDATA));
								pData->id = ID_DEFAULTFONTNAME;
								wcscpy_s(pData->name, 256, L"defaultfontname");
							
								pxmlData->add(ID_DEFAULTFONTNAME, pData);
							}
						}
						if( pData )
						{
							wcscpy_s(pData->value, 256, pwszValue);
						}
					}
				}

				break;
			default:
				continue;
			}
		}
	}
	catch(...)	
	{
		ret = -1;
	}
	
	return ret;
}

BOOL CFontXML::Save(CHashTblStringToOutFont* pfontOut)
{
	if( saveFontList(pfontOut) >= 0 )
		return TRUE;

	return FALSE;
}

int CFontXML::saveFontList(CHashTblStringToOutFont* pfontOut)
{
	int nRet = 0;
	BOOL bChanged = FALSE;
	CHashTblStringToOutFont* pOfontOut = new CHashTblStringToOutFont();
	if( pOfontOut == NULL ||
		pOfontOut->create(genHashKeyStrW, true) == NULL )
	{
		return 0;
	}
	CHashTblStringToOutFont* pfontLast = new CHashTblStringToOutFont();
	if( pfontLast == NULL ||
		pfontLast->create(genHashKeyStrW, true) == NULL )
	{
		delete pOfontOut;

		return 0;
	}

	if( readFontList(pOfontOut) >= 0 )
	{
	}

	PPOSITION pos = NULL;
	// add new font to Old One
	if( pfontOut )
	{
		pos = pfontOut->first();
		while( pos )
		{
			WCHAR* pwszFontName = NULL;
			LPOUTFONT lpFont = NULL;

			pfontOut->next(pos, pwszFontName, lpFont);
			if( lpFont )
			{
				// check old list
				LPOUTFONT lpOldFont = pOfontOut->get(pwszFontName);
				if( lpOldFont == NULL )
				{
					// insert to header
					lpFont->used++;
					pOfontOut->insert(pwszFontName, lpFont, 0);
					// Has New Data
					bChanged = TRUE;
				}
				else
				{
					// update
					if( lpFont->flags & UPD_DISPNAME )
						wcscpy_s(lpOldFont->displayName, 63, lpFont->displayName);
					if( lpFont->flags & UPD_PRNTNAME )
						wcscpy_s(lpOldFont->printName, 63, lpFont->printName);
					if( lpFont->flags & UPD_MODE )
						lpOldFont->mode |= lpFont->mode;
					if( lpFont->flags & UPD_LIKE )
						lpOldFont->like = lpFont->like;
					if( lpFont->flags & UPD_FONTNAME )
						wcscpy_s(lpOldFont->baseName, 63, lpFont->baseName);

					lpOldFont->update = true;
				}
			}
		}
		// clear old one
		pfontOut->removeall();
	}

	// clear
	pfontLast->removeall();

	// Sort
	int inspos = 0;
	pos = pOfontOut->first(true);
	while( pos )
	{
		WCHAR* pwszFontName = NULL;
		LPOUTFONT lpFont = NULL;

		pOfontOut->next(pos, pwszFontName, lpFont);
		if( lpFont )
		{
			lpFont->used++;
			if( lpFont->like  )
			{
				// Move Like=True to Last
				pfontLast->add(pwszFontName, lpFont);
				inspos++;
			}
			else
			{
				if( inspos == 0 )
				{
					pfontLast->add(pwszFontName, lpFont);
				}
				else
				{
					hash_size npos = pfontLast->size() - inspos;
					if( npos < 0 )
						npos = 0;
					pfontLast->insert(pwszFontName, lpFont, npos, INSERT_BEFORE);
				}
			}
		}
	}
	if( pOfontOut )
	{
		delete pOfontOut;
		pOfontOut = NULL;
	}

	CHashTblIntToXmlData* pXmlMap = GetXmlData();
	PreSetFontList(pXmlMap);

	HANDLE hFile = INVALID_HANDLE_VALUE;;
	TRY
	{
		WCHAR lasttm[21] = {0};
		time_t ltime;
		struct tm updtime;
		time(&ltime);
		// Convert to local time
		errno_t terr = localtime_s(&updtime, &ltime);
		if( terr == 0 )
		{
			// ext. 20130809110921
			swprintf_s( lasttm,
						_countof(lasttm),
						//L"%04d/%02d/%02d %02d:%02d:%02d",
						L"%04d%02d%02d%02d%02d%02d",
						updtime.tm_year + 1900,
						updtime.tm_mon + 1,
						updtime.tm_mday,
						updtime.tm_hour,
						updtime.tm_min,
						updtime.tm_sec );
		}

		FILETIME ftCreationTime;		// 作成日時
		FILETIME ftLastAccessTime;		// 最終アクセス日時
		FILETIME ftLastWriteTime;		// 最終更新日時

		BOOL bHasTime = FALSE;
		DWORD dwCreationDisposition = CREATE_ALWAYS;
		// ファイルの存在有無チェック
		if( ::PathFileExists(m_szFileName) )
		{
			struct _stat st;
			if( !_tstat(m_szFileName, &st) )
			{
				time_t now = time(0);

				ftCreationTime = UnixTimeToFileTime(st.st_ctime);
				ftLastWriteTime = UnixTimeToFileTime(st.st_mtime);
				ftLastAccessTime = UnixTimeToFileTime(now);

				bHasTime = TRUE;
			}
			dwCreationDisposition = TRUNCATE_EXISTING;
		}
		hFile = ::CreateFile(m_szFileName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AfxThrowFileException(-1, ::GetLastError(), m_szFileName);
		}

		// UTF8 BOMなし
		::SetFilePointer(hFile, 0, NULL, SEEK_SET);
		DWORD nBytesWritten = 0;
		DWORD nLen = 0;

		// Write Header
		CStringW szBuf = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
		{
			AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
		}

		szBuf = L"<PDF2W01>\n";
		if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
		{
			AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
		}

		// Common Section
		if( pXmlMap )
		{
			WCHAR buf[256] = {0};
			szBuf = L"	<COMMON>\n";
			if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
			{
				AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
			}

			int nID = 0;
			LPXMLDATA pData = NULL;
			PPOSITION pos = pXmlMap->first();
			while(pos != NULL )
			{
				nID = NULL;
				pData = NULL;

				// Get Value
				pXmlMap->next(pos, nID, pData);

				LPWSTR pwszKey0 = NULL;
				_wcslwr_s( pwszKey0 = _wcsdup(pData->name), wcslen(pData->name) + 1);
				// default font name
				swprintf_s(buf, 255, L"		<%s>%s</%s>\n", pwszKey0, pData->value, pwszKey0);

				if( pwszKey0 )
					free(pwszKey0);

				if( !WriteFileInUTF8(hFile, buf) )
				{
					AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
				}
			}

			szBuf = L"	</COMMON>\n";
			if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
			{
				AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
			}
		}

		// Font Section
		szBuf = L"	<FONTS>\n";
		if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
		{
			AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
		}

		PPOSITION pos = pfontLast->first(true);
		while(pos )
		{
			WCHAR line[513] = {0};
			WCHAR* pwszFontName = NULL;
			LPOUTFONT lpFont = NULL;

			pfontLast->next(pos, pwszFontName, lpFont);
			if( lpFont )
			{
				if( wcslen(lpFont->fontName) == 0 )
				{
					bChanged = TRUE;
					continue;
				}
				if( lpFont->update )
					swprintf_s(line, 512, L"		<FONT fontname=\"%s\" displayfontname=\"%s\" printfontname=\"%s\" mode=\"%d\" like=\"%s\"/>\n", 
								lpFont->fontName, 
								lpFont->displayName,
								lpFont->printName,
								lpFont->mode,
								lpFont->like ? L"True" : L"False"
								);
				else
					swprintf_s(line, 512, L"		<FONT fontname=\"%s\" displayfontname=\"%s\" printfontname=\"%s\" mode=\"%d\" like=\"%s\"/>\n", 
								lpFont->fontName, 
								lpFont->displayName,
								lpFont->printName,
								lpFont->mode,
								lpFont->like ? L"True" : L"False"
								);
				//if( lpFont->update )
				//	swprintf_s(line, 512, L"		<FONT fontname=\"%s\" displayfontname=\"%s\" printfontname=\"%s\" mode=\"%d\" like=\"%s\" updatetime=\"%s\"/>\n", 
				//				lpFont->baseName, 
				//				lpFont->displayName,
				//				lpFont->printName,
				//				lpFont->mode,
				//				lpFont->like ? L"True" : L"False",
				//				lasttm);
				//else
				//	swprintf_s(line, 512, L"		<FONT fontname=\"%s\" displayfontname=\"%s\" printfontname=\"%s\" mode=\"%d\" like=\"%s\" updatetime=\"%s\"/>\n", 
				//				lpFont->baseName, 
				//				lpFont->displayName,
				//				lpFont->printName,
				//				lpFont->mode,
				//				lpFont->like ? L"True" : L"False",
				//				lpFont->updtime);

				if( !WriteFileInUTF8(hFile, line) )
				{
					AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
				}
			}
		}
		szBuf = L"	</FONTS>\n";
		if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
		{
			AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
		}

		szBuf = L"</PDF2W01>\n";
		if( !WriteFileInUTF8(hFile, szBuf.GetBuffer()) )
		{
			AfxThrowFileException(-2, ::GetLastError(), m_szFileName);
		}
		
		if( !bChanged &&
			bHasTime )
		{
			SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);
		}

		CloseHandle(hFile);
	}
	CATCH(CException, e)
	{
		if( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		if( pOfontOut )
		{
			delete pOfontOut;
			pOfontOut = NULL;
		}
		nRet = -1;
	}
	END_CATCH

	if( pfontLast )
	{
		delete pfontLast;
		pfontLast = NULL;
	}
	if( pOfontOut )
	{
		delete pOfontOut;
		pOfontOut = NULL;
	}

	return nRet;
}
