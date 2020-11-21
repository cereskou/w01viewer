#ifndef __CUSTOMPROP_H__
#define __CUSTOMPROP_H__
#pragma once

#include "EveryCall.h"

#define AFX_PROP_HAS_LIST 0x0001
#define AFX_UWM_CHANGE_SELECTION	WM_USER + 104

class CPropertyGridPropertyEx : public CMFCPropertyGridProperty
{
// Construction
public:
	// Group constructor
	CPropertyGridPropertyEx(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE) : CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
	{
	}

	// Simple property
	CPropertyGridPropertyEx(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL) 
		 : CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
	{
	}

public:
	void SetModified(BOOL bFlag = TRUE)
	{
		m_bIsModified = bFlag;
	}
};

class CPropertyGridCtrlEx : public CMFCPropertyGridCtrl
{
	DECLARE_DYNAMIC(CPropertyGridCtrlEx)
public:
	CPropertyGridCtrlEx();
	virtual ~CPropertyGridCtrlEx();

protected:
	void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

public:
	void OnChangeSelection(CMFCPropertyGridProperty* pNewSel, CMFCPropertyGridProperty* pOldSel);
	void SetLeftColumnWidth(int cx);
	void AdjustLayout();

protected:
	int m_nLeftWidth;
};


class CDlgPropertyGridCtrl : public CMFCPropertyGridFileProperty
{
public:
	CDlgPropertyGridCtrl(CString& szText, CString& szValue, CiEveryCallback* pCall) : CMFCPropertyGridFileProperty(szText, szValue)
	{
		m_pCall = pCall;
	}

	void OnDrawValue(CDC* pDC, CRect rect)
	{
		CRect rc = rect;
		rc.right -= 21;
		pDC->FillSolidRect(rc, RGB(255, 235, 255));

		CMFCPropertyGridFileProperty::OnDrawValue(pDC, rect);
	}

	void OnClickButton(CPoint point)
	{
		if( m_pCall )
			m_pCall->DoModal();
	}
protected:
	CiEveryCallback* m_pCall;
};

class CSelectFontPropertyGridProperty : public CPropertyGridPropertyEx
{
public:
	CSelectFontPropertyGridProperty(CString& szText, CString& szValue, COLORREF clrBk = RGB(255, 255, 255), DWORD_PTR dwID = 0L) : CPropertyGridPropertyEx(szText, szValue, _T(""), dwID)
	{
		m_clrBk = clrBk;
		m_dwFlags = AFX_PROP_HAS_LIST;
	}

	void OnDrawValue(CDC* pDC, CRect rect)
	{
		CRect rc = rect;
		rc.top += 1;
		rc.bottom -= 1;
		rc.left += 1;
		rc.right -= 1;

		pDC->FillSolidRect(rc, m_clrBk);

		CPropertyGridPropertyEx::OnDrawValue(pDC, rect);
	}

	CComboBox* CreateCombo(CWnd* pWndParent, CRect rect)
	{
		ASSERT_VALID(this);
		rect.bottom = rect.top + 400;
		CMFCFontComboBox* pCombo = new CMFCFontComboBox();
		const DWORD style = WS_CHILD | WS_VSCROLL | CBS_DROPDOWN | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;
		if( !pCombo->Create(style, rect, pWndParent, AFX_PROPLIST_ID_INPLACE))
		{
			delete pCombo;
			return NULL;
		}

//		CMFCFontComboBox::m_bDrawUsingFont = FALSE;

		return pCombo;
	}
protected:
	COLORREF m_clrBk;
};


class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

#endif
