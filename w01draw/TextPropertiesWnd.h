#ifndef __TEXTPROP_H__
#define __TEXTPROP_H__
#pragma once

#include "BasePropertiesWnd.h"
#include "w01struct.h"

struct _freePosLineP
{
	void operator()(ULONG key, LinkedList<WEB_POS_RECDATA*>* lpLine) const
	{
		if( lpLine )
		{
			_delete(lpLine);
		}
	}
};
typedef HashTbl<ULONG, LinkedList<WEB_POS_RECDATA*>*, _freePosLineP>	CHashTblTextLine;

typedef struct _listPosP
{
	int index;		// index of List
	WEB_POS_RECDATA* pos;
} POSLIST, *LPPOSLIST;
struct _freeListPosP
{
	void operator()(LONG key, LPPOSLIST pLstPos) const
	{
		_free(pLstPos);
	}
};
typedef HashTbl<LONG, LPPOSLIST, _freeListPosP>	CHashTblTextPos;

// CTextPropertiesWnd
class CTextPropertiesWnd : public CBasePropertiesWnd
{
	DECLARE_DYNAMIC(CTextPropertiesWnd)

public:
	CTextPropertiesWnd();
	virtual ~CTextPropertiesWnd();

public:
	void AdjustLayout();
	void Refresh(BOOL bClosed = FALSE);
	int CreateControl();
	void SetControlFont();

	BOOL GetHighLight() { return m_bHighLight; }
	void SetSelectedPosition(WEB_POS_RECDATA* pos, BOOL bMulti = FALSE);

protected:
	//CMFCListCtrl	m_lstText;
	CListCtrl	m_lstText;
	CPropertiesToolBar m_wndToolBar;

	int m_nOffsetHeight;

	BOOL	m_bHighLight;
	BOOL	m_bBusy;

	LinkedList<WEB_POS_RECDATA*>* plstSelected;
	CHashTblTextLine*	pmapTextLine;
	CHashTblTextPos*	pmapTextPos;

protected:

	void InitListCtrl();
	void ClearSelected();
	void UpdateSelected();
	WEB_POS_RECDATA* GetPosRec(int nIndex);
	LPPOSLIST GetPosObject(int nIndex);
	LPPOSLIST GetPosObjectEx(LONG lKey);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRefresh();
	afx_msg void OnHighLight();
	afx_msg void OnUpdateHighLight(CCmdUI *pCmdUI);
	afx_msg void OnSelectLine();
	afx_msg void OnLvnItemChangedTextList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


#endif
