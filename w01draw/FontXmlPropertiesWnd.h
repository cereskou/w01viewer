#ifndef __FONTXMLPROP_H__
#define __FONTXMLPROP_H__
#pragma once

#include "BasePropertiesWnd.h"
#include "FontXML.h"

// CFontXmlPropertiesWnd

class CFontXmlPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CFontXmlPropertiesWnd)

public:
	CFontXmlPropertiesWnd();
	virtual ~CFontXmlPropertiesWnd();

protected:
	CMFCPropertyGridCtrl m_wndXMLList;	// XML font list
	CPropertiesToolBar	m_wndToolBar;

	CFontXML	m_xmlFont;
	BOOL		m_bUsingFont;

	//LinkedList<CPropertyGridPropertyEx*>*	pFontPtrList;

protected:
	BOOL LoadFontXML(LPCTSTR lpszName);
	void ClearXMLFont();

	void InitPropList();
	void SetControlFont();
	void AdjustLayout();
	int CreateControl();

	void OnPropertyChanged(DWORD_PTR nID, CPropertyGridPropertyEx* pProp);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnOpenXML();
	afx_msg void OnReload();
	afx_msg void OnSaveXML();
	afx_msg void OnUpdateSaveXML(CCmdUI *pCmdUI);
	afx_msg void OnCollapse();
	afx_msg void OnExpand();
	afx_msg void OnUsingFontDraw();
	afx_msg void OnUpdateUsingFontDraw(CCmdUI *pCmdUI);
};


#endif
