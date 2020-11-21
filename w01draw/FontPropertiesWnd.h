#ifndef __FONTPROP_H__
#define __FONTPROP_H__
#pragma once

#include "BasePropertiesWnd.h"

// CFontPropertiesWnd
class CFontPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CFontPropertiesWnd)

public:
	CFontPropertiesWnd();
	virtual ~CFontPropertiesWnd();

public:
	void AdjustLayout();
	int CreateControl();
	void SetControlFont();
	void Refresh(BOOL bClosed = FALSE);

protected:
	int m_nHeight;
	CMFCPropertyGridCtrl m_wndFontList;
	CPropertiesToolBar m_wndToolBar;

protected:
	void InitPropList();

	void OnPropertyChanged(DWORD_PTR nID, CPropertyGridPropertyEx* pProp);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnFontSyncTo();
	afx_msg void OnUpdateFontSyncTo(CCmdUI *pCmdUI);
	afx_msg void OnFontSyncFrom();
	afx_msg void OnUpdateFontSyncFrom(CCmdUI *pCmdUI);
	afx_msg void OnDocRefresh();
};


#endif
