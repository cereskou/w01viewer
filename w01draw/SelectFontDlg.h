#pragma once
#include "afxfontcombobox.h"


// CSelectFontDlg ダイアログ

class CSelectFontDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectFontDlg)

public:
	CSelectFontDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CSelectFontDlg();

// ダイアログ データ
	enum { IDD = IDD_SELFONTDIALOG };

protected:
	afx_msg void RefreshFontList();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	CString m_szFontName;
	BOOL m_bShowDeviceFont;
	BOOL m_bShowRasterFont;
	BOOL m_bShowTrueType;
	CMFCFontComboBox m_FontCombox;
	virtual void OnOK();
};
