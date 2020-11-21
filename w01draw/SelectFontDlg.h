#pragma once
#include "afxfontcombobox.h"


// CSelectFontDlg �_�C�A���O

class CSelectFontDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectFontDlg)

public:
	CSelectFontDlg(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~CSelectFontDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SELFONTDIALOG };

protected:
	afx_msg void RefreshFontList();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

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
