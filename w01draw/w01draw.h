
// w01draw.h : w01draw アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"       // メイン シンボル
#include "W01Doc.h"

#include <Gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#define Round(n, r) (n > 0 ? ceil(n / r) * r : floor(n / r) * r)

#define UWM_CONTEXTMENU		(WM_USER + 10)
#define UWM_DOC_REFRESH		WM_USER + 11
#define UWM_DOC_SYNC		WM_USER + 12
#define UWM_DOC_GOTO		WM_USER + 13
#define UWM_DOC_UPDATEUI	WM_USER + 14

// ----------------------------------------------------------------------------
// Macro
#define _free(p)	if( p != NULL ) { free(p); p = NULL; }
#define _release(p)	if( p != NULL ) { p->Release(); p = NULL; }
#define _delete(p)	if( p != NULL ) { delete (p); p = NULL; }
#define _del(p)		if( p != NULL ) { delete[] (p); p = NULL; }
#define _close(p)   if( p != NULL ) { ::CloseHandle(p); p = NULL; }

#define DOC_CACHE_PAGES		10

#define TEXTBLOCK_OFFSET	0x7FFF		// 32767

typedef struct __fontMapInfo
{
	int			count;
	BOOL		like;
	wchar_t		name[33];
	wchar_t		mapto[33];
} FONTMAPINFO, *LPFONTMAPINFO;

typedef struct __dockStateP
{
	BOOL	m_bShowFileView;
	BOOL	m_bShowTextView;
	BOOL	m_bShowFontView;
	BOOL	m_bShowFxmlView;
	BOOL	m_bShowDbgToolbar;
	BOOL	m_bShowMainToolbar;
	BOOL	m_bShowTopToolbar;
	BOOL	m_bShowDocInfo;
	BOOL    m_bShowPageInfo;
	BOOL    m_bShowNumView;
} DOCKSTATE, *LPDOCKSTATE;

// CW01DrawApp:
// このクラスの実装については、w01draw.cpp を参照してください。
//

class CW01DrawApp : public CWinAppEx
{
public:
	CW01DrawApp();

public:
	LPCWSTR GetDefaultFontName() {return (LPCWSTR)m_szDefaultFontName; }
	LPWSTR GetFontMap(LPCWSTR lpszName);

	LPCWSTR GetCellDefinePath() { return (LPCWSTR)m_szCelLDefPath; }
	LPCELLDEFOPTION GetCellOption() { return &m_cellOption; }
	BOOL GetUseCellOption() { return m_bUseCellOpt; }

	BOOL GetUseDefaultFont() { return m_bUseDefaultFont; }

	void ClearFont();
	void LoadConfig(LPTSTR lpszName, BOOL bOnlyFont = FALSE);

	UINT GetCachePages() { return m_docCachePages; }

	BOOL BrowseForFolder(HWND hWnd, UINT nMsgId, CString& strFolder);

#ifdef _DEBUG
	int GetMaxTextCount() { return m_nTextCount; }
	int GetMaxImageCount() { return m_nImageCount; }
#endif

	BOOL GetEnableDebug() { return m_bEnableDebug; }

	void SetOverlayTextColorIndex(UINT nIdx) { m_ovlyTextColor = nIdx; }
	UINT GetOverlayTextColorIndex() { return m_ovlyTextColor; }

	LPDOCKSTATE GetDockState() { return &m_dockState; }

protected:
	void LoadWindowPlacement();
	void SaveWindowPlacement();

protected:
	// マッピングフォント情報
	CMapStringToPtr*	m_pfontMap;
	// ディフォルトフォント名
	wchar_t				m_szDefaultFontName[33];
	// セル定義
	wchar_t				m_szCelLDefPath[_MAX_PATH];
	CELLDEFOPTION		m_cellOption;
	BOOL				m_bUseCellOpt;

#ifdef _DEBUG
	// DEBUG >>
	int		m_nTextCount;
	int		m_nImageCount;
	// DEBUG <<
#endif

	// オーバーレイ文字色
	UINT				m_ovlyTextColor;
	// Use Default Font
	BOOL				m_bUseDefaultFont;

	UINT				m_docCachePages;

	BOOL				m_bEnableDebug;

	ULONG_PTR			gdiplusToken;

	DOCKSTATE	m_dockState;
// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装
	BOOL  m_bHiColorIcons;

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CW01DrawApp theApp;
