// ImageSaveDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "ImageSaveDlg.h"
#include "afxdialogex.h"


// CImageSaveDlg ダイアログ

IMPLEMENT_DYNAMIC(CImageSaveDlg, CDialogEx)

CImageSaveDlg::CImageSaveDlg(CHashTblImage* pmapImg, CWnd* pParent /*=NULL*/)
	: CDialogEx(CImageSaveDlg::IDD, pParent)
	, m_mImages(pmapImg)
	, m_bSaveAll(FALSE)
{
	m_szW01Name = _T("");
}

CImageSaveDlg::~CImageSaveDlg()
{
}

void CImageSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cbImage);
	DDX_Check(pDX, IDC_SAVEALL, m_bSaveAll);
}


BEGIN_MESSAGE_MAP(CImageSaveDlg, CDialogEx)
	ON_BN_CLICKED(IDC_SAVEALL, &CImageSaveDlg::OnBnClickedSaveall)
	ON_BN_CLICKED(IDC_BROWSE, &CImageSaveDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDOK, &CImageSaveDlg::OnBnClickedOk)
END_MESSAGE_MAP()


void CImageSaveDlg::SetW01FileName(LPTSTR pName)
{
	TCHAR szDrive[_MAX_DRIVE] = {0};
	TCHAR szDir[_MAX_DIR] = {0};
	TCHAR szName[_MAX_FNAME] = {0};
	TCHAR szExt[_MAX_EXT] = {0};

	_tsplitpath_s(pName, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szName, _MAX_FNAME, szExt, _MAX_EXT);

	m_szW01Name = szName;
}

// CImageSaveDlg メッセージ ハンドラー
BOOL CImageSaveDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if( m_mImages != NULL &&
		m_mImages->size() > 0 )
	{
		PPOSITION pos = m_mImages->first(false);
		while(pos != NULL)
		{
			USHORT key = 0;
			LPIMAGEDATA pImg = NULL;
			m_mImages->next(pos, key, pImg);
			if( pImg != NULL )
			{
				CString szMsg;
				szMsg.Format(_T("%d (%dx%d)"), key, pImg->width, pImg->height);

				switch(pImg->type)
				{
				case 0x01:
					szMsg.Append(_T(" JPG "));
					break;
				case 0x04:
					szMsg.Append(_T(" PNG "));
					break;
				case 0x05:
					szMsg.Append(_T(" GIF "));
					break;
				default:
					szMsg.Append(_T("  "));
					break;
				}

				int index = m_cbImage.AddString(szMsg);
				if( index > -1 )
				{
					m_cbImage.SetItemData(index, (DWORD_PTR)key);
				}
				//ULONG	RecNo;
				//SHORT	width;
				//SHORT	height;
				//ULONG	length;
				//USHORT	type;
				//LPVOID	pImage;
			}
		}
	}

	TCHAR outpath[_MAX_PATH];
	HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, outpath);
	if( SUCCEEDED(hr))
	{
		GetDlgItem(IDC_TEXT_OUT)->SetWindowText(outpath);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


void CImageSaveDlg::OnBnClickedSaveall()
{
	UpdateData();
	if(m_bSaveAll)
	{
		m_cbImage.EnableWindow(FALSE);
	}
	else
	{
		m_cbImage.EnableWindow(TRUE);
	}
}


void CImageSaveDlg::OnBnClickedBrowse()
{
	CString szName;
	GetDlgItem(IDC_TEXT_OUT)->GetWindowText(szName);

	if( theApp.BrowseForFolder(GetSafeHwnd(), IDS_DIR_IMAGE, szName) )
	{
		GetDlgItem(IDC_TEXT_OUT)->SetWindowText(szName);
	}
}


void CImageSaveDlg::OnBnClickedOk()
{
	UpdateData();

	int nCount = 0;
	if( m_bSaveAll )
	{
		if( m_mImages != NULL &&
			m_mImages->size() > 0 )
		{
			PPOSITION pos = m_mImages->first(false);
			while(pos != NULL)
			{
				USHORT key = 0;
				LPIMAGEDATA pImg = NULL;

				m_mImages->next(pos, key, pImg);
				if( pImg == NULL )
					continue;

				if(SaveImage(pImg, key))
					nCount++;
			}
		}
	}
	else
	{
		int index = m_cbImage.GetCurSel();
		if(index > -1)
		{
			DWORD_PTR key = m_cbImage.GetItemData(index);
			if( m_mImages != NULL &&
				m_mImages->size() > 0 )
			{
				LPIMAGEDATA pImg = m_mImages->get((USHORT)key);
				if( pImg != NULL )
				{
					if(SaveImage(pImg, key))
						nCount++;
				}
			}
		}
	}
	if( nCount > 0 )
	{
		CString szMsg;
		szMsg.Format(IDS_FILE_SAVED, nCount);

		AfxMessageBox(szMsg, MB_OK | MB_ICONINFORMATION);
	}

	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDialogEx::OnOK();
}

BOOL CImageSaveDlg::SaveImage(LPIMAGEDATA pImg, USHORT key)
{
	BOOL bReturn = TRUE;

	CString szPath;
	GetDlgItem(IDC_TEXT_OUT)->GetWindowText(szPath);

	CString szFile = szPath;
	if( szFile.Right(1) != _T('\\'))
		szFile.Append(_T("\\"));

	TCHAR szKey[64] = {0};
	_stprintf_s(szKey, 64, _T("%d"), key);
	szFile.Append(m_szW01Name);
	szFile.Append(szKey);

	switch(pImg->type)
	{
	case 0x01:
		szFile.Append(_T(".jpg"));
		break;
	case 0x04:
		szFile.Append(_T(".png"));
		break;
	case 0x05:
		szFile.Append(_T(".gif"));
		break;
	default:
		break;
	}

	DWORD dwCreationDisposition = CREATE_ALWAYS;
	HANDLE hFile = ::CreateFile(szFile,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		AfxThrowFileException(-1, ::GetLastError(), szFile);
	}

	// UTF8 BOMなし
	::SetFilePointer(hFile, 0, NULL, SEEK_SET);
	DWORD nBytesWritten = 0;
	DWORD nLen = 0;

	DWORD pNumberOfBytesWritten = 0;
	// Write
	if( !::WriteFile(hFile, pImg->pImage, pImg->length, &pNumberOfBytesWritten, NULL) )
	{
		bReturn = FALSE;
	}

	::CloseHandle(hFile);

	return bReturn;
}
