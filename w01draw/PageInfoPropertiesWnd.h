#ifndef __PAGEINFOPW_H__
#define __PAGEINFOPW_H__
#pragma once

#include "BasePropertiesWnd.h"
class CPageInfoPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CPageInfoPropertiesWnd)

public:
	CPageInfoPropertiesWnd();
	virtual ~CPageInfoPropertiesWnd();

	void Refresh(BOOL bClosed = FALSE);

protected:
	CPropertiesToolBar	m_wndToolBar;
	CPropertyGridCtrlEx m_wndDocInfo;
	//CPropertyGridCtrlEx m_wndOvlInfo;

	BOOL	m_bOvlLoaded;
protected:
	void InitPropList();
	void SetControlFont();
	void AdjustLayout();
	int CreateControl();
	
protected:
	afx_msg LRESULT OnChangeSelection(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnReload();
	DECLARE_MESSAGE_MAP()
};

#endif