#include "stdafx.h"
#include "CustomProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CPropertyGridCtrlEx, CMFCPropertyGridCtrl)

CPropertyGridCtrlEx::CPropertyGridCtrlEx()
{
}

CPropertyGridCtrlEx::~CPropertyGridCtrlEx()
{
}

BEGIN_MESSAGE_MAP(CPropertyGridCtrlEx, CMFCPropertyGridCtrl)
END_MESSAGE_MAP()

void CPropertyGridCtrlEx::SetLeftColumnWidth(int cx)
{
	m_nLeftWidth = cx;

	AdjustLayout();
}

void CPropertyGridCtrlEx::AdjustLayout()
{
	m_nLeftColumnWidth = m_nLeftWidth;

	CMFCPropertyGridCtrl::AdjustLayout();
}

void CPropertyGridCtrlEx::OnChangeSelection(CMFCPropertyGridProperty* pNewSel, CMFCPropertyGridProperty* pOldSel)
{
	CWnd* pWnd = GetParent();
	if( pWnd )
	{
		DWORD_PTR dwNewSel = 0;
		DWORD_PTR dwOldSel = 0;
		if( pNewSel )
			dwNewSel = pNewSel->GetData();
		if( pOldSel )
			dwOldSel = pOldSel->GetData();
			
		pWnd->PostMessage(AFX_UWM_CHANGE_SELECTION, (WPARAM)dwNewSel, (LPARAM)dwOldSel);
	}

	CMFCPropertyGridCtrl::OnChangeSelection(pNewSel, pOldSel);
}
