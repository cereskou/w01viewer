#include "stdafx.h"
#include "w01draw.h"
#include "w01drawDoc.h"
#include "w01drawView.h"
#include "DocInfoPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_CTRL_DOCINFO		0x2004

IMPLEMENT_DYNAMIC(CDocInfoPropertiesWnd, CBasePropertiesWnd)

CDocInfoPropertiesWnd::CDocInfoPropertiesWnd(void)
{
}


CDocInfoPropertiesWnd::~CDocInfoPropertiesWnd(void)
{
}

BEGIN_MESSAGE_MAP(CDocInfoPropertiesWnd, CBasePropertiesWnd)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_PAGEINF_RELOAD, &CDocInfoPropertiesWnd::OnReload)
	ON_UPDATE_COMMAND_UI(ID_PAGEINF_RELOAD, &CDocInfoPropertiesWnd::OnUpdateReload)
END_MESSAGE_MAP()

int CDocInfoPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndDocInfo.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_DOCINFO))
	{
		TRACE0("プロパティ グリッドを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_FONTXML);
	m_wndToolBar.LoadToolBar(IDR_PAGEINFO);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	InitPropList();

	return 0;
}

void CDocInfoPropertiesWnd::AdjustLayout()
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

void CDocInfoPropertiesWnd::InitPropList()
{
	SetControlFont();

	m_wndDocInfo.SetLeftColumnWidth(130);

	m_wndDocInfo.EnableHeaderCtrl(FALSE);
	m_wndDocInfo.EnableDescriptionArea(FALSE);
	m_wndDocInfo.MarkModifiedProperties(TRUE);
}

void CDocInfoPropertiesWnd::SetControlFont()
{
	m_wndDocInfo.SetFont(GetBaseFont());
}

void CDocInfoPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CBasePropertiesWnd::OnSetFocus(pOldWnd);

	m_wndDocInfo.SetFocus();
}

void CDocInfoPropertiesWnd::Refresh(BOOL bClosed)
{
	CBasePropertiesWnd::Refresh(bClosed);

	TRACE(_T("CDocInfoPropertiesWnd::Refresh()\n"));

	CW01DrawDoc* pDoc = NULL;
	CW01DrawView* pView = (CW01DrawView*)GetWindow();
	if( pView )
	{
		pDoc = pView->GetDocument();
	}
	m_wndDocInfo.RemoveAll();

	if( !bClosed &&
		pDoc )
	{
		TCHAR szMsg[256] = {0};
		TCHAR szVal[256] = {0};
		CString szText;

		WEB_HEADER2 head = {0};
		if( pDoc->GetDocHeader(&head) )
		{
			szText.LoadString(IDS_HEADER_INFO);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			// version
			_stprintf_s(szMsg, 255, _T("%d"), head.head1.version);
			AddSubItem(pGroup, IDS_TXT_VERSION, szMsg);

			// createDate
			_stprintf_s(szMsg, 255, _T("%s"), head.createDate);
			AddSubItem(pGroup, IDS_TXT_CREATEDATE, szMsg);

			// updateDate
			_stprintf_s(szMsg, 255, _T("%s"), head.updateDate);
			AddSubItem(pGroup, IDS_TXT_UPDATEDATE, szMsg);

			// recordLen
			_stprintf_s(szMsg, 255, _T("%d"), head.recordLen);
			AddSubItem(pGroup, IDS_TXT_RECORDLEN, szMsg);

			// totalPage
			_stprintf_s(szMsg, 255, _T("%d"), head.totalPage);
			AddSubItem(pGroup, IDS_TXT_TOTALPAGE, szMsg);

			// registRecNo
			_stprintf_s(szMsg, 255, _T("%d"), head.registRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_REC, szMsg);

			_stprintf_s(szMsg, 255, _T("%d"), head.userRecNo );
			AddSubItem(pGroup, IDS_TXT_NO_USERINFO, szMsg);

			_stprintf_s(szMsg, 255, _T("%d"), head.fontRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_FONT, szMsg);

			// ovlyRecNo
			_stprintf_s(szMsg, 255, _T("%d"), head.ovlyRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_OVLY, szMsg);

			// pageRecNo
			_stprintf_s(szMsg, 255, _T("%d"), head.pageRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_PAGE, szMsg);

			// Image
			_stprintf_s(szMsg, 255, _T("%d"), head.imageRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_IMAGE, szMsg);

			// textRecNo
			_stprintf_s(szMsg, 255, _T("%d"), head.positionRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_TEXTTBL, szMsg);

			// groupRecNo
			_stprintf_s(szMsg, 255, _T("%d"), head.groupRecNo);
			AddSubItem(pGroup, IDS_TXT_NO_TEXTPOS, szMsg);

			// commentCnt
			_stprintf_s(szMsg, 255, _T("%d"), head.commentCnt);
			AddSubItem(pGroup, IDS_TXT_NO_COMMENT, szMsg);

			// stampCnt
			_stprintf_s(szMsg, 255, _T("%d"), head.stampCnt);
			AddSubItem(pGroup, IDS_TXT_NO_STAMP, szMsg);

			// tagCnt
			_stprintf_s(szMsg, 255, _T("%d"), head.tagCnt);
			AddSubItem(pGroup, IDS_TXT_NO_TAG, szMsg);

			// markerCnt
			_stprintf_s(szMsg, 255, _T("%d"), head.markerCnt);
			AddSubItem(pGroup, IDS_TXT_NO_MARKER, szMsg);


			m_wndDocInfo.AddProperty(pGroup);	
		}

		// Page
		LinkedList<LPDOCPAGE>* plstPage = pDoc->ReadAllPageInfo();
		if( plstPage &&
			plstPage->first() )
		{
			szText.LoadString(IDS_PAGE_INFO);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			int pageNo = 0;
			do
			{
				LPDOCPAGE pDocPage = plstPage->get();
				if( pDocPage )
				{
					_stprintf_s(szMsg, 255, _T("PAGE - %d"), pageNo++);
					CPropertyGridPropertyEx* pPageGrp = new CPropertyGridPropertyEx(szMsg);

					// PageNo
					_stprintf_s(szMsg, 255, _T("%d"), pDocPage->pageNo);
					AddSubItem(pPageGrp, IDS_PAGE_NO, szMsg);

					// ページ情報
					_stprintf_s(szMsg, 255, _T("%lu"), pDocPage->pageDef.ulRecNo);
					AddSubItem(pPageGrp, IDS_TXT_RECORD, szMsg);

					// 幅
					_stprintf_s(szMsg, 255, _T("%d"), pDocPage->pageDef.def.width);
					AddSubItem(pPageGrp, IDS_PAGE_WIDTH, szMsg);

					// 高さ
					_stprintf_s(szMsg, 255, _T("%d"), pDocPage->pageDef.def.height);
					AddSubItem(pPageGrp, IDS_PAGE_HEIGHT, szMsg);

					// 回転
					_stprintf_s(szMsg, 255, _T("%d"), pDocPage->pageDef.def.rotate);
					AddSubItem(pPageGrp, IDS_PAGE_ROTATE, szMsg);

					// テキスト
					// 文字列属性情報登録レコードNO
					_stprintf_s(szMsg, 255, _T("%lu"), pDocPage->txtRecNo);
					AddSubItem(pPageGrp, IDS_TXT_NO_TEXTPOS, szMsg);

					// 文字列属性情報登録情報数
					_stprintf_s(szMsg, 255, _T("%lu"), pDocPage->txtCnt);
					AddSubItem(pPageGrp, IDS_PAGE_TEXTCNT, szMsg);
					
					// 登録件数
					_stprintf_s(szMsg, 255, _T("%lu"), pDocPage->recCnt);
					AddSubItem(pPageGrp, IDS_OBJECT_CNT, szMsg);

					pPageGrp->Expand(FALSE);

					pGroup->AddSubItem(pPageGrp);
				}

			} while( plstPage->next() );

			pGroup->Expand(FALSE);
			m_wndDocInfo.AddProperty(pGroup);	
		}
		_delete(plstPage);

		// ユーザー情報
		LinkedList<WEB_USER_INFO_DATA*>* pUsrInfList = pDoc->GetUserInfoList();
		if( pUsrInfList &&
			pUsrInfList->first() )
		{
			szText.LoadString(IDS_USER_INFO);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			do
			{
				WEB_USER_INFO_DATA* pUser = pUsrInfList->get();
				if( pUser )
				{
					//if( _wcsicmp(pUser->pKey, L"FORMTYPE") == 0 )
					//{
					//	// formtype
					//	_stprintf_s(szMsg, 255, _T("%X"), (ULONG)(*pUser->pData));
					//	AddSubItem(pGroup, pUser->pKey, szMsg);
					//}
					//else
					{
						AddSubItem(pGroup, pUser->pKey, pUser->pData);
					}
				}
			} while( pUsrInfList->next());

			pGroup->Expand(FALSE);

			m_wndDocInfo.AddProperty(pGroup);	
		}

		// フォント情報
		LinkedList<WEB_FONT_DATA*>* pFontList = pDoc->GetFontList();
		if( pFontList &&
			pFontList->first() )
		{
			szText.LoadString(IDS_FONT_INFO);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);
			int nFontId = 0;
			do
			{
				WEB_FONT_DATA* pFont = pFontList->get();
				if( pFont )
				{
					_stprintf_s(szMsg, 255, _T("%d"), nFontId++);
					AddSubItem(pGroup, szMsg, pFont->name);
				}
			} while( pFontList->next());

			pGroup->Expand(FALSE);
			m_wndDocInfo.AddProperty(pGroup);	
		}

		// イメージ情報
		CHashTblImage* pImgMap = pDoc->GetImageMap();
		if( pImgMap &&
			pImgMap->size() > 0 )
		{
			szText.LoadString(IDS_TXT_NO_IMAGE);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			USHORT nImgNo = 0;
			LPIMAGEDATA pImgData = NULL;
			PPOSITION pos = pImgMap->first();
			while( pos != NULL )
			{
				pImgMap->next(pos, nImgNo, pImgData);
				if( pImgData )
				{
					_stprintf_s(szMsg, 255, _T("%d"), nImgNo);
					CPropertyGridPropertyEx* pImgGrp = new CPropertyGridPropertyEx(szMsg);
					//
					_tcscpy_s(szMsg, 255, _T("Width"));
					_stprintf_s(szVal, 255, _T("%d"), pImgData->width);
					AddSubItem(pImgGrp, szMsg, szVal);

					_tcscpy_s(szMsg, 255, _T("Height"));
					_stprintf_s(szVal, 255, _T("%d"), pImgData->height);
					AddSubItem(pImgGrp, szMsg, szVal);

					_tcscpy_s(szMsg, 255, _T("Format"));
					_stprintf_s(szVal, 255, _T("%d"), pImgData->type);
					AddSubItem(pImgGrp, szMsg, szVal);

					_tcscpy_s(szMsg, 255, _T("Image Size"));
					_stprintf_s(szVal, 255, _T("%lu"), pImgData->length);
					AddSubItem(pImgGrp, szMsg, szVal);

					pImgGrp->Expand(FALSE);

					pGroup->AddSubItem(pImgGrp);
				}
			}
			pGroup->Expand(FALSE);

			m_wndDocInfo.AddProperty(pGroup);	
		}

		//@QHGOU 20150722 >>
		// イメージ情報
		CHashTblBinary* pBinMap = pDoc->GetBinaryMap();
		if( pBinMap &&
			pBinMap->size() > 0 )
		{
			szText.LoadString(IDS_TXT_NO_BINARY);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			USHORT nBinNo = 0;
			LPBINDATA pBinData = NULL;
			PPOSITION pos = pBinMap->first();
			while( pos != NULL )
			{
				pBinMap->next(pos, nBinNo, pBinData);
				if( pBinData )
				{
					_stprintf_s(szMsg, 255, _T("%d"), nBinNo);
					CPropertyGridPropertyEx* pBinGrp = new CPropertyGridPropertyEx(szMsg);
					//
					_tcscpy_s(szMsg, 255, _T("Type"));
					_stprintf_s(szVal, 255, _T("%d"), pBinData->type);
					AddSubItem(pBinGrp, szMsg, szVal);

					_tcscpy_s(szMsg, 255, _T("Length"));
					_stprintf_s(szVal, 255, _T("%d"), pBinData->nDataLength);
					AddSubItem(pBinGrp, szMsg, szVal);

					_tcscpy_s(szMsg, 255, _T("ExtLength"));
					_stprintf_s(szVal, 255, _T("%d"), pBinData->nExDataLength);
					AddSubItem(pBinGrp, szMsg, szVal);

					pBinGrp->Expand(FALSE);

					pGroup->AddSubItem(pBinGrp);
				}
			}
			pGroup->Expand(FALSE);

			m_wndDocInfo.AddProperty(pGroup);	
		}
		//@QHGOU 20150722 <<

		// オーバーレイ情報
		LinkedList<LPDOCOVLINF>* pOvlyList = pDoc->GetOvlyInfList();
		if( pOvlyList &&
			pOvlyList->first() )
		{
			szText.LoadString(IDS_TXT_NO_OVLY);
			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

			do
			{
				LPDOCOVLINF lpOvlInf = pOvlyList->get();
				if( lpOvlInf )
				{
					_stprintf_s(szMsg, 255, _T("%d"), lpOvlInf->ovlyIndex);
					CPropertyGridPropertyEx* pOvlGrp = new CPropertyGridPropertyEx(szMsg);

					int nChildCnt = 0;
					// 線
					if( lpOvlInf->plstLine )
						nChildCnt = lpOvlInf->plstLine->size();
					_tcscpy_s(szMsg, 255, _T("Lines"));
					_stprintf_s(szVal, 255, _T("%d"), nChildCnt);
					AddSubItem(pOvlGrp, szMsg, szVal);

					// イメージ
					nChildCnt = 0;
					if( lpOvlInf->plstImage )
						nChildCnt = lpOvlInf->plstImage->size();
					_tcscpy_s(szMsg, 255, _T("Images"));
					_stprintf_s(szVal, 255, _T("%d"), nChildCnt);
					AddSubItem(pOvlGrp, szMsg, szVal);
					// 文字数
					nChildCnt = 0;
					if( lpOvlInf->plstTextPos )
						nChildCnt = lpOvlInf->plstTextPos->size();
					_tcscpy_s(szMsg, 255, _T("文字数"));
					_stprintf_s(szVal, 255, _T("%d"), nChildCnt);
					AddSubItem(pOvlGrp, szMsg, szVal);

					pOvlGrp->Expand(FALSE);

					pGroup->AddSubItem(pOvlGrp);
				}
			} while( pOvlyList->next() );

			pGroup->Expand(FALSE);
			m_wndDocInfo.AddProperty(pGroup);	
		}

		//// 1. フォント
		//szText.LoadString(IDS_FONT_INFO);
		//CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

		//LinkedList<WEB_FONT_DATA*>* pFontList = pDoc->GetFontList();
		//if( pFontList &&
		//	pFontList->first() )
		//{
		//	WEB_FONT_DATA* pFont = NULL;
		//	do
		//	{
		//		pFont = pFontList->get();
		//		if( pFont )
		//		{
		//			szVal = _T("");
		//			szText = pFont->name;

		//			CSelectFontPropertyGridProperty* pCtrl = new CSelectFontPropertyGridProperty(szText, szVal);
		//			if( pCtrl )
		//			{
		//				pGroup->AddSubItem(pCtrl);

		//				AddToMap(pFont->name, pCtrl);
		//			}
		//			
		//		}
		//	} while( pFontList->next() );
		//}

		//m_wndFontList.AddProperty(pGroup);	
	}
}

void CDocInfoPropertiesWnd::OnReload()
{
	Refresh();
}

void CDocInfoPropertiesWnd::OnUpdateReload(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetWindow() != NULL);
}
