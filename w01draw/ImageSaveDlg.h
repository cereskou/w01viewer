#pragma once

#include "W01Doc.h"
#include "afxwin.h"

// CImageSaveDlg ダイアログ

class CImageSaveDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImageSaveDlg)

public:
	CImageSaveDlg(CHashTblImage* pmapImg, CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CImageSaveDlg();

// ダイアログ データ
	enum { IDD = IDD_SAVEIMGDIALOG };

	void SetW01FileName(LPTSTR pName);

protected:
	CHashTblImage*	m_mImages;
	CComboBox m_cbImage;
	BOOL m_bSaveAll;
	CString m_szW01Name;

	BOOL SaveImage(LPIMAGEDATA pImg, USHORT key);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSaveall();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedOk();
};
