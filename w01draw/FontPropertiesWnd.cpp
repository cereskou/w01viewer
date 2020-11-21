// FontPropertiesWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "FontPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_CTRL_FONTLIST	0x2002

// CFontPropertiesWnd

IMPLEMENT_DYNAMIC(CFontPropertiesWnd, CBasePropertiesWnd)

CFontPropertiesWnd::CFontPropertiesWnd()
{
}

CFontPropertiesWnd::~CFontPropertiesWnd()
{
}


BEGIN_MESSAGE_MAP(CFontPropertiesWnd, CBasePropertiesWnd)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_DOCFONT_SYNCTO, &CFontPropertiesWnd::OnFontSyncTo)
	ON_UPDATE_COMMAND_UI(ID_DOCFONT_SYNCTO, &CFontPropertiesWnd::OnUpdateFontSyncTo)
	ON_COMMAND(ID_DOCFONT_SYNCFROM, &CFontPropertiesWnd::OnFontSyncFrom)
	ON_UPDATE_COMMAND_UI(ID_DOCFONT_SYNCFROM, &CFontPropertiesWnd::OnUpdateFontSyncFrom)
	ON_COMMAND(ID_DOCFONT_REFRESH, &CFontPropertiesWnd::OnDocRefresh)
END_MESSAGE_MAP()



// CFontPropertiesWnd メッセージ ハンドラー

void CFontPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	int nHeight = rectClient.Height();

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFontList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), nHeight - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFontPropertiesWnd::InitPropList()
{
	SetControlFont();

	m_wndFontList.EnableHeaderCtrl(FALSE);
	m_wndFontList.EnableDescriptionArea(FALSE);
	m_wndFontList.MarkModifiedProperties(FALSE);
}

void CFontPropertiesWnd::SetControlFont()
{
	m_wndFontList.SetFont(GetBaseFont());
}

int CFontPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndFontList.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_FONTLIST))
	{
		TRACE0("プロパティ グリッドを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_FONTXML);
	m_wndToolBar.LoadToolBar(IDR_DOCFONT);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	InitPropList();

	return 0;
}


void CFontPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFontList.SetFocus();
}

void CFontPropertiesWnd::Refresh(BOOL bClosed)
{
	CBasePropertiesWnd::Refresh(bClosed);

	TRACE(_T("CFontPropertiesWnd::Refresh()\n"));

	CW01DrawDoc* pDoc = NULL;
	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pDoc = pView->GetDocument();
	}

	m_wndFontList.RemoveAll();

	if( !bClosed && 
		pDoc )
	{
		CString szVal;
		CString szText;
		// 1. フォント
		szText.LoadString(IDS_FONT_INFO);
		CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

		LinkedList<WEB_FONT_DATA*>* pFontList = pDoc->GetFontList();
		if( pFontList &&
			pFontList->first() )
		{

			WEB_FONT_DATA* pFont = NULL;
			do
			{
				pFont = pFontList->get();
				if( pFont )
				{
					szText = pFont->name;
					szVal = pDoc->GetDisplayFontName(szText);
					if( !szVal.IsEmpty() )
					{
						SetModified();
					}
					CSelectFontPropertyGridProperty* pCtrl = new CSelectFontPropertyGridProperty(szText, szVal);
					if( pCtrl )
					{
						pGroup->AddSubItem(pCtrl);

						AddToMap(pFont->name, pCtrl);
					}
				}
			} while( pFontList->next() );
		}

		m_wndFontList.AddProperty(pGroup);	
	}

}

void CFontPropertiesWnd::OnFontSyncTo()
{
	CWnd* pWnd = GetWindow();
	if( pWnd )
	{
		pWnd->PostMessage(UWM_DOC_SYNC);
	}
}

void CFontPropertiesWnd::OnUpdateFontSyncTo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

void CFontPropertiesWnd::OnFontSyncFrom()
{
	CWnd* pWnd = GetWindow();
	if( pWnd )
	{
		pWnd->PostMessage(UWM_DOC_SYNC, 1);
	}
}

void CFontPropertiesWnd::OnUpdateFontSyncFrom(CCmdUI *pCmdUI)
{
	BOOL bEnable = FALSE;
	CHashTblStrToProperty* pMapS = GetStrToPropTable();
	if( pMapS && pMapS->size() )
		bEnable = TRUE;
	CHashTblLongToProperty* pMapL = GetLongToPropTable();
	if( pMapL && pMapL->size() )
		bEnable = TRUE;

	pCmdUI->Enable(bEnable);
}


void CFontPropertiesWnd::OnDocRefresh()
{
	CWnd* pWnd = GetWindow();
	if( pWnd )
	{
		pWnd->PostMessage(UWM_DOC_REFRESH);
	}
}

void CFontPropertiesWnd::OnPropertyChanged(DWORD_PTR nID, CPropertyGridPropertyEx* pProp)
{
}

