// TextToolTips.cpp : 実装ファイル
//

#include "stdafx.h"
#include "w01draw.h"
#include "TextToolTips.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTextToolTips

IMPLEMENT_DYNAMIC(CTextToolTips, CWnd)

CTextToolTips::CTextToolTips()
{
	m_pParentWnd = NULL;
	m_bShowStatus = FALSE;
	m_szText = _T("");
	m_szTitle = _T("");
}

CTextToolTips::~CTextToolTips()
{
}


BEGIN_MESSAGE_MAP(CTextToolTips, CWnd)
	//{{AFX_MSG_MAP(CTextToolTips)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// CTextToolTips メッセージ ハンドラー


BOOL CTextToolTips::Create(CWnd* pParentWnd)
{
	ASSERT(this != NULL);
	ASSERT(pParentWnd != NULL);

	m_pParentWnd = pParentWnd;

	CRect rectDummy(0, 0, 0, 0);

	LPCTSTR lpszClass = AfxRegisterWndClass(0, 0, (HBRUSH)(COLOR_BACKGROUND+1), NULL);
//	return CWnd::Create(NULL, NULL, WS_POPUP | WS_CLIPSIBLINGS, rectDummy, pParentWnd, 0);
	return CreateEx(0, lpszClass, NULL, WS_POPUP | WS_CLIPSIBLINGS, rectDummy, pParentWnd, 0);
	//return CreateEx(0, _T("Static"), NULL, WS_POPUP | WS_CLIPSIBLINGS, rectDummy, pParentWnd, 0);
}

void CTextToolTips::SetText(const CString& pszTitle, const CString& pszText)
{
	ASSERT(this != NULL);

	m_szTitle = pszTitle;
	m_szTitle.TrimRight();

	m_szText = pszText;
	m_szText.TrimRight();
}

BOOL CTextToolTips::Show(const CPoint& pt)
{
	ASSERT(this != NULL);
	ASSERT(m_hWnd != NULL);

	if( m_szText.IsEmpty() ) //|| m_bShowStatus )
		return FALSE;

	m_pt = pt;
	m_pt.x += 8;
	m_pt.y += 16;
	m_bShowStatus = TRUE;

	DisplayToolTip(m_pt);

	m_pParentWnd->Invalidate();

	return TRUE;
}

void CTextToolTips::Close()
{
	ASSERT(this != NULL);
	ASSERT(m_hWnd != NULL);

	ShowWindow(SW_HIDE);

	m_pParentWnd->Invalidate();

	m_bShowStatus = FALSE;
}

size_t GetByteLength(CString const& str)
{
	PTSTR begin = (PTSTR)(LPCTSTR)str;
	PTSTR end = _tcschr(begin, 0);
	return reinterpret_cast<BYTE*>(end) - reinterpret_cast<BYTE*>(begin);
}

void CTextToolTips::DisplayToolTip(const CPoint& pt)
{
	CDC* pDC = GetDC();

	CBrush* pOldBrush;
	CFont* pOldFont;
	CSize size;
	int top = 0;
	int width = 0;
	int line = 0;
	pOldFont = (CFont*)pDC->SelectObject(&afxGlobalData.fontBold);
	if( !m_szTitle.IsEmpty() )
	{
		line++;
		size = pDC->GetTextExtent(m_szTitle);
		pDC->LPtoDP(&size);

		top += size.cy;
		width = size.cx;
	}
	pDC->SelectObject(pOldFont);

	pOldFont = (CFont*)pDC->SelectObject(&afxGlobalData.fontTooltip);
	CString szText = m_szText;
	int start = 0;
	int maxlen = 0;
	while( start >= 0 )
	{
		int os = start;
		start = m_szText.Find(_T("\n"), start + 1);
		if( start > -1 )
		{
			if( (start - os) > maxlen )
			{
				maxlen = (start - os);
				szText = m_szText.Mid(os, start);
			}
		}
		else
		{
			if( m_szText.GetLength() - os > maxlen )
			{
				maxlen = (start - os);
				szText = m_szText.Mid(os);
			}
		}

		line++;
	}

	size = pDC->GetTextExtent(szText);
	pDC->LPtoDP(&size);

	size.cx = (size.cx > width) ? size.cx : width;
	size.cy = size.cy * line + top;

	pDC->SelectObject(pOldFont);

	CRect rc(pt.x, pt.y, pt.x + size.cx + 16, pt.y + size.cy + 7);
	pDC->SetBkMode(TRANSPARENT);

	CBrush brush(GetSysColor(COLOR_INFOBK));
	pOldBrush = pDC->SelectObject(&brush);

	// Create and select thick black pen
	CPen pen(PS_SOLID, 0, COLORREF(RGB(200, 200, 200)));
	CPen* pOldPen = pDC->SelectObject(&pen);

	pDC->Rectangle(0, 0, rc.Width(), rc.Height());

	pDC->MoveTo(0, top + 3);
	pDC->LineTo(rc.Width(), top + 3);

	// draw text
	pDC->SetTextColor(GetSysColor(COLOR_INFOTEXT));
	pDC->SetTextAlign(TA_LEFT);

	RECT rect;
	// DrawTitle
	if( !m_szTitle.IsEmpty() )
	{
		pOldFont = (CFont*)pDC->SelectObject(&afxGlobalData.fontBold);
		SetRect(&rect, 3, 2, rc.Width() - 6, top + 2 );
		pDC->DrawText(m_szTitle, &rect, DT_EDITCONTROL);

		pDC->SelectObject(pOldFont);
	}

	pOldFont = (CFont*)pDC->SelectObject(&afxGlobalData.fontTooltip);
	SetRect(&rect, 3, top + 4, rc.Width() - 6, rc.Height() - 2 );
	pDC->DrawText(m_szText, &rect, DT_EDITCONTROL | DT_WORDBREAK);

	CRect rectWnd = rc;
	m_pParentWnd->ClientToScreen(rectWnd);
	CPoint ptTipLeft = rectWnd.TopLeft();

	//if( GetStyle() & WS_POPUP )
	//{
	//	POINT curPt;
	//	GetCursorPos(&curPt);
	//	ptTipLeft.x = curPt.x;
	//	ptTipLeft.y = curPt.y;
	//}

	SetWindowPos(&wndTop, ptTipLeft.x + 1, ptTipLeft.y + 1, rectWnd.Width(), rectWnd.Height(), SWP_SHOWWINDOW | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);

	ReleaseDC(pDC);
}


void CTextToolTips::OnPaint()
{
	CPaintDC dc(this);

	DisplayToolTip(m_pt);
}