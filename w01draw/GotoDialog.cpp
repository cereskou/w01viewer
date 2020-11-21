// GotoDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "GotoDialog.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGotoDialog ダイアログ

IMPLEMENT_DYNAMIC(CGotoDialog, CDialogEx)

CGotoDialog::CGotoDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGotoDialog::IDD, pParent)
	, m_nSelectedPageNo(0)
{
	m_maxPageNo = 0;
	m_minPageNo = 0;
}

CGotoDialog::~CGotoDialog()
{
}

void CGotoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COM_PAGES, m_ctlCombPage);
//	DDX_CBIndex(pDX, IDC_COM_PAGES, m_nSelectedPageNo);
}


BEGIN_MESSAGE_MAP(CGotoDialog, CDialogEx)
END_MESSAGE_MAP()


// CGotoDialog メッセージ ハンドラー

void CGotoDialog::SetPageRange(int min, int max)
{
	m_maxPageNo = max;
	m_minPageNo = min;
}

BOOL CGotoDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString szText;
	for(int i = m_minPageNo; i <= m_maxPageNo; i++)
	{
		szText.Format(_T("%d"), i);

		m_ctlCombPage.AddString(szText);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


void CGotoDialog::OnOK()
{
	m_nSelectedPageNo = -1;

	TCHAR szNums[256] = {0};
	if( m_ctlCombPage.GetWindowText(szNums, 255) )
	{
		LPTSTR pszEnd = NULL;
		m_nSelectedPageNo = _tcstol(szNums, &pszEnd, 10);
	}

	CDialogEx::OnOK();
}
