#pragma once
#include "afxwin.h"


// CGotoDialog ダイアログ

class CGotoDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CGotoDialog)

public:
	CGotoDialog(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CGotoDialog();

// ダイアログ データ
	enum { IDD = IDD_GOTODIALOG };
public:
	void SetPageRange(int min, int max);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	int m_nSelectedPageNo;
	
protected:
	CComboBox m_ctlCombPage;
	int m_maxPageNo;
	int m_minPageNo;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
