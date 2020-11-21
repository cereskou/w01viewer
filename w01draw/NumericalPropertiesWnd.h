#ifndef __NUMERICALW_H__
#define __NUMERICALW_H__
#pragma once

#include "BasePropertiesWnd.h"

struct _freePosDataListP
{
	void operator()(DWORD_PTR key, LinkedList<WEB_POS_RECDATA*>* pList) const
	{
		_delete(pList);
	}
};
typedef HashTbl<DWORD_PTR, LinkedList<WEB_POS_RECDATA*>*, _freePosDataListP>	CHashTblDWordToPosData;

typedef LinkedList<CPropertyGridPropertyEx*>	CLinkedListGridCtrl;

class CNumericalPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CNumericalPropertiesWnd)

public:
	CNumericalPropertiesWnd();
	virtual ~CNumericalPropertiesWnd();

	void Refresh(BOOL bClosed = FALSE);

protected:
	CPropertiesToolBar	m_wndToolBar;
	CPropertyGridCtrlEx m_wndDocInfo;

	BOOL m_bHighLight;
	LinkedList<WEB_POS_RECDATA*>* plstSelected;
	CHashTblDWordToPosData* pmapNumsObjList;

	CLinkedListGridCtrl*	m_plnkGridCtrl;

protected:
	void InitPropList();
	void SetControlFont();
	void AdjustLayout();
	int CreateControl();

	void ClearSelected();
protected:
	afx_msg LRESULT OnChangeSelection(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnReload();
	afx_msg void OnUpdateReload(CCmdUI *pCmdUI);
	afx_msg void OnHighLight();
	afx_msg void OnUpdateHighLight(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#endif