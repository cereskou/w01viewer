#pragma once
#include "afxwin.h"


// CGotoDialog �_�C�A���O

class CGotoDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CGotoDialog)

public:
	CGotoDialog(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~CGotoDialog();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_GOTODIALOG };
public:
	void SetPageRange(int min, int max);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

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
