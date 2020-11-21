
// w01draw.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "w01draw.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "w01drawDoc.h"
#include "w01drawView.h"

#include "Config.h"

#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CW01DrawApp

BEGIN_MESSAGE_MAP(CW01DrawApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CW01DrawApp::OnAppAbout)
	// 標準のファイル基本ドキュメント コマンド
//	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// 標準の印刷セットアップ コマンド
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CW01DrawApp コンストラクション

CW01DrawApp::CW01DrawApp()
{
	m_bHiColorIcons = FALSE;
	m_bSaveState = FALSE;

	// TODO: 下のアプリケーション ID 文字列を一意の ID 文字列で置換します。推奨される
	// 文字列の形式は CompanyName.ProductName.SubProduct.VersionInformation です
	SetAppID(_T("w01draw.AppID.V2"));

	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	m_pfontMap = NULL;

	gdiplusToken = 0;
}

// 唯一の CW01DrawApp オブジェクトです。

CW01DrawApp theApp;


// CW01DrawApp 初期化

BOOL CW01DrawApp::InitInstance()
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//::_CrtSetBreakAlloc(6628);
	
#endif
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	::CoInitialize(NULL);

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// OLE ライブラリを初期化します。
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	//EnableTaskbarInteraction();

	// RichEdit コントロールを使用するには AfxInitRichEdit2() が必要です	
	// AfxInitRichEdit2();

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	SetRegistryKey(_T("W01DRAW.V3"));
	LoadStdProfileSettings(4);  // 標準の INI ファイルのオプションをロードします (MRU を含む)

	TCHAR szHis[_MAX_PATH] = {0};
	TCHAR szIni[_MAX_PATH] = {0};
	TCHAR szXml[_MAX_PATH] = {0};
	TCHAR szModule[_MAX_PATH] = {0};
	TCHAR szDrive[_MAX_DRIVE] = {0};
	TCHAR szDir[_MAX_DIR] = {0};
	TCHAR szFName[_MAX_FNAME] = {0};
	TCHAR szExt[_MAX_EXT] = {0};

	::GetModuleFileName(NULL, szModule, _MAX_PATH);
	_tsplitpath_s(szModule, szDrive, szDir, szFName, szExt);
	// .xml file
	_tmakepath_s(szXml, szDrive, szDir, _T("w01draw"), _T("xml"));

	LoadConfig(szXml);

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();

	LoadWindowPlacement();

	//CMFCToolTipInfo ttParams;
	//ttParams.m_bVislManagerTheme = TRUE;
	//theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
	//	RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// アプリケーション用のドキュメント テンプレートを登録します。ドキュメント テンプレート
	//  はドキュメント、フレーム ウィンドウとビューを結合するために機能します。
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_w01drawTYPE,
		RUNTIME_CLASS(CW01DrawDoc),
		RUNTIME_CLASS(CChildFrame), // カスタム MDI 子フレーム
		RUNTIME_CLASS(CW01DrawView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// メイン MDI フレーム ウィンドウを作成します。
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// 接尾辞が存在する場合にのみ DragAcceptFiles を呼び出します。
	//  MDI アプリケーションでは、この呼び出しは、m_pMainWnd を設定した直後に発生しなければなりません。
	// ドラッグ/ドロップ オープンを許可します。
	m_pMainWnd->DragAcceptFiles();

	// DDE、file open など標準のシェル コマンドのコマンド ラインを解析します。
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// DDE Execute open を使用可能にします。
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	if( !::PathFileExists(cmdInfo.m_strFileName) )
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	}

	// コマンド ラインで指定されたディスパッチ コマンドです。アプリケーションが
	// /RegServer、/Register、/Unregserver または /Unregister で起動された場合、False を返します。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// メイン ウィンドウが初期化されたので、表示と更新を行います。
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CW01DrawApp::ExitInstance()
{
	//_delete(m_pfontList);
	ClearFont();

	SaveWindowPlacement();

	//TODO: 追加したリソースがある場合にはそれらも処理してください
	AfxOleTerm(FALSE);

	// Shutdown GDI+
	Gdiplus::GdiplusShutdown(gdiplusToken);

	::CoUninitialize();

	return CWinAppEx::ExitInstance();
}

// CW01DrawApp メッセージ ハンドラー


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// ダイアログを実行するためのアプリケーション コマンド
void CW01DrawApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CW01DrawApp のカスタマイズされた読み込みメソッドと保存メソッド

// CW01DrawApp メッセージ ハンドラー

void CW01DrawApp::ClearFont()
{
	if( !m_pfontMap ) 
		return;

	// Free
	POSITION pos = m_pfontMap->GetStartPosition();
	while( pos != NULL )
	{
		CString key;
		LPFONTMAPINFO lpMap = NULL;
		m_pfontMap->GetNextAssoc(pos, key, (void* &)lpMap);
		if( lpMap )
		{
			lpMap->count--;
			if( lpMap->count == 0 )
			{
				_free(lpMap);
			}
		}
	}
	m_pfontMap->RemoveAll();

	_delete(m_pfontMap);

}


LPWSTR CW01DrawApp::GetFontMap(LPCWSTR lpszName)
{
	if( m_pfontMap == NULL )
		return NULL;

	WCHAR wszName[64] = {0};
	wcscpy_s(wszName, 63, lpszName);

	LPFONTMAPINFO lpFontMap = NULL;
	if( m_pfontMap->Lookup(wszName, (void* &)lpFontMap) )
	{
		if( lpFontMap != NULL )
		{
			return lpFontMap->mapto;
		}
	}
	else
	{
		// Like?
		POSITION pos = m_pfontMap->GetStartPosition();
		while( pos != NULL )
		{
			CString szLikeName;
			m_pfontMap->GetNextAssoc(pos, szLikeName, (void* &)lpFontMap);

			// Find
			if( lpFontMap != NULL &&
				lpFontMap->like )
			{
				if( wcsstr(wszName, (LPCTSTR)szLikeName) != NULL )
				{
					m_pfontMap->SetAt((LPCTSTR)wszName, lpFontMap);
					lpFontMap->count++;

					return lpFontMap->mapto;
				}
			}
		}
	}

	if( !GetUseDefaultFont() )
		return NULL;

	return m_szDefaultFontName;
}

#define	KEY_NONE			0
#define KEY_DEFAULTFONT		1
#define KEY_FONTMAP			2
#define KEY_ENABLEDEBUG		3
#define KEY_CELLDEF			4
#define KEY_USEDEFAULTFONT	5
#define KEY_CACHEPAGE		6
#ifdef _DEBUG
#define KEY_DBGTEXTCOUNT	7
#define KEY_DBGIMAGECOUNT	8
#endif

void CW01DrawApp::LoadConfig(LPTSTR lpszName, BOOL bOnlyFont)
{
	memset(m_szCelLDefPath, 0x00, sizeof(m_szCelLDefPath));
	if( !bOnlyFont )
	{
		memset(m_szCelLDefPath, 0x00, sizeof(m_szCelLDefPath));

		// Default
		wcscpy_s(m_szDefaultFontName, 32, _T("MS UI Gothic"));
		// Use Default Name
		m_bUseDefaultFont = TRUE;

		m_docCachePages = DOC_CACHE_PAGES;

		// Show Debug Menu
		m_bEnableDebug = TRUE;
	}

#ifdef _DEBUG
	m_nTextCount = 0;
	m_nImageCount = 0;
#endif

	m_ovlyTextColor = ID_COLOR_AUTO;

	ClearFont();

	if( m_pfontMap == NULL )
	{
		m_pfontMap = new CMapStringToPtr();
		if( m_pfontMap == NULL )
			return;
	}

	memset(&m_cellOption, 0x00, sizeof(CELLDEFOPTION));
	UTL::Config conf(lpszName);
	if( conf.IsValid() )
	{
		//conf.Dump();

		CString lpwzValue = NULL;
		lpwzValue = conf.GetString(_T("DefaultFontName"), _T("MS UI Gothic"));
		if( !lpwzValue.IsEmpty() )
		{
			wcscpy_s(m_szDefaultFontName, 32, lpwzValue);
		}
		// Default Font
		m_bUseDefaultFont = conf.GetBoolean(_T("UseDefaultFont"), TRUE);
		// EnableDebug
		m_bEnableDebug = conf.GetBoolean(_T("EnableDebug"), FALSE);

		// CachePage
		m_docCachePages =conf.GetLong(_T("CachePage"), 10);

#ifdef _DEBUG
		// DebugImageCount
		m_nImageCount = conf.GetLong(_T("DebugImageCount"), 10);
#endif

		// >>>>> CELL
		// CellDefFilePath
		lpwzValue = conf.GetString(_T("CellDefFilePath"));
		if( !lpwzValue.IsEmpty() )
		{
			wcscpy_s(m_szCelLDefPath, _MAX_PATH, lpwzValue);
		}
		//<CellMargin left="5" top="12" right="5" bottom="12">True</CellMargin>
		//<CellAreasSufferingPercent>70.1</CellAreasSufferingPercent>
		m_bUseCellOpt = conf.GetBoolean(_T("CellMargin"), FALSE);

		m_cellOption.marginLeft = conf.GetLong(_T("CellMargin"), _T("left"), (LONG)5);
		m_cellOption.marginTop = conf.GetLong(_T("CellMargin"), _T("top"), (LONG)5);
		m_cellOption.marginRight = conf.GetLong(_T("CellMargin"), _T("right"), (LONG)5);
		m_cellOption.marginBottom = conf.GetLong(_T("CellMargin"), _T("bottom"), (LONG)5);
		
		m_cellOption.overlapRatio = conf.GetDouble(L"CellAreasSufferingPercent", 66.7);

		// >>>>> NAME,NAME
		// <FontMap FontName="DRHGCODEPOS2" MapToName="DRHGCDPOS2" Like="False"/>
		int fontNum = 0;
		do {
			// FontName
			lpwzValue = conf.GetStringById(_T("FontMap"), _T("FontName"), fontNum);
			if( lpwzValue.IsEmpty() )
				break;

			LPFONTMAPINFO lpFontMap = (LPFONTMAPINFO)calloc(1, sizeof(FONTMAPINFO));
			if( lpFontMap )
			{
				wcscpy_s(lpFontMap->name, 32, lpwzValue);

				// MapToName
				lpwzValue = conf.GetStringById(_T("FontMap"), _T("FontName"), fontNum);
				if( !lpwzValue.IsEmpty() )
					wcscpy_s(lpFontMap->mapto, 32, lpwzValue);

				// Like
				lpFontMap->like = conf.GetBooleanById(_T("FontMap"), _T("MapToName"), fontNum);

				lpFontMap->count = 0;

				fontNum++;

				LPFONTMAPINFO lpOldFont = NULL;
				CStringW wszName = lpFontMap->name;
				CStringW token;
				int pos = 0;
				token = wszName.Tokenize(_T(","), pos);
				while( token != _T(""))
				{
					if( m_pfontMap->Lookup(token, (void*&)lpOldFont) )
					{
						int count = lpOldFont->count;
						memcpy(lpOldFont, lpFontMap, sizeof(FONTMAPINFO));
						lpOldFont->count = count;
						_free(lpFontMap);
					}
					else
					{
						m_pfontMap->SetAt(token, lpFontMap);
						lpFontMap->count++;
					}

					token = wszName.Tokenize(_T(","), pos);
				}

			}
		} while( 1 );
		// >>>>>> NAME,NAME

	}

	if( !bOnlyFont )
	{
		if( m_docCachePages == 0 )
			m_docCachePages = 1;
	}
}

void CW01DrawApp::SaveWindowPlacement()
{
	CString szWP = GetRegSectionPath(_T("WindowPlacement"));

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);
	if( !reg.Open(szWP) )
	{
		return;
	}

	int nShow = 0;
	nShow = m_dockState.m_bShowFileView ? 1 : 0;
	reg.Write(_T("Show.FileView"), nShow);

	nShow = m_dockState.m_bShowTextView ? 1 : 0;
	reg.Write(_T("Show.TextView"), nShow);

	nShow = m_dockState.m_bShowFontView ? 1 : 0;
	reg.Write(_T("Show.FontView"), nShow);

	nShow = m_dockState.m_bShowFxmlView ? 1 : 0;
	reg.Write(_T("Show.FontXMLView"), nShow);

	nShow = m_dockState.m_bShowDocInfo ? 1 : 0;
	reg.Write(_T("Show.DockInfo"), nShow);

	nShow = m_dockState.m_bShowPageInfo ? 1 : 0;
	reg.Write(_T("Show.PageInfo"), nShow);

	nShow = m_dockState.m_bShowNumView ? 1 : 0;
	reg.Write(_T("Show.Numerical"), nShow);

	nShow = m_dockState.m_bShowDbgToolbar ? 1 : 0;
	reg.Write(_T("TB.Debug"), nShow);

	nShow = m_dockState.m_bShowMainToolbar ? 1 : 0;
	reg.Write(_T("TB.Main"), nShow);
}

void CW01DrawApp::LoadWindowPlacement()
{
	CString szWP = GetRegSectionPath(_T("WindowPlacement"));

	m_dockState.m_bShowFileView = FALSE;
	m_dockState.m_bShowTextView = FALSE;
	m_dockState.m_bShowFontView = FALSE;
	m_dockState.m_bShowFxmlView = FALSE;
	m_dockState.m_bShowDbgToolbar = FALSE;
	m_dockState.m_bShowMainToolbar = FALSE;
	m_dockState.m_bShowTopToolbar = FALSE;
	m_dockState.m_bShowDocInfo = FALSE;
	m_dockState.m_bShowPageInfo = FALSE;
	m_dockState.m_bShowNumView = FALSE;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);
	if( !reg.Open(szWP) )
	{
		return;
	}

	int nShow = 0;
	if( reg.Read(_T("Show.FileView"), nShow)) 
	{
		m_dockState.m_bShowFileView = nShow;
	}
	if( reg.Read(_T("Show.TextView"), nShow)) 
	{
		m_dockState.m_bShowTextView = nShow;
	}
	if( reg.Read(_T("Show.FontView"), nShow)) 
	{
		m_dockState.m_bShowFontView = nShow;
	}
	if( reg.Read(_T("Show.FontXMLView"), nShow)) 
	{
		m_dockState.m_bShowFxmlView = nShow;
	}
	if( reg.Read(_T("Show.DockInfo"), nShow)) 
	{
		m_dockState.m_bShowDocInfo = nShow;
	}
	if( reg.Read(_T("Show.PageInfo"), nShow)) 
	{
		m_dockState.m_bShowPageInfo = nShow;
	}
	if( reg.Read(_T("Show.Numerical"), nShow)) 
	{
		m_dockState.m_bShowNumView = nShow;
	}

	if( reg.Read(_T("TB.Debug"), nShow)) 
	{
		m_dockState.m_bShowDbgToolbar = nShow;
	}
	if( reg.Read(_T("TB.Main"), nShow)) 
	{
		m_dockState.m_bShowMainToolbar = nShow;
	}
}


//@QHGOU 20150722 >>
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
    if(uMsg == BFFM_INITIALIZED)
	{
		if( lpData != NULL )
			SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)TRUE,lpData);
    }

    return 0;
}

BOOL CW01DrawApp::BrowseForFolder(HWND hWnd, UINT nMsgId, CString& strFolder)
{
    BROWSEINFO      bi;
    LPWSTR          lpBuffer;
    LPITEMIDLIST    pidlRoot;      // ブラウズのルートPIDL
    LPITEMIDLIST    pidlBrowse;    // ユーザーが選択したPIDL
    LPMALLOC        lpMalloc = NULL;
	BOOL			bOk;

	bOk = FALSE;
    HRESULT hr = SHGetMalloc(&lpMalloc);
    if(FAILED(hr)) return FALSE;

    // ブラウズ情報受け取りバッファ領域の確保
    if ((lpBuffer = (LPWSTR) lpMalloc->Alloc(_MAX_PATH)) == NULL) {
        return FALSE;
    }
	
    // ダイアログ表示時のルートフォルダのPIDLを取得
    // ※以下はデスクトップをルートとしている。デスクトップをルートとする
    //   場合は、単に bi.pidlRoot に０を設定するだけでもよい。その他の特
    //   殊フォルダをルートとする事もできる。詳細はSHGetSpecialFolderLoca
    //   tionのヘルプを参照の事。
	if (!SUCCEEDED(SHGetSpecialFolderLocation(  hWnd,
                                                CSIDL_DRIVES, //CSIDL_DESKTOP,
                                                &pidlRoot))) { 
        lpMalloc->Free(lpBuffer);
        return FALSE;
    }

	CString szTitle;
	szTitle.LoadString(nMsgId);
    // BROWSEINFO構造体の初期値設定
    bi.hwndOwner = hWnd;
    bi.pidlRoot = pidlRoot;
    bi.pszDisplayName = lpBuffer;
    bi.lpszTitle = szTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
//	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = &BrowseCallbackProc;
	bi.lParam = (LPARAM)(LPTSTR)(LPCTSTR)strFolder;

    // フォルダ選択ダイアログの表示 
    pidlBrowse = SHBrowseForFolder(&bi);
    if (pidlBrowse != NULL) {  
        // PIDL形式の戻り値のファイルシステムのパスに変換
        if (SHGetPathFromIDList(pidlBrowse, lpBuffer))
		{
            // 取得成功
			strFolder = lpBuffer;
			bOk = TRUE;
        }
        // SHBrowseForFolderの戻り値PIDLを解放
        lpMalloc->Free(pidlBrowse);
    }
    // クリーンアップ処理
    lpMalloc->Free(pidlRoot); 
    lpMalloc->Free(lpBuffer);
    lpMalloc->Release();

	return bOk;
}
//@QHGOU 20150722 <<