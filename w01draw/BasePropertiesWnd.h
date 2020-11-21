#ifndef __BASEPROP_H__
#define __BASEPROP_H__
#pragma once

#include "hashtbl.h"
#include "CustomProp.h"
#include "FontXML.h"

// ------------------------------------------------------------------
// FREE function for HashTbl<WCHAR, CPropertyGridPropertyEx*>
// ------------------------------------------------------------------
struct _freeMFCPropP
{
	void operator()(TCHAR* key, CMFCPropertyGridProperty* lpRec) const
	{
		if( key != NULL )
			free(key);
	}
};
typedef HashTbl<TCHAR*, CMFCPropertyGridProperty*, _freeMFCPropP, _compareKeyStr, _dupKeyStr>	 CHashTblStrToProperty;
typedef HashTbl<LONG, CMFCPropertyGridProperty*>	 CHashTblLongToProperty;

// CBasePropertiesWnd

class CBasePropertiesWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CBasePropertiesWnd)

public:
	CBasePropertiesWnd();
	virtual ~CBasePropertiesWnd();

public:
	virtual void AdjustLayout() {}
	virtual int CreateControl() { return 0; }
	virtual void SetControlFont() {}
	virtual void Refresh(BOOL bClosed = FALSE) {}

	CView* GetActiveView();

	BOOL IsModified();

	CFont* GetBaseFont() { return &m_baseFont; }
	void SetWindow(CWnd* pWnd, BOOL bForce = FALSE);
	CWnd* GetWindow() { return m_pWnd; }

	int CreateFontMap(CHashTblStringToOutFont* pOut);

	CHashTblStrToProperty* GetStrToPropTable() { return m_pTblStrProp; }
	CHashTblLongToProperty* GetLongToPropTable() { return m_pTblLongProp; }

	void AddToMap(LPTSTR pszKey, CMFCPropertyGridProperty* pProp);
	void AddToMap(LONG lKey, CMFCPropertyGridProperty* pProp);
	CMFCPropertyGridProperty* GetFromMap(LPTSTR pszKey);
	CMFCPropertyGridProperty* GetFromMap(LONG lKey);

	int SyncTo(CHashTblStrToProperty* pMap, BOOL bGroup = FALSE, DWORD_PTR nID = 0L);
	int SyncFrom(CHashTblStrToProperty* pMap, BOOL bGroup = FALSE, DWORD_PTR nID = 0L);
	void SetModified(BOOL bSet = TRUE) { m_bModified = bSet; }

	virtual CPropertyGridPropertyEx* AddSubItem(CMFCPropertyGridProperty* pGroup, UINT nPromptId, LPCTSTR pszVal, BOOL bAllowEdit = FALSE);
	virtual CPropertyGridPropertyEx* AddSubItem(CMFCPropertyGridProperty* pGroup, LPCTSTR pszText, LPCTSTR pszVal, BOOL bAllowEdit = FALSE);
	virtual CPropertyGridPropertyEx* AddSubItem(CMFCPropertyGridProperty* pGroup, DWORD_PTR nID, UINT nPromptId, LPCTSTR pszVal, BOOL bAllowEdit = FALSE);
	virtual CPropertyGridPropertyEx* AddSubItem(CMFCPropertyGridProperty* pGroup, DWORD_PTR nID, LPCTSTR pszText, LPCTSTR pszVal, BOOL bAllowEdit = FALSE);

	BOOL OnShowControlBarMenu(CPoint point);
protected:
	CFont	m_baseFont;
	CWnd*	m_pWnd;
	CHashTblStrToProperty*	m_pTblStrProp;
	CHashTblLongToProperty*	m_pTblLongProp;
	BOOL	m_bModified;

protected:
	void InitControlFont();
	virtual void OnPropertyChanged(DWORD_PTR nID, CPropertyGridPropertyEx* pProp) {}

protected:
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};


#endif
