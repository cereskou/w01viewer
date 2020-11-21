
// w01drawView.cpp : CW01DrawView クラスの実装
//

#include "stdafx.h"
// SHARED_HANDLERS は、プレビュー、サムネイル、および検索フィルター ハンドラーを実装している ATL プロジェクトで定義でき、
// そのプロジェクトとのドキュメント コードの共有を可能にします。
#ifndef SHARED_HANDLERS
#include "w01draw.h"
#endif

#include "MainFrm.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "GotoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CW01DrawView

IMPLEMENT_DYNCREATE(CW01DrawView, CZoomView)

BEGIN_MESSAGE_MAP(CW01DrawView, CZoomView)
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, &CZoomView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CZoomView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CZoomView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_VIEW_FULL, &CW01DrawView::OnViewFull)
	ON_COMMAND(ID_VIEW_ZOOMIN, &CW01DrawView::OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, &CW01DrawView::OnViewZoomout)
	ON_COMMAND(ID_VIEW_SHOWWIDE, &CW01DrawView::OnViewShowwide)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, &CW01DrawView::OnUpdateViewZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, &CW01DrawView::OnUpdateViewZoomout)
	ON_CBN_SELENDOK(ID_VIEW_ZOOMCOMBO, &CW01DrawView::OnViewZoom)
	ON_COMMAND(ID_VIEW_SHOWINFO, &CW01DrawView::OnViewShowinfo)
	ON_COMMAND(ID_PAGE_FIRST, &CW01DrawView::OnPageFirst)
	ON_UPDATE_COMMAND_UI(ID_PAGE_FIRST, &CW01DrawView::OnUpdatePageFirst)
	ON_COMMAND(ID_PAGE_LAST, &CW01DrawView::OnPageLast)
	ON_UPDATE_COMMAND_UI(ID_PAGE_LAST, &CW01DrawView::OnUpdatePageLast)
	ON_COMMAND(ID_PAGE_NEXT, &CW01DrawView::OnPageNext)
	ON_UPDATE_COMMAND_UI(ID_PAGE_NEXT, &CW01DrawView::OnUpdatePageNext)
	ON_COMMAND(ID_PAGE_PREV, &CW01DrawView::OnPagePrev)
	ON_UPDATE_COMMAND_UI(ID_PAGE_PREV, &CW01DrawView::OnUpdatePagePrev)
	ON_COMMAND(ID_VIEW_TEXT, &CW01DrawView::OnViewText)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXT, &CW01DrawView::OnUpdateViewText)
	ON_COMMAND(ID_VIEW_OVLY, &CW01DrawView::OnViewOvly)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OVLY, &CW01DrawView::OnUpdateViewOvly)
	ON_COMMAND(ID_VIEW_IMAGE, &CW01DrawView::OnViewImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMAGE, &CW01DrawView::OnUpdateViewImage)
	ON_COMMAND(ID_VIEW_LINE, &CW01DrawView::OnViewLine)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LINE, &CW01DrawView::OnUpdateViewLine)
	ON_COMMAND(ID_VIEW_TEXTXY, &CW01DrawView::OnViewTextXY)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTXY, &CW01DrawView::OnUpdateViewTextXY)
	ON_COMMAND(ID_VIEW_CELLDEF, &CW01DrawView::OnViewCellDef)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CELLDEF, &CW01DrawView::OnUpdateViewCellDef)
	ON_COMMAND(ID_VIEW_HIGHLIGHT, &CW01DrawView::OnViewHighLight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIGHLIGHT, &CW01DrawView::OnUpdateViewHighLight)
	ON_COMMAND(ID_VIEW_TIPS, &CW01DrawView::OnViewTips)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TIPS, &CW01DrawView::OnUpdateViewTips)
	ON_COMMAND(ID_VIEW_TEXTDUMP, &CW01DrawView::OnViewTextDump)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTDUMP, &CW01DrawView::OnUpdateViewTextDump)

	ON_COMMAND(ID_VIEW_IMGOUTPUT, &CW01DrawView::OnViewSaveImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMGOUTPUT, &CW01DrawView::OnUpdateViewSaveImage)

	ON_COMMAND(ID_VIEW_WSI, &CW01DrawView::OnViewWsi)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WSI, &CW01DrawView::OnUpdateViewWsi)

	ON_MESSAGE(UWM_DOC_REFRESH, &CW01DrawView::OnDocRefresh)
	ON_MESSAGE(UWM_DOC_SYNC, &CW01DrawView::OnDocFontSync)
	ON_MESSAGE(UWM_DOC_GOTO, &CW01DrawView::OnDocGotoPage)
	ON_MESSAGE(UWM_DOC_UPDATEUI, &CW01DrawView::OnDocUpdateUI)
	ON_COMMAND(ID_VIEW_ESCAPE, &CW01DrawView::OnViewEscape)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CW01DrawView コンストラクション/デストラクション

CW01DrawView::CW01DrawView()
{
	// TODO: 構築コードをここに追加します。
}

CW01DrawView::~CW01DrawView()
{
}

// CW01DrawView 描画

void CW01DrawView::Draw(CDC* pDC)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if( !pDC->IsPrinting() )
	{
		CRect rect;
		GetClientRect(&rect);
		pDC->DPtoLP(&rect);
		pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNSHADOW));
	}

	pDoc->Draw(pDC);
}

void CW01DrawView::OnInitialUpdate()
{
	CZoomView::OnInitialUpdate();

	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CWaitCursor wait;

	CSize docSize = pDoc->GetDocSize();
	if( docSize.cx <= 0 )
		docSize.cx = 1;
	if( docSize.cy <= 0 )
		docSize.cy = 1;

	docSize.cx += 2;
	docSize.cy += 2;

	SetZoomSizes(docSize);
	SetZoomMode(MODE_ZOOMOFF);

	SetMinZoomScale(0.1f);
	CRect rc;
	GetClientRect(rc);

	if( !m_toolTips.Create(this) )
	{
		TRACE(_T("Create failed.\n"));
	}

	//if( docSize.cx >  rc.Width() * 2 )
	//{
	//	ZoomWide();
	//}

	//lStyle = GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
	//lStyle |= WS_EX_STATICEDGE;
	//SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, lStyle);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame && theApp.GetEnableDebug() )
	{
		BOOL bForce = TRUE;
		if( pFrame->GetMDITabGroups().GetCount() > 1 )
		{
			bForce = FALSE; 
		}
		CTextPropertiesWnd* pTextWnd = pFrame->GetTextPropertiesWnd();
		if( pTextWnd )
		{
			pTextWnd->SetWindow(this, bForce);
		}
		CFontPropertiesWnd* pFontWnd = pFrame->GetFontPropertiesWnd();
		if( pFontWnd )
		{
			pFontWnd->SetWindow(this, bForce);
		}
		CDocInfoPropertiesWnd* pDocInfWnd = pFrame->GetDocInfoPropertiesWnd();
		if( pDocInfWnd )
		{
			pDocInfWnd->SetWindow(this, bForce);
		}
		CPageInfoPropertiesWnd* pPageInfWnd = pFrame->GetPageInfoPropertiesWnd();
		if( pPageInfWnd )
		{
			pPageInfWnd->SetWindow(this, bForce);
		}
		CNumericalPropertiesWnd* pNumericalWnd = pFrame->GetNumericalPropertiesWnd();
		if( pNumericalWnd )
		{
			pNumericalWnd->SetWindow(this, bForce);
		}
	}
}


// CW01DrawView 印刷


void CW01DrawView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CW01DrawView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 既定の印刷準備
	return DoPreparePrinting(pInfo);
}

void CW01DrawView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷前の特別な初期化処理を追加してください。
}

void CW01DrawView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 印刷後の後処理を追加してください。
}

void CW01DrawView::ClearViewClicked()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);

	m_toolTips.Close();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		if( theApp.GetEnableDebug() )
		{
			CTextPropertiesWnd* pTpWnd = pFrame->GetTextPropertiesWnd();
			if( pTpWnd )
			{
				pTpWnd->SetSelectedPosition(NULL);
			}
		}
	}
}

void CW01DrawView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClearViewClicked();

	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CW01DrawView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
//	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CW01DrawView 診断

#ifdef _DEBUG
void CW01DrawView::AssertValid() const
{
	CZoomView::AssertValid();
}

void CW01DrawView::Dump(CDumpContext& dc) const
{
	CZoomView::Dump(dc);
}

CW01DrawDoc* CW01DrawView::GetDocument() const // デバッグ以外のバージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CW01DrawDoc)));
	return (CW01DrawDoc*)m_pDocument;
}
#endif //_DEBUG


// CW01DrawView メッセージ ハンドラー

void CW01DrawView::OnViewFull()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);

	ZoomFull();
	SetZoomMode(MODE_ZOOMOFF);
	SetFocus();

	Invalidate();
}

void CW01DrawView::OnViewZoomin()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);

	if(  GetZoomMode() != MODE_ZOOMIN )
	{
		SetZoomMode(MODE_ZOOMIN);
		SetFocus();

		return;
	}
	ZoomIn();
	SetFocus();

	Invalidate();
}


void CW01DrawView::OnViewZoomout()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);

	if(  GetZoomMode() != MODE_ZOOMOUT )
	{
		SetZoomMode(MODE_ZOOMOUT);
		SetFocus();

		return;
	}
	ZoomOut();
	SetFocus();

	Invalidate();
}


void CW01DrawView::OnViewShowwide()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);

	ZoomWide();
	SetZoomMode(MODE_ZOOMOFF);
	SetFocus();

	Invalidate();
}


void CW01DrawView::OnUpdateViewZoomin(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMIN);
}


void CW01DrawView::OnUpdateViewZoomout(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(GetZoomMode() == MODE_ZOOMOUT);
}

void CW01DrawView::NotifyZoom()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		float scale = GetZoomScale();
		CComboBox* pZoom = pFrame->GetZoomComboBox();
		if( pZoom )
		{
			SetSelectedZoomScale(pZoom, scale);
		}
	}
	SetFocus();
}

void CW01DrawView::SetSelectedZoomScale(CComboBox* pBox, float scale)
{
	if( !pBox || scale <= 0.0 || scale > 64.0 )
	{
		return;
	}

	DWORD dwId = 0;
	if( scale == 0.10 )
	{
		dwId = IDS_ZOOM_10;
	}
	if( scale == 0.25 )
	{
		dwId = IDS_ZOOM_25;
	}
	else if( scale == 0.5 )
	{
		dwId = IDS_ZOOM_50;
	}
	else if( scale == 0.75 )
	{
		dwId = IDS_ZOOM_75;
	}
	else if( scale == 1.0 )
	{
		dwId = IDS_ZOOM_100;
	}
	else if( scale == 1.25 )
	{
		dwId = IDS_ZOOM_125;
	}
	else if( scale == 1.5 )
	{
		dwId = IDS_ZOOM_150;
	}
	else if( scale == 2.0 )
	{
		dwId = IDS_ZOOM_200;
	}
	else if( scale == 4.0 )
	{
		dwId = IDS_ZOOM_400;
	}
	else if( scale == 8.0 )
	{
		dwId = IDS_ZOOM_800;
	}
	else if( scale == 16.0 )
	{
		dwId = IDS_ZOOM_1600;
	}
	else if( scale == 24.0 )
	{
		dwId = IDS_ZOOM_2400;
	}
	else if( scale == 32.0 )
	{
		dwId = IDS_ZOOM_3200;
	}
	else if( scale == 64.0 )
	{
		dwId = IDS_ZOOM_6400;
	}

	int nCurSel = dwId - IDS_ZOOM_10;
	if( nCurSel > -1 )
	{
		pBox->SetCurSel(nCurSel);
	}
	else
	{
		pBox->SetCurSel(-1);
		TCHAR szScale[32] = {0};
		_stprintf_s(szScale, 32, _T("%.2f%%"), scale * 100.0);

		pBox->SetWindowText(szScale);
	}
}

float CW01DrawView::GetSelectedZoomScale(CComboBox* pBox)
{
	double scale = 0.0;
	if( !pBox )
		return (float)scale;

	int nCurSel = pBox->GetCurSel();
	if( nCurSel == -1 )
	{
		TCHAR szScale[32] = {0};
		pBox->GetWindowText(szScale, 32);
		LPTSTR pEndPtr = NULL;
		scale = _tcstod(szScale, &pEndPtr);

		return (float)scale;
	}

	if( nCurSel > -1 )
	{
		DWORD_PTR dwId = pBox->GetItemData(nCurSel);
		switch( dwId )
		{
		case IDS_ZOOM_10:
			scale = 0.10;
			break;
		case IDS_ZOOM_25:
			scale = 0.25;
			break;
		case IDS_ZOOM_50:
			scale = 0.5;
			break;
		case IDS_ZOOM_75:
			scale = 0.75;
			break;
		case IDS_ZOOM_100:
			scale = 1.00;
			break;
		case IDS_ZOOM_125:
			scale = 1.25;
			break;
		case IDS_ZOOM_150:
			scale = 1.5;
			break;
		case IDS_ZOOM_200:
			scale = 2.0;
			break;
		case IDS_ZOOM_400:
			scale = 4.0;
			break;
		case IDS_ZOOM_800:
			scale = 8.0;
			break;
		case IDS_ZOOM_1600:
			scale = 16.0;
			break;
		case IDS_ZOOM_2400:
			scale = 24.0;
			break;
		case IDS_ZOOM_3200:
			scale = 32.0;
			break;
		case IDS_ZOOM_6400:
			scale = 64.0;
			break;
		default:
			break;
		}
	}

	return (float)scale;
}

void CW01DrawView::OnViewZoom()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		float old = GetZoomScale();
		CComboBox* pZoom = pFrame->GetZoomComboBox();
		if( pZoom )
		{
			float scale = GetSelectedZoomScale(pZoom);
			if( scale > 0.0 && scale != old )
			{
				ZoomTo(NULL, scale);

				Invalidate();
			}
			SetFocus();
		}
	}

	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetCaptureHover(FALSE);
}


void CW01DrawView::OnViewShowinfo()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	Invalidate();

	if( pDoc->ShowDocInfo() )
	{
	}
}


void CW01DrawView::OnPageFirst()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	theApp.BeginWaitCursor();
	if( pDoc->MoveFirstPage() )
	{
		CSize docSize = pDoc->GetDocSize();
		if( docSize.cx <= 0 )
			docSize.cx = 1;
		if( docSize.cy <= 0 )
			docSize.cy = 1;

		SetZoomSizes(docSize);

		pDoc->SetCaptureHover(FALSE);

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame )
		{
			pFrame->RefreshDockPane();
		}
		SetFocus();
	}
	theApp.EndWaitCursor();
}


void CW01DrawView::OnUpdatePageFirst(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(pDoc->GetPage() != 0);
}


void CW01DrawView::OnPageLast()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	theApp.BeginWaitCursor();
	if( pDoc->MoveLastPage() )
	{
		CSize docSize = pDoc->GetDocSize();
		if( docSize.cx <= 0 )
			docSize.cx = 1;
		if( docSize.cy <= 0 )
			docSize.cy = 1;

		SetZoomSizes(docSize);

		pDoc->SetCaptureHover(FALSE);

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame )
		{
			pFrame->RefreshDockPane();
		}
		SetFocus();
	}
	theApp.EndWaitCursor();
}


void CW01DrawView::OnUpdatePageLast(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(pDoc->GetTotalPage() != pDoc->GetPage() + 1);
}


void CW01DrawView::OnPageNext()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	theApp.BeginWaitCursor();
	if( pDoc->MoveNextPage() )
	{
		CSize docSize = pDoc->GetDocSize();
		if( docSize.cx <= 0 )
			docSize.cx = 1;
		if( docSize.cy <= 0 )
			docSize.cy = 1;

		SetZoomSizes(docSize);

		pDoc->SetCaptureHover(FALSE);

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame )
		{
			pFrame->RefreshDockPane();
		}

		SetFocus();
	}
	theApp.EndWaitCursor();
}


void CW01DrawView::OnUpdatePageNext(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(pDoc->GetTotalPage() > pDoc->GetPage() + 1);
}


void CW01DrawView::OnPagePrev()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	theApp.BeginWaitCursor();
	if( pDoc->MovePrevPage() )
	{
		CSize docSize = pDoc->GetDocSize();
		if( docSize.cx <= 0 )
			docSize.cx = 1;
		if( docSize.cy <= 0 )
			docSize.cy = 1;

		SetZoomSizes(docSize);

		pDoc->SetCaptureHover(FALSE);

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if( pFrame )
		{
			pFrame->RefreshDockPane();
		}

		SetFocus();
	}
	theApp.EndWaitCursor();
}


void CW01DrawView::OnUpdatePagePrev(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(pDoc->GetPage() > 0);
}


void CW01DrawView::OnViewText()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowText() )
	{
		pDoc->SetShowText(FALSE);
		pDoc->SetCaptureHover(FALSE);

	}
	else
		pDoc->SetShowText(TRUE);

	SetFocus();
	Invalidate();
}


void CW01DrawView::OnUpdateViewText(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowText());
}

void CW01DrawView::OnViewOvly()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowOvly() )
		pDoc->SetShowOvly(FALSE);
	else
		pDoc->SetShowOvly(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewOvly(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowOvly());
}

void CW01DrawView::OnViewImage()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowImage() )
		pDoc->SetShowImage(FALSE);
	else
		pDoc->SetShowImage(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewImage(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowImage());
}

void CW01DrawView::OnViewLine()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowLine() )
		pDoc->SetShowLine(FALSE);
	else
		pDoc->SetShowLine(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewLine(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowLine());
}

void CW01DrawView::OnViewTextXY()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowTextXY() )
		pDoc->SetShowTextXY(FALSE);
	else
		pDoc->SetShowTextXY(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewTextXY(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowTextXY());
}

void CW01DrawView::OnViewCellDef()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowCellDef() )
		pDoc->SetShowCellDef(FALSE);
	else
		pDoc->SetShowCellDef(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewCellDef(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowCellDef());
//	pCmdUI->Enable(pDoc->
}

void CW01DrawView::OnViewWsi()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->IsShowWsi())
		pDoc->SetShowWsi(FALSE);
	else
		pDoc->SetShowWsi(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewWsi(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowWsi());
}

void CW01DrawView::SetHighLightID(INT id)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->SetHighLightID(id);
	Invalidate();
}

void CW01DrawView::OnViewHighLight()
{
	//LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	//if( lStyle & WS_CLIPSIBLINGS)
	//{
	//	lStyle &= ~WS_CLIPSIBLINGS;// | WS_CLIPCHILDREN);
	//}
	//SetWindowLong(GetSafeHwnd(), GWL_STYLE, lStyle);

	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( pDoc->IsShowHighLight() )
		pDoc->SetShowHighLight(FALSE);
	else
		pDoc->SetShowHighLight(TRUE);

	Invalidate();
}

void CW01DrawView::OnUpdateViewHighLight(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->SetCheck(pDoc->IsShowHighLight());
}


void CW01DrawView::OnViewTips()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( pDoc->IsCaptureHover() )
		pDoc->SetCaptureHover(FALSE);
	else
	{
		OnViewEscape();
		pDoc->SetCaptureHover(TRUE);
		SetFocus();
	}
}

void CW01DrawView::OnViewEscape()
{
	ClearViewClicked();

	SetZoomMode(MODE_ZOOMOFF);
}

void CW01DrawView::OnUpdateViewTips(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if( !pDoc->IsCaptureHover() )
	{
		m_toolTips.Close();

		pDoc->SetCurrentPos(NULL);
	}
	
	pCmdUI->SetCheck(pDoc->IsCaptureHover());
}

void CW01DrawView::OnViewTextDump()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->OnTextDump();
}


void CW01DrawView::OnUpdateViewTextDump(CCmdUI *pCmdUI)
{
	//CW01DrawDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);

	//pCmdUI->SetCheck(pDoc->IsShowText());
}


LRESULT CW01DrawView::OnDocRefresh(WPARAM wParam, LPARAM lParam)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( !pFrame )
	{
		return 0L;
	}

	if( theApp.GetEnableDebug() )
	{
		CFontPropertiesWnd* pFontWnd = pFrame->GetFontPropertiesWnd();
		if( !pFontWnd )
		{
			return 0L;
		}
		CHashTblStringToOutFont* pMapFont = new CHashTblStringToOutFont();
		if( pMapFont == NULL ||
			pMapFont->create(genHashKeyStr) == NULL )
		{
			_delete(pMapFont);
			return 0L;
		}

		// フォント作り直す
		if( pFontWnd->CreateFontMap(pMapFont) > 0 )
		{
			if( pDoc->RefreshFont(pMapFont) > 0 )
			{
				Invalidate();
				UpdateWindow();
			}
		}

		_delete(pMapFont);
	}

	return 0L;
}

LRESULT CW01DrawView::OnDocGotoPage(WPARAM wParam, LPARAM lParam)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int maxPage = pDoc->GetTotalPage();
	if( maxPage > 1 )
	{
		CGotoDialog dlg;

		dlg.SetPageRange(1, maxPage);
		if( dlg.DoModal() == IDOK )
		{
			int nPageNo = dlg.m_nSelectedPageNo - 1;

			if( pDoc->MoveToPage(nPageNo) )
			{
				Invalidate();

				SetFocus();
			}
		}
	}

	return 0L;
}

LRESULT CW01DrawView::OnDocUpdateUI(WPARAM wParam, LPARAM lParam)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->UpdateUI();

	return 0L;
}

LRESULT CW01DrawView::OnDocFontSync(WPARAM wParam, LPARAM lParam)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( !pFrame )
	{
		return 0L;
	}

	if( !theApp.GetEnableDebug() )
		return 0L;

	// Sync To
	CFontXmlPropertiesWnd* pXmlWnd = pFrame->GetFontXmlPropertiesWnd();
	CFontPropertiesWnd* pFontWnd = pFrame->GetFontPropertiesWnd();
	if( !pXmlWnd || !pFontWnd )
	{
		return 0L;
	}

	if( wParam == 0 )
	{
		if( pFontWnd->SyncTo(pXmlWnd->GetStrToPropTable(), TRUE, 1) )
		{
			pXmlWnd->SetModified();
		}
	}
	else if( wParam == 1 )
	{
		// Sync From
		if( pFontWnd->SyncFrom(pXmlWnd->GetStrToPropTable(), TRUE, 1) )
		{
			pFontWnd->SetModified(FALSE);
		}
	}

	return 0L;
}


void CW01DrawView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	TRACE(_T("OnActivateView bActivate:%d, %s\n"), bActivate, pDoc->GetPathName());
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	if( bActivate )
	{
		pDoc->UpdateUI();
	}

	CZoomView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CW01DrawView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	CPoint pt = point;
	ClientToDoc(pt);

	BOOL bMulti = FALSE;
	if( (GetKeyState(VK_SHIFT) & 0x8000) != 0 )
		bMulti = TRUE;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		if( theApp.GetEnableDebug() )
		{
			CTextPropertiesWnd* pTpWnd = pFrame->GetTextPropertiesWnd();
			if( pTpWnd )
			{
				if( pTpWnd->GetHighLight() )
				{
					LPTEXTPOSITION pos = pDoc->FindPosition(&pt);
					if( pos )
					{
						if( pos->pos )
						{
							if( pos->pos->index < TEXTBLOCK_OFFSET )
							{
								pos->pos->index += TEXTBLOCK_OFFSET;

								pTpWnd->SetSelectedPosition(pos->pos, bMulti);

								Invalidate();
							}
						}
					}
				}
			}
		}
	}
	TRACE(_T("MouseDown x:%d, y:%d\n"), pt.x, pt.y);
	m_bShowTips = pDoc->IsCaptureHover();
	pDoc->SetCaptureHover(FALSE);

	CZoomView::OnLButtonDown(nFlags, point);
}

void CW01DrawView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (m_bCaptured)
	{
		pDoc->SetCaptureHover(m_bShowTips);
	}

	Invalidate();

	CZoomView::OnLButtonUp(nFlags, point);
}

WCHAR* ConvertToHext(WCHAR* data, WCHAR* out, DWORD dwSize)
{
	WCHAR tmp[9];
	for(unsigned x = 0; x < wcslen(data); x++)
	{
		int value = (int)(data[x]);
		_stprintf_s(tmp, 8, L"%02X", value);
		wcscat_s(out, dwSize, tmp);
	}
	return out;	
}

void CW01DrawView::OnMouseMove(UINT nFlags, CPoint point)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame )
	{
		CPoint pt = point;
		ClientToDoc(pt);
		pFrame->SetCursorPosText(pt);
	}

	if( pDoc->IsShowText() && pDoc->IsCaptureHover() )
	{
		CPoint pt = point;
		ClientToDoc(pt);

		LPTEXTPOSITION pos = pDoc->FindPosition(&pt);
		if( pos &&
			pos != pDoc->GetCurrentPos() )
		{
			TCHAR szHex[32] = {0};
			TCHAR szText[64] = {0};
			//_tcscpy_s(szText, 4, pos->text);
			ConvertToHext(pos->text, szHex, 32);
			_stprintf_s(szText, 63, _T("%s [0x%s]"), pos->text, szHex);

			pDoc->SetCurrentPos(pos);

			int index = pos->pos->index;
			if( index >= TEXTBLOCK_OFFSET )
				index -= TEXTBLOCK_OFFSET;
			// 座標(x:%d, y:%d) サイズ(width:%d, height:%d)\nフォント名:%s\n表示用フォント名：%s\n
			// 位置：line:%d, number:%d, block:%d
			CString szMsg;
			//TCHAR szPt[64] = {0};
			LPWSTR lpMapFontName = pDoc->GetFontMap( pos->fontName);
			if( lpMapFontName )
			{
				szMsg.Format(IDS_MSG_TIPS, pos->pos->textX, pos->pos->textY,
					pos->width, pos->height, pos->rotate,
					pos->wmode,
					pos->fontName, 
					lpMapFontName,
					pos->pos->line, index, pos->pos->block,
					pos->pos->cellX, pos->pos->cellY);
			}
			else
			{
				szMsg.Format(IDS_MSG_TIPS, pos->pos->textX, pos->pos->textY,
					pos->width, pos->height, pos->rotate,
					pos->wmode,
					pos->fontName, 
					_T(""),
					pos->pos->line, index, pos->pos->block,
					pos->pos->cellX, pos->pos->cellY);
			}
			
			m_toolTips.SetText(szText, szMsg);

			//pt = point;
			pt.x = pos->pos->textX + pos->width;
			pt.y = pos->pos->textY;
			DocToClient(pt);

			m_toolTips.Show(pt);
		}
		else if( !pos )
		{
			pDoc->SetCurrentPos(NULL);
			m_toolTips.Close();
		}

		return;
	}

	CZoomView::OnMouseMove(nFlags, point);
}


void CW01DrawView::OnKillFocus(CWnd* pNewWnd)
{
	CZoomView::OnKillFocus(pNewWnd);

	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( pDoc )
	{
		pDoc->SetCaptureHover(FALSE);
	}
}

void CW01DrawView::OnViewSaveImage()
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	Invalidate();
	if( pDoc->SaveImageFile() )
	{
		
	}
}

void CW01DrawView::OnUpdateViewSaveImage(CCmdUI *pCmdUI)
{
	CW01DrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(pDoc->GetImageCount() > 0);
}
