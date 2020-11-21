// InfoDialog.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "InfoDialog.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GetNewID(id, x)		(id + 32767 + x)
#define ResetNewID(id, x)	(id - 32767 - x)

// CInfoDialog ダイアログ

IMPLEMENT_DYNAMIC(CInfoDialog, CDialogEx)

CInfoDialog::CInfoDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInfoDialog::IDD, pParent)
{
	m_szTitle = _T("");
	m_nTitleID = 0;
}

CInfoDialog::~CInfoDialog()
{
}

void CInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInfoDialog, CDialogEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CInfoDialog メッセージ ハンドラー
void CInfoDialog::SetTitle(LPCTSTR lpszTitle)
{
	m_szTitle = lpszTitle;
}

void CInfoDialog::SetTitle(UINT nID)
{
	if( nID )
	{
		if( m_szTitle.LoadString(nID) )
			m_nTitleID = nID;
	}
}

BOOL CInfoDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ここに初期化を追加してください
	if( !m_szTitle.IsEmpty() )
	{
		SetWindowText(m_szTitle);
	}
	AdjustLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


int CInfoDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// TODO:  ここに特定な作成コードを追加してください。
	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD | WS_BORDER, rectDummy, this, 2))
	{
		TRACE0("プロパティ グリッドを作成できませんでした\n");
		return -1;      // 作成できない場合
	}
	
	InitPropList(m_nTitleID);

	return 0;
}

void CInfoDialog::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	INT off = 0;
	CRect rectClient;
	CWnd* pRect = GetDlgItem(IDC_POS_FRAME);
	if( pRect != NULL )
	{
		pRect->GetWindowRect(rectClient);
		ScreenToClient(rectClient);

		off = 0;
	}
	else
	{
		GetClientRect(rectClient);
		off = 35;
	}

	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height() - off, SWP_NOACTIVATE | SWP_NOZORDER);
}


void CInfoDialog::InitPropList(UINT nID)
{
	SetPropListFont();

	m_wndPropList.SetLeftColumnWidth(150);

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea(FALSE);
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties(FALSE);

	switch( nID )
	{
	case IDS_DOC_INFO:
		InitPropList_DocInf();
		break;
	case IDS_FONT_INFO:
		InitPropList_FntInf();
		break;
	case IDS_USER_INFO:
		InitPropList_UsrInf();
		break;
	default:
		break;
	}

	m_wndPropList.ExpandAll();
}

void CInfoDialog::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}


BOOL CInfoDialog::AddSubItem(CPropertyGridPropertyEx* pGroup, UINT nID, UINT nTitleID)
{
	if( !pGroup ||
		!nID )
		return FALSE;

	CString szText;
	CString szVal;
	szVal = GetValue(nID);
	if( nTitleID != 0 )
		szText.LoadString(nTitleID);
	else
		szText.LoadString(nID);

	int nType = GetType(nID);
	if( nType == PROP_TYPE_BUTTON )
	{
		CiEveryCallback* pCall = GetCallBack(nID);
		CDlgPropertyGridCtrl* pCtrl = new CDlgPropertyGridCtrl(szText, szVal, pCall);
		if( pCtrl )
		{
			pCtrl->AllowEdit(FALSE);
			pGroup->AddSubItem(pCtrl);
		}
	}
	else 
	{
		if( nType == PROP_TYPE_LIST2 )
		{
			szText = GetKey(nID);
		}
		CPropertyGridPropertyEx* pCtrl = new CPropertyGridPropertyEx(szText, (_variant_t)szVal, _T(""));
		if( pCtrl )
		{
			pCtrl->AllowEdit(FALSE);
			pGroup->AddSubItem(pCtrl);
		}
	}

	return TRUE;
}

void CInfoDialog::InitPropList_UsrInf()
{
	// ユーザー情報
	//  +---  Key： Value
	//  +---  Key： Value

	CString szVal;
	CString szText;
	// 1. ユーザー情報
	szText.LoadString(IDS_USER_INFO);
	CPropertyGridPropertyEx* pGroup1 = new CPropertyGridPropertyEx(szText);
	// リスト処理
	if( m_lstKey.GetCount() > 0 )
	{
		for(int i = 0; i < m_lstKey.GetCount(); i++)
		{
			UINT nID = m_lstKey.GetAt(i);
			UINT nNewId = ResetNewID(nID, i);

			AddSubItem(pGroup1, nID, nNewId);
		}
	}

	m_wndPropList.AddProperty(pGroup1);	
}

void CInfoDialog::InitPropList_FntInf()
{
	// フォント一覧
	//  +---  フォント名： 
	//  +---  フォント名： 
	CString szVal;
	CString szText;
	// 1. 帳票
	szText.LoadString(IDS_TXT_NO_FONT);
	CPropertyGridPropertyEx* pGroup1 = new CPropertyGridPropertyEx(szText);
	// リスト処理
	if( m_lstKey.GetCount() > 0 )
	{
		for(int i = 0; i < m_lstKey.GetCount(); i++)
		{
			UINT nID = m_lstKey.GetAt(i);
			UINT nNewId = ResetNewID(nID, i);

			AddSubItem(pGroup1, nID, nNewId);
		}
	}

	m_wndPropList.AddProperty(pGroup1);
}

void CInfoDialog::InitPropList_DocInf()
{
	// List
	// 帳票
	//  +--- ファイル名：
	//  +--- タイトル
	// 基本情報
	//  +---  作成日時
	//  +---  更新日時
	//  +---  レコード長
	//  +---  ページ数
	// レコード番号
	//  +---  レコード登録情報
	//  +---  ユーザー情報
	//  +---  オーバーレイ情報
	//  +---  ページ情報
	//  +---  イメージ情報
	//  +---  文字テーブル情報
	//  +---  文字位置情報
	//  +---  コメント登録情報
	//  +---  検印情報"
	//  +---  付箋情報"
	//  +---  マーカー情報"
	CString szVal;
	CString szText;
	// 1. 帳票
	szText.LoadString(IDS_TXT_SHEET);
	CPropertyGridPropertyEx* pGroup1 = new CPropertyGridPropertyEx(szText);
	// ファイル名
	AddSubItem(pGroup1, IDS_TXT_FNAME);
	// タイトル
	AddSubItem(pGroup1, IDS_TXT_TITLE);

	m_wndPropList.AddProperty(pGroup1);

	// 2. 基本情報
	szText.LoadString(IDS_TXT_BASE);
	CPropertyGridPropertyEx* pGroup2 = new CPropertyGridPropertyEx(szText);
	// バージョン
	AddSubItem(pGroup2, IDS_TXT_VERSION);
	// 作成日時
	AddSubItem(pGroup2, IDS_TXT_CREATEDATE);
	// 更新日時
	AddSubItem(pGroup2, IDS_TXT_UPDATEDATE);
	// レコード長
	AddSubItem(pGroup2, IDS_TXT_RECORDLEN);
	// ページ数
	AddSubItem(pGroup2, IDS_TXT_TOTALPAGE);

	m_wndPropList.AddProperty(pGroup2);

	// 3. 登録レコード番号
	szText.LoadString(IDS_TXT_RECORD);
	CPropertyGridPropertyEx* pGroup3 = new CPropertyGridPropertyEx(szText);
	// レコード登録情報
	AddSubItem(pGroup3, IDS_TXT_NO_REC);
	// ユーザー情報
	AddSubItem(pGroup3, IDS_TXT_NO_USERINFO);
	// オーバーレイ情報
	AddSubItem(pGroup3, IDS_TXT_NO_OVLY);
	// ページ情報
	AddSubItem(pGroup3, IDS_TXT_NO_PAGE);
	// フォント情報
	AddSubItem(pGroup3, IDS_TXT_NO_FONT);
	// イメージ情報
	AddSubItem(pGroup3, IDS_TXT_NO_IMAGE);
	// 文字テーブル情報
	AddSubItem(pGroup3, IDS_TXT_NO_TEXTTBL);
	// 文字位置情報
	AddSubItem(pGroup3, IDS_TXT_NO_TEXTPOS);
	// コメント情報
	AddSubItem(pGroup3, IDS_TXT_NO_COMMENT);
	// 検印情報
	AddSubItem(pGroup3, IDS_TXT_NO_STAMP);
	// 付箋情報
	AddSubItem(pGroup3, IDS_TXT_NO_TAG);
	// マーカー情報
	AddSubItem(pGroup3, IDS_TXT_NO_MARKER);

	m_wndPropList.AddProperty(pGroup3);
}


void CInfoDialog::SetValue(UINT nID, LPCTSTR lpszKey, LPCTSTR lpszVal, int nType, CiEveryCallback* pCall)
{
	if( nType == PROP_TYPE_LIST2 )
	{
		UINT nNewID = GetNewID(nID, m_lstKey.GetCount());

		m_lstKey.Add(nNewID);

		m_mapKey.SetAt(nNewID, lpszKey);
		m_mapVal.SetAt(nNewID, lpszVal);
		m_mapType.SetAt(nNewID, nType);
	}
}

void CInfoDialog::SetValue(UINT nID, LPCTSTR lpszVal, int nType, CiEveryCallback* pCall)
{
	if( nType == PROP_TYPE_LIST )
	{
		UINT nNewID = GetNewID(nID, m_lstKey.GetCount());

		m_lstKey.Add(nNewID);

		m_mapVal.SetAt(nNewID, lpszVal);
		m_mapType.SetAt(nNewID, nType);
	}
	else
	{
		m_mapVal.SetAt(nID, lpszVal);
		if( nType == PROP_TYPE_BUTTON )
		{
			m_mapType.SetAt(nID, nType);

			if( pCall != NULL )
				m_mapCall.SetAt(nID, pCall);
		}
	}
}

LPCTSTR CInfoDialog::GetKey(UINT nID)
{
	CString szVal;
	if( m_mapKey.Lookup(nID, szVal) )
	{
		return szVal;
	}

	return _T("");
}

LPCTSTR CInfoDialog::GetValue(UINT nID)
{
	CString szVal;
	if( m_mapVal.Lookup(nID, szVal) )
	{
		return szVal;
	}

	return _T("");
}

int CInfoDialog::GetType(UINT nID)
{
	int nType = -1;
	if( m_mapType.Lookup(nID, nType) )
		return nType;

	return -1;
}

CiEveryCallback* CInfoDialog::GetCallBack(UINT nID)
{
	CiEveryCallback* pCall = NULL;
	if( m_mapCall.Lookup(nID, (void*&)pCall) )
		return pCall;

	return NULL;
}