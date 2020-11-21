
// MainFrm.h : CMainFrame クラスのインターフェイス
//

#pragma once
#include "DirectoryView.h"
#include "FontPropertiesWnd.h"
#include "TextPropertiesWnd.h"
#include "FontXmlPropertiesWnd.h"
#include "DocInfoPropertiesWnd.h"
#include "PageInfoPropertiesWnd.h"
#include "NumericalPropertiesWnd.h"

#include "ToolbarEx.h"

//class CMDIClientWnd : public CWnd
//{
//	DECLARE_DYNAMIC(CMDIClientWnd)
//// 生成された、メッセージ割り当て関数
//protected:
//	afx_msg void OnPaint();
//	afx_msg LRESULT OnSetMenuNotify(WPARAM wParam, LPARAM lParam);
//	DECLARE_MESSAGE_MAP()
//};

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// 属性
public:

// 操作
public:

// オーバーライド
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// 実装
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CComboBox* GetZoomComboBox() { return m_pComboZoom; }
	CStatic* GetPageStatic() { return m_pStaticPage; }
	CComboBox* GetOverlayTextColorBox() { return m_pComboColor; }

	CTextPropertiesWnd* GetTextPropertiesWnd() { return &m_wndTextProp; }
	CFontPropertiesWnd* GetFontPropertiesWnd() { return &m_wndFontProp; }
	CFontXmlPropertiesWnd* GetFontXmlPropertiesWnd() { return &m_wndFxmlProp; }
	CDocInfoPropertiesWnd* GetDocInfoPropertiesWnd() { return &m_wndDocInfoProp; }
	CPageInfoPropertiesWnd* GetPageInfoPropertiesWnd() { return &m_wndPageInfoProp; }
	CNumericalPropertiesWnd* GetNumericalPropertiesWnd() { return &m_wndNumerialProp; }

	void SetCursorPosText(CPoint pt, BOOL bClear = FALSE);
	void SetPaper(int width, int height, BOOL bClear = FALSE);

	void ModifyMenuBar(CWnd* pChild);

	void RefreshDockPane();
protected:

	BOOL SelectToolBar(UINT nResourceId);

protected:  // コントロール バー用メンバー
	CMFCMenuBar		m_wndMenuBar;
	CMFCStatusBar	m_wndStatusBar;
	//CMDIClientWnd	m_wndMDIClient;

	UINT			m_nToolBarId;
	CToolbarEx		m_wndToolBar;
	CToolbarEx		m_wndToolBarDoc;
	CToolbarEx		m_wndToolBarDebug;

	CComboBox*		m_pComboZoom;		// Zoom
	CStatic*		m_pStaticPage;		// Page
	CComboBox*		m_pComboColor;		// Overlay Color

	CDirectoryView		m_wndDirectory;
	CFontPropertiesWnd	m_wndFontProp;
	CFontXmlPropertiesWnd m_wndFxmlProp;
	CTextPropertiesWnd	m_wndTextProp;
	CDocInfoPropertiesWnd	m_wndDocInfoProp;
	CPageInfoPropertiesWnd m_wndPageInfoProp;
	CNumericalPropertiesWnd	m_wndNumerialProp;

	CMapWordToPtr	m_mapMenu;

	int	m_iCurrentTabCount;
// 生成された、メッセージ割り当て関数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg LRESULT OnShowContextMenu(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
public:
	afx_msg void OnViewFilectrl();
	afx_msg void OnUpdateViewFilectrl(CCmdUI *pCmdUI);
	afx_msg void OnViewDebug();
	afx_msg void OnUpdateViewDebug(CCmdUI *pCmdUI);
	afx_msg void OnViewTextList();
	afx_msg void OnUpdateViewTextList(CCmdUI *pCmdUI);
	afx_msg void OnViewFontPane();
	afx_msg void OnUpdateViewFontPane(CCmdUI *pCmdUI);
	afx_msg void OnViewFontXmlPane();
	afx_msg void OnUpdateViewFontXmlPane(CCmdUI *pCmdUI);
	afx_msg void OnViewDocInfoPane();
	afx_msg void OnUpdateViewDocInfoPane(CCmdUI *pCmdUI);
	afx_msg void OnViewPageInfoPane();
	afx_msg void OnUpdateViewPageInfoPane(CCmdUI *pCmdUI);
	afx_msg void OnViewNumericalPane();
	afx_msg void OnUpdateViewNumericalPane(CCmdUI *pCmdUI);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnFileCloseall();
	afx_msg void OnFileCloseother();
	afx_msg void OnUpdateFileCloseother(CCmdUI *pCmdUI);
	afx_msg void OnGotoPage();
	afx_msg LRESULT OnAfxWmChangingActiveTab(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCloseDockingPane(CDockablePane* pWnd);
	virtual BOOL OnCloseMiniFrame(CPaneFrameWnd* pWnd);
};


