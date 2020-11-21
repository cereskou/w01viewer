// ToolbarEx.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "ToolbarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CToolBarEx
void _freeChildWndPtr(CWnd* pWnd)
{
	if( pWnd )
	{
		if( pWnd->GetSafeHwnd()  )
			pWnd->DestroyWindow();
		_delete(pWnd);
	}
}

// CToolbarEx

IMPLEMENT_DYNAMIC(CToolbarEx, CMFCToolBar)

CToolbarEx::CToolbarEx()
{
	m_pLstChild = NULL;
}

CToolbarEx::~CToolbarEx()
{
	if( m_pLstChild )
	{
		delete m_pLstChild;
		m_pLstChild = NULL;
	}
}


BEGIN_MESSAGE_MAP(CToolbarEx, CMFCToolBar)
END_MESSAGE_MAP()



// CToolbarEx メッセージ ハンドラー
BOOL CToolbarEx::Create(CWnd* pParentWnd, UINT nResourceId, BOOL bAutoDelete)
{
	m_pLstChild = new LinkedList<CWnd*>(true, _freeChildWndPtr);
	if( !m_pLstChild )
		return FALSE;

	// Create toolbar
	const DWORD dwStyle = WS_CHILD | CBRS_TOP | CBRS_TOOLTIPS; // | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER | CBRS_FLYBY;
	const DWORD dwCtrlStyle = TBSTYLE_FLAT;// | TBSTYLE_TRANSPARENT;
	const CRect rect(0, 0, 0, 0);

	if( !CreateEx(pParentWnd, dwCtrlStyle, dwStyle, rect, nResourceId) ||
		!LoadToolBar(nResourceId) )
	{
		if( m_pLstChild )
		{
			delete m_pLstChild;
			m_pLstChild = NULL;
		}
		TRACE0("ツール バーの作成に失敗しました。\n");
		return FALSE;
	}

	// Change the toolbar style
	//const DWORD dwNewStyle =  CBRS_FLYBY | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC;
	//SetPaneStyle(GetPaneStyle() | dwNewStyle);

	return TRUE;
}

CWnd* CToolbarEx::CreateControl(CRuntimeClass* pClass, LPCTSTR lpszName, UINT nId, CSize size, DWORD dwStyle, const CFont* pFont)
{
	CWnd* pCtrl = NULL;
	int nIndex = CommandToIndex(nId);
	if( nIndex == -1 )
		return FALSE;

	LONG width = size.cx;
	CRect rect;
	SetButtonInfo(nIndex, nId, TBBS_SEPARATOR, width);
	GetItemRect(nIndex, &rect);

	CSize dockSize = CalcSize(FALSE);
	// Adjust position
	// size.cy = margin
	rect.left += size.cy;
//	rect.top  += size.cy;
	rect.right -= size.cy;
	rect.bottom -= size.cy;

	if( pClass->IsDerivedFrom(RUNTIME_CLASS(CComboBox)) )
	{
		pCtrl = new CComboBox;
		if( pCtrl == NULL )
			return NULL;

		if( !((CComboBox*)pCtrl)->Create(WS_CHILD | WS_VISIBLE | dwStyle, rect, this, nId) )
		{
			_delete(pCtrl);
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom(RUNTIME_CLASS(CEdit)) )
	{
		pCtrl = new CEdit;
		if( pCtrl == NULL )
			return NULL;

		if( !((CEdit*)pCtrl)->Create(WS_CHILD | WS_VISIBLE | dwStyle, rect, this, nId) )
		{
			_delete(pCtrl);
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom(RUNTIME_CLASS(CButton)) )
	{
		pCtrl = new CButton;
		if( pCtrl == NULL )
			return NULL;

		if( !((CButton*)pCtrl)->Create(lpszName, WS_CHILD | WS_VISIBLE | dwStyle, rect, this, nId) )
		{
			_delete(pCtrl);
			return NULL;
		}
	}
	else if( pClass->IsDerivedFrom(RUNTIME_CLASS(CStatic)) )
	{
		pCtrl = new CStatic;
		if( pCtrl == NULL )
			return NULL;

		if( !((CStatic*)pCtrl)->Create(lpszName, WS_CHILD | WS_VISIBLE | dwStyle, rect, this, nId) )
		{
			_delete(pCtrl);
			return NULL;
		}
	}
	dockSize = CalcSize(FALSE);
	// Add to List
	if( pCtrl )
	{
		m_pLstChild->add(pCtrl);

		CFont* pToolbarFont = (CFont*)pFont;
		if( pToolbarFont == NULL )
		{
			pToolbarFont = this->GetFont();
		}
		if( pToolbarFont )
		{
			pCtrl->SetFont(pToolbarFont);
		}
	}
	
	AdjustLayout();

	return pCtrl;
}



