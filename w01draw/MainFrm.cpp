
// MainFrm.cpp : CMainFrame クラスの実装
//

#include "stdafx.h"
#include "w01draw.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//// CMDIClientWnd
//IMPLEMENT_DYNAMIC(CMDIClientWnd, CWnd)
//
//BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
//	ON_WM_PAINT()
//	ON_MESSAGE(WM_MDISETMENU, &CMDIClientWnd::OnSetMenuNotify)
//END_MESSAGE_MAP()
//
//void CMDIClientWnd::OnPaint()
//{
//	CPaintDC dc(this);
//}
//
//
//LRESULT CMDIClientWnd::OnSetMenuNotify(WPARAM wParam, LPARAM lParam)
//{
//	return 0L;
//}

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_FILEVIEW, &CMainFrame::OnViewFilectrl)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILEVIEW, &CMainFrame::OnUpdateViewFilectrl)
	ON_COMMAND(ID_VIEW_DEBUG, &CMainFrame::OnViewDebug)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEBUG, &CMainFrame::OnUpdateViewDebug)
	ON_COMMAND(ID_VIEW_TEXTVIEW, &CMainFrame::OnViewTextList)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTVIEW, &CMainFrame::OnUpdateViewTextList)
	ON_COMMAND(ID_VIEW_FONTVIEW, &CMainFrame::OnViewFontPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FONTVIEW, &CMainFrame::OnUpdateViewFontPane)
	ON_COMMAND(ID_VIEW_FONTXML, &CMainFrame::OnViewFontXmlPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FONTXML, &CMainFrame::OnUpdateViewFontXmlPane)
	ON_COMMAND(ID_VIEW_DOCINFVIEW, &CMainFrame::OnViewDocInfoPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOCINFVIEW, &CMainFrame::OnUpdateViewDocInfoPane)
	ON_COMMAND(ID_VIEW_PAGEINFVIEW, &CMainFrame::OnViewPageInfoPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PAGEINFVIEW, &CMainFrame::OnUpdateViewPageInfoPane)
	ON_COMMAND(ID_VIEW_NUMERICALVIEW, &CMainFrame::OnViewNumericalPane)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NUMERICALVIEW, &CMainFrame::OnUpdateViewNumericalPane)

	ON_MESSAGE(UWM_CONTEXTMENU, &CMainFrame::OnShowContextMenu)
	ON_COMMAND(ID_FILE_CLOSEALL, &CMainFrame::OnFileCloseall)
	ON_COMMAND(ID_FILE_CLOSEOTHER, &CMainFrame::OnFileCloseother)
	ON_COMMAND(ID_PAGE_COUNT, &CMainFrame::OnGotoPage)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSEOTHER, &CMainFrame::OnUpdateFileCloseother)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGING_ACTIVE_TAB, &CMainFrame::OnAfxWmChangingActiveTab)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ステータス ライン インジケーター
	ID_INDICATOR_POSTEXT,
	ID_INDICATOR_PAPER,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};

// CMainFrame コンストラクション/デストラクション

CMainFrame::CMainFrame()
{
	// TODO: メンバー初期化コードをここに追加してください。
	m_nToolBarId = 0;

	m_pComboZoom = NULL;
	m_pStaticPage = NULL;

	m_iCurrentTabCount = 0;
}

CMainFrame::~CMainFrame()
{
	WORD wID = 0;
	CMenu* pMenu = NULL;
	POSITION pos = m_mapMenu.GetStartPosition();
	while( pos != NULL )
	{
		m_mapMenu.GetNextAssoc(pos, wID, (void*&)pMenu);
		if( pMenu )
		{
			_delete(pMenu);
		}
	}
	//m_wndMDIClient.Detach();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//CMDIFrameWndEx::m_bDoSubclass = FALSE;

	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	//if( m_wndMDIClient.Attach(m_hWndMDIClient) == 0 )
	//{
	//	TRACE(_T("Failed to attach MDIClient.\n"));
	//	return -1;
	//}

	BOOL bNameValid;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_tabLocation = CMFCTabCtrl::Location::LOCATION_TOP;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_SCROLLED; // 使用可能なその他の視覚スタイル...
	mdiTabParams.m_bTabIcons = TRUE;    // MDI タブでドキュメント アイコンを有効にするには、TRUE に設定します
	mdiTabParams.m_bTabCloseButton = TRUE;
	mdiTabParams.m_bTabCustomTooltips = TRUE;
	mdiTabParams.m_bAutoColor = FALSE;    // MDI タブの自動色設定を無効にするには、FALSE に設定します
	mdiTabParams.m_bDocumentMenu = TRUE; // タブ領域の右端にあるドキュメント メニューを有効にします
	mdiTabParams.m_bEnableTabSwap = FALSE;
	mdiTabParams.m_bFlatFrame = TRUE;
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // タブ領域の右部に [閉じる] ボタンを配置するには、FALSE に設定します
	mdiTabParams.m_nTabBorderSize = 2;
	//mdiTabParams.m_bReuseRemovedTabGroups = FALSE;
	//BOOL m_bTabIcons;
	//BOOL m_bTabCloseButton;
	//BOOL m_bTabCustomTooltips;
	//BOOL m_bAutoColor;
	//BOOL m_bDocumentMenu;
	//BOOL m_bEnableTabSwap;
	//BOOL m_bFlatFrame;
	//BOOL m_bActiveTabCloseButton;
	//int  m_nTabBorderSize;
	//BOOL m_bReuseRemovedTabGroups;

	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("メニュー バーを作成できませんでした\n");
		return -1;      // 作成できませんでした。
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_TOOLTIPS );

	// アクティブになったときメニュー バーにフォーカスを移動しない
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.Create(this, IDR_MAINFRAME) )
	{
		TRACE0("ツール バーの作成に失敗しました。\n");
		return -1;      // 作成できませんでした。
	}
	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	if( !m_wndToolBarDoc.Create(this, IDR_w01drawTYPE) )
	{
		TRACE0("ツール バーの作成に失敗しました。\n");
		return -1;      // 作成できませんでした。
	}
	m_wndToolBarDoc.SetWindowText(strToolBarName);

	CString szMsg;
	CSize size;
	size.SetSize(90, 2);

	const CFont& menuFont = m_wndMenuBar.GetMenuFont();

	// Zoom
	CComboBox* pZoom = (CComboBox*)m_wndToolBarDoc.CreateControl(RUNTIME_CLASS(CComboBox), _T(""), ID_VIEW_ZOOMCOMBO, size, CBS_DROPDOWN, &menuFont);
	if( pZoom )
	{
		// 10% 25% 50% 75% 100% 125% 150% 200% 400% 800% 1600% 2400% 3200% 6400%
		for(UINT uId = IDS_ZOOM_10; uId <= IDS_ZOOM_6400; uId++)
		{
			szMsg.LoadString(uId);

			int nIndex = pZoom->AddString(szMsg);
			if( nIndex > -1 )
			{
				pZoom->SetItemData(nIndex, uId);
			}
		}
		
		m_pComboZoom = pZoom;
	}
	// Page Number
	size.SetSize(70, 0);
	CStatic* pPageCount = (CStatic*)m_wndToolBarDoc.CreateControl(RUNTIME_CLASS(CStatic), _T("/"), ID_PAGE_COUNT, size, SS_NOTIFY | SS_CENTER | SS_CENTERIMAGE, &menuFont);
	if( pPageCount )
	{
		pPageCount->SetWindowText(_T(""));

		m_pStaticPage = pPageCount;
	}
	
	if( theApp.GetEnableDebug() )
	{
		CString strDebugName;
		bNameValid = strDebugName.LoadString(IDS_TOOLBAR_DEBUG);
		ASSERT(bNameValid);
		if( !m_wndToolBarDebug.Create(this, IDR_DEBUG) )
		{
			TRACE0("ツール バーの作成に失敗しました。\n");
			return -1;      // 作成できませんでした。
		}
		m_wndToolBarDebug.SetWindowText(strDebugName);

		// Overlay Text prompt
		size.SetSize(120, 2);
		CStatic* pOvlText = (CStatic*)m_wndToolBarDebug.CreateControl(RUNTIME_CLASS(CStatic), _T(""), ID_TEXT_OVLTEXTCOLOR, size, SS_RIGHT | SS_CENTERIMAGE, &menuFont);
		if( pOvlText )
		{
			szMsg.LoadString(ID_TEXT_OVLTEXTCOLOR);

			pOvlText->SetWindowText(szMsg);
		}

		// オーバーレイ文字色
		size.SetSize(110, 2);
		// Zoom
		CComboBox* pComColor = (CComboBox*)m_wndToolBarDebug.CreateControl(RUNTIME_CLASS(CComboBox), _T(""), ID_COMBO_OVLTXTCOLOR, size, CBS_DROPDOWN, &menuFont);
		if( pComColor )
		{
			int nIndex = 0;

			for(UINT uId = ID_COLOR_AUTO; uId <= ID_COLOR_CYAN; uId++)
			{
				szMsg.LoadString(uId);

				nIndex = pComColor->AddString(szMsg);
				if( nIndex > -1 )
				{
					pComColor->SetItemData(nIndex, uId);
				}
			}
			// AUTO
			pComColor->SetCurSel(0);

			m_pComboColor = pComColor;
		}
	}

	// ステータス バー
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("ステータス バーの作成に失敗しました。\n");
		return -1;      // 作成できない場合
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POSTEXT, SBPS_NORMAL, 90);
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_PAPER, SBPS_NORMAL, 130);
	m_wndStatusBar.SetPaneTextColor(1);
	m_wndStatusBar.SetPaneTextColor(2);
	m_wndStatusBar.SetPaneBackgroundColor(1);
	m_wndStatusBar.SetPaneBackgroundColor(2);

	EnableDocking(CBRS_ALIGN_ANY);

	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndToolBarDoc);
	if( theApp.GetEnableDebug() )
	{
		DockPane(&m_wndToolBarDebug);
	}
	else
	{
		// Remove DEBUG
	}

	// ドッキング ウィンドウを作成します
	if (!CreateDockingWindows())
	{
		TRACE0("ドッキング ウィンドウを作成できませんでした\n");
		return -1;
	}

	m_wndDirectory.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndDirectory);

	if( theApp.GetEnableDebug() )
	{
		m_wndDocInfoProp.EnableDocking(CBRS_ALIGN_ANY);
		m_wndPageInfoProp.EnableDocking(CBRS_ALIGN_ANY);

		CDockablePane* pTabbedBar = NULL;
		m_wndDocInfoProp.AttachToTabWnd(&m_wndDirectory, DM_SHOW, FALSE, &pTabbedBar);
		m_wndPageInfoProp.AttachToTabWnd(&m_wndDirectory, DM_SHOW, FALSE, &pTabbedBar);

		// --------------------------------------------
		//                      +--------------------+
		//                      |                    |
		//                      | m_wndFontProp      |
		//                      |                    |
		//                      +--------------------+
		//                      |                    |
		//                      | m_wndFxmlProp      |
		//                      |                    |
		//                      +--------------------+
		// --------------------------------------------
		m_wndFontProp.EnableDocking(CBRS_ALIGN_ANY);
		m_wndFxmlProp.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndFontProp);
		DockPane(&m_wndFxmlProp);

		// 文字情報
		m_wndTextProp.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndTextProp);

		RECT fxmlRect;
		SetRect(&fxmlRect, 0, 0, 250, 300);
		m_wndFxmlProp.DockToWindow(&m_wndFontProp, CBRS_ALIGN_BOTTOM, &fxmlRect);

		m_wndNumerialProp.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndNumerialProp);
	}

	ShowPane(&m_wndToolBarDoc, FALSE, TRUE, FALSE);

	if( theApp.GetEnableDebug() )
	{
		// 初期化非表示
		ShowPane(&m_wndTextProp, FALSE, TRUE, FALSE);
		ShowPane(&m_wndFxmlProp, FALSE, TRUE, FALSE);
		ShowPane(&m_wndFontProp, FALSE, TRUE, FALSE);
		ShowPane(&m_wndPageInfoProp, FALSE, TRUE, FALSE);
		ShowPane(&m_wndDocInfoProp, FALSE, TRUE, FALSE);
		ShowPane(&m_wndToolBarDebug, FALSE, TRUE, FALSE);
		ShowPane(&m_wndNumerialProp, FALSE, TRUE, FALSE);
	}
	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		if( pDockState->m_bShowFileView )
			ShowPane(&m_wndDirectory, TRUE, FALSE, TRUE);
		else
			ShowPane(&m_wndDirectory, FALSE, TRUE, FALSE);
	}

	RecalcLayout();

	// ウィンドウ タイトル バーでドキュメント名とアプリケーション名の順序を切り替えます。これにより、
	// ドキュメント名をサムネイルで表示できるため、タスク バーの使用性が向上します。
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;
	// ファイル ビューを作成します
	CString strFileView;
	bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndDirectory.Create(strFileView, this, CRect(0, 0, 250, 400), TRUE, ID_VIEW_FILEVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOATING))
	{
		TRACE0("ファイル ビュー ウィンドウを作成できませんでした\n");
		return FALSE; // 作成できませんでした
	}

	if( theApp.GetEnableDebug() )
	{
		// テキストプロパティ ウィンドウを作成します
		CString strTextWndName;
		bNameValid = strTextWndName.LoadString(IDS_TEXT_INFO);
		ASSERT(bNameValid);
		if (!m_wndTextProp.Create(strTextWndName, this, CRect(0, 0, 640, 200), TRUE, ID_VIEW_TEXTVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}

		// フォントプロパティ ウィンドウを作成します
		CString strFontWndName;
		bNameValid = strFontWndName.LoadString(IDS_FONT_INFO);
		ASSERT(bNameValid);
		if (!m_wndFontProp.Create(strFontWndName, this, CRect(0, 0, 350, 350), TRUE, ID_VIEW_FONTVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}
	
		CString strFontXmlWndName;
		bNameValid = strFontXmlWndName.LoadString(IDS_XMLF_INFO);
		ASSERT(bNameValid);
		if (!m_wndFxmlProp.Create(strFontXmlWndName, this, CRect(0, 0, 350, 500), TRUE, ID_VIEW_FONTXMLVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}

		CString strDocInfoWndName;
		bNameValid = strDocInfoWndName.LoadString(IDS_DOCINF_INFO);
		ASSERT(bNameValid);
		if (!m_wndDocInfoProp.Create(strDocInfoWndName, this, CRect(0, 0, 350, 500), TRUE, ID_VIEW_DOCINFVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}

		CString strPageInfoWndName;
		bNameValid = strPageInfoWndName.LoadString(IDS_PAGE_INFO);
		ASSERT(bNameValid);
		if (!m_wndPageInfoProp.Create(strPageInfoWndName, this, CRect(0, 0, 350, 500), TRUE, ID_VIEW_PAGEINFVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}

		CString strNumericalWndName;
		bNameValid = strNumericalWndName.LoadString(IDS_NUMERICAL_INFO);
		ASSERT(bNameValid);
		if (!m_wndNumerialProp.Create(strNumericalWndName, this, CRect(0, 0, 350, 500), TRUE, ID_VIEW_NUMERICALVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
		{
			TRACE0("プロパティ ウィンドウを作成できませんでした\n");
			return FALSE; // 作成できませんでした
		}
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);

	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndDirectory.SetIcon(hFileViewIcon, FALSE);

	if( theApp.GetEnableDebug() )
	{
		HICON hDocInfIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DOCINF_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_wndDocInfoProp.SetIcon(hDocInfIcon, FALSE);

		//HICON hFontViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FONT_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		//m_wndFontProp.SetIcon(hFontViewIcon, FALSE);

		//HICON hFontXmlViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FONTXML_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		//m_wndFxmlProp.SetIcon(hFontXmlViewIcon, FALSE);
	}

	UpdateMDITabbedBarsIcons();
}

// CMainFrame 診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame メッセージ ハンドラー

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// 基本クラスが実際の動作を行います。

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}
	// すべてのユーザー定義ツール バーのボタンのカスタマイズを有効にします
	//BOOL bNameValid;
	//CString strCustomize;
	//bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	//ASSERT(bNameValid);

	//for (int i = 0; i < iMaxUserToolbars; i ++)
	//{
	//	CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
	//	if (pUserToolbar != NULL)
	//	{
	//		pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	//	}
	//}

	return TRUE;
}

void CMainFrame::OnViewNumericalPane()
{
	BOOL bShow = FALSE;

	if( !(m_wndNumerialProp.GetStyle () & WS_VISIBLE ) )
		bShow = TRUE;

	ShowPane (&m_wndNumerialProp, bShow, FALSE, TRUE);

	RecalcLayout ();

	if( bShow )
	{
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			m_wndNumerialProp.SetWindow(pChild->GetActiveView());
		}
	}

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowNumView = FALSE;
		if( bShow )
			pDockState->m_bShowNumView = TRUE;
	}
}

void CMainFrame::OnUpdateViewNumericalPane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndNumerialProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewPageInfoPane()
{
	BOOL bShow = FALSE;

	if( !(m_wndPageInfoProp.GetStyle () & WS_VISIBLE ) )
		bShow = TRUE;

	ShowPane (&m_wndPageInfoProp, bShow, FALSE, TRUE);

	RecalcLayout ();

	if( bShow )
	{
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			m_wndPageInfoProp.SetWindow(pChild->GetActiveView());
		}
	}

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowPageInfo = FALSE;
		if( m_wndPageInfoProp.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowPageInfo = TRUE;
	}
}

void CMainFrame::OnUpdateViewPageInfoPane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndPageInfoProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewDocInfoPane()
{
	BOOL bShow = FALSE;

	if( !(m_wndDocInfoProp.GetStyle () & WS_VISIBLE ) )
		bShow = TRUE;

	ShowPane (&m_wndDocInfoProp, bShow, FALSE, TRUE);

	RecalcLayout ();

	if( bShow )
	{
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			m_wndDocInfoProp.SetWindow(pChild->GetActiveView());
		}
	}

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowDocInfo = FALSE;
		if( m_wndDocInfoProp.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowDocInfo = TRUE;
	}
}

void CMainFrame::OnUpdateViewDocInfoPane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndDocInfoProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewFontXmlPane()
{
	ShowPane (&m_wndFxmlProp,
					!(m_wndFxmlProp.GetStyle () & WS_VISIBLE),
					FALSE,TRUE);
	RecalcLayout ();

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowFxmlView = FALSE;
		if( m_wndFxmlProp.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowFxmlView = TRUE;
	}
}

void CMainFrame::OnUpdateViewFontXmlPane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndFxmlProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewFontPane()
{
	BOOL bShow = FALSE;
	if( !(m_wndFontProp.GetStyle () & WS_VISIBLE ) )
		bShow = TRUE;
		
	ShowPane(&m_wndFontProp, bShow, FALSE,TRUE);

	RecalcLayout ();

	if( bShow )
	{
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			m_wndFontProp.SetWindow(pChild->GetActiveView());
		}
	}

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowFontView = FALSE;
		if( m_wndFontProp.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowFontView = TRUE;
	}
}

void CMainFrame::OnUpdateViewFontPane(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndFontProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewTextList()
{
	BOOL bShow = FALSE;
	if( !(m_wndTextProp.GetStyle () & WS_VISIBLE ) )
		bShow = TRUE;
		
	ShowPane(&m_wndTextProp, bShow, FALSE,TRUE);

	RecalcLayout ();

	if( bShow )
	{
		// Get the active MDI child window.
		CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			m_wndTextProp.SetWindow(pChild->GetActiveView());
		}
	}

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowTextView = FALSE;
		if( bShow )
			pDockState->m_bShowTextView = TRUE;
	}
}

void CMainFrame::OnUpdateViewTextList(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndTextProp.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewDebug()
{
	ShowPane (&m_wndToolBarDebug,
					!(m_wndToolBarDebug.GetStyle () & WS_VISIBLE),
					FALSE,TRUE);
	RecalcLayout ();

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowDbgToolbar = FALSE;
		if( m_wndToolBarDebug.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowDbgToolbar = TRUE;
	}
}

void CMainFrame::OnUpdateViewDebug(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndToolBarDebug.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewFilectrl()
{
	ShowPane (&m_wndDirectory,
					!(m_wndDirectory.GetStyle () & WS_VISIBLE),
					FALSE,TRUE);
	RecalcLayout ();

	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		pDockState->m_bShowFileView = FALSE;
		if( m_wndDirectory.GetStyle () & WS_VISIBLE )
			pDockState->m_bShowFileView = TRUE;
	}
}


void CMainFrame::OnUpdateViewFilectrl(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_wndDirectory.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnIdleUpdateCmdUI()
{
	CMDIChildWnd* pChild = MDIGetActive();
	if( pChild == NULL )
	{
		SelectToolBar(IDR_MAINFRAME);
		CMDIFrameWnd::OnIdleUpdateCmdUI();

		return;
	}

	CView* pView = pChild->GetActiveView();
	if( pView == NULL )
		return;

	CDocument* pDoc = pView->GetDocument();
	if( pDoc == NULL )
		return;

	CDocTemplate* pDocTemplate = pDoc->GetDocTemplate();
	if( pDocTemplate )
	{
		class CHelperDocTemplate : public CDocTemplate
		{
		public:
			CHelperDocTemplate() : CDocTemplate(0, NULL, NULL, NULL) {}
			UINT GetResourceId() { return m_nIDResource; }
		};

		CHelperDocTemplate* pHelper = (CHelperDocTemplate*)pDocTemplate;
		UINT nId = pHelper->GetResourceId();
		if( SelectToolBar(nId) )
			return;
	}
	SelectToolBar(IDR_MAINFRAME);

	CMDIFrameWnd::OnIdleUpdateCmdUI();
}


BOOL CMainFrame::SelectToolBar(UINT nResourceId)
{
	if( nResourceId == m_nToolBarId )
		return TRUE;

	CBasePane* pPane = GetPane(nResourceId);
	if( pPane == NULL )
		return FALSE;

	CBasePane* pCurr = GetPane(AFX_IDW_TOOLBAR);
	if( pCurr == NULL )
	{
		pPane->SetDlgCtrlID(AFX_IDW_TOOLBAR);
		m_nToolBarId = nResourceId;
		ShowPane(pPane, TRUE, FALSE, TRUE);

		return FALSE;
	}

	// Switch
	pCurr->SetDlgCtrlID(m_nToolBarId);
	pPane->SetDlgCtrlID(AFX_IDW_TOOLBAR);

	m_nToolBarId = nResourceId;

	BOOL bVisible = pCurr->IsWindowVisible();
	if( bVisible )
	{
		ShowPane(pCurr, FALSE, TRUE, FALSE);
		ShowPane(pPane, TRUE, FALSE, TRUE);
	}

	if( m_nToolBarId == IDR_MAINFRAME )
	{
		if( theApp.GetEnableDebug() )
		{
			ShowPane(&m_wndFontProp, FALSE, TRUE, FALSE);
			ShowPane(&m_wndFxmlProp, FALSE, TRUE, FALSE);
			ShowPane(&m_wndTextProp, FALSE, TRUE, FALSE);
			ShowPane(&m_wndDocInfoProp, FALSE, TRUE, FALSE);
			ShowPane(&m_wndPageInfoProp, FALSE, TRUE, FALSE);
			ShowPane(&m_wndNumerialProp, FALSE, TRUE, FALSE);
			
			ShowPane(&m_wndToolBarDebug, FALSE, TRUE, FALSE);
		}

		ShowPane(&m_wndToolBarDoc, FALSE, TRUE, FALSE);

		LPDOCKSTATE pDockState = theApp.GetDockState();
		if( pDockState )
		{
			if( !pDockState->m_bShowFileView )
				ShowPane(&m_wndDirectory, FALSE, TRUE, FALSE);
		}
	}
	else if( m_nToolBarId == IDR_w01drawTYPE )
	{
		LPDOCKSTATE pDockState = theApp.GetDockState();
		if( pDockState )
		{
			if( pDockState->m_bShowFileView )
				ShowPane(&m_wndDirectory, TRUE, FALSE, TRUE);
			else
				ShowPane(&m_wndDirectory, FALSE, TRUE, FALSE);

			if( theApp.GetEnableDebug() )
			{
				if( pDockState->m_bShowFontView )
					ShowPane(&m_wndFontProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndFontProp, FALSE, TRUE, FALSE);

				if( pDockState->m_bShowFxmlView )
					ShowPane(&m_wndFxmlProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndFxmlProp, FALSE, TRUE, FALSE);

				if( pDockState->m_bShowTextView )
					ShowPane(&m_wndTextProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndTextProp, FALSE, TRUE, FALSE);
		
				if( pDockState->m_bShowDbgToolbar )
					ShowPane(&m_wndToolBarDebug, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndToolBarDebug, FALSE, TRUE, FALSE);

				if( pDockState->m_bShowDocInfo )
					ShowPane(&m_wndDocInfoProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndDocInfoProp, FALSE, TRUE, FALSE);

				if( pDockState->m_bShowPageInfo )
					ShowPane(&m_wndPageInfoProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndPageInfoProp, FALSE, TRUE, FALSE);

				if( pDockState->m_bShowNumView )
					ShowPane(&m_wndNumerialProp, TRUE, FALSE, TRUE);
				else
					ShowPane(&m_wndNumerialProp, FALSE, TRUE, FALSE);
				
			}			
		}
	}
	RecalcLayout ();

	return TRUE;
}

void CMainFrame::OnGotoPage()
{
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
	if( pChild )
	{
		CView* pActiveView = pChild->GetActiveView();
		if( pActiveView )
		{
			pActiveView->PostMessage(UWM_DOC_GOTO, 0, 0);
		}
	}
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_RBUTTONDOWN:
		{
			CWnd* pWnd = FromHandle(pMsg->hwnd);
			CMFCTabCtrl* pTabGroupCtl = dynamic_cast<CMFCTabCtrl*>(pWnd);
			if( pTabGroupCtl )
			{
				CPoint pt = pMsg->pt;
				pTabGroupCtl->ScreenToClient(&pt);

				int tabIndex = pTabGroupCtl->GetTabFromPoint(pt);
				if( tabIndex != -1 )
				{
					CWnd* pTabCtl = pTabGroupCtl->GetTabWnd(tabIndex);
					if( pTabCtl )
					{
						pTabGroupCtl->SetActiveTab(tabIndex);
						return SendMessage(UWM_CONTEXTMENU, (WPARAM)pTabCtl->GetSafeHwnd(), (LPARAM)MAKELPARAM(pMsg->pt.x, pMsg->pt.y));
//						pTabCtl->SendMessage(WM_CLOSE, 0, 0);
					}
				}
			}
		}
		break;
	default:
		break;
	}

	return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}


LRESULT CMainFrame::OnShowContextMenu(WPARAM wParam, LPARAM lParam)
{
	CPoint point;
	point.x = GET_X_LPARAM(lParam);
	point.y = GET_Y_LPARAM(lParam);

	TRACE(_T("OnShowContextMenu\n"));
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_TAB);

	CMenu* pMenu;
	pMenu = menu.GetSubMenu(0);
	if( pMenu )
	{
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

		PostMessage(WM_NULL, 0, 0);
	}

	return 0L;
}


void CMainFrame::OnFileCloseall()
{
	const CObList& tabGroups = GetMDITabGroups();
	if( tabGroups.GetCount() > 0 )
	{
		CMFCTabCtrl* pTabCtl = NULL;
		POSITION pos = tabGroups.GetHeadPosition();
		do
		{
			pTabCtl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos));
			if( pTabCtl )
			{
				int nActive = pTabCtl->GetActiveTab();
				pTabCtl->LockWindowUpdate();
				int nTabsNum = pTabCtl->GetVisibleTabsNum();
				for(int i = (nTabsNum - 1); i >= 0; i--)
				{
					if( i == nActive ||
						pTabCtl->SetActiveTab(i) )
					{
						// 閉じる
						SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
					}
				}
				pTabCtl->UnlockWindowUpdate();
				pTabCtl->Invalidate();
			}

		} while( pos != NULL );		
	}
}


void CMainFrame::OnFileCloseother()
{
	const CObList& tabGroups = GetMDITabGroups();
	if( tabGroups.GetCount() > 0 )
	{
		CMFCTabCtrl* pTabCtl = NULL;
		POSITION pos = tabGroups.GetHeadPosition();
		do
		{
			pTabCtl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos));
			if( pTabCtl )
			{
				int nActive = pTabCtl->GetActiveTab();
				pTabCtl->LockWindowUpdate();
				int nTabsNum = pTabCtl->GetVisibleTabsNum();
				for(int i = (nTabsNum - 1); i >= 0; i--)
				{
					// 自分以外
					if( nActive != i )
					{
						if( pTabCtl->SetActiveTab(i) )
						{
							// 閉じる
							SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
						}
					}
				}
				pTabCtl->UnlockWindowUpdate();
				pTabCtl->Invalidate();
			}

		} while( pos != NULL );		
	}
}

void CMainFrame::OnUpdateFileCloseother(CCmdUI *pCmdUI)
{
	int nTabsNum = 0;
	const CObList& tabGroups = GetMDITabGroups();
	if( tabGroups.GetCount() > 0 )
	{
		CMFCTabCtrl* pTabCtl = NULL;
		POSITION pos = tabGroups.GetHeadPosition();
		do
		{
			pTabCtl = DYNAMIC_DOWNCAST(CMFCTabCtrl, tabGroups.GetNext(pos));
			if( pTabCtl )
			{
				int nNums = pTabCtl->GetVisibleTabsNum();
				if( nTabsNum < nNums )
					nTabsNum = nNums;
			}

		} while( pos != NULL );		
	}

	pCmdUI->Enable(nTabsNum > 1);
}

LRESULT CMainFrame::OnAfxWmChangingActiveTab(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("OnAfxWmChangingActiveTab: wp: %d, lp:%d\n"), wParam, lParam);
	CMFCTabCtrl* pTab = (CMFCTabCtrl*)lParam;
	int iTabsCount = pTab->GetTabsNum();

    CWnd * pWnd = pTab->GetTabWndNoWrapper( (int)wParam ); 

	CMDIChildWnd * pChild = dynamic_cast<CMDIChildWnd*>(pWnd);
	if (!pChild )
		return 0;

	CView* pView = pChild->GetActiveView();

	if(!pView )
		return NULL;

	if( theApp.GetEnableDebug() )
	{
		CTextPropertiesWnd* pTextWnd = GetTextPropertiesWnd();
		if( pTextWnd && pTextWnd->IsWindowVisible() )
		{
			pTextWnd->SetWindow(pView);
		}
		CFontPropertiesWnd* pFontWnd = GetFontPropertiesWnd();
		if( pFontWnd && pFontWnd->IsWindowVisible() )
		{
			pFontWnd->SetWindow(pView);
		}
		CDocInfoPropertiesWnd* pDocInfWnd = GetDocInfoPropertiesWnd();
		if( pDocInfWnd && pDocInfWnd->IsWindowVisible() )
		{
			pDocInfWnd->SetWindow(pView);
		}
		CPageInfoPropertiesWnd* pPageInfWnd = GetPageInfoPropertiesWnd();
		if( pPageInfWnd )
		{
			pPageInfWnd->SetWindow(pView);
		}
		CNumericalPropertiesWnd* pNumericalWnd = GetNumericalPropertiesWnd();
		if( pNumericalWnd )
		{
			pNumericalWnd->SetWindow(pView);
		}
	}
	pView->SetFocus();
	pView->PostMessage(UWM_DOC_UPDATEUI, 0, 0);

	return 0;
}


BOOL CMainFrame::OnCloseDockingPane(CDockablePane* pWnd)
{
	UINT uID = 0;
	if( pWnd->IsKindOf(RUNTIME_CLASS(CTabbedPane)))
	{
		CTabbedPane* pTabPane = (CTabbedPane*)pWnd;
		CMFCBaseTabCtrl* pTabWnd = pTabPane->GetUnderlyingWindow();
		int num = pTabWnd->GetActiveTab();
		if( num >= 0 )
		{
			CDockablePane* pDock = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetTabWnd(num));
			if( pDock != NULL )
			{
				uID = pDock->GetDlgCtrlID();
			}
		}
	}
	else
	{
		CDockablePane* pDock = (CDockablePane*)pWnd;
		if( pDock->IsKindOf(RUNTIME_CLASS(CDockablePane)) ||
			pDock->IsKindOf(RUNTIME_CLASS(CPane)) && !pDock->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
		{
			uID = pDock->GetDlgCtrlID();
		}
	}
	//// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		switch(uID)
		{
			case ID_VIEW_FILEVIEW:
				pDockState->m_bShowFileView = FALSE;
				break;
			case ID_VIEW_TEXTVIEW:
				{
					pDockState->m_bShowTextView = FALSE;
					CTextPropertiesWnd* pPanel = GetTextPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_FONTVIEW:
				{
					pDockState->m_bShowFontView  = FALSE;
					CFontPropertiesWnd* pPanel = GetFontPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_FONTXMLVIEW:
				{
					pDockState->m_bShowFxmlView = FALSE;
					CFontXmlPropertiesWnd* pPanel =  GetFontXmlPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_DOCINFVIEW:
				{
					pDockState->m_bShowDocInfo = FALSE;
					CDocInfoPropertiesWnd* pPanel = GetDocInfoPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_PAGEINFVIEW:
				{
					pDockState->m_bShowPageInfo = FALSE;
					CPageInfoPropertiesWnd* pPanel = GetPageInfoPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_NUMERICALVIEW:
				{
					pDockState->m_bShowNumView = FALSE;
					CNumericalPropertiesWnd* pNumericalnd = GetNumericalPropertiesWnd();
					if( pNumericalnd )
					{
						pNumericalnd->Refresh(TRUE);
					}
				}
				break;
		}
	}

	return CMDIFrameWndEx::OnCloseDockingPane(pWnd);
}


BOOL CMainFrame::OnCloseMiniFrame(CPaneFrameWnd* pWnd)
{
	UINT uID = 0;
	CWnd* pPane = pWnd->GetPane();
	if( pPane->IsKindOf(RUNTIME_CLASS(CTabbedPane)))
	{
		// Multi-Windows
		CTabbedPane* pTabPane = (CTabbedPane*)pPane;
		CMFCBaseTabCtrl* pTabWnd = pTabPane->GetUnderlyingWindow();
		int num = pTabWnd->GetActiveTab();
		if( num >= 0 )
		{
			CDockablePane* pDock = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetTabWnd(num));
			if( pDock != NULL )
			{
				uID = pDock->GetDlgCtrlID();
			}
		}
	}
	else
	{
		// Single-Window
		CDockablePane* pDock = (CDockablePane*)pWnd;
		if( pDock->IsKindOf(RUNTIME_CLASS(CDockablePane)) ||
			pDock->IsKindOf(RUNTIME_CLASS(CPane)) && pDock->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			uID = pDock->GetDlgCtrlID();
		}
	}
	LPDOCKSTATE pDockState = theApp.GetDockState();
	if( pDockState )
	{
		switch(uID)
		{
			case ID_VIEW_FILEVIEW:
				pDockState->m_bShowFileView = FALSE;
				break;
			case ID_VIEW_TEXTVIEW:
				{
					pDockState->m_bShowTextView = FALSE;
					CTextPropertiesWnd* pPanel = GetTextPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_FONTVIEW:
				{
					pDockState->m_bShowFontView  = FALSE;
					CFontPropertiesWnd* pPanel = GetFontPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_FONTXMLVIEW:
				{
					pDockState->m_bShowFxmlView = FALSE;
					CFontXmlPropertiesWnd* pPanel =  GetFontXmlPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_DOCINFVIEW:
				{
					pDockState->m_bShowDocInfo = FALSE;
					CDocInfoPropertiesWnd* pPanel = GetDocInfoPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_PAGEINFVIEW:
				{
					pDockState->m_bShowPageInfo = FALSE;
					CPageInfoPropertiesWnd* pPanel = GetPageInfoPropertiesWnd();
					if( pPanel )
					{
						pPanel->Refresh(TRUE);
					}
				}
				break;
			case ID_VIEW_NUMERICALVIEW:
				{
					pDockState->m_bShowNumView = FALSE;
					CNumericalPropertiesWnd* pNumericalnd = GetNumericalPropertiesWnd();
					if( pNumericalnd )
					{
						pNumericalnd->Refresh(TRUE);
					}
				}
				break;
		}
	}

	return CMDIFrameWndEx::OnCloseMiniFrame(pWnd);
}


void CMainFrame::RefreshDockPane()
{
	if( !theApp.GetEnableDebug() )
		return;

	CTextPropertiesWnd* pTextWnd = GetTextPropertiesWnd();
	if( pTextWnd )
	{
		if( (pTextWnd->GetStyle () & WS_VISIBLE) )
			pTextWnd->Refresh();
	}

	CFontPropertiesWnd* pFontWnd = GetFontPropertiesWnd();
	if( pFontWnd )
	{
		if( (pFontWnd->GetStyle () & WS_VISIBLE) )
			pFontWnd->Refresh();
	}

	CPageInfoPropertiesWnd* pPageWnd = GetPageInfoPropertiesWnd();
	if( pPageWnd )
	{
		if( (pPageWnd->GetStyle () & WS_VISIBLE) )
			pPageWnd->Refresh();
	}

	CDocInfoPropertiesWnd* pDocWnd = GetDocInfoPropertiesWnd();
	if( pPageWnd )
	{
		if( (pPageWnd->GetStyle () & WS_VISIBLE) )
			pPageWnd->Refresh();
	}
	CNumericalPropertiesWnd* pNumericalnd = GetNumericalPropertiesWnd();
	if( pNumericalnd )
	{
		if( (pNumericalnd->GetStyle () & WS_VISIBLE) )
			pNumericalnd->Refresh();
	}
}

void CMainFrame::SetCursorPosText(CPoint pt, BOOL bClear)
{
	TCHAR szText[64] = {0};
	if( !bClear )
	{
		_stprintf_s(szText, 63, _T("%d, %dpx"), pt.x, pt.y);
	}
	m_wndStatusBar.SetPaneText(1, szText);
}

void CMainFrame::SetPaper(int width, int height, BOOL bClear)
{
	TCHAR szText[64] = {0};
	if( !bClear )
	{
		_stprintf_s(szText, 63, _T("%d x %dpx"), width, height);
	}
	m_wndStatusBar.SetPaneText(2, szText);
}


void CMainFrame::ModifyMenuBar(CWnd* pChild)
{
	if( theApp.GetEnableDebug() )
	{
		TRACE(_T("CMainFrame::ModifyMenuBar()\n"));

		CMenu* pMenu = NULL;
		WORD wID = IDR_w01drawDEBUG;
		if( !m_mapMenu.Lookup(wID, (void*&)pMenu) )
		{
			pMenu = new CMenu();
			pMenu->LoadMenu(wID);

			m_mapMenu.SetAt(wID, pMenu);
		}

		if( pMenu  )
		{
			m_wndMenuBar.CreateFromMenu(pMenu->GetSafeHmenu());
		}
	}
}
