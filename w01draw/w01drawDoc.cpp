
// w01drawDoc.cpp : CW01DrawDoc クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、サムネイル、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "w01draw.h"
#endif
#include "ByteStream.h"

#include "MainFrm.h"
#include "w01drawDoc.h"
#include <math.h>
#include <propkey.h>
#include "InfoDialog.h"
#include "ImageSaveDlg.h"
#include "FontSmoothingType.h"

#pragma comment(lib, "w01doc.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// RGBA
#define RGBA_R(argb) (BYTE)(argb >> 24)
#define RGBA_G(argb) (BYTE)(argb >> 16)
#define RGBA_B(argb) (BYTE)(argb >> 8)
#define RGBA_A(argb) (BYTE)(argb >> 0)

#define ID_PROP_IMAGE		0x1000
#define ID_PROP_OVLY		0x2000
#define ID_PROP_IMGINOVL	0x3000

struct _colorList
{
	SHORT		index;
	COLORREF	color;
} colorList[] = {
	{ 1, RGB(255, 0, 0) },
	{ 2, RGB(63, 72, 204) },
	{ 3, RGB(255, 127, 39) },
	{ 4, RGB(24, 177, 76) },
	{ 5, RGB(0, 162, 232) },
	{ 6, RGB(163, 73, 164) },
	{ 7, RGB(112, 146, 190) },
	{ -1, 0 }
};

long TwipsPerPixelX(CDC* pDC)
{
	int factor = 1440 / pDC->GetDeviceCaps(LOGPIXELSX);
	return factor;
}

long TwipsPerPixelY(CDC* pDC)
{
	int factor = 1440 / pDC->GetDeviceCaps(LOGPIXELSY);
	return factor;
}

long TwipsPerPixelX(HDC hDC)
{
	int factor = 1440 / ::GetDeviceCaps(hDC, LOGPIXELSX);
	return factor;
}

long TwipsPerPixelY(HDC hDC)
{
	int factor = 1440 / ::GetDeviceCaps(hDC, LOGPIXELSY);
	return factor;
}

// ------------------------------------------------------------------
// 解放処理
// ------------------------------------------------------------------
void _freeTextPosP(LPTEXTPOSITION pTxtPos)
{
	if( pTxtPos )
	{
		pTxtPos->used--;
		if( pTxtPos->used < 0 )
			free(pTxtPos);
	}
}

void _deleteFontLink(LPFONTLINK pLnk)
{
	if( pLnk )
	{
		_free(pLnk);
	}
}

void _freeUserInfo(WEB_USER_INFO_DATA* pUsr)
{
	if( pUsr )
	{
		_free(pUsr->pKey);
		_free(pUsr->pData);

		_free(pUsr);
	}
}

void _freeOvlPtr(LPDOCOVLINF pData)
{
	if( pData )
	{
		_delete(pData->plstPageNo);
		_delete(pData->plstRec);
		_delete(pData->plstTextPos);
		_delete(pData->plstImage);
		_delete(pData->plstLine);

		_free(pData);
	}
}


// CW01DrawDoc

IMPLEMENT_DYNCREATE(CW01DrawDoc, CDocument)

BEGIN_MESSAGE_MAP(CW01DrawDoc, CDocument)
	ON_COMMAND(ID_FILE_PDF, &CW01DrawDoc::OnFilePdf)
	ON_UPDATE_COMMAND_UI(ID_FILE_PDF, &CW01DrawDoc::OnUpdateFilePdf)
	ON_COMMAND(ID_FILE_OPENCELL, &CW01DrawDoc::OnFileOpencell)
	ON_COMMAND(ID_FILE_WSI, &CW01DrawDoc::OnFileOpenWsi)
END_MESSAGE_MAP()


// CW01DrawDoc コンストラクション/デストラクション

CW01DrawDoc::CW01DrawDoc()
{
	m_pW01Doc = NULL;

	m_nScreenDpi = 96;

	m_rcSize.cx = 0;
	m_rcSize.cy = 0;

	showLine = TRUE;
	showImage = TRUE;
	showText = TRUE;
	showOvly = TRUE;
	showTextXY = FALSE;
	showCellDef = FALSE;
	showHighLight = FALSE;
	bCaptureHover = FALSE;
	showWsi = FALSE;

	m_pmapContent = NULL;

	m_plstText = NULL;
	m_plstTextOvl = NULL;

	m_plstOvlInf = NULL;
	m_pmapOvlInf = NULL;
	m_pmapImage = NULL;
	m_pmapTextPos = NULL;
	//@QHGOU 20150722 >>
	m_pmapBinary = NULL;
	//@QHGOU 20150722 <<
	m_pmapNumerical = NULL;
	m_pmapNumericalLink = NULL;

	m_nPageNo = 0;

	ovlyScale = SCALE_DENOMINATOR;

	m_pTblFontList = NULL;

	m_posCurrent = NULL;
	m_areaLevel = 8;

	m_objHighLight = -1;

	HDC dc = ::GetDC(NULL);
	m_tppx = TwipsPerPixelX(dc);
	m_tppy = TwipsPerPixelY(dc);
	::ReleaseDC(NULL, dc);

	//@20161018
	m_pWsiFile = NULL;
}

CW01DrawDoc::~CW01DrawDoc()
{
}


// CW01DrawDoc シリアル化

#ifdef SHARED_HANDLERS

// サムネイルのサポート
void CW01DrawDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// このコードを変更してドキュメントのデータを描画します
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 検索ハンドラーのサポート
void CW01DrawDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ドキュメントのデータから検索コンテンツを設定します。 
	// コンテンツの各部分は ";" で区切る必要があります

	// 例:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CW01DrawDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CW01DrawDoc 診断

#ifdef _DEBUG
void CW01DrawDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CW01DrawDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CW01DrawDoc コマンド
long GetScreenDpi()
{
	::SetProcessDPIAware();
	HDC screen = ::GetDC(NULL);

	int dpiX = ::GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = ::GetDeviceCaps(screen, LOGPIXELSY);
	::ReleaseDC(NULL, screen);

	return (long)((dpiX + dpiY) * 0.5);
}


BOOL CW01DrawDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	WEB_HEADER2 header = { 0 };
	BOOL bReturn = FALSE;
	theApp.BeginWaitCursor();
	m_rcSize.cx = 0;
	m_rcSize.cy = 0;

	// メモリー初期化
	if( !InitMemory() )
	{
		goto OPEN_END;
	}

	int flags = CW01FileF::modeRead | CW01FileF::shareDenyWrite; 
	m_pW01Doc = new CW01Doc();
	if( !m_pW01Doc->Open(lpszPathName, flags) )
	{
		goto OPEN_END;
	}
	
	// セル定義ファイルの格納フォルダ
	if( !m_pW01Doc->SetCellDefPath(theApp.GetCellDefinePath()) )
	{
		goto OPEN_END;
	}

	if( theApp.GetUseCellOption() )
	{
		LPCELLDEFOPTION pCellOpt = theApp.GetCellOption();
		if( pCellOpt )
			m_pW01Doc->SetCellDefOptions(pCellOpt);
	}
	// SAMPLE <<

	// 3 ユーザー情報
	if( !m_pW01Doc->ReadUserInfoData(m_plstUser) )
	{
		goto OPEN_END;
	}

	// 105 フォント情報
	if( !m_pW01Doc->ReadFontData(m_plstFont) )
	{
		goto OPEN_END;
	}

	// 212 文字列属性詳細情報
	if( !m_pW01Doc->ReadTextAttributes(m_pAttTbl) )
	{
		goto OPEN_END;
	}
	
	// 文字テーブルの読込
	if( !m_pW01Doc->ReadTextTable(m_plstText, OBJ_TYPE_PAGE) )
	{
		goto OPEN_END;
	}
	// 文字テーブルの読込(オーバーレイ）
	if( !m_pW01Doc->ReadTextTable(m_plstTextOvl, OBJ_TYPE_OVERLAY) )
	{
		goto OPEN_END;
	}
	
	// 数値テーブルの読込
	if( !m_pW01Doc->ReadNumericalTable(m_pmapNumerical, m_pmapNumericalLink, OBJ_TYPE_PAGE))
	{
		goto OPEN_END;
	}

	// 18 イメージ情報
	if( !m_pW01Doc->ReadImageData(m_pmapImage) )
	{
		goto OPEN_END;
	}

	m_pW01Doc->GetHeader(&header);

	//@QHGOU 20150722 >>
	if( m_pW01Doc->GetDocVersion() >= WEB_VERSION_400 )
	{
		// 30 バイナリ情報
		if( !m_pW01Doc->ReadBinaryData(m_pmapBinary, TRUE))
		{
			goto OPEN_END;
		}
	} else {
	//@QHGOU 20150722 <<
		// オーバーレイ情報
		if( !m_pW01Doc->ReadOverlayData(m_plstOvlInf) )
		{
			goto OPEN_END;
		}
	//@QHGOU 20150722 >>
	}
	//@QHGOU 20150722 <<

	// List to Map
	if( m_plstOvlInf &&
		m_plstOvlInf->first() )
	{
		do
		{
			m_pmapOvlInf->insert(m_plstOvlInf->index(), m_plstOvlInf->get());
		} while( m_plstOvlInf->next() );
	}

	// 9 ページ情報
	if(! ReadPageData(0) )
	{
		goto OPEN_END;
	}

	int dpi = m_pW01Doc->GetDPI();
	if( dpi == 0 )
		dpi = 240;

	m_nScreenDpi = GetScreenDpi();

	//// Create CFont list 事前作成
	//LinkedList<WEB_FONT_DATA*>* pFntLst = m_plstFont;
	//CHashTblIntToAttr* pAttTbl = m_pAttTbl;
	//if( pAttTbl && pFntLst )
	//{
	//	CFont* pNewFont = NULL;
	//	WCHAR fontName[64] = {0};
	//	WORD attrId = 0;
	//	PPOSITION pos = pAttTbl->first(true);
	//	while( pos )
	//	{
	//		pNewFont = NULL;
	//		INT nId = 0;
	//		WEB_ATTR_RECDATA* pAttr = NULL;
	//		pAttTbl->next(pos, nId, pAttr);
	//		if( pAttr )
	//		{
	//			WEB_FONT_DATA* pFont = pFntLst->at(pAttr->fontId);
	//			if( pFont )
	//			{
	//				LPWSTR lpMapFontName = theApp.GetFontMap(pFont->name);

	//				if( lpMapFontName != NULL && wcslen(lpMapFontName) > 0 )
	//					wcscpy_s(fontName, 63, lpMapFontName);
	//				else
	//					wcscpy_s(fontName, 63, pFont->name);
	//			}
	//			else
	//			{
	//				wcscpy_s(fontName, 63, theApp.GetDefaultFontName());
	//			}

	//			pNewFont = new CFont;
	//			if( pNewFont )
	//			{
	//				int nWeight = FW_NORMAL;
	//				BOOL bItalic = FALSE;
	//				// イタリック
	//				if( pAttr->fontStyle & FONT_STYLE_ITALIC )
	//				{
	//					bItalic = TRUE;
	//				}
	//				// ボルト
	//				if( pAttr->fontStyle & FONT_STYLE_BOLD )
	//				{
	//					nWeight = FW_BOLD;
	//				}

	//				// フォント回転角度（0, 90, 180, 270）
	//				int angle = (360 - pAttr->fontRotate) * 10;
	//				if( pAttr->fontRotate == 0 )
	//					angle = 0;

	//				HDC dc = ::GetDC(NULL);
	//				int tppx = TwipsPerPixelX(dc);
	//				int tppy = TwipsPerPixelY(dc);
	//				int cHeight = pAttr->fontHeight * 20 / tppx;
	//				int cWidth = pAttr->fontWidth * 10 / tppy;
	//				::ReleaseDC(NULL, dc);

	//				cHeight = (int)floor((double)(cHeight * 72 / m_nScreenDpi));
	//				cWidth = (int)floor((double)(cWidth * 72 / m_nScreenDpi));
	//					
	//				pNewFont->CreateFont(
	//					cHeight,
	//					cWidth,
	//					angle,
	//					0,
	//					nWeight,
	//					bItalic,
	//					FALSE,
	//					0,
	//					DEFAULT_CHARSET, // ANSI_CHARSET,
	//					OUT_DEFAULT_PRECIS,
	//					CLIP_DEFAULT_PRECIS,
	//					CLEARTYPE_NATURAL_QUALITY, // DEFAULT_QUALITY,
	//					DEFAULT_PITCH | FF_DONTCARE,
	//					fontName);

	//			//{
	//			//	LPSTR pfName = _ConvW2A(fontName);
	//			//	TRACE(_T("[FONTNAME]attrId: %d, %S, cHeight:%d, cWidth:%d\n"), pAttr->attrId, pfName, cHeight, cWidth);
	//			//	_del(pfName);
	//			//}

	//				m_pmapFont.SetAt(attrId, pNewFont);
	//				attrId++;
	//			}
	//		}
	//	}
	//}

	bReturn = TRUE;
OPEN_END:
	if( !bReturn )
	{
		_delete(m_pWsiFile);
		_delete(m_pW01Doc);
	}
	theApp.EndWaitCursor();

	return bReturn;
}


void CW01DrawDoc::OnCloseDocument()
{
	// Update List
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		CPoint pt;
		pFrame->SetCursorPosText(pt, TRUE);
		pFrame->SetPaper(0, 0, TRUE);

		if( theApp.GetEnableDebug() )
		{
			CTextPropertiesWnd* pTextWnd = pFrame->GetTextPropertiesWnd();
			if( pTextWnd )
			{
				pTextWnd->SetWindow(NULL, TRUE);
			}
			CFontPropertiesWnd* pFontWnd = pFrame->GetFontPropertiesWnd();
			if( pFontWnd )
			{
				pFontWnd->SetWindow(NULL, TRUE);
			}
			CDocInfoPropertiesWnd* pDocInfWnd = pFrame->GetDocInfoPropertiesWnd();
			if( pDocInfWnd )
			{
				pDocInfWnd->SetWindow(NULL, TRUE);
			}
			CPageInfoPropertiesWnd* pPageInfWnd = pFrame->GetPageInfoPropertiesWnd();
			if( pPageInfWnd )
			{
				pPageInfWnd->SetWindow(NULL, TRUE);
			}
			CNumericalPropertiesWnd* pNumericalWnd = pFrame->GetNumericalPropertiesWnd();
			if( pNumericalWnd )
			{
				pNumericalWnd->SetWindow(NULL, TRUE);
			}
		}
	}

	ClearMemory();

	//@QHGOU 20161018
	if (m_pWsiFile) {
		m_pWsiFile->Close();
	}
	_delete(m_pWsiFile);

	TRACE(_T("W01Doc Close() ... "));
	if( m_pW01Doc )
	{
		m_pW01Doc->Close();
	}
	_delete(m_pW01Doc);

	TRACE(_T("Done.\n"));

	CDocument::OnCloseDocument();
}

void CW01DrawDoc::Draw(CDC* pDC, int nPageNo, UINT nHlId)
{
	if( m_pW01Doc == NULL ||
		m_pmapContent == NULL )
		return;

	if( nPageNo == -1 )
		nPageNo = GetPage();

	LPPAGECONTENT pContent = m_pmapContent->get(nPageNo);
	if( !pContent )
		return;

	CHashTblIntToAttr* pTextAttr = m_pAttTbl;
	if( pTextAttr == NULL ||
		pTextAttr->size() == 0 )
	{
		TRACE(_T("No TEXT!\n"));
		//return;
	}

	int lineWidth = 3;

	int dpi = m_pW01Doc->GetDPI();
	if( dpi == 0 )
		dpi = 240;

	lineWidth = lineWidth * dpi / m_nScreenDpi;

	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);

	// Draw Paper
	RECT docRect;
	docRect.left = docRect.top = 0;
	docRect.right = pContent->width;
	docRect.bottom = pContent->height;

	pDC->SetGraphicsMode(GM_ADVANCED);

	pDC->FillSolidRect(&docRect, ::GetSysColor(COLOR_WINDOW));

	int nCount = 0;

	// Save
	int OldBkMode = pDC->GetBkMode();
	pDC->SetBkMode(TRANSPARENT);

	UINT txtA = pDC->SetTextAlign(TA_BASELINE);
	COLORREF orgColorText = pDC->GetTextColor();
	COLORREF oldColorText = 0;
	COLORREF oldBkColor = 0;

	int pixelsX = pDC->GetDeviceCaps(LOGPIXELSX);
	int pixelsY = pDC->GetDeviceCaps(LOGPIXELSY);

	INT dwHlId = GetHighLightID();
	
	// Draw Overlay
	LinkedList<LPOVLYLINK>* pLinkOvly = pContent->plstOvly;
	if( IsShowOvly() &&
		pLinkOvly &&
		pLinkOvly->first() )
	{
		COLORREF clrBefore = pDC->GetTextColor();
		COLORREF clrBkBefore = pDC->GetBkColor();
		int nModeBefore = pDC->GetBkMode();

		INT nHlId = -1;
		if( dwHlId > ID_PROP_OVLY && 
			dwHlId < ID_PROP_IMGINOVL )
		{
			nHlId = dwHlId - ID_PROP_OVLY - 1;
		}

		INT nShowIndex = 0;
		LPOVLYLINK pOvly = NULL;
		do
		{
			pOvly = pLinkOvly->get();
			if( pOvly && pOvly->bShow )
			{
				DrawOverlay(pDC, pOvly->ovl.ovlid, 
							pOvly->ovl.x, pOvly->ovl.y, 
							pOvly->ovl.height, pOvly->ovl.width,
							pOvly->ovl.scalex, pOvly->ovl.scaley);

				if( IsShowHighLight() )
				{
					if( nHlId >= 0 &&
						nHlId == nShowIndex )
					{
						if( !pDC->IsPrinting() )
						{
							COLORREF colorOvly = colorList[nShowIndex % 8].color;

							CPen ovlPen(PS_SOLID, lineWidth, colorOvly);
							CPen* pOvlPen = pDC->SelectObject(&ovlPen);

							//float sx = (float)(pOvly->scalex / ovlyScale);
							//float sy = (float)(pOvly->scaley / ovlyScale);
							float sx = 1.0f;
							float sy = 1.0f;

							LONG olvx0 = (LONG)Round(pOvly->ovl.x * sx, 0.1);
							LONG olvy0 = (LONG)Round(pOvly->ovl.y * sy, 0.1);
							LONG olvx1 = olvx0 + (LONG)Round(pOvly->ovl.width * sx, 0.1);
							LONG olvy1 = olvy0 + (LONG)Round(pOvly->ovl.height * sy, 0.1);

							// Draw Page Rectange
							pDC->MoveTo(olvx0, olvy0);
							pDC->LineTo(olvx0, olvy1);
							pDC->LineTo(olvx1, olvy1);
							pDC->LineTo(olvx1, olvy0);
							pDC->LineTo(olvx0, olvy0);
							//pDC->Draw3dRect(&ovlyRect, RGB(255, 0, 0), RGB(255, 0, 0));

							if( pOvlPen )
								pDC->SelectObject(pOvlPen);

							CFont* pOldDrawFont = NULL;
							CFont* pDrawFont = LoadFont(DEFAULT_FONT);
							if( pDrawFont )
							{
								COLORREF colorOvlRestore = pDC->SetTextColor(colorOvly);

								pOldDrawFont = pDC->SelectObject(pDrawFont);

								WCHAR szOvlId[64] = {0};
								swprintf_s(szOvlId, 63, L"OVL#%d [id:%d rect(%d,%d,%d,%d)]", nShowIndex, pOvly->ovl.ovlid, olvx0, olvy0, olvx1, olvy1);
							
								pDC->TextOutW(olvx0 + 3, olvy0 + DEFAULT_FONT_SIZE + 3, szOvlId);

								pDC->SelectObject(pOldDrawFont);
								pDC->SetTextColor(colorOvlRestore);
							}
						}
					}
				} // if( IsShowHighLight() )

			}

			nShowIndex++;
		} while( pLinkOvly->next());

		pDC->SetTextColor(clrBefore);
		pDC->SetBkColor(clrBkBefore);
		pDC->SetBkMode(nModeBefore);
	}

	LinkedList<WEB_LINE_DATA*>* plstLine = pContent->plstLine;
	// Draw Line
	if( IsShowLine() &&
		plstLine &&
		plstLine->first() )
	{
		//WEB_DEF_CL
		WEB_LINE_DATA* pLine = NULL;

		while((pLine = plstLine->get()) != NULL)
		{
			DrawLine(pDC, pLine->sx, pLine->sy, pLine->ex, pLine->ey, pLine->width, pLine->color);

			if( !plstLine->next() )
				break;
		}

	}

	LinkedList<LPIMAGELINK>* plstImage = pContent->plstImage;
	// Draw Image
	if( IsShowImage() &&
		m_pmapImage &&
		plstImage &&
		plstImage->first() )
	{
		INT nHlId = -1;
		if( dwHlId > ID_PROP_IMAGE &&
			dwHlId < ID_PROP_OVLY )
		{
			nHlId = dwHlId - ID_PROP_IMAGE - 1;
		}

		LPIMAGELINK pImgLnk = NULL;
		INT nShowIndex = 0;
		while((pImgLnk = plstImage->get()) != NULL)
		{
			//if( pImgLnk->bShow )
			{
				LPIMAGEDATA lpImgLnk = GetImageData(pImgLnk->link.imgNo);
				if( lpImgLnk != NULL )
				{
					DrawImage(pDC, pImgLnk->link.x, pImgLnk->link.y, pImgLnk->link.width, pImgLnk->link.height, lpImgLnk->pImage, lpImgLnk->length);

					if( IsShowHighLight() )
					{
						if( nHlId >= 0 &&
							nHlId == nShowIndex )
						{
							if( !pDC->IsPrinting() )
							{
//								COLORREF colorBoder = colorList[nShowIndex % 8].color;
								COLORREF colorBoder = RGB(255, 0, 0);

								CPen imgPen(PS_SOLID, lineWidth, colorBoder);
								CPen* pImgPen = pDC->SelectObject(&imgPen);

								//int nROP2 = pDC->SetROP2(R2_NOTXORPEN);

								LONG olvx0 = (LONG)pImgLnk->link.x;
								LONG olvy0 = (LONG)pImgLnk->link.y;
								LONG olvx1 = olvx0 + pImgLnk->link.width;
								LONG olvy1 = olvy0 + pImgLnk->link.height;

								// Draw Page Rectange
								pDC->MoveTo(olvx0, olvy0);
								pDC->LineTo(olvx0, olvy1);
								pDC->LineTo(olvx1, olvy1);
								pDC->LineTo(olvx1, olvy0);
								pDC->LineTo(olvx0, olvy0);

								if( pImgPen )
									pDC->SelectObject(pImgPen);

								//pDC->SetROP2(nROP2);
								imgPen.DeleteObject();
								//CFont* pOldDrawFont = NULL;
								//CFont* pDrawFont = LoadFont(DEFAULT_FONT);
								//if( pDrawFont )
								//{
								//	COLORREF colorOvlRestore = pDC->SetTextColor(colorBoder);

								//	pOldDrawFont = pDC->SelectObject(pDrawFont);

								//	WCHAR szOvlId[64] = {0};
								//	swprintf_s(szOvlId, 63, L"IMG#%d [id:%d rect(%d,%d,%d,%d)]", nShowIndex, pImgLnk->link.imgNo, olvx0, olvy0, olvx1, olvy1);
							
								//	pDC->TextOutW(olvx0 + 3, olvy0 + DEFAULT_FONT_SIZE + 3, szOvlId);

								//	pDC->SelectObject(pOldDrawFont);
								//	pDC->SetTextColor(colorOvlRestore);
								//}
							}
						}
					} // if( IsShowHighLight() )

				}
			}
			nShowIndex++;

			if(!plstImage->next() )
				break;
		}
	}

	//CFontSmoothingType smooth;
	//smooth.SetClearType();

	// Draw Text
	LinkedList<WEB_POS_RECDATA*>* pLinks = pContent->plstTextPos;
	if( IsShowText() &&
		pLinks &&
		pLinks->first() )
	{
		int nFindBkMode = 0;
		CFont* pRstFont = NULL;	// Restore
		CFont* pOldFont = NULL;
		do
		{
			WEB_POS_RECDATA* pTxtPos = NULL;
			pTxtPos = pLinks->get();
			if( pTxtPos )
			{
				CFont* pNewFont = LoadFont(pTxtPos->attrId);
				if( pNewFont == NULL )
				{
					if( !pLinks->next() )
						break;

					continue;
				}

				LPTEXTADDR pTxt = NULL;
				if( m_plstText )
				{
					pTxt = m_plstText->at(pTxtPos->textNum);
				}

				LPWSTR lpStr = NULL;
				WCHAR wChrs[5] = {0};
				if( pTxt )
				{
					wcscpy_s(wChrs, 4, pTxt->text);
					lpStr = (LPWSTR)wChrs;
				}

				if( lpStr == NULL )
				{
					if( !pLinks->next() )
						break;

					continue;
				}

				int X = pTxtPos->textX;
				int Y = pTxtPos->textY;

				COLORREF colorText = RGB(0, 0, 0);
				WEB_ATTR_RECDATA* lpDocAttr = pTextAttr->get(pTxtPos->attrId);
				if( lpDocAttr )
				{
//					colorText = lpDocAttr->data.foregroundColor;
					BYTE r = RGBA_R(lpDocAttr->foregroundColor);
					BYTE g = RGBA_G(lpDocAttr->foregroundColor);
					BYTE b = RGBA_B(lpDocAttr->foregroundColor);
					colorText = RGB(r, g, b);
//					colorText = (DWORD)(lpDocAttr->data.foregroundColor >> 8);
				}
				// 強調表示
				if( pTxtPos->index >= TEXTBLOCK_OFFSET )
				{
					colorText = RGB(255, 255, 255);
					oldBkColor = pDC->GetBkColor();
					nFindBkMode = pDC->GetBkMode();

					pDC->SetBkColor(RGB(255, 0, 0));
					pDC->SetBkMode(OPAQUE);
				}
				else
				{
					if( oldBkColor )
						pDC->SetBkColor(oldBkColor);
					if( nFindBkMode )
						pDC->SetBkMode(TRANSPARENT);

					oldBkColor = 0;
					nFindBkMode = 0;
				}
				if( oldColorText != colorText )
				{
					pDC->SetTextColor(colorText);

					oldColorText = colorText;
				}

				if( pOldFont != pNewFont )
				{
					if( pRstFont )
						pDC->SelectObject(pRstFont);

					pRstFont = pDC->SelectObject(pNewFont);

					pOldFont = pNewFont;
				}

				// 座標強調表示
				if( IsShowTextXY() )
				{
					int tx0 = X;
					int ty0 = Y;
					int tx1 = tx0 + 10;
					int ty1 = ty0;
					int tx2 = tx0;
					int ty2 = ty0 - 10;
					if( lpDocAttr )
					{
						tx1 = tx0 + (int)Round(lpDocAttr->fontWidth, 0.1);
						ty2 = ty0 -  (int)Round(lpDocAttr->fontHeight, 0.1);
					}

					CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
					CPen* pOld = pDC->SelectObject(&pen);
		
					POINT pts = {tx0, ty0};
					POINT pte1 = {tx1, ty1};
					POINT pte2 = {tx2, ty2};
					pDC->MoveTo(pts);
					pDC->LineTo(pte1);
					pDC->MoveTo(pts);
					pDC->LineTo(pte2);

					if( pOld )
						pDC->SelectObject(pOld);
				}

				pDC->TextOutW(X, Y, lpStr);
			}

		} while( pLinks->next() );

		if( pRstFont )
			pDC->SelectObject(pRstFont);
	}
	//smooth.SetNoSmoothing();

	// Draw Current Pos
	if( m_posCurrent )
	{
		CPen pen(PS_SOLID, 2, RGB(255, 0, 0));
		CPen* pOldPen = pDC->SelectObject(&pen);

		int cx0 = m_posCurrent->pos->textX;
		int cy0 = m_posCurrent->pos->textY - m_posCurrent->height;
		int cx1 = m_posCurrent->pos->textX + m_posCurrent->width;
		int cy1 = m_posCurrent->pos->textY ;

		// Draw Rectange
		pDC->MoveTo(cx0, cy0);
		pDC->LineTo(cx0, cy1);
		pDC->LineTo(cx1, cy1);
		pDC->LineTo(cx1, cy0);
		pDC->LineTo(cx0, cy0);

		if( pOldPen )
			pDC->SelectObject(pOldPen);
	}

	// セル定義表示
	if( IsShowCellDef() )
	{
		DrawCellDefine(pDC);
	}

	//@QHGOU 20161018
	// Draw Wsi
	if (IsShowWsi()) {
		DrawWsi(pDC);
	}

	pDC->SetBkMode(OldBkMode);

	// Reset
	pDC->SetTextColor(orgColorText);
	pDC->SetTextAlign(txtA);

	if( pOldPen )
	{
		pDC->SelectObject(pOldPen);
		pOldPen = NULL;
	}
}


BOOL CW01DrawDoc::InitMemory()
{
	BOOL bReturn = FALSE;

	// Page
	m_pmapContent = new CHashTblPageContent();
	if( m_pmapContent == NULL ||
		m_pmapContent->create(genHashKey) == NULL )
		goto END;

	// Text table
	m_plstText = new LinkedList<LPTEXTADDR>(false);
	if( m_plstText == NULL )
		goto END;

	m_pTblFontList = new CHashTblFontToList();
	if( m_pTblFontList == NULL ||
		m_pTblFontList->create(genHashKeyStr, false, 128) == NULL )
		goto END;

	//// 112
	//m_ppmapTextPos = new CHashTblTextPosMap();
	//if( m_ppmapTextPos == NULL ||
	//	m_ppmapTextPos->create(genHashKey) == NULL )
	//	goto END;
	//// 113
	//m_pomapTextPos = new CHashTblTextPosMap();
	//if( m_pomapTextPos == NULL ||
	//	m_pomapTextPos->create(genHashKey) == NULL )
	//	goto END;

	// フォント情報（表示用）
	m_plstFont = new LinkedList<WEB_FONT_DATA*>(true, _freeptr);
	if( m_plstFont == NULL )
		goto END;

	// ユーザー情報
	m_plstUser = new LinkedList<WEB_USER_INFO_DATA*>(true, _freeUserInfo);
	if( m_plstUser == NULL )
		goto END;

	// 文字属性詳細情報(表示用）
	m_pAttTbl = new CHashTblIntToAttr();
	if( m_pAttTbl == NULL ||
		m_pAttTbl->create(genHashKey, true) == NULL )
		goto END;

	// オーバーレイ情報
	m_plstOvlInf = new LinkedList<LPDOCOVLINF>(true, _freeOvlPtr);
	if( m_plstOvlInf == NULL )
		goto END;

	// イメージ情報	18
	m_pmapImage = new CHashTblImage();
	if( m_pmapImage == NULL ||
		m_pmapImage->create(genHashKey, true) == NULL )
		goto END;

	//@QHGOU 20150722 >>
	// バイナリ情報 30
	m_pmapBinary = new CHashTblBinary();
	if( m_pmapBinary == NULL ||
		m_pmapBinary->create(genHashKey, true) == NULL)
		goto END;
	//@QHGOU 20150722 <<

	// 数値テーブル
	m_pmapNumerical = new CHashTblNumsObject();
	if( m_pmapNumerical == NULL ||
		m_pmapNumerical->create(genHashKey, true) == NULL )
		goto END;

	// 数値テーブル（リンク情報）
	m_pmapNumericalLink = new CHashTblNumsObjectLink();
	if( m_pmapNumericalLink == NULL ||
		m_pmapNumericalLink->create(genHashKey) == NULL )
		goto END;

	// オーバーレイ情報(検索用）
	m_pmapOvlInf = new CHashTblIntToOvlInf();
	if( m_pmapOvlInf == NULL ||
		m_pmapOvlInf->create(genHashKey) == NULL )
		goto END;

	//m_pmapTextPos = new CHashTblTextPosS();
	//if( m_pmapTextPos == NULL ||
	//	m_pmapTextPos->create(genHashKeyStr) == NULL )
	//	goto END;
	m_pmapTextPos = new CHashTblPageTextPos();
	if( m_pmapTextPos == NULL ||
		m_pmapTextPos->create(genHashKey) == NULL )
		goto END;
	
	bReturn = TRUE;
END:
	if( !bReturn )
	{
		ClearMemory();
	}

	return bReturn;
}

void CW01DrawDoc::ClearMemory()
{
	TRACE(_T("CW01DrawDoc::ClearMemory() ... "));
	_delete(m_plstUser);
	_delete(m_plstFont);
	_delete(m_plstText);

	_delete(m_pAttTbl);
	_delete(m_plstOvlInf);
	_delete(m_pmapOvlInf);
	_delete(m_pmapTextPos);

	_delete(m_pmapImage);

	//@QHGOU 20150722 >>
	_delete(m_pmapBinary);
	//@QHGOU 20150722 <<
	_delete(m_pmapNumerical);
	_delete(m_pmapNumericalLink);

	//_delete(m_ppmapTextPos);
	//_delete(m_pomapTextPos);

	_delete(m_pmapContent);

	_delete(m_pTblFontList);

	POSITION mpos = m_pmapFont.GetStartPosition();
	while( mpos != NULL )
	{
		LONG nFontId = 0;
		CFont* pFont = NULL;
		m_pmapFont.GetNextAssoc(mpos, nFontId, pFont);
		if( pFont )
			delete pFont;
	}
	m_pmapFont.RemoveAll();

	TRACE(_T("Done\n"));
}

CFont* CW01DrawDoc::LoadFont(LONG attrId, float scale, INT nParent)
{
	if( m_pW01Doc == NULL )
		return NULL;

	CHashTblIntToAttr* pAttTbl = m_pAttTbl;
	if( pAttTbl == NULL ||
		pAttTbl->size() == NULL )
		return NULL;

	// フォント一覧
	LinkedList<WEB_FONT_DATA*>* pFntLst = m_plstFont;
	if( pFntLst == NULL ||
		pFntLst->size() == NULL )
		return NULL;

	int dpi = m_pW01Doc->GetDPI();
	if( dpi == 0 )
		dpi = 240;

	CFont* pNewFont = NULL;
	if( attrId == DEFAULT_FONT )
	{
		if( !m_pmapFont.Lookup(attrId, pNewFont) )
		{
			WCHAR fontName[64] = {0};
			wcscpy_s(fontName, 63, theApp.GetDefaultFontName());

			pNewFont = new CFont;
			if( pNewFont )
			{
				int fontSize = DEFAULT_FONT_SIZE * dpi / 300;

				// 固定フォント
				int cHeight = (int)Round(fontSize * scale, 0.1);
				//cHeight = cHeight * 72 / m_nScreenDpi;

				cHeight = (int)floor((double)(cHeight * 72 / m_nScreenDpi));

				pNewFont->CreateFont(
					cHeight,
					0,
					0,
					0,
					FW_BOLD,
					FALSE,
					FALSE,
					0,
					DEFAULT_CHARSET, // ANSI_CHARSET,
					OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS,
					CLEARTYPE_QUALITY, // DEFAULT_QUALITY,
					DEFAULT_PITCH | FF_DONTCARE,
					fontName);

				{
					//LPSTR pfName = _ConvW2A(fontName);
					//TRACE(_T("[FONTNAME-DEFAULT] %S, cHeight:%d\n"), pfName, cHeight);
					//_del(pfName);
				}
				m_pmapFont.SetAt(attrId, pNewFont);	
			}
		}
	}
	else
	{
		WEB_ATTR_RECDATA* lpDocAttr = pAttTbl->get(attrId);
		if( lpDocAttr )
		{
			nParent++;

			LinkedList<LPFONTLINK>* pFontL = NULL;
			LONG key = MAKELONG(attrId, nParent);
			if( !m_pmapFont.Lookup(key, pNewFont) )
			{
				LPFONTLINK pFontLink = (LPFONTLINK)malloc(sizeof(FONTLINK));
				if( pFontLink )
				{
					pFontLink->lKey = key;
				}

				WCHAR fontName[64] = {0};
				if( pFntLst &&
					pFntLst->size() > 0 )
				{
					WEB_FONT_DATA* pFont = (WEB_FONT_DATA*)pFntLst->at(lpDocAttr->fontId);
					if( pFont != NULL )
					{
						//LPWSTR lpMapFontName = theApp.GetFontMap( pFont->name);
						LPWSTR lpMapFontName = GetFontMap( pFont->name);

						if( lpMapFontName != NULL && wcslen(lpMapFontName) > 0 )
							wcscpy_s(fontName, 63, lpMapFontName);
						else
							wcscpy_s(fontName, 63, pFont->name);

						// Add To Font List
						 pFontL = m_pTblFontList->get(pFont->name);
						 if( pFontL == NULL )
						 {
							 pFontL = new LinkedList<LPFONTLINK>(true, _deleteFontLink);
							 if( pFontL )
							 {
								 m_pTblFontList->add(pFont->name, pFontL);
							 }
						 }
					}
				}
				else
				{
					wcscpy_s(fontName, 63, theApp.GetDefaultFontName());
				}
				
				if( pFontL == NULL )
				{
					_free(pFontLink);
				}

				pNewFont = new CFont;
				if( pNewFont )
				{
					int nWeight = FW_NORMAL;
					BOOL bItalic = FALSE;
					// イタリック
					if( lpDocAttr->fontStyle & FONT_STYLE_ITALIC )
					{
						bItalic = TRUE;
					}
					// ボルト
					if( lpDocAttr->fontStyle & FONT_STYLE_BOLD )
					{
						nWeight = FW_BOLD;
					}
					// フォント回転角度（0, 90, 180, 270）
					int angle = (360 - lpDocAttr->fontRotate) * 10;
					if( lpDocAttr->fontRotate == 0 )
						angle = 0;

					//int cHeight = (int)Round(lpDocAttr->data.fontHeight * scale, 0.1);
					//int cWidth = (int)Round(lpDocAttr->data.fontWidth * scale, 0.1);
					int cHeight = lpDocAttr->fontHeight * 20 / m_tppx;
					int cWidth = lpDocAttr->fontWidth * 10 / m_tppy;

					cHeight = cHeight * 72 / m_nScreenDpi;
					cWidth = cWidth * 72 / m_nScreenDpi;

					//cHeight = (int)Round(cHeight * scale, 0.1);
					//cWidth = (int)Round(cWidth * scale, 0.1);
					cHeight = (int)floor((double)(cHeight * scale ));
					cWidth = (int)floor((double)(cWidth * scale ));

					pNewFont->CreateFont(
						cHeight,
						cWidth,
						0,
						angle,
						nWeight,
						bItalic,
						FALSE,
						0,
						DEFAULT_CHARSET, // ANSI_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						CLEARTYPE_QUALITY, // DEFAULT_QUALITY,
						DEFAULT_PITCH | FF_DONTCARE,
						fontName);

					{
						//LPSTR pfName = _ConvW2A(fontName);
						//TRACE(_T("[FONTNAME]parent:%d, attrId:%d, %S, cHeight:%d, cWidth:%d\n"), nParent, lpDocAttr->attrId, pfName, cHeight, cWidth);
						//_del(pfName);
					}

					m_pmapFont.SetAt(key, pNewFont);

					// Add To List
					if( pFontLink )
					{
						_tcscpy_s(pFontLink->fontName, 63, fontName);

						pFontLink->pFont = pNewFont;

						if( pFontL )
						{
							pFontL->add(pFontLink);
						}
					}
					pFontL = NULL;
				}
			}
		}
	}

	return pNewFont;
}

BOOL CW01DrawDoc::SaveImageFile()
{
	BOOL bReturn = FALSE;
	CImageSaveDlg dlg(m_pmapImage);

	CString szName = GetPathName();
	dlg.SetW01FileName(szName.GetBuffer());

	if( dlg.DoModal() )
	{
		bReturn = TRUE;
	}
	return bReturn;
}

UINT CW01DrawDoc::GetImageCount()
{
	UINT nCount = 0;
	if(	m_pmapImage != NULL )
		nCount = m_pmapImage->size();

	return nCount;
}

BOOL CW01DrawDoc::ShowDocInfo()
{
	CInfoDialog dlg;

	BOOL bReturn = FALSE;
	LinkedList<CiEveryCall<CW01DrawDoc>*>*	lstCall = new LinkedList<CiEveryCall<CW01DrawDoc>*>(true, _deleteptr);

	dlg.SetTitle(IDS_DOC_INFO);
	dlg.SetValue(IDS_TXT_FNAME, GetPathName());

	if( m_pW01Doc )
	{
		WEB_HEADER2 head = {0};
		m_pW01Doc->GetHeader(&head);
		TCHAR szMsg[256] = {0};

		// version
		_stprintf_s(szMsg, 255, _T("%d"), head.head1.version);
		dlg.SetValue(IDS_TXT_VERSION, szMsg);
		
		// createDate
		_stprintf_s(szMsg, 255, _T("%s"), head.createDate);
		dlg.SetValue(IDS_TXT_CREATEDATE, szMsg);

		// updateDate
		_stprintf_s(szMsg, 255, _T("%s"), head.updateDate);
		dlg.SetValue(IDS_TXT_UPDATEDATE, szMsg);

		// recordLen
		_stprintf_s(szMsg, 255, _T("%d"), head.recordLen);
		dlg.SetValue(IDS_TXT_RECORDLEN, szMsg);

		// totalPage
		_stprintf_s(szMsg, 255, _T("%d"), head.totalPage);
		dlg.SetValue(IDS_TXT_TOTALPAGE, szMsg);

		// registRecNo
		_stprintf_s(szMsg, 255, _T("%d"), head.registRecNo);
		dlg.SetValue(IDS_TXT_NO_REC, szMsg);

		// userRecNo
		CiEveryCall<CW01DrawDoc>* pUsrCall = new CiEveryCall<CW01DrawDoc>();
		if( pUsrCall )
		{
			lstCall->add(pUsrCall);
			pUsrCall->SetCallback(this, &CW01DrawDoc::ShowUserInfo);
		}
		_stprintf_s(szMsg, 255, _T("%d"), head.userRecNo );
		dlg.SetValue(IDS_TXT_NO_USERINFO, szMsg, PROP_TYPE_BUTTON, pUsrCall);

		// fontRecNo
		CiEveryCall<CW01DrawDoc>* pFntCall = new CiEveryCall<CW01DrawDoc>();
		if( pFntCall )
		{
			lstCall->add(pFntCall);
			pFntCall->SetCallback(this, &CW01DrawDoc::ShowFontInfo);
		}
		_stprintf_s(szMsg, 255, _T("%d"), head.fontRecNo);
		dlg.SetValue(IDS_TXT_NO_FONT, szMsg, PROP_TYPE_BUTTON, pFntCall);

		// ovlyRecNo
		_stprintf_s(szMsg, 255, _T("%d"), head.ovlyRecNo);
		dlg.SetValue(IDS_TXT_NO_OVLY, szMsg);

		// pageRecNo
		_stprintf_s(szMsg, 255, _T("%d"), head.pageRecNo);
		dlg.SetValue(IDS_TXT_NO_PAGE, szMsg);

		// Image
		_stprintf_s(szMsg, 255, _T("%d"), head.imageRecNo);
		dlg.SetValue(IDS_TXT_NO_IMAGE, szMsg);

		// textRecNo
		_stprintf_s(szMsg, 255, _T("%d"), head.positionRecNo);
		dlg.SetValue(IDS_TXT_NO_TEXTTBL, szMsg);

		// groupRecNo
		_stprintf_s(szMsg, 255, _T("%d"), head.groupRecNo);
		dlg.SetValue(IDS_TXT_NO_TEXTPOS, szMsg);

		// commentCnt
		_stprintf_s(szMsg, 255, _T("%d"), head.commentCnt);
		dlg.SetValue(IDS_TXT_NO_COMMENT, szMsg);

		// stampCnt
		_stprintf_s(szMsg, 255, _T("%d"), head.stampCnt);
		dlg.SetValue(IDS_TXT_NO_STAMP, szMsg);

		// tagCnt
		_stprintf_s(szMsg, 255, _T("%d"), head.tagCnt);
		dlg.SetValue(IDS_TXT_NO_TAG, szMsg);

		// markerCnt
		_stprintf_s(szMsg, 255, _T("%d"), head.markerCnt);
		dlg.SetValue(IDS_TXT_NO_MARKER, szMsg);

	}

	if( dlg.DoModal() )
	{
		bReturn = TRUE;
	}

	_delete(lstCall);

	return bReturn;
}

int CW01DrawDoc::ShowFontInfo(LPVOID pData)
{
	CInfoDialog dlg;
	
	dlg.SetTitle(IDS_FONT_INFO);

	if( m_plstFont &&
		m_plstFont->first() )
	{
		TCHAR szMsg[64] = {0};
		do
		{
			WEB_FONT_DATA* pFont = m_plstFont->get();
			if( pFont )
			{
				dlg.SetValue(IDS_TXT_FONTNAME, pFont->name, PROP_TYPE_LIST);
			}
		} while(m_plstFont->next());
	}

	dlg.DoModal();

	return 0;
}

int CW01DrawDoc::ShowUserInfo(LPVOID pData)
{
	CInfoDialog dlg;
	
	dlg.SetTitle(IDS_USER_INFO);

	WCHAR szData[256] = {0};
	if( m_plstUser &&
		m_plstUser->first() )
	{
		do
		{
			WEB_USER_INFO_DATA* pUser = m_plstUser->get();
			if( pUser )
			{
				//if( _wcsicmp(pUser->pKey, L"FORMTYPE") == 0 )
				//{
				//	// formtype
				//	_stprintf_s(szData, 255, _T("%X"), (ULONG)(*pUser->pData));
				//	dlg.SetValue(IDS_USER_INFO, pUser->pKey, szData, PROP_TYPE_LIST2);
				//}
				//else
				{
					dlg.SetValue(IDS_USER_INFO, pUser->pKey, pUser->pData, PROP_TYPE_LIST2);
				}

			}
		} while( m_plstUser->next());
	}

	dlg.DoModal();

	return 0;
}

// イメージデータ取得
LPIMAGEDATA CW01DrawDoc::GetImageData(USHORT id)
{
	if( !m_pmapImage ||
		!m_pmapImage->size() )
		return NULL;

	return m_pmapImage->get(id);
}

LinkedList<LPDOCPAGE>* CW01DrawDoc::ReadAllPageInfo(int pageNo)
{
	if( !m_pW01Doc )
		return NULL;

	return m_pW01Doc->ReadAllPageInfo(pageNo);
}

BOOL CW01DrawDoc::ReadPageData(int pageNo)
{
	UINT cacheSize = theApp.GetCachePages();

	m_posCurrent = NULL;

	DOCPAGE docRemovePage = {0};
	LPDOCPAGE pDocPage = m_pW01Doc->ReadPageInfo(pageNo, cacheSize, &docRemovePage);
	if( pDocPage != NULL )
	{
		m_rcSize.cx = pDocPage->pageDef.def.width;
		m_rcSize.cy = pDocPage->pageDef.def.height;

		LPPAGECONTENT lpContent = m_pmapContent->get(pageNo);
		if( lpContent == NULL )
		{
			// Clear Old One
			if( cacheSize > 0 )
			{
				if( m_pmapContent->size() > cacheSize )
				{
					INT pageNoOld = m_plstPageNo.GetAt(0);
					if( m_pmapContent->remove(pageNoOld) )
					{
						m_plstPageNo.RemoveAt(0);
					}
				}
			}

			lpContent = (LPPAGECONTENT)malloc(sizeof(PAGECONTENT));
			if( !lpContent )
			{
				AfxMessageBox(IDS_MSG_NOMEMORY, MB_OK | MB_ICONERROR);

				return FALSE;
			}
			lpContent->nPageNo = pageNo;
			lpContent->bLoaded = FALSE;
			// 109
			lpContent->RecNo = pDocPage->pageDef.ulRecNo;
			lpContent->width = pDocPage->pageDef.def.width;
			lpContent->height = pDocPage->pageDef.def.height;
			lpContent->rotate = pDocPage->pageDef.def.rotate;

			// レコード情報
			lpContent->plstRec = new LinkedList<WEB_PAGE_RECDATA*>(true, _freeptr);
			// 文字情報
			lpContent->plstTextPos = new LinkedList<WEB_POS_RECDATA*>(true, _freeptr);
			// オーバーレイ情報
			lpContent->plstOvly = new LinkedList<LPOVLYLINK>(true, _freeptr);
			// イメージ情報
			lpContent->plstImage = new LinkedList<LPIMAGELINK>(true, _freeptr);
			// 線情報
			lpContent->plstLine = new LinkedList<WEB_LINE_DATA*>(true, _freeptr);
			// 数値情報
			lpContent->pmapNumerical = new CHashTblNumsObject();
			// Error
			if( lpContent->plstRec == NULL ||
				lpContent->plstTextPos == NULL ||
				lpContent->plstOvly == NULL ||
				lpContent->plstImage == NULL ||
				lpContent->plstLine == NULL ||
				lpContent->pmapNumerical == NULL ||
				lpContent->pmapNumerical->create(genHashKey, true) == NULL )
			{
				_freePageContentP()(0, lpContent);

				AfxMessageBox(IDS_MSG_NOMEMORY, MB_OK | MB_ICONERROR);
				return FALSE;
			}
#ifdef _DEBUG
			//int nMaxText = theApp.GetMaxTextCount();
			//if( nMaxText > 0 )
			//	lpContent->plstTextPos->set_limit(nMaxText);

			//int nMaxImage = theApp.GetMaxImageCount();
			//if( nMaxImage > 0 )
			//	lpContent->plstImage->set_limit(nMaxImage);
#endif
			// SetAt
			m_pmapContent->insert(pageNo, lpContent);
			m_plstPageNo.Add(pageNo);
		}

		// 読込しない場合
		if( !lpContent->bLoaded )
		{
			if( !m_pW01Doc->ReadPageContent(pDocPage, lpContent) )
			{
				lpContent->bLoaded = FALSE;

				return FALSE;
			}
			// 数値情報
			if( m_pmapNumericalLink != NULL &&
				m_pmapNumericalLink->size() > 0 )
			{
				lpContent->pmapNumerical->removeall();

				LinkedList<WEB_POS_RECDATA*>* pTpList = lpContent->plstTextPos;
				if( pTpList &&
					pTpList->size() > 0 )
				{
					LinkedList<LPNUMSOBJ>* pLinkNum = m_pmapNumericalLink->get(pDocPage->txtRecNo);
					if( pLinkNum != NULL &&
						pLinkNum->first() )
					{
						do
						{
							
							LPNUMSOBJ pNumsObj = pLinkNum->get();
							if( pNumsObj )
							{
								LinkedList<LPNUMSOBJ>* pLnk = lpContent->pmapNumerical->get(pNumsObj->value);
								if( pLnk == NULL )
								{
									pLnk = new LinkedList<LPNUMSOBJ>(false);
									if( pLnk == NULL )
									{
										//
										AfxMessageBox(IDS_MSG_NOMEMORY, MB_OK | MB_ICONERROR);

										return FALSE;
									}

									lpContent->pmapNumerical->insert(pNumsObj->value, pLnk);
								}

								for(int x = 0; x < pNumsObj->charCnt; x++)
								{
									pNumsObj->pNumList->add(pTpList->at(pNumsObj->idxNo + x));
								}

								pLnk->add(pNumsObj);
							}
						} while( pLinkNum->next() );
					}
				}
			}
		}
		
		// 検索用
		SplitAreaTable(lpContent);

		// Current Page
		m_nPageNo = pageNo;

		// Set Page
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame )
		{
			TCHAR szMsg[32];
			_stprintf_s(szMsg, 31, _T("%d / %d"), GetPage() + 1, GetTotalPage());
			pFrame->GetPageStatic()->SetWindowText(szMsg);

			pFrame->SetPaper(m_rcSize.cx, m_rcSize.cy);
		}

		return TRUE;
	}

	return FALSE;
}

void CW01DrawDoc::UpdateUI()
{
	// Set Page
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		TCHAR szMsg[32];
		_stprintf_s(szMsg, 31, _T("%d / %d"), GetPage() + 1, GetTotalPage());
		pFrame->GetPageStatic()->SetWindowText(szMsg);

		pFrame->SetPaper(m_rcSize.cx, m_rcSize.cy);
	}
}

int CW01DrawDoc::GetTotalPage()
{
	if( !m_pW01Doc )
		return 0;

	return m_pW01Doc->GetTotalPage();
}

BOOL CW01DrawDoc::MoveToPage(int nPageNo)
{
	if( !m_pW01Doc )
		return FALSE;

	if( nPageNo < 0 || nPageNo > GetTotalPage() )
		return FALSE;

	if( !ReadPageData(nPageNo) )
		return FALSE;

	return TRUE;
}

BOOL CW01DrawDoc::MoveFirstPage()
{
	int pageNo = 0;

	if( !m_pW01Doc )
		return FALSE;

	ULONG ulMaxPage = m_pW01Doc->GetTotalPage();
	if( pageNo > (int)ulMaxPage )
		return FALSE;

	if( !ReadPageData(pageNo) )
		return FALSE;

	return TRUE;
}

BOOL CW01DrawDoc::MoveLastPage()
{
	int pageNo = 0;

	if( !m_pW01Doc )
		return FALSE;

	ULONG ulMaxPage = GetTotalPage();
	pageNo = ulMaxPage - 1;

	if( !ReadPageData(pageNo) )
		return FALSE;

	return TRUE;
}

BOOL CW01DrawDoc::MoveNextPage()
{
	if( !m_pW01Doc )
		return FALSE;

	int pageNo = GetPage();
	pageNo ++;

	if( pageNo > GetTotalPage() )
		return FALSE;

	if( !ReadPageData(pageNo ) )
		return FALSE;

	return TRUE;
}


BOOL CW01DrawDoc::MovePrevPage()
{
	if( !m_pW01Doc )
		return FALSE;

	int pageNo = GetPage();
	pageNo --;

	if( pageNo < 0 )
		return FALSE;

	if( !ReadPageData(pageNo ) )
		return FALSE;

	return TRUE;
}

LPDOCOVLINF CW01DrawDoc::GetOverlay(int ovlNo)
{
	if( m_pmapOvlInf == NULL ||
		!m_pmapOvlInf->first() )
		return NULL;

	return m_pmapOvlInf->get(ovlNo);
}

void CW01DrawDoc::DrawOverlay(CDC* pDC, USHORT id, SHORT x, SHORT y, SHORT height, SHORT width, LONG scalex, LONG scaley)
{
	LPDOCOVLINF lpDocOvl = GetOverlay(id);
	if( !lpDocOvl )
		return;
	
	float sx = (float)(scalex / ovlyScale);
	float sy = (float)(scaley / ovlyScale);
	if (sx == 0) {
		sx = 1.0;
	}
	if (sy == 0) {
		sy = 1.0;
	}

	COLORREF oldColorText = 0;
	COLORREF orgColorText = pDC->GetTextColor();
	COLORREF oldBkColor = 0;
	int nFindBkMode = 0;

	// 文字情報（位置）登録レコードNO
	ULONG ulTextRectNo = lpDocOvl->txtRecNo;

	int lineWidth = 3;
	int dpi = m_pW01Doc->GetDPI();
	if( dpi == 0 )
		dpi = 240;
	lineWidth = lineWidth * dpi / m_nScreenDpi;

	// Draw Line
	if( IsShowLine() &&
		lpDocOvl->plstLine &&
		lpDocOvl->plstLine->first() )
	{
		do
		{
			WEB_LINE_DATA* pLine = lpDocOvl->plstLine->get();
			if( pLine )
			{
				// 座標調整
				
				SHORT lsx = (SHORT)Round(pLine->sx * sx, 0.1) + x;
				SHORT lsy = (SHORT)Round(pLine->sy * sy, 0.1) + y;
				SHORT lex = (SHORT)Round(pLine->ex * sx, 0.1) + x;
				SHORT ley = (SHORT)Round(pLine->ey * sy, 0.1) + y;

				float scale = (sx + sy) / 2.0f;
				SHORT linewidth = (SHORT)Round(pLine->width * scale, 0.1);
				if( linewidth < 0 )
					linewidth = 1;
				DrawLine(pDC, lsx, lsy, lex, ley, linewidth, pLine->color);
			}

		} while( lpDocOvl->plstLine->next());

	}

	// Draw Image
	if( IsShowImage() &&
		m_pmapImage &&
		lpDocOvl->plstImage &&
		lpDocOvl->plstImage->first() )
	{
		INT dwHlId = GetHighLightID();

		INT nHlId = -1;
		if( dwHlId > ID_PROP_IMGINOVL )
		{
			nHlId = dwHlId - ID_PROP_IMGINOVL - 1;
		}

		int nImgIdx = 0;
		LPIMAGELINK pImgLnk = NULL;
		do
		{
			pImgLnk = lpDocOvl->plstImage->get();
			if( pImgLnk && pImgLnk->bShow )
			{
				LPIMAGEDATA lpImgLnk = GetImageData(pImgLnk->link.imgNo);
				if( lpImgLnk != NULL )
				{
					SHORT ix = (SHORT)(pImgLnk->link.x * sx) + x;
					SHORT iy = (SHORT)(pImgLnk->link.y * sy) + y;
					USHORT iw = (USHORT)(pImgLnk->link.width * sx);
					USHORT ih = (USHORT)(pImgLnk->link.height * sy);

					DrawImage(pDC, ix, iy, iw, ih, lpImgLnk->pImage, lpImgLnk->length);

					if( IsShowHighLight() )
					{
						if( nHlId >= 0 &&
							nHlId == nImgIdx )
						{
							if( !pDC->IsPrinting() )
							{
								COLORREF colorBoder = RGB(0, 255, 0);

								CPen imgPen(PS_SOLID, lineWidth, colorBoder);
								CPen* pImgPen = pDC->SelectObject(&imgPen);

								LONG olvx0 = ix;
								LONG olvy0 = iy;
								LONG olvx1 = ix + iw;
								LONG olvy1 = iy + ih;

								// Draw Page Rectange
								pDC->MoveTo(olvx0, olvy0);
								pDC->LineTo(olvx0, olvy1);
								pDC->LineTo(olvx1, olvy1);
								pDC->LineTo(olvx1, olvy0);
								pDC->LineTo(olvx0, olvy0);
								//pDC->Draw3dRect(&ovlyRect, RGB(255, 0, 0), RGB(255, 0, 0));

								if( pImgPen )
									pDC->SelectObject(pImgPen);
							}
						}
					} // if( IsShowHighLight() )

				}
			}
			nImgIdx++;
		} while( lpDocOvl->plstImage->next() );
	}

	// Draw Text
	if( IsShowText() &&
		lpDocOvl->plstTextPos &&
		lpDocOvl->plstTextPos->first() )
	{
		oldBkColor = pDC->GetBkColor();
		nFindBkMode = pDC->GetBkMode();

		CFont* pRstFont = NULL;	// Restore 
		CFont* pOldFont = NULL;
		WEB_POS_RECDATA* pTxtPos = NULL;
		do
		{
			pTxtPos = lpDocOvl->plstTextPos->get();

			CFont* pNewFont = LoadFont(pTxtPos->attrId, sy, id);
			if( pNewFont == NULL )
			{
				if( !lpDocOvl->plstTextPos->next() )
					break;
				continue;
			}
			
			LPTEXTADDR pTxt = NULL;
			if( m_plstTextOvl )
			{
				pTxt = m_plstTextOvl->at(pTxtPos->textNum);
			}

			LPWSTR lpStr = NULL;
			WCHAR wChrs[5] = {0};
			if( pTxt )
			{
				wcscpy_s(wChrs, 4, pTxt->text);
				lpStr = (LPWSTR)wChrs;
			}
			if( lpStr == NULL )
			{
				if( !lpDocOvl->plstTextPos->next() )
					break;

				continue;
			}

			// 位置調整
			int X = (int)(pTxtPos->textX * sx) + x;
			int Y = (int)(pTxtPos->textY * sy) + y;

			COLORREF colorText = RGB(0, 0, 0);
			WEB_ATTR_RECDATA* lpDocAttr = m_pAttTbl->get(pTxtPos->attrId);
			if( lpDocAttr )
			{
//				colorText = lpDocAttr->data.foregroundColor;
				BYTE r = RGBA_R(lpDocAttr->foregroundColor);
				BYTE g = RGBA_G(lpDocAttr->foregroundColor);
				BYTE b = RGBA_B(lpDocAttr->foregroundColor);
				colorText = RGB(r, g, b);
				//colorText = (DWORD)(lpDocAttr->data.foregroundColor >> 8);
			}

			pDC->SetBkColor(oldBkColor);
			pDC->SetBkMode(nFindBkMode);

			if( oldColorText != colorText )
			{
				pDC->SetTextColor(colorText);

				oldColorText = colorText;
			}

			BOOL bAuto = FALSE;
			COLORREF clrOvlText = GetOverlayTextColor(bAuto);
			if( !bAuto )
			{
				if( oldColorText != clrOvlText )
				{
					pDC->SetTextColor(clrOvlText);

					oldColorText = clrOvlText;
				}
			}
			else
			{
				if( oldColorText != colorText )
				{
					pDC->SetTextColor(colorText);

					oldColorText = colorText;
				}
			}
			BOOL bFind = FALSE;

			if( pOldFont != pNewFont )
			{
				if( pRstFont )
					pDC->SelectObject(pRstFont);

				pRstFont = pDC->SelectObject(pNewFont);
				pOldFont = pNewFont;
			}

			// 座標強調表示
			if( IsShowTextXY() )
			{
				int tx0 = X;
				int ty0 = Y;
				int tx1 = tx0 + 10;
				int ty1 = ty0;
				int tx2 = tx0;
				int ty2 = ty0 - 10;
				if( lpDocAttr )
				{
					tx1 = tx0 + (int)Round(lpDocAttr->fontWidth * sx, 0.1);
					ty2 = ty0 -  (int)Round(lpDocAttr->fontHeight * sy, 0.1);
				}

				CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
				CPen* pOld = pDC->SelectObject(&pen);
		
				POINT pts = {tx0, ty0};
				POINT pte1 = {tx1, ty1};
				POINT pte2 = {tx2, ty2};
				pDC->MoveTo(pts);
				pDC->LineTo(pte1);
				pDC->MoveTo(pts);
				pDC->LineTo(pte2);

				if( pOld )
					pDC->SelectObject(pOld);
			}

			// Draw Text
			pDC->TextOutW(X, Y, lpStr);

		} while( lpDocOvl->plstTextPos->next());

		pDC->SetBkColor(oldBkColor);
		pDC->SetBkMode(nFindBkMode);

		if( pRstFont )
			pDC->SelectObject(pRstFont);
	}

	// Restore Color
	pDC->SetTextColor(orgColorText);

}

void CW01DrawDoc::DrawLine(CDC* pDC, SHORT x0, SHORT y0, SHORT x1, SHORT y1, USHORT width, ULONG color)
{
	if( pDC->IsPrinting() )
	{
		CPen pen(PS_SOLID, width, color);
		CPen* pOld = pDC->SelectObject(&pen);
		
		POINT pts = {x0, y0};
		POINT pte = {x1, y1};
		pDC->MoveTo(pts);
		pDC->LineTo(pte);

		if( pOld )
			pDC->SelectObject(pOld);
	}
	else
	{
		Gdiplus::Graphics g(pDC->GetSafeHdc());

		g.SetSmoothingMode(SmoothingModeAntiAlias);

		Gdiplus::Color clr;
		clr.SetFromCOLORREF(color);

		Gdiplus::Pen pen(clr, width);

		g.DrawLine(&pen, x0, y0, x1, y1);
	}
}

void CW01DrawDoc::DrawImage(CDC* pDC, SHORT x, SHORT y, USHORT width, USHORT height, LPVOID pImage, ULONG size)
{
	ULONG nByteOfWritten = 0;
	ByteStream bs;
	bs.Write(pImage, size, &nByteOfWritten);

	Gdiplus::Bitmap bmp(&bs);

	Gdiplus::Rect rect;
	rect.X = x;
	rect.Y = y;
	rect.Width = width;
	rect.Height = height;

	Gdiplus::Graphics g(pDC->GetSafeHdc());
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetCompositingQuality(CompositingQualityHighQuality);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			
	g.DrawImage(&bmp, rect);
}

void CW01DrawDoc::DrawImage(CDC* pDC, LPSTR lpszFileName, SHORT x, SHORT y, USHORT width, USHORT height)
{
	HBITMAP hBmp = NULL;
	Gdiplus::Status status;
	LPCWSTR lpwFileName = _AA2W(lpszFileName);
	Gdiplus::Bitmap* pImg = Gdiplus::Bitmap::FromFile(lpwFileName, TRUE);
	if( pImg )
	{
		Gdiplus::Color clr;
		clr = Gdiplus::Color::Transparent;

		if( pDC->IsPrinting() )
		{
			HBITMAP hBitmap;
			status = pImg->GetHBITMAP(clr, &hBitmap);
			if( status == Gdiplus::Ok )
			{
				BITMAP bm;
				CBitmap* pBmp = CBitmap::FromHandle(hBitmap);
				pBmp->GetBitmap(&bm);

				CDC memDC;
				memDC.CreateCompatibleDC(pDC);
				CBitmap* pOld = memDC.SelectObject(pBmp);

				pDC->StretchBlt(x, y, width, height, &memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

				if( pOld )
				{
					memDC.SelectObject(pOld);
				}
				memDC.DeleteDC();
			}
		}
		else
		{

			Gdiplus::Rect rect;
			rect.X = x;
			rect.Y = y;
			rect.Width = width;
			rect.Height = height;

			Gdiplus::Graphics g(pDC->GetSafeHdc());
			g.SetSmoothingMode(SmoothingModeAntiAlias);
			g.SetCompositingQuality(CompositingQualityHighQuality);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
			
			g.DrawImage(pImg, rect);
		}
	}
	_del(lpwFileName);
}

void CW01DrawDoc::DrawWsi(CDC* pDC)
{
	if (!m_pW01Doc)
		return;

	if (m_pWsiFile == NULL)
		return;

	CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
	CPen* pOld = pDC->SelectObject(&pen);

	LPITEMNODE pNode = m_pWsiFile->GetGlobalNode();
	if (pNode != NULL) {
		if (pNode->pStrs != NULL) {
			CPen pen0(PS_SOLID, 1, RGB(255, 0, 0));
			CPen* pOld0 = pDC->SelectObject(&pen0);

			POINT pt;
			POSITION pos = pNode->pStrs->GetHeadPosition();
			while (pos != NULL)
			{
				LPITEMSTR pStr = pNode->pStrs->GetNext(pos);
				if (pStr != NULL) {
					// Draw Border
					// left - top
					pt.x = pStr->sx;
					pt.y = pStr->sy;
					pDC->MoveTo(pt);
					// right - top
					pt.x = pStr->ex;
					pt.y = pStr->sy;
					pDC->LineTo(pt);
					// right - bottom
					pt.x = pStr->ex;
					pt.y = pStr->ey;
					pDC->LineTo(pt);
					// left - bottom
					pt.x = pStr->sx;
					pt.y = pStr->ey;
					pDC->LineTo(pt);
					// left - top
					pt.x = pStr->sx;
					pt.y = pStr->sy;
					pDC->LineTo(pt);

					// Draw Text
					CFont* pOldDrawFont = NULL;
					CFont* pDrawFont = LoadFont(DEFAULT_FONT);
					if (pDrawFont)
					{
						POINT pt0;
						pt0.x = pStr->sx + 3;
						pt0.y = pStr->sy + DEFAULT_FONT_SIZE + 3;

						COLORREF colorOvlRestore = 0;
						pOldDrawFont = pDC->SelectObject(pDrawFont);

						WCHAR szWsiText[64] = { 0 };
						if (pStr->type == WPIKEY_STRKEY) {
							colorOvlRestore = pDC->SetTextColor(RGB(0, 0, 255));
							swprintf_s(szWsiText, 64, L"S%02d - KEY", pStr->no);
						}
						else {
							colorOvlRestore = pDC->SetTextColor(RGB(255, 0, 0));
							swprintf_s(szWsiText, 64, L"S%02d", pStr->no);
						}

						pDC->TextOutW(pt.x, pt.y, szWsiText);

						pDC->SelectObject(pOldDrawFont);
						pDC->SetTextColor(colorOvlRestore);
					}

				}
			}

			if (pOld0)
				pDC->SelectObject(pOld0);
		}
	}
	if (pOld)
		pDC->SelectObject(pOld);
}
void CW01DrawDoc::DrawCellDefine(CDC* pDC)
{
	if( !m_pW01Doc )
		return;

	CW01Cell* pCellFile = m_pW01Doc->GetCellFile();
	if( !pCellFile )
		return;

	LinkedList<W01_CELL*>* pCellLst = pCellFile->GetCellDefine();
	if( pCellLst &&
		pCellLst->first() )
	{
		
		CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
		CPen* pOld = pDC->SelectObject(&pen);

		SHORT dpi = m_pW01Doc->GetDPI();
		if( dpi == 0 )
			dpi = 240;

		do
		{
			W01_CELL* pCell = pCellLst->get();
			if( pCell )
			{
				// Draw Area
				POINT pt;

				// Draw Detail Line
				if( pCell->pList &&
					pCell->pList->first() )
				{
					CPen pen0(PS_DOT, 1, RGB(255, 0, 0));
					CPen* pOld0 = pDC->SelectObject(&pen0);
					do
					{
						W01_CELL_RANGE* pRange = pCell->pList->get();
						if( pRange )
						{
							// Draw Border
							// left - top
							pt.x = pRange->em1.x;
							pt.y = pRange->em1.y;
							pt = pCellFile->GetPoint(pt, dpi);
							pDC->MoveTo(pt);
							// right - top
							pt.x = pRange->em2.x;
							pt.y = pRange->em1.y;
							pt = pCellFile->GetPoint(pt, dpi);
							pDC->LineTo(pt);
							// right - bottom
							pt.x = pRange->em2.x;
							pt.y = pRange->em2.y;
							pt = pCellFile->GetPoint(pt, dpi);
							pDC->LineTo(pt);
							// left - bottom
							pt.x = pRange->em1.x;
							pt.y = pRange->em2.y;
							pt = pCellFile->GetPoint(pt, dpi);
							pDC->LineTo(pt);
							// left - top
							pt.x = pRange->em1.x;
							pt.y = pRange->em1.y;
							pt = pCellFile->GetPoint(pt, dpi);
							pDC->LineTo(pt);

							// Cell(x, y)
							CFont* pOldDrawFont = NULL;
							CFont* pDrawFont = LoadFont(DEFAULT_FONT);
							if( pDrawFont )
							{
								POINT pt0;
								pt0.x = pRange->em1.x + 3;
								pt0.y = pRange->em1.y + DEFAULT_FONT_SIZE + 3;

								pt = pCellFile->GetPoint(pt0, dpi);

								COLORREF colorOvlRestore = pDC->SetTextColor(RGB(255, 0, 0));

								pOldDrawFont = pDC->SelectObject(pDrawFont);

								WCHAR szCellId[64] = {0};
								//swprintf_s(szCellId, 63, L"Cell(%d, %d) Rect[(%d,%d)-(%d,%d)]", pRange->cellX, pRange->cellY, pRange->sm1.x, pRange->sm1.y, pRange->em2.x, pRange->em2.y);
								swprintf_s(szCellId, 63, L"Cell(%d, %d)", pRange->cellX, pRange->cellY);
							
								pDC->TextOutW(pt.x, pt.y, szCellId);

								pDC->SelectObject(pOldDrawFont);
								pDC->SetTextColor(colorOvlRestore);
							}

						}
					} while( pCell->pList->next() );

					if( pOld0 )
						pDC->SelectObject(pOld0);
				}
				else
				{
					// Cell(x, y)
					CFont* pOldDrawFont = NULL;
					CFont* pDrawFont = LoadFont(DEFAULT_FONT);
					if( pDrawFont )
					{
						POINT pt0;
						pt0.x = pCell->range.em1.x + 3;
						pt0.y = pCell->range.em1.y + DEFAULT_FONT_SIZE + 3;

						pt = pCellFile->GetPoint(pt0, dpi);

						COLORREF colorOvlRestore = pDC->SetTextColor(RGB(255, 0, 0));

						pOldDrawFont = pDC->SelectObject(pDrawFont);

						WCHAR szCellId[32] = {0};
						swprintf_s(szCellId, 32, L"Cell(%d, %d)", pCell->range.cellX, pCell->range.cellY);
							
						pDC->TextOutW(pt.x, pt.y, szCellId);

						pDC->SelectObject(pOldDrawFont);
						pDC->SetTextColor(colorOvlRestore);
					}
				}

				// Draw Area Border
				// left - top
				pt.x = pCell->range.sm1.x;
				pt.y = pCell->range.sm1.y;
				pt = pCellFile->GetPoint(pt, dpi);
				pDC->MoveTo(pt);
				// right - top
				pt.x = pCell->range.sm2.x;
				pt.y = pCell->range.sm1.y;
				pt = pCellFile->GetPoint(pt, dpi);
				pDC->LineTo(pt);
				// right - bottom
				pt.x = pCell->range.sm2.x;
				pt.y = pCell->range.sm2.y;
				pt = pCellFile->GetPoint(pt, dpi);
				pDC->LineTo(pt);
				// left - bottom
				pt.x = pCell->range.sm1.x;
				pt.y = pCell->range.sm2.y;
				pt = pCellFile->GetPoint(pt, dpi);
				pDC->LineTo(pt);
				// left - top
				pt.x = pCell->range.sm1.x;
				pt.y = pCell->range.sm1.y;
				pt = pCellFile->GetPoint(pt, dpi);
				pDC->LineTo(pt);

			}

		} while( pCellLst->next() );

		if( pOld )
			pDC->SelectObject(pOld);
	}
}

COLORREF CW01DrawDoc::GetOverlayTextColor(BOOL& bAuto)
{
	bAuto = FALSE;
	COLORREF color = RGB(0, 0, 0);
	switch(theApp.GetOverlayTextColorIndex())
	{
	case ID_COLOR_RED:
		color = RGB(255, 0, 0);
		break;
	case ID_COLOR_GREEN:
		color = RGB(0, 255, 0);
		break;
	case ID_COLOR_BLUE:
		color = RGB(0, 0, 255);
		break;
	case ID_COLOR_DARKRED:
		color = RGB(139, 0, 0);
		break;
	case ID_COLOR_DARKGREEN:
		color = RGB(0, 100, 0);
		break;
	case ID_COLOR_DARKBLUE:
		color = RGB(0, 0, 139);
		break;
	case ID_COLOR_MAGENTA:
		color = RGB(139, 0, 139);
		break;
	case ID_COLOR_DARKYELLOW:
		color = RGB(204, 204, 0);
		break;
	case ID_COLOR_CYAN:
		color = RGB(0, 139, 139);
		break;
	case ID_COLOR_AUTO:
		color = RGB(0, 0, 0);
		bAuto = TRUE;
		break;
	default:
		color = RGB(0, 0, 0);
		break;
	}

	return color;
}


// GET - 帳票データ
LPPAGECONTENT CW01DrawDoc::GetPageContent(int pageNo)
{
	if( pageNo == -1 )
		pageNo = GetPage();
	if( m_pmapContent == NULL ||
		!m_pmapContent->size() )
		return NULL;

	return m_pmapContent->get(pageNo);
}

LinkedList<LPTEXTADDR>*	CW01DrawDoc::GetTextList(USHORT type)
{
	LinkedList<LPTEXTADDR>* pList = NULL;
	if( type == OBJ_TYPE_PAGE )
		pList = m_plstText;
	else if( type == OBJ_TYPE_OVERLAY )
		pList = m_plstTextOvl;
	else
		return NULL;

	return pList;
}

LPTEXTADDR CW01DrawDoc::GetText(hash_size index, USHORT type)
{
	LinkedList<LPTEXTADDR>* pList = NULL;
	if( type == OBJ_TYPE_PAGE )
		pList = m_plstText;
	else if( type == OBJ_TYPE_OVERLAY )
		pList = m_plstTextOvl;
	else
		return NULL;

	if( index < 0 || index >= pList->size() )
		return NULL;

	if( !pList ||
		!pList->first() )
		return NULL;

	return pList->at(index);
}

WEB_FONT_DATA* CW01DrawDoc::GetFont(hash_size index)
{
	if( !m_plstFont ||
		!m_plstFont->size() )
		return NULL;

	if( index < 0 || index >= m_plstFont->size() )
		return NULL;

	return m_plstFont->at(index);
}

WEB_ATTR_RECDATA* CW01DrawDoc::GetTextAttr(hash_size index)
{
	if( !m_pAttTbl ||
		!m_pAttTbl->size() )
		return NULL;

	return m_pAttTbl->get(index);
}

LPWSTR CW01DrawDoc::GetFontMap(LPCWSTR lpszName)
{
	if( !m_pTblFontList &&
		!m_pTblFontList->size() )
	{
		return theApp.GetFontMap(lpszName);
	}

	LinkedList<LPFONTLINK>* pLnkList = m_pTblFontList->get((LPWSTR)lpszName);
	if( pLnkList &&
		pLnkList->first() )
	{
		LPFONTLINK pLL = pLnkList->get();
		if( pLL )
		{
			return pLL->fontName;
		}
	}

	return theApp.GetFontMap(lpszName);
}

BOOL CW01DrawDoc::GetDocHeader(WEB_HEADER2* pHead)
{
	if( !pHead ||
		!m_pW01Doc )
		return FALSE;

	ZeroMemory(pHead, sizeof(WEB_HEADER2));

	m_pW01Doc->GetHeader(pHead);

	return TRUE;
}

void CW01DrawDoc::SetHighLightID(INT dwID)
{
	m_objHighLight = dwID;
}

CString CW01DrawDoc::GetDisplayFontName(LPCTSTR pszFontName)
{
	if( !m_pTblFontList ||
		!m_pTblFontList->size() )
		return theApp.GetFontMap(pszFontName);

	LinkedList<LPFONTLINK>* pLnkList = m_pTblFontList->get((LPWSTR)pszFontName);
	if( pLnkList &&
		pLnkList->first() )
	{
		LPFONTLINK pLL = pLnkList->get();
		if( pLL )
		{
			CFont* pFont = pLL->pFont;
			if( pFont )
			{
				LOGFONT lf = {0};
				pFont->GetLogFont(&lf);

				return CString(lf.lfFaceName);
			}
		}
	}

	return theApp.GetFontMap(pszFontName);
}

int CW01DrawDoc::RefreshFont(CHashTblStringToOutFont* pMap)
{
	if( !pMap )
		return 0;

	int nCount = 0;
	LPOUTFONT pof = NULL;
	LPTSTR pszName = NULL;
	PPOSITION pos = pMap->first();
	while( pos != NULL )
	{
		pMap->next(pos, pszName, pof);
		if( pof )
		{
			LinkedList<LPFONTLINK>* pLnkList = m_pTblFontList->get(pszName);
			if( pLnkList &&
				pLnkList->first() )
			{
				do
				{
					LPFONTLINK pLL = pLnkList->get();
					if( pLL )
					{
						CFont* pFont = pLL->pFont;
						if( pFont )
						{
							LOGFONT lf = {0};
							pFont->GetLogFont(&lf);
							_tcscpy_s(lf.lfFaceName, 32, pof->displayName);

							// FontName
							_tcscpy_s(pLL->fontName, 63, pof->displayName);
							
							pFont->DeleteObject();

							pFont->CreateFontIndirect(&lf);

							nCount++;
						}
					}
				} while( pLnkList->next() );
				
			}
		}
	}
	
	return nCount;
}

typedef struct _areaS
{
	int x_min;
	int x_max;
	int y_min;
	int y_max;
} AREA, *LPAREA;

//int AREA_Encode(CString& code, int x, int y, LPAREA range, int level = 0, int maxLevel = 3)
//{
//	int w = range->x_max - range->x_min;
//	int h = range->y_max - range->y_min;
//
//	int halfw = w / 2;
//	int halfh = h / 2;
//
//	int x_new = (range->x_max + range->x_min) / 2;
//	int y_new = (range->y_max + range->y_min) / 2;
//
//	//                       ((x0+x1)/2, y0)
//	//        (x0,y0) +---------------------+ (x1, y0)
//	//                | x:0      | x:0      |
//	//                | y:0      | y:1      |
//	//                |          |          |
//	//                |          |          |
//	// (x0,(y0+y1)/2) +----------+----------+ (x1, (y0+y1)/2)
//	//                | x:1      | x:1      |
//	//                | y:0      | y:1      |
//	//                |          |          |
//	//                |          |          |
//	//        (x0,y1) +----------+----------+ (x1,y1)
//	//                       ((x0+x1)/2, y1)
//	AREA r;
//	if( x > x_new )
//	{
//		code += _T("1");
//		r.x_min = x_new;
//		r.x_max = range->x_max;
//	}
//	else
//	{
//		code += _T("0");
//		r.x_min = range->x_min;
//		r.x_max = x_new;
//	}
//
//	if( y > y_new )
//	{
//		code += _T("1");
//		r.y_min = y_new;
//		r.y_max = range->y_max;
//	}
//	else
//	{
//		code += _T("0");
//		r.y_min = range->y_min;
//		r.y_max = y_new;
//	}
//
//	if( level >= maxLevel )
//		return 0;
//	
//	return AREA_Encode(code, x, y, &r, level + 1, maxLevel);
//}

int AREA_Encode(CString& code, int x, int y, LPAREA range, int level = 0, int maxLevel = 8)
{
	int w = range->x_max - range->x_min;
	int h = range->y_max - range->y_min;

	int halfw = w / 2;
	int halfh = h / 2;

	AREA r;
	if( x > halfw )
	{
		code += _T("1");
		r.x_min = range->x_min + halfw;
		r.x_max = range->x_max;
	}
	else
	{
		code += _T("0");
		r.x_min = range->x_min;
		r.x_max = range->x_max - halfw;
	}

	if( y > halfh )
	{
		code += _T("1");
		r.y_min = range->x_min + halfh;
		r.y_max = range->y_max;
	}
	else
	{
		code += _T("0");
		r.y_min = range->y_min;
		r.y_max = range->y_max - halfh;
	}

	if( level > maxLevel )
		return 0;
	
	return AREA_Encode(code, x, y, &r, level + 1, maxLevel);
}

void CW01DrawDoc::AddAreaCode(CHashTblTextPosS* pmapTextPos, CString& code, LPTEXTPOSITION pTxtPos)
{
	if (pmapTextPos == NULL || pTxtPos == NULL)
		return;

	LinkedList<LPTEXTPOSITION>* pGroup = pmapTextPos->get(code.GetBuffer());
	if( pGroup == NULL )
	{
		pGroup = new LinkedList<LPTEXTPOSITION>(true, _freeTextPosP);
		if( pGroup )
		{
			pmapTextPos->add(code.GetBuffer(), pGroup);
		}
	}
	if( pGroup )
	{
		pTxtPos->used++;

		pGroup->add(pTxtPos);
	}
}

LPTEXTPOSITION CW01DrawDoc::AddAreaCode(CHashTblTextPosS* pmapTextPos, CString& code, WEB_POS_RECDATA* pos)
{
	if (pmapTextPos == NULL)
		return NULL;

	LinkedList<LPTEXTPOSITION>* pGroup = pmapTextPos->get(code.GetBuffer());
	if( pGroup == NULL )
	{
		pGroup = new LinkedList<LPTEXTPOSITION>(true, _freeTextPosP);
		if( pGroup )
		{
			pmapTextPos->add(code.GetBuffer(), pGroup);
		}
	}

	if( pGroup )
	{
		LPTEXTPOSITION txtpos = (LPTEXTPOSITION)malloc(sizeof(TEXTPOSITION));
		if( txtpos )
		{
			txtpos->used = 0;
			txtpos->width = 0;
			txtpos->height = 0;
			txtpos->wmode = 0;
			txtpos->text[0] = L'\0';
			txtpos->pos = pos;
			txtpos->flag = 0;

			pGroup->add(txtpos);

			return txtpos;
		}
	}
	return NULL;
}

CHashTblTextPosS* CW01DrawDoc::GetTextPosP(int nPageNo)
{
	if (m_pmapTextPos == NULL ||
		m_pmapTextPos->size() == 0)
		return NULL;

	int page = nPageNo;
	if (page == -1)
		page = GetPage();

	return m_pmapTextPos->get(page);
}

LPTEXTPOSITION CW01DrawDoc::FindPosition(LPPOINT pt)
{
	CHashTblTextPosS* pmapTextPos = GetTextPosP();

	if( pt == NULL ||
		!pmapTextPos ||
		!pmapTextPos->size())
		return NULL;

	LPPAGECONTENT pContent = GetPageContent();
	if( !pContent )
		return NULL;

	if( pt->x < 0 || pt->x > pContent->width )
		return NULL;
	if( pt->y < 0 || pt->y > pContent->height )
		return NULL;

	AREA page = {0};
	page.x_min = 0;
	page.x_max = pContent->width;
	page.y_min = 0;
	page.y_max = pContent->height;

	CString code = _T("");
	AREA_Encode(code, pt->x, pt->y, &page, 0, m_areaLevel);
	//TRACE(_T("F[%s] pt(x:%d, y:%d)\n"), code, pt->x, pt->y);

	LinkedList<LPTEXTPOSITION>* pList = pmapTextPos->get(code.GetBuffer());
	if( !pList ||
		!pList->first() )
		return NULL;

	BOOL bFound = FALSE;
	LPTEXTPOSITION pTextPos = NULL;
	CRect rc;
	do
	{
		pTextPos = pList->get();

		rc.SetRect( pTextPos->pos->textX, pTextPos->pos->textY - pTextPos->height, 
					pTextPos->pos->textX + pTextPos->width,
					pTextPos->pos->textY );
		//LPSTR pStr = _ConvW2A(pTextPos->text);
		//if( pStr && strlen(pStr) )
		//{
		//	TRACE(_T("%S RECT(l:%d, t:%d, r:%d, b:%d)\n"), pStr, rc.left, rc.top, rc.right, rc.bottom);
		//	_del(pStr);
		//}
		if( rc.PtInRect(*pt) )
		{
			bFound = TRUE;
			// Found
			break;
		}
	}while( pList->next() );

	if( bFound &&
		pTextPos )
		return pTextPos;

	return NULL;
}

void CW01DrawDoc::SplitAreaTable(LPPAGECONTENT pContent)
{
	int nPageNo = pContent->nPageNo;
	CHashTblTextPosS* pmapTextPos = GetTextPosP(nPageNo);
	if (pmapTextPos != NULL)
	{
		return;
	}
	pmapTextPos = new CHashTblTextPosS();
	if (pmapTextPos == NULL ||
		pmapTextPos->create(genHashKeyStr) == NULL)
	{
		_delete(pmapTextPos);

		return;
	}

	UINT cacheSize = theApp.GetCachePages();
	if (cacheSize > 0)
	{
		if (m_pmapTextPos->size() > cacheSize)
		{
			INT pageNoOld = m_plstPageNo.GetAt(0);
			if (m_pmapTextPos->remove(pageNoOld)) {

			}
		}
	}
	m_pmapTextPos->add(nPageNo, pmapTextPos);

	LinkedList<WEB_POS_RECDATA*>* pPosList = pContent->plstTextPos;
	if( pPosList &&
		pPosList->first() )
	{
		// 2 ^ 4 = 32
		m_areaLevel = 6;
		//if( pPosList->size() > 2000 )
		//{
		//	m_areaLevel = 7;
		//}
		//else if( pPosList->size() > 1000 )
		//{
		//	m_areaLevel = 5;
		//}
		int level = m_areaLevel;

		AREA page = {0};
		page.x_min = 0;
		page.x_max = pContent->width;
		page.y_min = 0;
		page.y_max = pContent->height;

		CString areacode;
		CString areacodeT;
		WEB_POS_RECDATA* ppos = NULL;
		WEB_ATTR_RECDATA* pAttr = NULL;
		LPTEXTPOSITION pTextPos = NULL;
		LPTEXTADDR pTextAdd = NULL;
		do
		{
			ppos = pPosList->get();
			if( ppos )
			{
				pTextAdd = NULL;
				pTextPos = NULL;
				pAttr = NULL;
				areacode = _T("");

				int x = ppos->textX;
				int y = ppos->textY;
				AREA_Encode(areacode, x, y, &page, 0, level);

				pTextPos = AddAreaCode(pmapTextPos, areacode, ppos);
				if( m_pAttTbl )
				{
					pAttr = m_pAttTbl->get(ppos->attrId);
				}
				if( m_plstText )
				{
					pTextAdd = m_plstText->at(ppos->textNum);
				}
				if( pTextAdd != NULL )
				{
					wcscpy_s(pTextPos->text, 4, pTextAdd->text);

					pTextPos->flag = pTextAdd->flag;
				}
#if 0
				LPSTR pStr = _ConvW2A(pTextPos->text);
				if( pStr && strlen(pStr) )
				{
					TRACE(_T("[%s] %S\n"), areacode, pStr);
					_del(pStr);
				}
				else
				{
					TRACE(_T("[%s]\n"), areacode);
				}
#endif
				if( pAttr )
				{
					int w0 = (int)Round(pAttr->fontWidth, 0.1);
					int h0 = (int)Round(pAttr->fontHeight, 0.1);
					int x0 = x + w0;
					int y0 = y;
					y = y - h0;
					
					
					if( pTextPos )
					{
						//if (CheckHalfWidth(pTextPos->text))
						if (BIT_CHECK(pTextPos->flag, ISSBCS))
						{
							w0 = w0 / 2;
						}

						WEB_FONT_DATA* pFont = GetFont(pAttr->fontId);
						if( pFont )
						{
							wcscpy_s(pTextPos->fontName, 63, pFont->name);
						}

						int cHeight = h0 * 20 / m_tppx;
						int cWidth = w0 * 20 / m_tppy;
						cHeight = cHeight * 72 / m_nScreenDpi;
						cWidth = cWidth * 72 / m_nScreenDpi;

						pTextPos->width = cWidth;
						pTextPos->height = cHeight;
						pTextPos->rotate = pAttr->fontRotate;
						pTextPos->wmode = 0;
						if( pAttr->fontStyle & FONT_STYLE_WMODE )
							pTextPos->wmode = 1;
					}

					areacodeT = _T("");
					AREA_Encode(areacodeT, x, y0, &page, 0, level);
					if( areacode.Compare(areacodeT) != 0 )
					{
						AddAreaCode(pmapTextPos, areacodeT, pTextPos);
#if 0
						TRACE(_T("T[%s]\n"), areacodeT);
#endif
					}

					areacodeT = _T("");
					AREA_Encode(areacodeT, x0, y0, &page, 0, level);
					if( areacode.Compare(areacodeT) != 0 )
					{
						AddAreaCode(pmapTextPos, areacodeT, pTextPos);
#if 0
						TRACE(_T("T[%s]\n"), areacodeT);
#endif
					}

					areacodeT = _T("");
					AREA_Encode(areacodeT, x0, y, &page, 0, level);
					if( areacode.Compare(areacodeT) != 0 )
					{
						AddAreaCode(pmapTextPos, areacodeT, pTextPos);
#if 0
						TRACE(_T("T[%s]\n"), areacodeT);
#endif
					}
				}
			}
		} while( pPosList->next() );
	}
}

void CW01DrawDoc::SetCaptureHover(BOOL bSet)
{
	bCaptureHover = bSet;

	if( !bCaptureHover )
		m_posCurrent = NULL;
}


void CW01DrawDoc::OnFilePdf()
{
	if( m_pW01Doc )
	{
		CString szOutName;
		if( theApp.BrowseForFolder(AfxGetMainWnd()->GetSafeHwnd(), IDS_DIR_OUTPUT, szOutName))
		{
			CString szMsg;
			
			//IDS_MSG_FILEEXIST
			TCHAR szPdfId[_MAX_PATH] = {0};
			if( m_pW01Doc->ReadBinaryData(BIN_TYPE_PDF, szOutName, szPdfId, _MAX_PATH) )
			{
				szMsg.LoadString(IDS_MSG_OUTPUTPDF);
				szMsg.Append(_T("\n"));
				szMsg.Append(szPdfId);

				AfxMessageBox(szMsg, MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				AfxMessageBox(IDS_ERRMSG_OUTPUTPDF, MB_OK | MB_ICONERROR);
			}
		}
	}
}


void CW01DrawDoc::OnUpdateFilePdf(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	CHashTblBinary* pMap = GetBinaryMap();
	if( pMap )
	{
		LPBINDATA pBin = NULL;
		USHORT nBinNo = 0;
		PPOSITION pos = pMap->first();
		while(pos)
		{
			pMap->next(pos, nBinNo, pBin);
			if( pBin )
			{
				if( pBin->type == BIN_TYPE_PDF )
				{
					bEnable = TRUE;
					break;
				}
			}
		}
	}

	pCmdUI->Enable(bEnable);
}

void CW01DrawDoc::OnFileOpenWsi()
{
	if (m_pW01Doc)
	{
		CString szFilter;
		szFilter.LoadString(IDS_WSI_FILTER);

		CFileDialog dlg(TRUE,
			_T("WSI"),
			_T(""),
			OFN_FILEMUSTEXIST,
			szFilter,
			AfxGetMainWnd());

		if (dlg.DoModal() == IDOK)
		{
			CString fname = dlg.GetPathName();

			if (!ReadWsiFile(fname.GetBuffer())) {
				//
				AfxMessageBox(IDS_MSG_OPNEWSI, MB_OK | MB_ICONERROR);
				return;
			}
			SetShowWsi(TRUE);
		}
	}
}

void CW01DrawDoc::OnFileOpencell()
{
	if( m_pW01Doc )
	{
		CString szDummy;
		CString szFilter;
		szFilter.LoadString(IDS_CELL_FILTER);

		szDummy = theApp.GetCellDefinePath();
		szDummy.Append(_T("\\"));

		CFileDialog dlg(TRUE, 
					_T("CDF"), 
					szDummy, 
					OFN_FILEMUSTEXIST,
					szFilter, 
					AfxGetMainWnd());

		if( dlg.DoModal() == IDOK )
		{
			CString fname = dlg.GetFileName();
			fname.MakeUpper();
			fname.Replace(_T(".CDF"), _T(""));

			m_pW01Doc->ReadCellFile(fname.GetBuffer());
		}
	}
}

void CW01DrawDoc::OnTextDump()
{
}

BOOL CW01DrawDoc::ReadWsiFile(LPWSTR pwszWsi)
{
	_delete(m_pWsiFile);

	m_pWsiFile = new CWsiFile();
	if (m_pWsiFile == NULL) {
		// Not enough memory
		return FALSE;
	}

	if (!m_pWsiFile->Load(pwszWsi)) {
		// Not a valid wsi file
		_delete(m_pWsiFile);
		return FALSE;
	}

	if (!m_pWsiFile->IsValide()) {
		// Not a valid wsi file
		_delete(m_pWsiFile);
		return FALSE;
	}

	return TRUE;
}
