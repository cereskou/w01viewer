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
	wchar_t baseName[64];		// �X�V�p
	wchar_t	fontName[64];		// �t�H���g��
	wchar_t displayName[64];	// �\���p�t�H���g��
	wchar_t printName[64];		// ����p�t�H���g��
	bool	like;				// ���S��v
	wchar_t updtime[20];		// �ŏI�X�V����
	unsigned short	mode;		// 0: ������  1: �c����
	bool	update;
	int		used;
	int		flags;				// �X�V�t���O
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
			// ���̃}�b�v�Ɉ��p���ꂽ�ꍇ�A������Ȃ�
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
