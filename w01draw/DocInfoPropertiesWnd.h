#ifndef __DOCINFOPW_H__
#define __DOCINFOPW_H__
#pragma once

#include "BasePropertiesWnd.h"
class CDocInfoPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CDocInfoPropertiesWnd)

public:
	CDocInfoPropertiesWnd();
	virtual ~CDocInfoPropertiesWnd();

	void Refresh(BOOL bClosed = FALSE);

protected:
	CPropertiesToolBar	m_wndToolBar;
	CPropertyGridCtrlEx m_wndDocInfo;

protected:
	void InitPropList();
	void SetControlFont();
	void AdjustLayout();
	int CreateControl();
	
protected:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnReload();
	afx_msg void OnUpdateReload(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#endif