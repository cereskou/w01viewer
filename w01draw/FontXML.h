#ifndef __FONTXML_H__
#define __FONTXML_H__
#pragma once

#include "hashtbl.h"

#define UPD_DISPNAME	0x01
#define UPD_PRNTNAME	0x02
#define UPD_MODE		0x04
#define UPD_LIKE		0x08
#define UPD_FONTNAME	0x10

// Output Font List
typedef struct _OUTFONT
{
	wchar_t baseName[64];		// 更新用
	wchar_t	fontName[64];		// フォント名
	wchar_t displayName[64];	// 表示用フォント名
	wchar_t printName[64];		// 印刷用フォント名
	bool	like;				// 完全一致
	wchar_t updtime[20];		// 最終更新時刻
	unsigned short	mode;		// 0: 横書き  1: 縦書き
	bool	update;
	int		used;
	int		flags;				// 更新フラグ
} OUTFONT, *LPOUTFONT;
// ------------------------------------------------------------------
// FREE function for HashTbl<WCHAR*, LPOUTFONT>
// ------------------------------------------------------------------
struct _freeOutFontP
{
	void operator()(WCHAR* key, LPOUTFONT lpRec) const
	{
		if( key != NULL )
		{
			free(key);
		}
		if( lpRec )
		{
			// 他のマップに引用された場合、解放しない
			lpRec->used--;
			if( lpRec->used < 0 )
			{
				if( lpRec != NULL )
					free(lpRec);
			}
		}
	}
};
typedef HashTbl<WCHAR*, LPOUTFONT, _freeOutFontP, _compareKeyStrW, _dupKeyStrW>	CHashTblStringToOutFont;

// XML Data
#define ID_NODATA				0x00
#define ID_DEFAULTFONTNAME		0x01

typedef struct _XMLDATA
{
	int		id;
	wchar_t name[256];
	wchar_t	value[256];
} XMLDATA, *LPXMLDATA;

struct _freeXmlData
{
	void operator()(int, LPXMLDATA lpData) const
	{
		if( lpData )
		{
			free(lpData);
		}
	}
};
typedef HashTbl<int, LPXMLDATA, _freeXmlData>	CHashTblIntToXmlData;

class CFontXML
{
public:
	CFontXML(void);
	~CFontXML(void);

public:
	BOOL Open(LPCTSTR lpszName);
	BOOL Save(CHashTblStringToOutFont* pfontOut);

	CHashTblStringToOutFont* GetFontList() { return m_pFontTbl; }
	CHashTblIntToXmlData* GetXmlData() { return m_pDataTbl; }

	CString& GetFileName() { return m_szFileName; }

protected:
	BOOL InitMemory();
	void ClearMemory();

	int readFontList(CHashTblStringToOutFont* pfontOut, CHashTblIntToXmlData* pxmlData = NULL);
	int saveFontList(CHashTblStringToOutFont* pfontOut);
private:
	CString m_szFileName;
	CHashTblStringToOutFont*	m_pFontTbl;
	CHashTblIntToXmlData*		m_pDataTbl;
};

#endif
