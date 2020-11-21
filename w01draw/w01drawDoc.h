
// w01drawDoc.h : CW01DrawDoc �N���X�̃C���^�[�t�F�C�X
//


#pragma once

#include "W01Doc.h"
#include "FontXML.h"
#include "WsiFile.h"

// �X�g���[�WID
#define STID_PAGE				0x00		
#define STID_OVERLAY			0x01

#define FONT_STYLE_BOLD				0x01		// �{���g
#define FONT_STYLE_ITALIC			0x02		// �C�^���b�N
#define FONT_STYLE_UNDERLINE		0x04		// 
#define FONT_STYLE_STRIKETHROUGH	0x08		// ��������
#define FONT_STYLE_STROKE			0x10		// �X�g���[�N
#define FONT_STYLE_WMODE			0x20		// wMode = 1 (�c�����t�H���g�j

typedef CMap<LONG, LONG, CFont*, CFont*>	CMapFontP;

#define DEFAULT_FONT		(-1)
#define DEFAULT_FONT_SIZE	(18)

typedef HashTbl<INT, LPDOCOVLINF>	CHashTblIntToOvlInf;

typedef struct _posSrch
{
	int used;
	WEB_POS_RECDATA*	pos;
	USHORT				width;				// ��p�̈�̕�
	USHORT				height;				// ��p�̈�̍���
	SHORT				rotate;				// ��]
	SHORT				wmode;				// wMode�@�c�����H
	WCHAR				text[4];			// ������
	WCHAR				fontName[64];		// �t�H���g��
	WCHAR				dispfontName[64];
	CHAR				flag;
} TEXTPOSITION, *LPTEXTPOSITION;

struct _freeTextPosSP
{
	void operator()(TCHAR* pKey,  LinkedList<LPTEXTPOSITION>* lpRec) const
	{
		_free(pKey);
		_delete(lpRec);
	}
};
typedef HashTbl<TCHAR*, LinkedList<LPTEXTPOSITION>*, _freeTextPosSP, _compareKeyStr, _dupKeyStr> CHashTblTextPosS;	// �����p

struct _freeTextPosPL
{
	void operator()(LONG nPage, CHashTblTextPosS* lpRec) const
	{
		_delete(lpRec);
	}
};
typedef HashTbl<LONG, CHashTblTextPosS*, _freeTextPosPL> CHashTblPageTextPos;

typedef struct _fontLinkP
{
	LONG	lKey;
	CFont*	pFont;
	TCHAR	fontName[64];
} FONTLINK, *LPFONTLINK;
typedef LinkedList<LPFONTLINK>	CListFontLink;
// ------------------------------------------------------------------
// FREE function for HashTbl<WCHAR*,  LinkedList<LPFONTLINK>*>
// ------------------------------------------------------------------
struct _freeFontListP
{
	void operator()(WCHAR* key,  LinkedList<LPFONTLINK>* lpRec) const
	{
		_free(key);
		_delete(lpRec);
	}
};
typedef HashTbl<WCHAR*, LinkedList<LPFONTLINK>*, _freeFontListP, _compareKeyStr, _dupKeyStr> CHashTblFontToList;


class CW01DrawDoc : public CDocument
{
protected: // �V���A��������̂ݍ쐬���܂��B
	CW01DrawDoc();
	DECLARE_DYNCREATE(CW01DrawDoc)

// ����
public:
	CMapFontP		m_pmapFont;			// �t�H���g���i�����p�j
	CHashTblFontToList*	m_pTblFontList;		// �t�H���g���X�g

// ����
public:
	CSize GetDocSize() { return m_rcSize; }

	void Draw(CDC* pDC, int nPageNo = -1, UINT nHlId = 0);

	BOOL MoveFirstPage();
	BOOL MoveLastPage();
	BOOL MoveNextPage();
	BOOL MovePrevPage();
	BOOL MoveToPage(int nPageNo);

	void SetShowLine(BOOL bSet) { showLine = bSet; }
	void SetShowImage(BOOL bSet) { showImage = bSet; }
	void SetShowText(BOOL bSet) { showText = bSet; }
	void SetShowTextXY(BOOL bSet) { showTextXY = bSet; }
	void SetShowOvly(BOOL bSet) { showOvly = bSet; }
	void SetShowCellDef(BOOL bSet) { showCellDef = bSet; }
	void SetShowHighLight(BOOL bSet) { showHighLight = bSet; }
	void SetCaptureHover(BOOL bSet);
	void SetShowWsi(BOOL bSet) { showWsi = bSet; }

	BOOL IsShowLine()    { return showLine; }
	BOOL IsShowImage()   { return showImage; }
	BOOL IsShowText()    { return showText; }
	BOOL IsShowOvly()    { return showOvly; }
	BOOL IsShowTextXY()  { return showTextXY; }
	BOOL IsShowCellDef() { return showCellDef; }
	BOOL IsShowHighLight() { return showHighLight; }
	BOOL IsCaptureHover() { return bCaptureHover; }

	BOOL IsShowWsi() { return showWsi; }

	void OnTextDump();

	// GET - ���[�f�[�^
	LPPAGECONTENT GetPageContent(int pageNo = -1);

	LinkedList<LPTEXTADDR>*	GetTextList(USHORT type = 0x00);
	LPTEXTADDR GetText(hash_size index, USHORT type = 0x00);

	// GET - HEADER
	BOOL GetDocHeader(WEB_HEADER2* pHead);

	WEB_ATTR_RECDATA* GetTextAttr(hash_size index);
	WEB_FONT_DATA* GetFont(hash_size index);
	// GET - FONT
	LinkedList<WEB_FONT_DATA*>* GetFontList() { return m_plstFont; }
	// ���[�U�[���
	LinkedList<WEB_USER_INFO_DATA*>* GetUserInfoList() { return m_plstUser;	 }
	// �C���[�W���
	CHashTblImage* GetImageMap() { return m_pmapImage; }
	//@QHGOU 20150722 >>
	CHashTblBinary* GetBinaryMap() { return m_pmapBinary; }
	//@QHGOU 20150722 <<
	// ���l�e�[�u�����
	CHashTblNumsObject*	GetNumericalTable();

	// �I�[�o�[���C���
	LinkedList<LPDOCOVLINF>* GetOvlyInfList() { return m_plstOvlInf; }
	LPDOCOVLINF GetOverlay(int ovlNo);

	float GetScaleMultiples() { return ovlyScale; }

	void SetHighLightID(INT dwID);
	INT GetHighLightID() { return m_objHighLight; }
	
	CString GetDisplayFontName(LPCTSTR pszFontName);
	int RefreshFont(CHashTblStringToOutFont* pMap);
	LPWSTR GetFontMap(LPCWSTR lpszName);

	LinkedList<LPDOCPAGE>* ReadAllPageInfo(int pageNo = -1);
// �I�[�o�[���C�h
public:
	BOOL SaveImageFile();
	UINT GetImageCount();

	BOOL ShowDocInfo();
	int GetPage() { return m_nPageNo; }
	int GetTotalPage();
	void UpdateUI();
	
	CHashTblTextPosS* GetTextPosP(int nPageNo = -1);
	LPTEXTPOSITION FindPosition(LPPOINT pt);
	void SetCurrentPos(LPTEXTPOSITION pos) { m_posCurrent = pos; }
	LPTEXTPOSITION GetCurrentPos () { return m_posCurrent; }

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// ����
public:
	virtual ~CW01DrawDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void ClearMemory();
	BOOL InitMemory();
	CFont* LoadFont(LONG attrId, float scale = 1.0, INT nParent = -1);

	int ShowFontInfo(LPVOID pData);
	int ShowUserInfo(LPVOID pData);

	BOOL ReadPageData(int pageNo);
	// �C���[�W�f�[�^�擾
	LPIMAGEDATA GetImageData(USHORT id);

	void DrawImage(CDC* pDC, SHORT x, SHORT y, USHORT width, USHORT height, LPVOID pImage, ULONG size);
	void DrawImage(CDC* pDC, LPSTR lpszFileName, SHORT x, SHORT y, USHORT width, USHORT height);
	void DrawLine(CDC* pDC, SHORT x0, SHORT y0, SHORT x1, SHORT y1, USHORT width, ULONG color);
	void DrawOverlay(CDC* pDC, USHORT id, SHORT x, SHORT y, SHORT height, SHORT width, LONG scalex, LONG scaley);
	void DrawCellDefine(CDC* pDC);
	
	COLORREF GetOverlayTextColor(BOOL& bAuto);
	
	void SplitAreaTable(LPPAGECONTENT pContent);
	LPTEXTPOSITION AddAreaCode(CHashTblTextPosS* pmapTextPos, CString& code, WEB_POS_RECDATA* pos);
	void AddAreaCode(CHashTblTextPosS* pmapTextPos, CString& code, LPTEXTPOSITION pTxtPos);

	//@QHGOU 20161018 Wis File >>
	BOOL ReadWsiFile(LPWSTR pwszWsi);
	void DrawWsi(CDC* pDC);
	//@QHGOU 20161018 Wis File <<

protected:
	CW01Doc*	m_pW01Doc;
	INT			m_nScreenDpi;
	CSize		m_rcSize;
	int			m_nPageNo;

	float		ovlyScale;

	BOOL		showLine;
	BOOL		showImage;
	BOOL		showText;
	BOOL		showOvly;
	BOOL		showTextXY;
	BOOL		showCellDef;
	BOOL		showHighLight;
	BOOL		bCaptureHover;
	BOOL		showWsi;

	INT			m_objHighLight;

	LPTEXTPOSITION	m_posCurrent;
	int			m_areaLevel;

	int			m_tppx;
	int			m_tppy;

	// ���[�f�[�^
	LinkedList<LPTEXTADDR>*				m_plstTextOvl;		// �����e�[�u��(�I�[�o�[���C�p�j
	LinkedList<LPTEXTADDR>*				m_plstText;			// �����e�[�u��
	LinkedList<WEB_FONT_DATA*>*			m_plstFont;			// �t�H���g���
	LinkedList<WEB_USER_INFO_DATA*>*	m_plstUser;	// ���[�U�[���
	CHashTblIntToAttr*					m_pAttTbl;			// ���������ڍ׏��
	CHashTblImage*						m_pmapImage;		// �C���[�W���
	LinkedList<LPDOCOVLINF>*			m_plstOvlInf;		// �I�[�o�[���C���
	CHashTblIntToOvlInf*				m_pmapOvlInf;		// �����p
	//@QHGOU 20150722 >>
	CHashTblBinary*						m_pmapBinary;		// �o�C�i���f�[�^
	//@QHGOU 20150722 <<
	CHashTblNumsObject*					m_pmapNumerical;		// ���l�e�[�u�����(double -> Number List)
	CHashTblNumsObjectLink*				m_pmapNumericalLink;	// �y�[�W���̐��l���(page recNo -> Number List)

	//CHashTblTextPosS*					m_pmapTextPos;		// �����p
	CHashTblPageTextPos*				m_pmapTextPos;		// �����p

	// �y�[�W�f�[�^
	CHashTblPageContent*				m_pmapContent;		// �y�[�W���R���e���c
	CArray<INT, INT>					m_plstPageNo;		// �y�[�W�ԍ�

	//@QHGOU 20161018 Wis File >>
	CWsiFile*	m_pWsiFile;
	//@QHGOU 20161018 Wis File <<

// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// �����n���h���[�̌����R���e���c��ݒ肷��w���p�[�֐�
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	afx_msg void OnFilePdf();
	afx_msg void OnUpdateFilePdf(CCmdUI *pCmdUI);
	afx_msg void OnFileOpencell();
	afx_msg void OnFileOpenWsi();
};
