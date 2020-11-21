#ifndef __TOOLBAREX_H__
#define __TOOLBAREX_H__

#pragma once

#include "linkedlist.h"

// CToolbarEx

class CToolbarEx : public CMFCToolBar
{
	DECLARE_DYNAMIC(CToolbarEx)

public:
	CToolbarEx();
	virtual ~CToolbarEx();

public:
	BOOL Create(CWnd* pParentWnd, UINT nResourceId, BOOL bAutoDelete = FALSE);
	CWnd* CreateControl(CRuntimeClass* pClass, LPCTSTR lpszName, UINT nId, CSize size, DWORD dwStyle, const CFont* pFont = NULL);

protected:
	LinkedList<CWnd*>*	m_pLstChild;

protected:
	DECLARE_MESSAGE_MAP()
};


#endif
