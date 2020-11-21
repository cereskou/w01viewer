#include "stdafx.h"
#include "w01draw.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "PageInfoPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_CTRL_PAGEINFO		0x2005
#define ID_CTRL_OVLYINFO		0x2006


#define ID_PROP_IMAGE		0x1000
#define ID_PROP_OVLY		0x2000
#define ID_PROP_IMGINOVL	0x3000

IMPLEMENT_DYNAMIC(CPageInfoPropertiesWnd, CBasePropertiesWnd)

CPageInfoPropertiesWnd::CPageInfoPropertiesWnd(void)
{
	m_bOvlLoaded = FALSE;
}


CPageInfoPropertiesWnd::~CPageInfoPropertiesWnd(void)
{
}

BEGIN_MESSAGE_MAP(CPageInfoPropertiesWnd, CBasePropertiesWnd)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_PAGEINF_RELOAD, &CPageInfoPropertiesWnd::OnReload)
	ON_MESSAGE(AFX_UWM_CHANGE_SELECTION, &CPageInfoPropertiesWnd::OnChangeSelection)
END_MESSAGE_MAP()

int CPageInfoPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndDocInfo.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_PAGEINFO))
	{
		TRACE0("�v���p�e�B �O���b�h���쐬�ł��܂���ł���\n");
		return -1;      // �쐬�ł��Ȃ��ꍇ
	}

	//if (!m_wndOvlInfo.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_OVLYINFO))
	//{
	//	TRACE0("�v���p�e�B �O���b�h���쐬�ł��܂���ł���\n");
	//	return -1;      // �쐬�ł��Ȃ��ꍇ
	//}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_FONTXML);
	m_wndToolBar.LoadToolBar(IDR_PAGEINFO);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// ���ׂẴR�}���h���A�e�t���[���o�R�ł͂Ȃ����̃R���g���[���o�R�œn����܂�:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	InitPropList();

	return 0;
}

void CPageInfoPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	int nHeight = rectClient.Height();
	int nTop = rectClient.top;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	nTop += cyTlb;
	//if( m_wndOvlInfo.IsWindowVisible() )
	//{
	//	nHeight = (nHeight - cyTlb) * 2 / 5;
	//	m_wndOvlInfo.SetWindowPos(NULL, rectClient.left, nTop, rectClient.Width(), nHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	//	nTop += nHeight;
	//	nHeight = rectClient.Height() - nHeight - cyTlb;
	//}
	//else
	{
		nHeight = nHeight - cyTlb;
	}

	m_wndDocInfo.SetWindowPos(NULL, rectClient.left, nTop, rectClient.Width(), nHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPageInfoPropertiesWnd::InitPropList()
{
	SetControlFont();

	//// Overlay
	//m_wndOvlInfo.SetLeftColumnWidth(160);

	//m_wndOvlInfo.EnableHeaderCtrl(FALSE);
	//m_wndOvlInfo.EnableDescriptionArea(FALSE);
	//m_wndOvlInfo.MarkModifiedProperties(TRUE);
	
	// Page
	m_wndDocInfo.SetLeftColumnWidth(160);

	m_wndDocInfo.EnableHeaderCtrl(FALSE);
	m_wndDocInfo.EnableDescriptionArea(FALSE);
	m_wndDocInfo.MarkModifiedProperties(TRUE);
}

void CPageInfoPropertiesWnd::SetControlFont()
{
	m_wndDocInfo.SetFont(GetBaseFont());
}

void CPageInfoPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndDocInfo.SetFocus();
}

void CPageInfoPropertiesWnd::Refresh(BOOL bClosed)
{
	CBasePropertiesWnd::Refresh(bClosed);

	TRACE(_T("CPageInfoPropertiesWnd::Refresh()\n"));

	m_wndDocInfo.RemoveAll();

	CW01DrawDoc* pDoc = NULL;
	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pDoc = pView->GetDocument();
	}
	else
	{
		// Clear
		m_bOvlLoaded = FALSE;
		//m_wndOvlInfo.RemoveAll();
		m_wndDocInfo.RemoveAll();

		return;
	}

	if( !bClosed &&
		pDoc )
	{
		TCHAR szMsg[256] = {0};
		TCHAR szVal[256] = {0};
		CString szText;

		//if( !m_bOvlLoaded )
		//{
		//	m_wndOvlInfo.RemoveAll();
		//	szText.LoadString(IDS_DOCINF_INFO);
		//	CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);
		//	// ���[���
		//	m_wndOvlInfo.AddProperty(pGroup);	

		//	// �C���[�W���
		//	CHashTblImage* pmapImage = pDoc->GetImageMap();
		//	if( pmapImage &&
		//		pmapImage->size() )
		//	{
		//		// Imaeg count
		//		_stprintf_s(szMsg, 255, _T("%d"), pmapImage->size());
		//		AddSubItem(pGroup, IDS_PAGE_IMAGENT, szMsg);

		//		szText.LoadString(IDS_TXT_NO_IMAGE);
		//		CPropertyGridPropertyEx* pImgGroup = new CPropertyGridPropertyEx(szText);

		//		USHORT nImgId = 0;
		//		LPIMAGEDATA pImgData = NULL;
		//		PPOSITION pos = pmapImage->first(true, true);
		//		while( pos != NULL )
		//		{
		//			pmapImage->next(pos, nImgId, pImgData);

		//			DWORD_PTR dwId = ID_PROP_IMAGE + nImgId;

		//			_stprintf_s(szMsg, 255, _T("IMG - %d"), nImgId);
		//			CPropertyGridPropertyEx* pImgGrp = new CPropertyGridPropertyEx(szMsg, dwId);

		//			if( pImgData )
		//			{
		//				// �C���[�W�ԍ�
		//				_stprintf_s(szMsg, 255, _T("%d"), nImgId);
		//				AddSubItem(pImgGrp, dwId, IDS_OBJECT_NO, szMsg);

		//				// �C���[�W�^�C�v  0:GIF 1:JPG 4:PNG
		//				_stprintf_s(szMsg, 255, _T("%d"), pImgData->type);
		//				AddSubItem(pImgGrp, dwId, IDS_OBJECT_TYPE, szMsg);
		//				
		//				// ��
		//				_stprintf_s(szMsg, 255, _T("%d"), pImgData->width);
		//				AddSubItem(pImgGrp, dwId, IDS_OBJECT_WIDTH, szMsg);

		//				// ����
		//				_stprintf_s(szMsg, 255, _T("%d"), pImgData->height);
		//				AddSubItem(pImgGrp, dwId, IDS_OBJECT_HEIGHT, szMsg);

		//				// �f�[�^�T�C�Y
		//				_stprintf_s(szMsg, 255, _T("%d"), pImgData->length);
		//				AddSubItem(pImgGrp, dwId, IDS_OBJECT_SIZE, szMsg);

		//				pImgGrp->Expand(FALSE);
		//				pImgGroup->AddSubItem(pImgGrp);
		//			}
		//			else
		//			{
		//				_delete(pImgGrp);
		//			}
		//		}

		//		m_wndOvlInfo.AddProperty(pImgGroup);
		//	}

		//	// �I�[�o�[���C���
		//	LinkedList<LPDOCOVLINF>* pLstOvly = pDoc->GetOvlyInfList();
		//	if( pLstOvly &&
		//		pLstOvly->first() )
		//	{
		//		// Overlay count
		//		_stprintf_s(szMsg, 255, _T("%d"), pLstOvly->size());
		//		AddSubItem(pGroup, IDS_PAGE_OVLYCNT, szMsg);
	
		//		szText.LoadString(IDS_OVLY_INFO);
		//		CPropertyGridPropertyEx* pOvlGroup = new CPropertyGridPropertyEx(szText);

		//		do
		//		{
		//			LPDOCOVLINF lpOvly = pLstOvly->get();
		//			if( lpOvly )
		//			{
		//				DWORD_PTR dwId = ID_PROP_OVLY + lpOvly->ovlNo;

		//				_stprintf_s(szMsg, 255, _T("Overlay - %d"), lpOvly->ovlNo);
		//				CPropertyGridPropertyEx* pOvlGrp = new CPropertyGridPropertyEx(szMsg, dwId);
		//				
		//				// �I�[�o�[���C�ԍ�
		//				_stprintf_s(szMsg, 255, _T("%d"), lpOvly->ovlNo);
		//				AddSubItem(pOvlGrp, IDS_OBJECT_NO, szMsg);

		//				// �o�^����
		//				_stprintf_s(szMsg, 255, _T("%d"), lpOvly->recCnt);
		//				AddSubItem(pOvlGrp, IDS_OBJECT_CNT, szMsg);
		//				
		//				LinkedList<LPIMAGELINK>* pImgLink = lpOvly->plstImage;
		//				if( pImgLink &&
		//					pImgLink->first() )
		//				{
		//					do
		//					{
		//						LPIMAGELINK pImg = pImgLink->get();
		//						if( pImg )
		//						{
		//							_stprintf_s(szMsg, 255, _T("IMG - %d"), pImg->link.imgNo);
		//							CPropertyGridPropertyEx* pImgGrp = new CPropertyGridPropertyEx(szMsg);
		//							
		//							// �C���[�W�ԍ�
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.imgNo);
		//							AddSubItem(pImgGrp, IDS_OBJECT_NO, szMsg);

		//							// �C���[�W�^�C�v  0:GIF 1:JPG 4:PNG
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.type);
		//							AddSubItem(pImgGrp, IDS_OBJECT_TYPE, szMsg);
		//				
		//							// �`��J�n�w���W
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.x);
		//							AddSubItem(pImgGrp, IDS_OBJECT_X, szMsg);

		//							// �`��J�n�x���W
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.y);
		//							AddSubItem(pImgGrp, IDS_OBJECT_Y, szMsg);

		//							// ��
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.width);
		//							AddSubItem(pImgGrp, IDS_OBJECT_WIDTH, szMsg);

		//							// ����
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.height);
		//							AddSubItem(pImgGrp, IDS_OBJECT_HEIGHT, szMsg);

		//							// �f�[�^�T�C�Y
		//							_stprintf_s(szMsg, 255, _T("%d"), pImg->link.size);
		//							AddSubItem(pImgGrp, IDS_OBJECT_SIZE, szMsg);

		//							pImgGrp->Expand(FALSE);
		//							pOvlGrp->AddSubItem(pImgGrp);
		//						}
		//					} while( pImgLink->next() );
		//				}

		//				pOvlGrp->Expand(FALSE);
		//				pOvlGroup->AddSubItem(pOvlGrp);
		//			}
		//		} while( pLstOvly->next() );

		//		m_wndOvlInfo.AddProperty(pOvlGroup);
		//	}

		//	m_bOvlLoaded = TRUE;
		//}

		// Page Data
		LPPAGECONTENT pContent = pDoc->GetPageContent();
		if( pContent )
		{
			szText.LoadString(IDS_PAGE_INFO);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			// PageNo
			_stprintf_s(szMsg, 255, _T("%d"), pContent->nPageNo);
			AddSubItem(pGroup, IDS_PAGE_NO, szMsg);

			// Width
			_stprintf_s(szMsg, 255, _T("%d"), pContent->width);
			AddSubItem(pGroup, IDS_PAGE_WIDTH, szMsg);

			// height
			_stprintf_s(szMsg, 255, _T("%d"), pContent->height);
			AddSubItem(pGroup, IDS_PAGE_HEIGHT, szMsg);

			// rotate
			_stprintf_s(szMsg, 255, _T("%d"), pContent->rotate);
			AddSubItem(pGroup, IDS_PAGE_ROTATE, szMsg);

			// ������
			if( pContent->plstTextPos && 
				pContent->plstTextPos->size() > 0 )
			{
				_stprintf_s(szMsg, 255, _T("%d"), pContent->plstTextPos->size());
				AddSubItem(pGroup, IDS_PAGE_TEXTCNT, szMsg);
			}

			// �����
			if( pContent->plstLine && 
				pContent->plstLine->first() )
			{
				_stprintf_s(szMsg, 255, _T("%d"), pContent->plstLine->size());
				AddSubItem(pGroup, IDS_PAGE_LINECNT, szMsg);
			}

			// �C���[�W���
			if( pContent->plstImage && 
				pContent->plstImage->first() )
			{
				_stprintf_s(szMsg, 255, _T("%d"), pContent->plstImage->size());
				AddSubItem(pGroup, IDS_PAGE_IMAGENT, szMsg);

				int nImgNo = 0;
				do
				{
					LPIMAGELINK pImgLnk = pContent->plstImage->get();
					if( pImgLnk )
					{
						_stprintf_s(szMsg, 255, _T("IMG - %d"), nImgNo++);
						DWORD_PTR dwId = ID_PROP_IMAGE + nImgNo;
						CPropertyGridPropertyEx* pImgGrp = new CPropertyGridPropertyEx(szMsg, dwId);
						
						// �C���[�W�ԍ�
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.imgNo);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_NO, szMsg);

						// �C���[�W�^�C�v  0:GIF 1:JPG 4:PNG
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.type);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_TYPE, szMsg);
						
						// �`��J�n�w���W
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.x);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_X, szMsg);

						// �`��J�n�x���W
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.y);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_Y, szMsg);

						// ��
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.width);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_WIDTH, szMsg);

						// ����
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.height);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_HEIGHT, szMsg);

						// �f�[�^�T�C�Y
						_stprintf_s(szMsg, 255, _T("%d"), pImgLnk->link.size);
						AddSubItem(pImgGrp, dwId, IDS_OBJECT_SIZE, szMsg);

						pImgGrp->Expand(FALSE);

						pGroup->AddSubItem(pImgGrp);
					}
				} while( pContent->plstImage->next() );
			}

			// �I�[�o�[���C���
			if( pContent->plstOvly && 
				pContent->plstOvly->first() )
			{
				_stprintf_s(szMsg, 255, _T("%d"), pContent->plstOvly->size());
				AddSubItem(pGroup, IDS_PAGE_OVLYCNT, szMsg);

				float scale = pDoc->GetScaleMultiples();
				int nOvlNo = 0;
				do
				{
					LPOVLYLINK pOvlLnk = pContent->plstOvly->get();
					if( pOvlLnk )
					{
						_stprintf_s(szMsg, 255, _T("OVL - %d"), nOvlNo++);
						DWORD_PTR dwId = ID_PROP_OVLY + nOvlNo;

						CPropertyGridPropertyEx* pOvlGrp = new CPropertyGridPropertyEx(szMsg, dwId);

						// �I�[�o�[���CID
						_stprintf_s(szMsg, 255, _T("%d"), pOvlLnk->ovl.ovlid);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_NO, szMsg);

						// �`��J�n�w���W
						_stprintf_s(szMsg, 255, _T("%d"), pOvlLnk->ovl.x);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_X, szMsg);

						// �`��J�n�x���W
						_stprintf_s(szMsg, 255, _T("%d"), pOvlLnk->ovl.y);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_Y, szMsg);

						// ��
						_stprintf_s(szMsg, 255, _T("%d"), pOvlLnk->ovl.width);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_WIDTH, szMsg);

						// ����
						_stprintf_s(szMsg, 255, _T("%d"), pOvlLnk->ovl.height);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_HEIGHT, szMsg);

						float sx = (float)(pOvlLnk->ovl.scalex / scale);
						float sy = (float)(pOvlLnk->ovl.scaley / scale);

						// �g��k���F�c�i���j
						_stprintf_s(szMsg, 255, _T("%f"), sx);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_SCALEX, szMsg);

						// �g��k���F���i���j
						_stprintf_s(szMsg, 255, _T("%f"), sy);
						AddSubItem(pOvlGrp, dwId, IDS_OBJECT_SCALEY, szMsg);

						//pOvlGrp->Expand(FALSE);
						// IMAGE

						LPDOCOVLINF pDocOvlInf = pDoc->GetOverlay(pOvlLnk->ovl.ovlid);
						if( pDocOvlInf )
						{
							LinkedList<LPIMAGELINK>* pImgLink = pDocOvlInf->plstImage;
							if( pImgLink &&
								pImgLink->first() )
							{
								int nImgIdx = 0;
								do
								{
									LPIMAGELINK pImg = pImgLink->get();
									if( pImg )
									{
										_stprintf_s(szMsg, 255, _T("IMG - %d"), nImgIdx++);
										DWORD_PTR dwId = ID_PROP_IMGINOVL + nImgIdx;
										CPropertyGridPropertyEx* pImgGrp = new CPropertyGridPropertyEx(szMsg, dwId);

										// �C���[�W�ԍ�
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.imgNo);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_NO, szMsg);

										// �C���[�W�^�C�v  0:GIF 1:JPG 4:PNG
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.type);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_TYPE, szMsg);
						
										// �`��J�n�w���W
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.x);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_X, szMsg);

										// �`��J�n�x���W
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.y);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_Y, szMsg);

										// ��
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.width);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_WIDTH, szMsg);

										// ����
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.height);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_HEIGHT, szMsg);

										// �f�[�^�T�C�Y
										_stprintf_s(szMsg, 255, _T("%d"), pImg->link.size);
										AddSubItem(pImgGrp, dwId, IDS_OBJECT_SIZE, szMsg);

										pImgGrp->Expand(FALSE);
										pOvlGrp->AddSubItem(pImgGrp);
									}
								} while( pImgLink->next() );
							}
						}
						// IMAGE <<
						pOvlGrp->Expand(FALSE);

						pGroup->AddSubItem(pOvlGrp);
					}
				} while( pContent->plstOvly->next() );
			}

			m_wndDocInfo.AddProperty(pGroup);	
		}
	}
}

void CPageInfoPropertiesWnd::OnReload()
{
	m_bOvlLoaded = FALSE;

	Refresh();
}

LRESULT CPageInfoPropertiesWnd::OnChangeSelection(WPARAM wParam, LPARAM lParam)
{
	DWORD_PTR dwNewSel = (DWORD_PTR)wParam;
	DWORD_PTR dwOldSel = (DWORD_PTR)lParam;

	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pView->SetHighLightID((INT)dwNewSel);
	}

	return 0L;
}

