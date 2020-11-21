// BasePropertiesWnd.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "BasePropertiesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CBasePropertiesWnd

IMPLEMENT_DYNAMIC(CBasePropertiesWnd, CDockablePane)

CBasePropertiesWnd::CBasePropertiesWnd()
{
	m_pTblStrProp = NULL;
	m_pTblLongProp = NULL;

	m_bModified = FALSE;

	m_pWnd = NULL;
}

CBasePropertiesWnd::~CBasePropertiesWnd()
{
	_delete(m_pTblStrProp);
	_delete(m_pTblLongProp);
}


BEGIN_MESSAGE_MAP(CBasePropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()



// CBasePropertiesWnd メッセージ ハンドラー


int CBasePropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pTblStrProp = new CHashTblStrToProperty();
	if( m_pTblStrProp == NULL ||
		m_pTblStrProp->create(genHashKeyStr, false, 128) == NULL )
	{
		return -1;
	}
	m_pTblLongProp = new CHashTblLongToProperty();
	if( m_pTblLongProp == NULL ||
		m_pTblLongProp->create(genHashKey, false, 128) == NULL )
	{
		return -1;
	}

	InitControlFont();

	if( CreateControl() < 0 )
		return -1;

	AdjustLayout();

	return 0;
}


void CBasePropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
}


void CBasePropertiesWnd::InitControlFont()
{
	::DeleteObject(m_baseFont.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_baseFont.CreateFontIndirect(&lf);
}

void CBasePropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);

	SetControlFont();
}

void CBasePropertiesWnd::SetWindow(CWnd* pWnd, BOOL bForce)
{
	m_pWnd = pWnd;

	if( m_pTblStrProp )
		m_pTblStrProp->removeall();
	if( m_pTblLongProp )
		m_pTblLongProp->removeall();

	if( !bForce )
	{
		if( !(GetStyle () & WS_VISIBLE) )
			return;
	}

	Refresh(bForce);
}

BOOL CBasePropertiesWnd::IsModified()
{
	BOOL bModified = FALSE;
	CHashTblStrToProperty* pMap = GetStrToPropTable();
	if( pMap )
	{
		LPTSTR pKey = NULL;
		CMFCPropertyGridProperty* pProp = NULL;
		PPOSITION pos = pMap->first();
		while( pos )
		{
			pMap->next(pos, pKey, pProp);
			if( pProp )
			{
				if( pProp->IsModified() )
				{
					bModified = TRUE;
					break;
				}
			}
		}
	}

	return bModified | m_bModified;
}

void CBasePropertiesWnd::AddToMap(LPTSTR pszKey, CMFCPropertyGridProperty* pProp)
{
	CHashTblStrToProperty* pMap = GetStrToPropTable();
	if( pMap && pProp )
	{
		pMap->add(pszKey, pProp);
	}
}

void CBasePropertiesWnd::AddToMap(LONG lKey, CMFCPropertyGridProperty* pProp)
{
	CHashTblLongToProperty* pMap = GetLongToPropTable();
	if( pMap && pProp )
	{
		pMap->add(lKey, pProp);
	}
}

CMFCPropertyGridProperty* CBasePropertiesWnd::GetFromMap(LPTSTR pszKey)
{
	CHashTblStrToProperty* pMap = GetStrToPropTable();
	if( pMap )
	{
		return pMap->get(pszKey);
	}
	return NULL;
}

CMFCPropertyGridProperty* CBasePropertiesWnd::GetFromMap(LONG lKey)
{
	CHashTblLongToProperty* pMap = GetLongToPropTable();
	if( pMap )
	{
		return pMap->get(lKey);
	}
	return NULL;
}

LRESULT CBasePropertiesWnd::OnPropertyChanged(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("OnPropertyChanged wParam:%lu, lParam:%lu\n"), wParam, lParam);
	static BOOL bInProgress = FALSE;
	if( bInProgress )
		return 0L;

	bInProgress = TRUE;
	OnPropertyChanged((DWORD_PTR)wParam, (CPropertyGridPropertyEx*)(lParam));
	bInProgress = FALSE;

	return 0L;
}


int CBasePropertiesWnd::SyncTo(CHashTblStrToProperty* pMap, BOOL bGroup, DWORD_PTR nID)
{
	if( pMap == NULL )
		return 0;

	int nUpdCount = 0;
	CHashTblStrToProperty* pSrc = GetStrToPropTable();
	LPTSTR pszKey = NULL;
	CMFCPropertyGridProperty* pPropS = NULL;
	CMFCPropertyGridProperty* pPropT = NULL;
	PPOSITION pos = pSrc->first();
	while( pos != NULL )
	{
		pSrc->next(pos, pszKey, pPropS);
		if( pPropS )
		{
			pPropT = pMap->get(pszKey);
			if( pPropT )
			{
				if( bGroup && 
					pPropT->IsGroup() )
				{
					int nCount = pPropT->GetSubItemsCount();
					for(int i = 0; i < nCount; i++)
					{
						CPropertyGridPropertyEx* pChild = (CPropertyGridPropertyEx*)pPropT->GetSubItem(i);
						if( pChild && 
							nID == pChild->GetData() )
						{
							pChild->SetValue(pPropS->GetValue());
							nUpdCount++;
							pChild->SetModified(TRUE);
							
						}
					}
				}
				else
				{
					pPropT->SetValue(pPropS->GetValue());
					nUpdCount++;
				}
			}
		}
	}
	if( nUpdCount > 0 )
		SetModified(TRUE);

	return nUpdCount;
}

int CBasePropertiesWnd::SyncFrom(CHashTblStrToProperty* pMap, BOOL bGroup, DWORD_PTR nID)
{
	if( pMap == NULL )
		return 0;

	int nUpdCount = 0;
	CHashTblStrToProperty* pMapTo = GetStrToPropTable();
	LPTSTR pszKey = NULL;
	CMFCPropertyGridProperty* pPropS = NULL;
	CMFCPropertyGridProperty* pPropT = NULL;
	PPOSITION pos = pMap->first();
	while( pos != NULL )
	{
		pMap->next(pos, pszKey, pPropS);
		if( pPropS )
		{
			pPropT = pMapTo->get(pszKey);
			if( pPropT )
			{
				if( bGroup && 
					pPropS->IsGroup() )
				{
					int nCount = pPropS->GetSubItemsCount();
					for(int i = 0; i < nCount; i++)
					{
						CPropertyGridPropertyEx* pChild = (CPropertyGridPropertyEx*)pPropS->GetSubItem(i);
						if( pChild && 
							nID == pChild->GetData() )
						{
							pPropT->SetValue(pChild->GetValue());
							nUpdCount++;
						}
					}
				}
				else
				{
					pPropT->SetValue(pPropS->GetValue());
					nUpdCount++;
				}
			}
		}
	}

	if( nUpdCount > 0 )
		SetModified(TRUE);

	return nUpdCount;
}

int CBasePropertiesWnd::CreateFontMap(CHashTblStringToOutFont* pOut)
{
	if( pOut == NULL )
		return 0;

	CHashTblStrToProperty* pTab = GetStrToPropTable();
	if( pTab == NULL )
		return 0;

	int nUpdCount = 0;
	LPTSTR pszKey = NULL;
	CMFCPropertyGridProperty* pProp = NULL;
	CString pszVal = _T("");

	PPOSITION pos = pTab->first();
	while( pos != NULL )
	{
		pTab->next(pos, pszKey, pProp);
		if( pProp &&
			!pProp->IsGroup() )
		{
			pszVal = pProp->GetValue();
			if( pszVal && 
				!pszVal.IsEmpty() )
			{
				LPOUTFONT lof = (LPOUTFONT)malloc(sizeof(OUTFONT));
				if( lof )
				{
					ZeroMemory(lof, sizeof(OUTFONT));

					_tcscpy_s(lof->fontName, 63, (CString)pProp->GetName());
					_tcscpy_s(lof->baseName, 63, (CString)pProp->GetName());
					_tcscpy_s(lof->displayName, 63, pszVal);

					pOut->add(lof->fontName, lof);

					nUpdCount++;
				}
			}
		}
	}

	return nUpdCount;
}


CView* CBasePropertiesWnd::GetActiveView()
{
	CFrameWnd* pFrame = (CFrameWnd*)AfxGetMainWnd();
	if( pFrame )
	{
		// Get the active MDI child window.
		CFrameWnd *pChild = (CFrameWnd*)pFrame->GetActiveFrame();
		if( pChild )
		{
			// Get the active view attached to the active MDI child window.
			return pChild->GetActiveView();
		}
	}
	return NULL;
}

CPropertyGridPropertyEx* CBasePropertiesWnd::AddSubItem(CMFCPropertyGridProperty* pGroup, UINT nPromptId, LPCTSTR pszVal, BOOL bAllowEdit)
{
	if( !pGroup )
		return NULL;
	CString szText;
	szText.LoadString(nPromptId);

	//CPropertyGridPropertyEx* pCtrl = NULL;
	//std::auto_ptr<CPropertyGridPropertyEx> grid(new CPropertyGridPropertyEx(szText, (_variant_t)pszVal, _T("")));
	//pCtrl = grid.get();
	//if( pCtrl )
	//{
	//	pCtrl->AllowEdit(bAllowEdit);
	//	pGroup->AddSubItem(grid.release());
	//}
	CPropertyGridPropertyEx* pCtrl = new CPropertyGridPropertyEx(szText, (_variant_t)pszVal, _T(""));
	if( pCtrl )
	{
		pCtrl->AllowEdit(bAllowEdit);
		pGroup->AddSubItem(pCtrl);
	}
	return pCtrl;
}

CPropertyGridPropertyEx* CBasePropertiesWnd::AddSubItem(CMFCPropertyGridProperty* pGroup, LPCTSTR pszText, LPCTSTR pszVal, BOOL bAllowEdit)
{
	if( !pGroup )
		return NULL;

	//CPropertyGridPropertyEx* pCtrl = NULL;
	//std::auto_ptr<CPropertyGridPropertyEx> grid(new CPropertyGridPropertyEx(pszText, (_variant_t)pszVal, _T("")));
	//pCtrl = grid.get();
	//if( pCtrl )
	//{
	//	pCtrl->AllowEdit(bAllowEdit);
	//	pGroup->AddSubItem(grid.release());
	//}

	CPropertyGridPropertyEx* pCtrl = new CPropertyGridPropertyEx(pszText, (_variant_t)pszVal, _T(""));
	if( pCtrl )
	{
		pCtrl->AllowEdit(bAllowEdit);
		pGroup->AddSubItem(pCtrl);
	}
	return pCtrl;
}

CPropertyGridPropertyEx* CBasePropertiesWnd::AddSubItem(CMFCPropertyGridProperty* pGroup, DWORD_PTR nID, UINT nPromptId, LPCTSTR pszVal, BOOL bAllowEdit)
{
	if( !pGroup )
		return NULL;
	CString szText;
	szText.LoadString(nPromptId);

	//CPropertyGridPropertyEx* pCtrl = NULL;
	//std::auto_ptr<CPropertyGridPropertyEx> grid(new CPropertyGridPropertyEx(szText, (_variant_t)pszVal, _T(""), nID));
	//pCtrl = grid.get();
	//if( pCtrl )
	//{
	//	pCtrl->AllowEdit(bAllowEdit);
	//	pGroup->AddSubItem(grid.release());
	//}
	CPropertyGridPropertyEx* pCtrl = new CPropertyGridPropertyEx(szText, (_variant_t)pszVal, _T(""), nID);
	if( pCtrl )
	{
		pCtrl->AllowEdit(bAllowEdit);
		pGroup->AddSubItem(pCtrl);
	}
	return pCtrl;
}

CPropertyGridPropertyEx* CBasePropertiesWnd::AddSubItem(CMFCPropertyGridProperty* pGroup, DWORD_PTR nID, LPCTSTR pszText, LPCTSTR pszVal, BOOL bAllowEdit)
{
	if( !pGroup )
		return NULL;

	//CPropertyGridPropertyEx* pCtrl = NULL;
	//std::auto_ptr<CPropertyGridPropertyEx> grid(new CPropertyGridPropertyEx(pszText, (_variant_t)pszVal, _T(""), nID));
	//pCtrl = grid.get();
	//if( pCtrl )
	//{
	//	pCtrl->AllowEdit(bAllowEdit);
	//	pGroup->AddSubItem(grid.release());
	//}
	CPropertyGridPropertyEx* pCtrl = new CPropertyGridPropertyEx(pszText, (_variant_t)pszVal, _T(""), nID);
	if( pCtrl )
	{
		pCtrl->AllowEdit(bAllowEdit);
		if( !pGroup->AddSubItem(pCtrl))
		{
			TRACE(_T("AddSubItem failed.\n"));
		}
	}
	return pCtrl;
}

BOOL CBasePropertiesWnd::OnShowControlBarMenu(CPoint point)
{
	return TRUE;
}