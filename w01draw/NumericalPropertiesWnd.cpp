#include "stdafx.h"
#include "w01draw.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "NumericalPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_CTRL_NUMERICAL		0x2008

void _freeLinkedGridCtrlP(CPropertyGridPropertyEx* pItem)
{
//	_delete(pItem);
}

IMPLEMENT_DYNAMIC(CNumericalPropertiesWnd, CBasePropertiesWnd)

CNumericalPropertiesWnd::CNumericalPropertiesWnd(void)
{
	plstSelected = NULL;
	pmapNumsObjList = NULL;
	m_plnkGridCtrl = NULL;
	m_bHighLight = TRUE;
}


CNumericalPropertiesWnd::~CNumericalPropertiesWnd(void)
{
	_delete(plstSelected);
	_delete(pmapNumsObjList);
	_delete(m_plnkGridCtrl);
}

BEGIN_MESSAGE_MAP(CNumericalPropertiesWnd, CBasePropertiesWnd)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_NUMS_RELOAD, &CNumericalPropertiesWnd::OnReload)
	ON_UPDATE_COMMAND_UI(ID_NUMS_RELOAD, &CNumericalPropertiesWnd::OnUpdateReload)
	ON_COMMAND(ID_HIGHLIGHT, &CNumericalPropertiesWnd::OnHighLight)
	ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT, &CNumericalPropertiesWnd::OnUpdateHighLight)
	
	ON_MESSAGE(AFX_UWM_CHANGE_SELECTION, &CNumericalPropertiesWnd::OnChangeSelection)
END_MESSAGE_MAP()

int CNumericalPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndDocInfo.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_NUMERICAL))
	{
		TRACE0("プロパティ グリッドを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_NUMERICAL);
	m_wndToolBar.LoadToolBar(IDR_NUMERICAL);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	plstSelected = new LinkedList<WEB_POS_RECDATA*>(false);
	if( !plstSelected )
	{
		TRACE0("LinkedListを作成できませんでした\n");
		return -1;
	}

	pmapNumsObjList = new CHashTblDWordToPosData();
	if( pmapNumsObjList == NULL ||
		pmapNumsObjList->create(genHashKey) == NULL )
	{
		TRACE0("CHashTblDWordToNumsObjectを作成できませんでした\n");
		return -1;
	}

	m_plnkGridCtrl = new CLinkedListGridCtrl(true, _freeLinkedGridCtrlP);
	if( !m_plnkGridCtrl )
	{
		TRACE0("CHashTblDWordToNumsObjectを作成できませんでした\n");
		return -1;
	}

	InitPropList();

	return 0;
}

void CNumericalPropertiesWnd::AdjustLayout()
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
	m_wndDocInfo.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), nHeight - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CNumericalPropertiesWnd::InitPropList()
{
	SetControlFont();

	m_wndDocInfo.SetLeftColumnWidth(130);

	m_wndDocInfo.EnableHeaderCtrl(FALSE);
	m_wndDocInfo.EnableDescriptionArea(FALSE);
	m_wndDocInfo.MarkModifiedProperties(TRUE);
}

void CNumericalPropertiesWnd::SetControlFont()
{
	m_wndDocInfo.SetFont(GetBaseFont());
}

void CNumericalPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CBasePropertiesWnd::OnSetFocus(pOldWnd);

	m_wndDocInfo.SetFocus();
}

void CNumericalPropertiesWnd::Refresh(BOOL bClosed)
{
	CBasePropertiesWnd::Refresh(bClosed);

	TRACE(_T("CNumericalPropertiesWnd::Refresh()\n"));

	CW01DrawDoc* pDoc = NULL;
	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pDoc = pView->GetDocument();
	}

	theApp.BeginWaitCursor();

	// clear
	TRACE(_T("Clear Grid... \n"));
	m_wndDocInfo.RemoveAll();
	pmapNumsObjList->removeall();
	m_plnkGridCtrl->clear();
	ClearSelected();
	TRACE(_T("Clear Done.\n"));

	if( !bClosed &&
		pDoc )
	{
		TCHAR szMsg[256] = {0};
		TCHAR szVal[256] = {0};
		CString szText;

		// Page Data
		LPPAGECONTENT pContent = pDoc->GetPageContent();
		if( pContent )
		{
			CHashTblNumsObject* pMap = pContent->pmapNumerical;
			// 数値情報
			if( pMap )
			{
				PPOSITION pos = pMap->first(true, true);
				
				DWORD dwId = 0;
				while(pos)
				{
					double value;
					LinkedList<LPNUMSOBJ>* pLnkList = NULL;
					pMap->next(pos, value, pLnkList);
					if( pLnkList != NULL &&
						pLnkList->first() )
					{
						DWORD dwSubId = dwId + 1;

						LinkedList<WEB_POS_RECDATA*>* pnumsLL = new LinkedList<WEB_POS_RECDATA*>(false);
						if( pnumsLL == NULL )
						{
							pmapNumsObjList->removeall();

							return;
						}

						pmapNumsObjList->insert(dwId, pnumsLL);

						_stprintf_s(szMsg, 255, _T("%f"), value);
						//std::auto_ptr<CPropertyGridPropertyEx> group(new CPropertyGridPropertyEx(szMsg));
						//CPropertyGridPropertyEx* pGroup = group.get();
						CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szMsg);
						if( pGroup == NULL )
						{
							pmapNumsObjList->removeall();
							m_plnkGridCtrl->clear();

							return;
						}

						m_plnkGridCtrl->add(pGroup);
						do
						{
							LPNUMSOBJ pNumObj = pLnkList->get();
							if( pNumObj )
							{
								LinkedList<WEB_POS_RECDATA*>* plstPos = new LinkedList<WEB_POS_RECDATA*>(false);
								if( plstPos == NULL )
								{
									pmapNumsObjList->removeall();
									m_plnkGridCtrl->clear();

									return;
								}
							
								szText = _T("");
								pmapNumsObjList->insert(dwSubId, plstPos);
								if( pNumObj->pNumList &&
									pNumObj->pNumList->first() )
								{
									do
									{
										WEB_POS_RECDATA* tpos = pNumObj->pNumList->get();

										LPTEXTADDR pTa = pDoc->GetText(tpos->textNum);
										if( pTa )
										{
											szText.Append(pTa->text);
										}

										plstPos->add(tpos);
										pnumsLL->add(tpos);
									} while( pNumObj->pNumList->next());
								}
								//plstPos->append(pNumObj->pNumList);
								//pnumsLL->append(pNumObj->pNumList);

								_stprintf_s(szMsg, 255, _T("%d (%d) - %d"), pNumObj->recNo, pNumObj->charCnt, pNumObj->idxNo);
								AddSubItem(pGroup, dwSubId++, szText, szMsg); 
							}

						} while( pLnkList->next() );

						pGroup->SetData(dwId);
						pGroup->Expand(FALSE);

						dwId += dwSubId;

						if( m_wndDocInfo.AddProperty(pGroup) < 0 )
						//if( m_wndDocInfo.AddProperty(group.release()) < 0 )
						{
							//
							TRACE(_T("AddProperty Failed. %d\n"), dwId);
						}

					}
				}
			}
		}
	}
	if( pView )
	{
		pView->Invalidate();
	}
	theApp.EndWaitCursor();
}

void CNumericalPropertiesWnd::OnReload()
{
	Refresh();
}

void CNumericalPropertiesWnd::OnUpdateReload(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetWindow() != NULL);
}

LRESULT CNumericalPropertiesWnd::OnChangeSelection(WPARAM wParam, LPARAM lParam)
{
	DWORD_PTR dwNewSel = (DWORD_PTR)wParam;
	DWORD_PTR dwOldSel = (DWORD_PTR)lParam;

	ClearSelected();

	if( m_bHighLight )
	{
		LinkedList<WEB_POS_RECDATA*>* pLstPos = NULL;
		pLstPos = pmapNumsObjList->get(dwNewSel);
		if( pLstPos != NULL &&
			pLstPos->first() )
		{
			do
			{
				WEB_POS_RECDATA* pDat = pLstPos->get();
				if( pDat &&
					pDat->index < TEXTBLOCK_OFFSET )
				{
					pDat->index += TEXTBLOCK_OFFSET;

					plstSelected->add(pDat);
				}
			} while(pLstPos->next());
		}
	}

	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pView->Invalidate();
	}

	return 0L;
}


void CNumericalPropertiesWnd::ClearSelected()
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

void CNumericalPropertiesWnd::OnHighLight()
{
	BOOL bUpdate = FALSE;
	m_bHighLight = !m_bHighLight;

	if( !m_bHighLight )
	{
		if( plstSelected && 
			plstSelected->size() )
		{
			ClearSelected();
			bUpdate = TRUE;
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

void CNumericalPropertiesWnd::OnUpdateHighLight(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighLight);
}
