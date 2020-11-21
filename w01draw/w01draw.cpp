
// w01draw.cpp : �A�v���P�[�V�����̃N���X������`���܂��B
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
	// �W���̃t�@�C����{�h�L�������g �R�}���h
//	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// �W���̈���Z�b�g�A�b�v �R�}���h
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CW01DrawApp �R���X�g���N�V����

CW01DrawApp::CW01DrawApp()
{
	m_bHiColorIcons = FALSE;
	m_bSaveState = FALSE;

	// TODO: ���̃A�v���P�[�V���� ID ���������ӂ� ID ������Œu�����܂��B���������
	// ������̌`���� CompanyName.ProductName.SubProduct.VersionInformation �ł�
	SetAppID(_T("w01draw.AppID.V2"));

	// TODO: ���̈ʒu�ɍ\�z�p�R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
	m_pfontMap = NULL;

	gdiplusToken = 0;
}

// �B��� CW01DrawApp �I�u�W�F�N�g�ł��B

CW01DrawApp theApp;


// CW01DrawApp ������

BOOL CW01DrawApp::InitInstance()
{
#ifdef _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//::_CrtSetBreakAlloc(6628);
	
#endif
	// �A�v���P�[�V���� �}�j�t�F�X�g�� visual �X�^�C����L���ɂ��邽�߂ɁA
	// ComCtl32.dll Version 6 �ȍ~�̎g�p���w�肷��ꍇ�́A
	// Windows XP �� InitCommonControlsEx() ���K�v�ł��B�����Ȃ���΁A�E�B���h�E�쐬�͂��ׂĎ��s���܂��B
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// �A�v���P�[�V�����Ŏg�p���邷�ׂẴR���� �R���g���[�� �N���X���܂߂�ɂ́A
	// �����ݒ肵�܂��B
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	::CoInitialize(NULL);

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// OLE ���C�u���������������܂��B
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	//EnableTaskbarInteraction();

	// RichEdit �R���g���[�����g�p����ɂ� AfxInitRichEdit2() ���K�v�ł�	
	// AfxInitRichEdit2();

	// �W��������
	// �����̋@�\���g�킸�ɍŏI�I�Ȏ��s�\�t�@�C����
	// �T�C�Y���k���������ꍇ�́A�ȉ�����s�v�ȏ�����
	// ���[�`�����폜���Ă��������B
	// �ݒ肪�i�[����Ă��郌�W�X�g�� �L�[��ύX���܂��B
	// TODO: ��Ж��܂��͑g�D���Ȃǂ̓K�؂ȕ������
	// ���̕������ύX���Ă��������B
	SetRegistryKey(_T("W01DRAW.V3"));
	LoadStdProfileSettings(4);  // �W���� INI �t�@�C���̃I�v�V���������[�h���܂� (MRU ���܂�)

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

	// �A�v���P�[�V�����p�̃h�L�������g �e���v���[�g��o�^���܂��B�h�L�������g �e���v���[�g
	//  �̓h�L�������g�A�t���[�� �E�B���h�E�ƃr���[���������邽�߂ɋ@�\���܂��B
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_w01drawTYPE,
		RUNTIME_CLASS(CW01DrawDoc),
		RUNTIME_CLASS(CChildFrame), // �J�X�^�� MDI �q�t���[��
		RUNTIME_CLASS(CW01DrawView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// ���C�� MDI �t���[�� �E�B���h�E���쐬���܂��B
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// �ڔ��������݂���ꍇ�ɂ̂� DragAcceptFiles ���Ăяo���܂��B
	//  MDI �A�v���P�[�V�����ł́A���̌Ăяo���́Am_pMainWnd ��ݒ肵������ɔ������Ȃ���΂Ȃ�܂���B
	// �h���b�O/�h���b�v �I�[�v���������܂��B
	m_pMainWnd->DragAcceptFiles();

	// DDE�Afile open �ȂǕW���̃V�F�� �R�}���h�̃R�}���h ���C������͂��܂��B
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// DDE Execute open ���g�p�\�ɂ��܂��B
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	if( !::PathFileExists(cmdInfo.m_strFileName) )
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	}

	// �R�}���h ���C���Ŏw�肳�ꂽ�f�B�X�p�b�` �R�}���h�ł��B�A�v���P�[�V������
	// /RegServer�A/Register�A/Unregserver �܂��� /Unregister �ŋN�����ꂽ�ꍇ�AFalse ��Ԃ��܂��B
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// ���C�� �E�B���h�E�����������ꂽ�̂ŁA�\���ƍX�V���s���܂��B
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CW01DrawApp::ExitInstance()
{
	//_delete(m_pfontList);
	ClearFont();

	SaveWindowPlacement();

	//TODO: �ǉ��������\�[�X������ꍇ�ɂ͂������������Ă�������
	AfxOleTerm(FALSE);

	// Shutdown GDI+
	Gdiplus::GdiplusShutdown(gdiplusToken);

	::CoUninitialize();

	return CWinAppEx::ExitInstance();
}

// CW01DrawApp ���b�Z�[�W �n���h���[


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
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

// �_�C�A���O�����s���邽�߂̃A�v���P�[�V���� �R�}���h
void CW01DrawApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CW01DrawApp �̃J�X�^�}�C�Y���ꂽ�ǂݍ��݃��\�b�h�ƕۑ����\�b�h

// CW01DrawApp ���b�Z�[�W �n���h���[

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
    LPITEMIDLIST    pidlRoot;      // �u���E�Y�̃��[�gPIDL
    LPITEMIDLIST    pidlBrowse;    // ���[�U�[���I������PIDL
    LPMALLOC        lpMalloc = NULL;
	BOOL			bOk;

	bOk = FALSE;
    HRESULT hr = SHGetMalloc(&lpMalloc);
    if(FAILED(hr)) return FALSE;

    // �u���E�Y���󂯎��o�b�t�@�̈�̊m��
    if ((lpBuffer = (LPWSTR) lpMalloc->Alloc(_MAX_PATH)) == NULL) {
        return FALSE;
    }
	
    // �_�C�A���O�\�����̃��[�g�t�H���_��PIDL���擾
    // ���ȉ��̓f�X�N�g�b�v�����[�g�Ƃ��Ă���B�f�X�N�g�b�v�����[�g�Ƃ���
    //   �ꍇ�́A�P�� bi.pidlRoot �ɂO��ݒ肷�邾���ł��悢�B���̑��̓�
    //   ��t�H���_�����[�g�Ƃ��鎖���ł���B�ڍׂ�SHGetSpecialFolderLoca
    //   tion�̃w���v���Q�Ƃ̎��B
	if (!SUCCEEDED(SHGetSpecialFolderLocation(  hWnd,
                                                CSIDL_DRIVES, //CSIDL_DESKTOP,
                                                &pidlRoot))) { 
        lpMalloc->Free(lpBuffer);
        return FALSE;
    }

	CString szTitle;
	szTitle.LoadString(nMsgId);
    // BROWSEINFO�\���̂̏����l�ݒ�
    bi.hwndOwner = hWnd;
    bi.pidlRoot = pidlRoot;
    bi.pszDisplayName = lpBuffer;
    bi.lpszTitle = szTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
//	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn = &BrowseCallbackProc;
	bi.lParam = (LPARAM)(LPTSTR)(LPCTSTR)strFolder;

    // �t�H���_�I���_�C�A���O�̕\�� 
    pidlBrowse = SHBrowseForFolder(&bi);
    if (pidlBrowse != NULL) {  
        // PIDL�`���̖߂�l�̃t�@�C���V�X�e���̃p�X�ɕϊ�
        if (SHGetPathFromIDList(pidlBrowse, lpBuffer))
		{
            // �擾����
			strFolder = lpBuffer;
			bOk = TRUE;
        }
        // SHBrowseForFolder�̖߂�lPIDL�����
        lpMalloc->Free(pidlBrowse);
    }
    // �N���[���A�b�v����
    lpMalloc->Free(pidlRoot); 
    lpMalloc->Free(lpBuffer);
    lpMalloc->Release();

	return bOk;
}
//@QHGOU 20150722 <<