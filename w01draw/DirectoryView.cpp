// DirectoryView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "DirectoryView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDirectoryView

IMPLEMENT_DYNAMIC(CDirectoryView, CDockablePane)

CDirectoryView::CDirectoryView()
{
}

CDirectoryView::~CDirectoryView()
{
}


BEGIN_MESSAGE_MAP(CDirectoryView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_PROPERTIES, &CDirectoryView::OnProperties)
	ON_COMMAND(ID_LIST_EXPAND, &CDirectoryView::OnListExpand)
	ON_NOTIFY(NM_DBLCLK, ID_VIEW_FILEVIEW, &CDirectoryView::OnFileListSelected)
END_MESSAGE_MAP()



// CDirectoryView メッセージ ハンドラー




int CDirectoryView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy (0, 0, 0, 0);
	const DWORD dwViewStyle =	WS_CHILD | WS_VISIBLE | TVS_HASLINES | 
								TVS_LINESATROOT | TVS_HASBUTTONS;

	m_wndShellTree.Create (dwViewStyle, rectDummy, this, ID_VIEW_FILEVIEW);
	DWORD dwFlags = m_wndShellTree.GetFlags();
	dwFlags |= SHCONTF_NONFOLDERS;
	m_wndShellTree.SetFlags(dwFlags);
	//m_wndShellTree.EnableShellContextMenu(FALSE);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* ロックされています*/);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();

	return 0;
}

void CDirectoryView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndShellTree.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDirectoryView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}


BOOL CDirectoryView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDockablePane::OnEraseBkgnd(pDC);
}

void CDirectoryView::OnProperties()
{
	// TODO: ここにコマンド ハンドラー コードを追加します。
}

void CDirectoryView::OnListExpand()
{
	HTREEITEM hItem = m_wndShellTree.GetSelectedItem();
	if( hItem )
	{
		if( m_wndShellTree.ItemHasChildren(hItem) )
		{
			m_wndShellTree.Expand(hItem, TVE_EXPAND);	// TVE_COLLAPSE
		}
	}
}

CString& CDirectoryView::GetSelectedPath()
{
	m_szDocName = _T("");
	HTREEITEM hItem = m_wndShellTree.GetSelectedItem();
	if( hItem != NULL )
	{
		LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)m_wndShellTree.GetItemData(hItem);
		if( pItem )
		{
			TCHAR szFullPath[_MAX_PATH] = {0};
			if( ::SHGetPathFromIDList(pItem->pidlFQ, szFullPath) )
			{
				m_szDocName = szFullPath;
			}
		}
	}
	return m_szDocName;
}

void CDirectoryView::OnFileListSelected(NMHDR* pNMHDR, LRESULT* pResult)
{
	TRACE(_T("OnFileListSelected\n"));
	//CString strPath;
	//if( m_wndShellTree.GetItemPath(strPath, NULL) )
	//{
	//	TRACE(_T("Path:%s\n"), strPath);
	//}
	CString lpPath = GetSelectedPath();
	if( !lpPath.IsEmpty() )
	{
		TRACE(_T("FullPath:%s\n"), lpPath);

		LPTSTR lpExt = ::PathFindExtension(lpPath);
		if( lpExt && _tcsicmp(lpExt, _T(".w01")) == 0 )
		{
			DWORD dwFileAttrs = ::GetFileAttributes(lpPath);
			if( dwFileAttrs & FILE_ATTRIBUTE_DIRECTORY )
			{
			}
			else
			{
				theApp.OpenDocumentFile(lpPath);
			}
		}
	}

	*pResult = 0;
}

BOOL CDirectoryView::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->hwnd == m_wndShellTree.GetSafeHwnd() &&
		pMsg->message == WM_KEYDOWN )
	{
		if( (pMsg->wParam == VK_RETURN && GetKeyState(VK_RETURN) < 0) )
		{
			CString lpPath = GetSelectedPath();
			if( !lpPath.IsEmpty() )
			{
				TRACE(_T("FullPath:%s\n"), lpPath);
			}

			return TRUE;
		}
	}

	return CDockablePane::PreTranslateMessage(pMsg);
}
