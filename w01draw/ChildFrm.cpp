
// ChildFrm.cpp : CChildFrame クラスの実装
//

#include "stdafx.h"
#include "w01draw.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

// CChildFrame コンストラクション/デストラクション

CChildFrame::CChildFrame()
{
	// TODO: メンバー初期化コードをここに追加してください。
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_MAXIMIZE | WS_VISIBLE;

	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	//WNDCLASS wc;
	//GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wc);

	//cs.lpszClass = AfxRegisterWndClass(wc.style & ~(CS_VREDRAW|CS_HREDRAW),
	//	wc.hCursor, wc.hbrBackground, wc.hIcon);
	//////cs.lpszClass = AfxRegisterWndClass(0, NULL, NULL, AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	////	
	//ASSERT(cs.lpszClass);

	return TRUE;
}

// CChildFrame 診断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame メッセージ ハンドラー


BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle , const RECT& rect , CMDIFrameWnd* pParentWnd , CCreateContext* pContext)
{
	if( pParentWnd == NULL )
	{
		CWnd* pMainWnd = AfxGetThread()->m_pMainWnd;
		ASSERT(pMainWnd != NULL);
		ASSERT_KINDOF(CMDIFrameWnd, pMainWnd);
		pParentWnd = (CMDIFrameWnd*)pMainWnd;
	}
	ASSERT(::IsWindow(pParentWnd->m_hWndMDIClient));

	// insure corrent windows positioning
	pParentWnd->RecalcLayout();

	// copy into a CREATESTRUCT for PreCreateWindow
	CREATESTRUCT cs = {0};
	cs.dwExStyle = 0;
	cs.lpszClass = lpszClassName;
	cs.lpszName = lpszWindowName;

	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.right - rect.left;
	cs.cy = rect.bottom - rect.top;

	cs.style = dwStyle;
	cs.hwndParent = pParentWnd->m_hWnd;
	cs.hMenu = NULL;
	cs.hInstance = AfxGetInstanceHandle();
	cs.lpCreateParams = (LPVOID)pContext;

	if( !PreCreateWindow(cs) )
	{
		PostNcDestroy();
		return FALSE;
	}

	// must not change the HWND
	ASSERT(cs.hwndParent == pParentWnd->m_hWnd);

	// copy into MDICREATESTRUCT for real create
	MDICREATESTRUCT mcs = {0};
	mcs.szClass = cs.lpszClass;
	mcs.szTitle = cs.lpszName;
	mcs.hOwner = cs.hInstance;
	
	mcs.x = rect.left;
	mcs.y = rect.top;
	mcs.cx = rect.right - rect.left;
	mcs.cy = rect.bottom - rect.top;

	mcs.style = cs.style;
	mcs.lParam = (LONG)cs.lpCreateParams;
	
	AfxHookWindowCreate(this);

	HWND hWnd = (HWND)::SendMessage(pParentWnd->m_hWndMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);

	if( !AfxUnhookWindowCreate())
		PostNcDestroy();

	if( hWnd == NULL )
		return FALSE;

	::SendMessage(hWnd, WM_INITIALUPDATE, 0, 0);

	ASSERT(hWnd == m_hWnd);

	return TRUE;
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	if( pActivateWnd == NULL ||
		pDeactivateWnd == NULL )
		CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if( !bActivate )
		return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if( pFrame && pDeactivateWnd == NULL )
	{
		pFrame->ModifyMenuBar(pActivateWnd);
	}
}
