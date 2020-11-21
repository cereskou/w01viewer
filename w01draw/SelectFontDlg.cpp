// SelectFontDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "SelectFontDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSelectFontDlg ダイアログ

IMPLEMENT_DYNAMIC(CSelectFontDlg, CDialogEx)

CSelectFontDlg::CSelectFontDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSelectFontDlg::IDD, pParent)
	, m_bShowDeviceFont(FALSE)
	, m_bShowRasterFont(FALSE)
	, m_bShowTrueType(TRUE)
{
	m_szFontName = _T("");
}

CSelectFontDlg::~CSelectFontDlg()
{
}

void CSelectFontDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_DEVICE, m_bShowDeviceFont);
	DDX_Check(pDX, IDC_CHK_RASTER, m_bShowRasterFont);
	DDX_Check(pDX, IDC_CHK_TRUETYPE, m_bShowTrueType);
	DDX_Control(pDX, IDC_FONTCOMBO, m_FontCombox);
}


BEGIN_MESSAGE_MAP(CSelectFontDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHK_DEVICE, &CSelectFontDlg::RefreshFontList)
	ON_BN_CLICKED(IDC_CHK_RASTER, &CSelectFontDlg::RefreshFontList)
	ON_BN_CLICKED(IDC_CHK_TRUETYPE, &CSelectFontDlg::RefreshFontList)
END_MESSAGE_MAP()


// CSelectFontDlg メッセージ ハンドラー


BOOL CSelectFontDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if( !m_szFontName.IsEmpty() )
	{
		m_FontCombox.SelectFont(m_szFontName);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CSelectFontDlg::RefreshFontList()
{
	UpdateData();

	int nFontType = 0;
	if( m_bShowDeviceFont )
		nFontType |= DEVICE_FONTTYPE;
	if( m_bShowRasterFont )
		nFontType |= RASTER_FONTTYPE;
	if( m_bShowTrueType )
		nFontType |= TRUETYPE_FONTTYPE;

	CWaitCursor wait;
	m_FontCombox.Setup(nFontType);
}



void CSelectFontDlg::OnOK()
{
	CMFCFontInfo* pFontInfo = m_FontCombox.GetSelFont();
	if( pFontInfo )
	{
		m_szFontName = pFontInfo->m_strName;
	}

	CDialogEx::OnOK();
}
