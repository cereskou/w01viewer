// TextPropertiesWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "TextPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define COL_TEXT	0		// 文字
#define COL_POS		1		// 位置
#define COL_CELL	2		// セル
#define COL_ATTR	3		// 属性
#define COL_FONT	4		// フォント
#define COL_DISP	5		// 表示用フォント
#define COL_PPOS	6		// ページ内情報（行番号、文字番号など）

// CTextPropertiesWnd
#define ID_CTRL_TEXTLIST	0x2001

IMPLEMENT_DYNAMIC(CTextPropertiesWnd, CBasePropertiesWnd)

CTextPropertiesWnd::CTextPropertiesWnd()
{
	m_pWnd = NULL;
	m_bHighLight = FALSE;
	plstSelected = NULL;
	pmapTextLine = NULL;
	pmapTextPos = NULL;
	m_bBusy = FALSE;
}

CTextPropertiesWnd::~CTextPropertiesWnd()
{
	_delete(pmapTextPos);
	_delete(pmapTextLine);
	_delete(plstSelected);
}


BEGIN_MESSAGE_MAP(CTextPropertiesWnd, CBasePropertiesWnd)
	ON_COMMAND(ID_REFRESH, &CTextPropertiesWnd::OnRefresh)
	ON_COMMAND(ID_HIGHLIGHT, &CTextPropertiesWnd::OnHighLight)
	ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT, &CTextPropertiesWnd::OnUpdateHighLight)
	ON_COMMAND(ID_SELECTLINE, &CTextPropertiesWnd::OnSelectLine)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_CTRL_TEXTLIST, OnLvnItemChangedTextList)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CTextPropertiesWnd メッセージ ハンドラー

int CTextPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_nOffsetHeight = 0;

	const DWORD dwListStyle = LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | WS_CHILD | WS_VISIBLE | WS_BORDER;// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if( !m_lstText.Create(dwListStyle, rectDummy, this, ID_CTRL_TEXTLIST) )
	{
		TRACE0("TextListを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	pmapTextPos = new CHashTblTextPos();
	if( pmapTextPos == NULL ||
		pmapTextPos->create(genHashKey) == NULL )
	{
		TRACE0("CHashTblTextPosを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	pmapTextLine = new CHashTblTextLine();
	if( pmapTextLine == NULL ||
		pmapTextLine->create(genHashKey) == NULL )
	{
		TRACE0("CHashTblTextLineを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	plstSelected = new LinkedList<WEB_POS_RECDATA*>(false);
	if( !plstSelected )
	{
		TRACE0("LinkedListを作成できませんでした\n");
		return -1;
	}

	InitListCtrl();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES);//, 0, 0, TRUE /* ロックされています*/);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* ロックされました*/);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	return 0;
}

void CTextPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);
	
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	int nClientHeight = rectClient.Height() - cyTlb;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nOffsetHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_lstText.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nOffsetHeight + cyTlb, rectClient.Width(), nClientHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


void CTextPropertiesWnd::SetControlFont()
{
	m_lstText.SetFont(&m_baseFont);
}

void CTextPropertiesWnd::InitListCtrl()
{
	SetControlFont();

	// 
	m_lstText.SetExtendedStyle(m_lstText.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Header
	CString szColumn;
	// 文字
	szColumn.LoadString(IDS_COLNAME_TEXT);
	m_lstText.InsertColumn(COL_TEXT, szColumn, LVCFMT_LEFT, 40);
	// 位置
	szColumn.LoadString(IDS_COLNAME_TEXTPOS);
	m_lstText.InsertColumn(COL_POS, szColumn, LVCFMT_LEFT, 80);
	// セル
	szColumn.LoadString(IDS_COLNAME_CELL);
	m_lstText.InsertColumn(COL_CELL, szColumn, LVCFMT_LEFT, 80);
	// 属性
	szColumn.LoadString(IDS_COLNUME_TEXTATTR);
	m_lstText.InsertColumn(COL_ATTR, szColumn, LVCFMT_LEFT, 450);
	// フォント
	szColumn.LoadString(IDS_COLNUME_TEXTFONT);
	m_lstText.InsertColumn(COL_FONT, szColumn, LVCFMT_LEFT, 150);
	// 表示用フォント
	szColumn.LoadString(IDS_COLNAME_DISPFONT);
	m_lstText.InsertColumn(COL_DISP, szColumn, LVCFMT_LEFT, 150);
	// 表示用フォント
	szColumn = _T("");
	m_lstText.InsertColumn(COL_PPOS, szColumn, LVCFMT_LEFT, 200);
}


void CTextPropertiesWnd::Refresh(BOOL bClosed)
{
	CBasePropertiesWnd::Refresh(bClosed);

	if( !(GetStyle () & WS_VISIBLE) )
		return;

	if( plstSelected )
		plstSelected->clear();

	TRACE(_T("CTextPropertiesWnd::Refresh()\n"));

	m_lstText.LockWindowUpdate();

	m_lstText.DeleteAllItems();
	if( pmapTextLine )
	{
		pmapTextLine->removeall();
	}
	if( pmapTextPos )
	{
		pmapTextPos->removeall();
	}

	CW01DrawDoc* pDoc = NULL;
	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pDoc = pView->GetDocument();
	}

	if( !bClosed && 
		pDoc )
	{
		m_bBusy = TRUE;

		TCHAR szTxt[32] = {0};
		TCHAR szPos[64] = {0};
		TCHAR szAtt[128] = {0};
		TCHAR szFnt[65] = {0};
		TCHAR szDsp[65] = {0};
		TCHAR szCel[64] = {0};
		TCHAR szDes[128] = {0};

		// add data
		// テキストテーブル
		LinkedList<LPTEXTADDR>* pLstText = pDoc->GetTextList();

		LPPAGECONTENT pContent = pDoc->GetPageContent();
		if( pContent )
		{
			LinkedList<WEB_POS_RECDATA*>* plstPos = pContent->plstTextPos;
			if( plstPos &&
				plstPos->first() )
			{
				theApp.BeginWaitCursor();

				LinkedList<WEB_POS_RECDATA*>* pline = NULL;
				WEB_POS_RECDATA* pPos = NULL;
				do
				{
					pPos = plstPos->get();
					if( pPos )
					{
						if( pmapTextLine )
						{
							pline = pmapTextLine->get(pPos->textY);
							if( pline == NULL )
							{
								pline = new LinkedList<WEB_POS_RECDATA*>();
								pmapTextLine->add(pPos->textY, pline);
							}

							if( pline )
							{
								pline->add(pPos);
							}
						}

						// Clear
						szTxt[0] = _T('\0');
						szPos[0] = _T('\0');
						szAtt[0] = _T('\0');
						szFnt[0] = _T('\0');
						szDsp[0] = _T('\0');
						szCel[0] = _T('\0');
						szDes[0] = _T('\0');

						// 文字
						LPTEXTADDR pText = pDoc->GetText(pPos->textNum);
						if( pText )
						{
							_stprintf_s(szTxt, 32, _T("%s"), pText->text);
						}
						// POS
						_stprintf_s(szPos, 64, _T("%d, %d"), pPos->textX, pPos->textY);
						// CELL
						_stprintf_s(szCel, 64, _T("%d, %d"), pPos->cellX, pPos->cellY);
						// 属性
						WEB_ATTR_RECDATA* pAttr = pDoc->GetTextAttr(pPos->attrId);
						if( pAttr )
						{
							_stprintf_s(szAtt, 128, _T("w:%d, h:%d, style:0x%X, rotate:%d, fc(RGBA): 0x%08x, bc(RGBA): 0x%08x"), pAttr->fontWidth, pAttr->fontHeight, pAttr->fontStyle, pAttr->fontRotate, pAttr->backgroundColor, pAttr->backgroundColor);
							// フォント
							WEB_FONT_DATA* pFont = pDoc->GetFont(pAttr->fontId);
							if( pFont )
							{
								_tcscpy_s(szFnt, 64, pFont->name);

								LPWSTR lpMapFontName = pDoc->GetFontMap(pFont->name);

								if( lpMapFontName != NULL && wcslen(lpMapFontName) > 0 )
									wcscpy_s(szDsp, 63, lpMapFontName);
								else
									wcscpy_s(szDsp, 63, pFont->name);
							}
						}

						// 文字位置情報（行番号など）
						int txtIdx = pPos->index;
						if( txtIdx >= TEXTBLOCK_OFFSET )
							txtIdx -= TEXTBLOCK_OFFSET;
						_stprintf_s(szDes, 127, _T("line:%d, number:%d, block:%d"), pPos->line, txtIdx, pPos->block);

						// Insert
						int nCount = m_lstText.GetItemCount();
						// 文字
						int nIndex = m_lstText.InsertItem(nCount, szTxt);
						if( nIndex > -1 )
						{
							// POS
							m_lstText.SetItemText(nIndex, COL_POS, szPos);
							// CELL
							m_lstText.SetItemText(nIndex, COL_CELL, szCel);
							// 属性
							m_lstText.SetItemText(nIndex, COL_ATTR, szAtt);
							// フォント
							m_lstText.SetItemText(nIndex, COL_FONT, szFnt);
							// 表示用フォント名
							m_lstText.SetItemText(nIndex, COL_DISP, szDsp);
							// 文字位置情報（行番号など）
							m_lstText.SetItemText(nIndex, COL_PPOS, szDes);

							LONG lKey = MAKELONG(pPos->line, pPos->index);
							LPPOSLIST poslst = (LPPOSLIST)malloc(sizeof(POSLIST));
							if( poslst )
							{
								poslst->index = nIndex;
								poslst->pos = pPos;

								if( pmapTextPos->add(lKey, poslst) )
								{
									m_lstText.SetItemData(nIndex, (DWORD_PTR)lKey);
								}
							}
						}
					}
				} while( plstPos->next() );

				theApp.EndWaitCursor();
			}
		}
		m_bBusy = FALSE;
	}
	m_lstText.UnlockWindowUpdate();
}


void CTextPropertiesWnd::OnRefresh()
{
	if( plstSelected && 
		plstSelected->size() )
	{
		ClearSelected();

		// Refresh
		CWnd* pWnd = GetWindow();
		if( pWnd )
		{
			pWnd->Invalidate();
			pWnd->UpdateWindow();
		}
	}
	if( GetWindow() == NULL )
	{
		SetWindow(GetActiveView());
	}
	Refresh();
}

void CTextPropertiesWnd::SetSelectedPosition(WEB_POS_RECDATA* pos, BOOL bMulti)
{
	if( !plstSelected )
		return;

	if( !bMulti )
	{
		ClearSelected();
	}

	if( pos )
	{
		plstSelected->add(pos);
		int index = pos->index;
		if( index >= TEXTBLOCK_OFFSET)
			index -= TEXTBLOCK_OFFSET;

		if( !bMulti )
		{
			LONG lKey = MAKELONG(pos->line, index);
			LPPOSLIST plst = GetPosObjectEx(lKey);
			if( plst )
			{
				if( plst->index > 0 && plst->index < m_lstText.GetItemCount() )
				{
					m_lstText.EnsureVisible(plst->index, FALSE);
				}
			}
		}
	}
}

LPPOSLIST CTextPropertiesWnd::GetPosObjectEx(LONG lKey)
{
	if( !pmapTextPos ||
		!pmapTextPos->size() )
		return NULL;

	LPPOSLIST plst = pmapTextPos->get(lKey);
	if( plst )
	{
		return plst;
	}

	return NULL;
}

LPPOSLIST CTextPropertiesWnd::GetPosObject(int nIndex)
{
	if( nIndex < 0 || nIndex > m_lstText.GetItemCount() )
		return NULL;

	// CHANGED
	LONG lKey = (LONG)m_lstText.GetItemData(nIndex);
	LPPOSLIST plst = GetPosObjectEx(lKey);
	if( plst )
	{
		return plst;
	}

	return NULL;
}

WEB_POS_RECDATA* CTextPropertiesWnd::GetPosRec(int nIndex)
{
	LPPOSLIST plst = GetPosObject(nIndex);
	if( plst )
	{
		return plst->pos;
	}

	return NULL;
}

void CTextPropertiesWnd::OnHighLight()
{
	m_bHighLight = !m_bHighLight;

	BOOL bUpdate = FALSE;
	if( !m_bHighLight )
	{
		if( m_lstText.GetSelectedCount() )
		{
			POSITION pos = m_lstText.GetFirstSelectedItemPosition();
			while( pos != NULL )
			{
				int nIndex = m_lstText.GetNextSelectedItem(pos);
				if( nIndex > -1 )
				{
					m_lstText.SetItemState(nIndex, ~LVIS_SELECTED, LVIS_SELECTED);
				}
			}

			m_lstText.SetFocus();
		}
		if( plstSelected && 
			plstSelected->size() )
		{
			ClearSelected();
			bUpdate = TRUE;
		}
	}
	else
	{
		if( m_lstText.GetSelectedCount() )
		{
			WEB_POS_RECDATA* pPos = NULL;
			POSITION pos = m_lstText.GetFirstSelectedItemPosition();
			while( pos != NULL )
			{
				pPos = NULL;
				int nIndex = m_lstText.GetNextSelectedItem(pos);
				if( nIndex > -1 )
				{
					pPos = GetPosRec(nIndex);
					if( pPos &&
						pPos->index < TEXTBLOCK_OFFSET )
					{
						pPos->index += TEXTBLOCK_OFFSET;

						// add to selected list
						plstSelected->add(pPos);

						bUpdate = TRUE;
					}
					// CHANGED
					//pPos = (WEB_POS_RECDATA*)m_lstText.GetItemData(nIndex);
					//if( pPos &&
					//	pPos->index < TEXTBLOCK_OFFSET )
					//{
					//	pPos->index += TEXTBLOCK_OFFSET;

					//	// add to selected list
					//	plstSelected->add(pPos);

					//	bUpdate = TRUE;
					//}
				}
			}
		}
	}

	if( bUpdate )
	{
		// Refresh
		CWnd* pWnd = GetWindow();
		if( pWnd )
		{
			pWnd->Invalidate();
			pWnd->UpdateWindow();
		}
	}
}

void CTextPropertiesWnd::OnUpdateHighLight(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighLight);
}

void CTextPropertiesWnd::ClearSelected()
{
	WEB_POS_RECDATA* pPos = NULL;
	// Clear Old
	if( plstSelected && 
		plstSelected->size() )
	{
		do
		{
			pPos = plstSelected->get();
			if( pPos && 
				pPos->index >= TEXTBLOCK_OFFSET )
			{
				pPos->index -= TEXTBLOCK_OFFSET;
			}
		} while( plstSelected->next() );

		plstSelected->clear();
	}
}

void CTextPropertiesWnd::UpdateSelected()
{
	// Clear Old
	ClearSelected();

	WEB_POS_RECDATA* pPos = NULL;
	if( m_lstText.GetSelectedCount() )
	{
		POSITION pos = m_lstText.GetFirstSelectedItemPosition();
		while( pos != NULL )
		{
			pPos = NULL;
			int nIndex = m_lstText.GetNextSelectedItem(pos);
			if( nIndex > -1 )
			{
				if( m_lstText.GetItemState(nIndex, LVIS_SELECTED) == LVIS_SELECTED )
				{
					pPos = GetPosRec(nIndex);
					if( pPos &&
						pPos->index < TEXTBLOCK_OFFSET )
					{
						pPos->index += TEXTBLOCK_OFFSET;

						// add to selected list
						plstSelected->add(pPos);
					}

					//pPos = (WEB_POS_RECDATA*)m_lstText.GetItemData(nIndex);
					//if( pPos &&
					//	pPos->index < TEXTBLOCK_OFFSET )
					//{
					//	pPos->index += TEXTBLOCK_OFFSET;

					//	// add to selected list
					//	plstSelected->add(pPos);
					//}
				}
			}
		}
	}
}

void CTextPropertiesWnd::OnLvnItemChangedTextList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( m_bBusy )
		return;

	BOOL bSelected = FALSE;
	BOOL bUpdate = TRUE;

	if( !m_bHighLight )
		return;

	int nItemIndex = pNMLV->iItem;

	//// Clear Old
	ClearSelected();

	if( (pNMLV->uChanged & LVIF_STATE ) &&
		(pNMLV->uNewState & LVIS_SELECTED ))
	{
		bSelected = TRUE;

		WEB_POS_RECDATA* pPos = NULL;
		// CHANGED
		pPos = GetPosRec(nItemIndex);
		if( pPos &&
			pPos->index < TEXTBLOCK_OFFSET )
		{
			pPos->index += TEXTBLOCK_OFFSET;

			// add to selected list
			plstSelected->add(pPos);
		}
		//pPos = (WEB_POS_RECDATA*)m_lstText.GetItemData(nItemIndex);
		//if( pPos &&
		//	pPos->index < TEXTBLOCK_OFFSET )
		//{
		//	pPos->index += TEXTBLOCK_OFFSET;

		//	// add to selected list
		//	plstSelected->add(pPos);
		//}
	}
	//if( (pNMLV->uNewState & LVIS_SELECTED) && !(pNMLV->uOldState & LVIS_SELECTED) )
	//{
	//	bSelected = TRUE;
	//}

	if( !bSelected )
	{
		return;
	}

	// 複数選択時
	if( (GetKeyState(VK_SHIFT) & 0x8000) != 0 ||
		(GetKeyState(VK_CONTROL) & 0x8000) != 0 )
	{
		bUpdate = FALSE;
	}

	TRACE(_T("OnTextListClick\n"));

	m_bBusy = TRUE;
	//// Clear Old
	//ClearSelected();

	//WEB_POS_RECDATA* pPos = NULL;
	//if( m_lstText.GetSelectedCount() )
	//{
	//	POSITION pos = m_lstText.GetFirstSelectedItemPosition();
	//	while( pos != NULL )
	//	{
	//		pPos = NULL;
	//		int nIndex = m_lstText.GetNextSelectedItem(pos);
	//		if( nIndex > -1 )
	//		{
	//			if( m_lstText.GetItemState(nIndex, LVIS_SELECTED) == LVIS_SELECTED )
	//			{
	//			pPos = (WEB_POS_RECDATA*)m_lstText.GetItemData(nIndex);
	//			if( pPos &&
	//				pPos->index < TEXTBLOCK_OFFSET )
	//			{
	//				pPos->index += TEXTBLOCK_OFFSET;

	//				// add to selected list
	//				plstSelected->add(pPos);
	//			}
	//			}
	//		}
	//	}

	//}

	if( bUpdate )
	{
		// Refresh
		CWnd* pWnd = GetWindow();
		if( pWnd )
		{
			TRACE(_T("UpdateView\n"));
			pWnd->Invalidate();
			pWnd->UpdateWindow();
		}
	}

	m_bBusy = FALSE;

	*pResult = 0;
}

void CTextPropertiesWnd::OnSelectLine()
{
	if( m_lstText.GetSelectedCount() != 1 || !pmapTextLine || !plstSelected )
	{
		return;
	}

	BOOL bUpdate = FALSE;
	m_bBusy = TRUE;
	// Clear Old
	ClearSelected();

	WEB_POS_RECDATA* pPos = NULL;
	POSITION pos = m_lstText.GetFirstSelectedItemPosition();
	while( pos != NULL )
	{
		pPos = NULL;
		int nIndex = m_lstText.GetNextSelectedItem(pos);
		if( nIndex > -1 )
		{
			pPos = GetPosRec(nIndex);
			if( pPos )
			{
				LinkedList<WEB_POS_RECDATA*>* pline = pmapTextLine->get(pPos->textY);
				if( pline && pline->first() )
				{
					do
					{
						pPos = pline->get();

						if( pPos &&
							pPos->index < TEXTBLOCK_OFFSET )
						{
							pPos->index += TEXTBLOCK_OFFSET;
						}
						// add to selected list
						plstSelected->add(pPos);
						bUpdate = TRUE;

					} while( pline->next() );
				}
			}
		}
	}
	if( bUpdate )
	{
		// Refresh
		CWnd* pWnd = GetWindow();
		if( pWnd )
		{
			pWnd->Invalidate();
			pWnd->UpdateWindow();
		}
	}

	m_bBusy = FALSE;
}

void CTextPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CBasePropertiesWnd::OnSetFocus(pOldWnd);

	m_lstText.SetFocus();
}


BOOL CTextPropertiesWnd::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYUP )
	{
		if( pMsg->hwnd == m_lstText.GetSafeHwnd() )
		{
			if( pMsg->wParam == VK_SHIFT ||
				pMsg->wParam == VK_CONTROL )
			{
				if( m_bHighLight )
				{
					UpdateSelected();

					if( plstSelected && 
						plstSelected->size() )
					{
						// Refresh
						CWnd* pWnd = GetWindow();
						if( pWnd )
						{
							pWnd->Invalidate();
							pWnd->UpdateWindow();
						}
					}
				}
			}
		}
	}

	return CBasePropertiesWnd::PreTranslateMessage(pMsg);
}
