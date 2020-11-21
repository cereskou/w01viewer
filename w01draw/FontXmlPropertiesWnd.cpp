// FontXmlPropertiesWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "FontXmlPropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_CTRL_XMLFONT		0x2003
#define ID_XML_COMMON		0x1000
#define ID_XML_FONTS		0x2000

// CFontXmlPropertiesWnd

IMPLEMENT_DYNAMIC(CFontXmlPropertiesWnd, CBasePropertiesWnd)

CFontXmlPropertiesWnd::CFontXmlPropertiesWnd()
{
	m_bUsingFont = FALSE;
}

CFontXmlPropertiesWnd::~CFontXmlPropertiesWnd()
{
}


BEGIN_MESSAGE_MAP(CFontXmlPropertiesWnd, CBasePropertiesWnd)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_XML_RELOAD, &CFontXmlPropertiesWnd::OnReload)
	ON_COMMAND(ID_XML_OPEN, &CFontXmlPropertiesWnd::OnOpenXML)
	ON_COMMAND(ID_XML_SAVE, &CFontXmlPropertiesWnd::OnSaveXML)
	ON_UPDATE_COMMAND_UI(ID_XML_SAVE, &CFontXmlPropertiesWnd::OnUpdateSaveXML)
	ON_COMMAND(ID_XML_COLLAPSE, &CFontXmlPropertiesWnd::OnCollapse)
	ON_COMMAND(ID_XML_EXPAND, &CFontXmlPropertiesWnd::OnExpand)
	ON_COMMAND(ID_XML_DRAWFONT, &CFontXmlPropertiesWnd::OnUsingFontDraw)
	ON_UPDATE_COMMAND_UI(ID_XML_DRAWFONT, &CFontXmlPropertiesWnd::OnUpdateUsingFontDraw)
END_MESSAGE_MAP()

	


// CFontXmlPropertiesWnd メッセージ ハンドラー
int CFontXmlPropertiesWnd::CreateControl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndXMLList.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, ID_CTRL_XMLFONT))
	{
		TRACE0("プロパティ グリッドを作成できませんでした\n");
		return -1;      // 作成できない場合
	}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_FONTXML);
	m_wndToolBar.LoadToolBar(IDR_FONTXML);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// すべてのコマンドが、親フレーム経由ではなくこのコントロール経由で渡されます:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	InitPropList();

	return 0;
}

void CFontXmlPropertiesWnd::AdjustLayout()
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
	m_wndXMLList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), nHeight - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFontXmlPropertiesWnd::InitPropList()
{
	SetControlFont();

	m_wndXMLList.EnableHeaderCtrl(FALSE);
	m_wndXMLList.EnableDescriptionArea(FALSE);
	m_wndXMLList.MarkModifiedProperties(TRUE);

	CString szVal;
	CString szText;
	// 1. ファイル
	szText.LoadString(IDS_XMLF_INFO);
	CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText);

	CString szFilter;
	szFilter.LoadString(IDS_XMLF_FILTER);
	CMFCPropertyGridFileProperty* pCtrl = new CMFCPropertyGridFileProperty(szText, TRUE, _T(""), _T("xml"), 0, szFilter, _T(""), IDS_XMLF_FILTER);
	if( pCtrl )
	{
		pGroup->AddSubItem(pCtrl);

		AddToMap(IDS_XMLF_INFO, pCtrl);
	}

	m_wndXMLList.AddProperty(pGroup);	

}

void CFontXmlPropertiesWnd::SetControlFont()
{
	m_wndXMLList.SetFont(GetBaseFont());
}

void CFontXmlPropertiesWnd::ClearXMLFont()
{
	CHashTblStrToProperty* pMap = GetStrToPropTable();

	if( !pMap ||
		!pMap->size() )
		return;

	CWaitCursor wait;
	LPTSTR pszName = NULL;
	CMFCPropertyGridProperty* pGroup = NULL;
	PPOSITION pos = pMap->first();
	while( pos )
	{
		pMap->next(pos, pszName, pGroup );
		if( pGroup )
		{
			m_wndXMLList.DeleteProperty(pGroup, FALSE, FALSE);
		}
	}
	pMap->removeall();
}

BOOL CFontXmlPropertiesWnd::LoadFontXML(LPCTSTR lpszName)
{
	if( m_xmlFont.Open(lpszName) )
	{
		// Clear Old
		ClearXMLFont();

		// Common
		CHashTblIntToXmlData* pDataList = m_xmlFont.GetXmlData();
		if( pDataList )
		{
			CString szText;
			m_wndXMLList.SetRedraw(FALSE);

			szText.LoadString(IDS_XML_COMMON);
			DWORD_PTR dwID = ID_XML_COMMON + 1;

			CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(szText, dwID);
			if( pGroup )
			{
				AddToMap((LPTSTR)(LPCTSTR)szText, pGroup);

				int inspos = 1;
				LPXMLDATA pData = NULL;
				int nID = 0;
				PPOSITION pos = pDataList->first();
				while( pos != NULL )
				{
					dwID += inspos;
					// Get Data
					pDataList->next(pos, nID, pData);
					CString szName  = pData->name;
					CString szValue = pData->value;

					if( nID == ID_DEFAULTFONTNAME )
					{
						CSelectFontPropertyGridProperty* pDataItem = new CSelectFontPropertyGridProperty(szName, szValue, RGB(255, 220, 220), nID);
						if( pDataItem )
						{
							pGroup->AddSubItem(pDataItem);

							inspos++;
						}
					}
					else
					{
						CPropertyGridPropertyEx* pDataItem = new CPropertyGridPropertyEx(szName, (_variant_t)szValue, _T(""), nID);
						if( pDataItem )
						{
							pGroup->AddSubItem(pDataItem);

							inspos++;
						}
					}
				}

				m_wndXMLList.AddProperty(pGroup);
			}
			m_wndXMLList.SetRedraw(TRUE);
			m_wndXMLList.Invalidate();
			m_wndXMLList.UpdateWindow();
		}

		CHashTblStringToOutFont* pFontList = m_xmlFont.GetFontList();
		if( pFontList )
		{
			m_wndXMLList.SetRedraw(FALSE);

			DWORD_PTR dwBaseID = ID_XML_FONTS + 1;
			CString szName;
			CString szValue;
			int inspos = 0;
			PPOSITION pos = pFontList->first(true);
			while( pos )
			{
				WCHAR* pwszFontName = NULL;
				LPOUTFONT lpFont = NULL;
				
				pFontList->next(pos, pwszFontName, lpFont);
				if( lpFont )
				{
					DWORD_PTR dwID = dwBaseID + inspos;
					CPropertyGridPropertyEx* pGroup = new CPropertyGridPropertyEx(lpFont->fontName, dwID);
					if( pGroup )
					{
						AddToMap(lpFont->fontName, pGroup);

						// fontName
						szName = _T("FontName");
						szValue = lpFont->fontName;
						CPropertyGridPropertyEx* fontName = new CPropertyGridPropertyEx(szName, (_variant_t)szValue, _T(""), 9);
						if( fontName )
						{
							pGroup->AddSubItem(fontName);
						}

						// displayName
						szName = _T("DisplayFontName");
						szValue = lpFont->displayName;
						CSelectFontPropertyGridProperty* displayName = new CSelectFontPropertyGridProperty(szName, szValue, RGB(255, 255, 235), 1);
						if( displayName )
						{
							pGroup->AddSubItem(displayName);
						}

						// printName
						szName = _T("PrintFontName");
						szValue = lpFont->printName;
						CPropertyGridPropertyEx* printName = new CPropertyGridPropertyEx(szName, (_variant_t)szValue, _T(""), 2);
						if( printName )
						{
							pGroup->AddSubItem(printName);
						}

						//// mode
						//szName = _T("Mode");
						////szValue = lpFont->mode;
						//CPropertyGridPropertyEx* mode = new CPropertyGridPropertyEx(szName, (_variant_t)lpFont->mode, _T(""), 3);
						//if( mode )
						//{
						//	mode->AllowEdit(FALSE);
						//	pGroup->AddSubItem(mode);
						//}

						// Like
						szName = _T("Like");
						CPropertyGridPropertyEx* like = new CPropertyGridPropertyEx(szName, (_variant_t)lpFont->like, _T(""), 4);
						if( like )
						{
							pGroup->AddSubItem(like);
						}

						//pGroup->Expand(FALSE);

						m_wndXMLList.AddProperty(pGroup);
					}
					
				}
				inspos++;
			}
			m_wndXMLList.SetRedraw(TRUE);
			m_wndXMLList.Invalidate();
			m_wndXMLList.UpdateWindow();
		}
	}
	SetModified(FALSE);

	return TRUE;
}

void CFontXmlPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndXMLList.SetFocus();
}

void CFontXmlPropertiesWnd::OnPropertyChanged(DWORD_PTR nID, CPropertyGridPropertyEx* pProp)
{
	if( ID_CTRL_XMLFONT == nID && 
		pProp != NULL )
	{
		int nID = pProp->GetData();
		if( nID == IDS_XMLF_FILTER )
		{
			CString szName = pProp->GetValue();

			LoadFontXML(szName);
		}
		else
		{
			SetModified();
		}
	}
}


void CFontXmlPropertiesWnd::OnOpenXML()
{
	CString szFilter;
	szFilter.LoadString(IDS_XMLF_FILTER);

	CFileDialog dlg(TRUE, 
				NULL, 
				_T("Fontlist.xml"), 
				OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
				szFilter, 
				this);

	if( dlg.DoModal() == IDOK )
	{

		if( LoadFontXML(dlg.GetPathName()) )
		{
			CMFCPropertyGridFileProperty* pSelFile = (CMFCPropertyGridFileProperty*)GetFromMap(IDS_XMLF_INFO);
			if( pSelFile )
			{
				pSelFile->SetValue(dlg.GetPathName());
			}
		}
	}
}

void CFontXmlPropertiesWnd::OnReload()
{
	
	CString szName = m_xmlFont.GetFileName();
	if( !szName.IsEmpty() )
		LoadFontXML(szName);
}

void CFontXmlPropertiesWnd::OnSaveXML()
{
	if( !IsModified() )
		return;

	CHashTblStrToProperty* pMap = GetStrToPropTable();

	if( !pMap ||
		!pMap->size() )
		return;

	CHashTblStringToOutFont* pfontOut = new CHashTblStringToOutFont();
	if( pfontOut == NULL ||
		pfontOut->create(genHashKeyStrW, true) == NULL )
	{
		return;
	}
	
	int nUpdFlag = 0;
	OUTFONT outfont = {0};

	CHashTblIntToXmlData* pDataMap = m_xmlFont.GetXmlData();

	CMFCPropertyGridProperty* pGroup = NULL;
	LPTSTR pszName = NULL;
	PPOSITION pos = pMap->first();
	while( pos != NULL )
	{
		ZeroMemory(&outfont, sizeof(OUTFONT));
		
		pMap->next(pos, pszName, pGroup);
		
		if( pGroup &&
			pGroup->IsGroup() )
		{
			nUpdFlag = 0;
			CString pValue;
			BOOL bValue = FALSE;

			DWORD_PTR dwId = pGroup->GetData();
			if( dwId > ID_XML_COMMON && dwId < ID_XML_FONTS )
			{
				// Get Child
				int nCount = pGroup->GetSubItemsCount();
				for(int i = 0; i < nCount; i++)
				{
					CMFCPropertyGridProperty* pChild = pGroup->GetSubItem(i);
					if( pChild && pChild->IsModified() )
					{
						DWORD_PTR dwID = pChild->GetData();

						LPXMLDATA pData = pDataMap->get(dwID);
						if( pData )
						{
							pValue = pChild->GetValue();
							wcscpy_s(pData->value, 256, pValue);
						}
					}
				}
			}
			else if( dwId > ID_XML_FONTS )
			{
				// Get Child
				int nCount = pGroup->GetSubItemsCount();
				for(int i = 0; i < nCount; i++)
				{
					CMFCPropertyGridProperty* pChild = pGroup->GetSubItem(i);
					if( pChild && pChild->IsModified() )
					{
						DWORD_PTR dwID = pChild->GetData();
						switch(dwID)
						{
						case 1:
							pValue = pChild->GetValue();
							_tcscpy_s(outfont.displayName, 63, pValue);
							nUpdFlag |= UPD_DISPNAME;
							break;
						case 2:
							pValue = pChild->GetValue();
							_tcscpy_s(outfont.printName, 63, pValue);
							nUpdFlag |= UPD_PRNTNAME;
							break;
						case 3:
							nUpdFlag |= UPD_MODE;
							break;
						case 4:
							bValue = (BOOL)pChild->GetValue().boolVal;
							outfont.like = (bValue == FALSE) ? false : true;
							nUpdFlag |= UPD_LIKE;
							break;
						case 9:
							pValue = pChild->GetValue();
							_tcscpy_s(outfont.baseName, 63, pValue);
							nUpdFlag |= UPD_FONTNAME;
						}
					}
				}

				if( nUpdFlag != 0 )
				{
					LPCTSTR pszName = pGroup->GetName();

					LPOUTFONT pFont = pfontOut->get((LPTSTR)pszName);
					if( pFont == NULL )
					{
						pFont = (LPOUTFONT)malloc(sizeof(OUTFONT));
						if( pFont )
						{
							memcpy(pFont, &outfont, sizeof(OUTFONT));

							_tcscpy_s(pFont->fontName, 63, pszName);
							pFont->flags = nUpdFlag;

							pfontOut->insert((LPTSTR)pszName, pFont);
						}
					}
					else
					{
						pFont->flags = nUpdFlag;

						if( nUpdFlag & UPD_FONTNAME )
							_tcscpy_s(pFont->baseName, 63, outfont.baseName);

						if( nUpdFlag & UPD_DISPNAME )
							_tcscpy_s(pFont->displayName, 63, outfont.displayName);

						if( nUpdFlag & UPD_PRNTNAME )
							_tcscpy_s(pFont->printName, 63, outfont.printName);

						if( nUpdFlag & UPD_LIKE )
							pFont->like = outfont.like;
					}
				}
			}
		}

	}

	if( !m_xmlFont.Save(pfontOut) )
	{
		AfxMessageBox(IDS_ERR_FILESAVE, MB_OK | MB_ICONERROR);
	}
	//else
	//{
	//	OnReload();
	//}

	_delete(pfontOut);
}


void CFontXmlPropertiesWnd::OnUpdateSaveXML(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsModified());
}

void CFontXmlPropertiesWnd::OnCollapse()
{
	CMFCPropertyGridFileProperty* pSelFile = (CMFCPropertyGridFileProperty*)GetFromMap(IDS_XMLF_INFO);
	if( pSelFile )
	{
		m_wndXMLList.ExpandAll(FALSE);
		if( pSelFile->GetParent() )
		{
			pSelFile->GetParent()->Expand(TRUE);
		}
	}
}

void CFontXmlPropertiesWnd::OnExpand()
{
	m_wndXMLList.ExpandAll(TRUE);
}

void CFontXmlPropertiesWnd::OnUsingFontDraw()
{
	if( m_bUsingFont )
	{
		m_bUsingFont = FALSE;
		CMFCFontComboBox::m_bDrawUsingFont = FALSE;
	}
	else
	{
		m_bUsingFont = TRUE;
		CMFCFontComboBox::m_bDrawUsingFont = TRUE;
	}
}

void CFontXmlPropertiesWnd::OnUpdateUsingFontDraw(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bUsingFont);
}
