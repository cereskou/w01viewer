// ZoomView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Resource.h"
#include "W01Draw.h"
#include "ZoomView.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PICKMARGIN	10	// Screen pixels apart for region zoom
#define MAXZOOMIN	4		// Maximum zoom-in factor

// CZoomView
double roundd(double x)
{
	if( x > 0.0 )
		return floor(x + 0.5);
	else
		return -1.0 * floor(fabs(x) + 0.5);
}


IMPLEMENT_DYNCREATE(CZoomView, CScrollView)

CZoomView::CZoomView() : CScrollView()
{
	m_zoomMode = MODE_ZOOMOFF;
	m_bCaptured = FALSE;
	m_zoomScale = (float)1.0;
	m_maxScale = (float)64.0;
	m_minScale = (float)0.2;

	m_dragRect.SetRectEmpty();

	m_hCursor = NULL;
	//m_hCursor = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_ARROW));
	m_hHandOver = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_HANDOVER));
	m_hHandDrag = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_HANDDRAG));

	m_bCenter = TRUE;
}

CZoomView::~CZoomView()
{
	if( m_hCursor )
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	if( m_hHandOver )
	{
		::DestroyCursor(m_hHandOver);
		m_hHandOver = NULL;
	}

	if( m_hHandDrag )
	{
		::DestroyCursor(m_hHandDrag);
		m_hHandDrag = NULL;
	}
}


BEGIN_MESSAGE_MAP(CZoomView, CScrollView)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


void CZoomView::SetZoomSizes(SIZE sizeTotal, const SIZE& sizePage, const SIZE& sizeLine)
{
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);

	m_nMapMode = MM_ANISOTROPIC;		// Need for arbitrary scaling
	m_totalLog = sizeTotal;

	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);

		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
	}

	m_origTotalDev = m_totalDev;
	m_origPageDev = sizePage;
	m_origLineDev = sizeLine;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	// Fugure out scroll bar
	CalcBars();

	NotifyZoom();
}

void CZoomView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	if( !::IsWindow(m_hWnd) ||
		!::IsWindowVisible(m_hWnd) )
		return;

	if( m_nMapMode != MM_ANISOTROPIC )
		return;

#ifdef _DEBUG
	if( m_nMapMode != MM_ANISOTROPIC )
	{
		TRACE0("Error: must call SetZoomSizes() before painting zoom view\n");
		ASSERT(FALSE);
		return;
	}
#endif

	ASSERT_VALID(pDC);
	ASSERT(m_totalLog.cx >= 0 && m_totalLog.cy >= 0);
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	// Set the Mapping mode
	pDC->SetMapMode(m_nMapMode);
	pDC->SetWindowExt(m_totalLog);

	CPoint ptVpOrg;

	if( !pDC->IsPrinting() )
	{
		pDC->SetViewportExt(m_totalDev);

		ASSERT(pDC->GetWindowOrg() == CPoint(0, 0));
		ptVpOrg = - GetDeviceScrollPosition();

		// Center full fit
		if( m_bCenter )
		{
			CRect rect;

			GetClientRect(&rect);

			if( m_totalDev.cx < rect.Width() )
				ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
			if( m_totalDev.cy < rect.Height() )
				ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
		}
	}
	else
	{
		CSize printSize;

		printSize.cx = pDC->GetDeviceCaps(HORZRES);
		printSize.cy = pDC->GetDeviceCaps(VERTRES);

		PersistRatio(m_totalLog, printSize, ptVpOrg);

		// Zoom completely out
		pDC->SetViewportExt(printSize);
	}

	// Set the new origin
	pDC->SetViewportOrg(ptVpOrg);

	CView::OnPrepareDC(pDC, pInfo);
}

void CZoomView::CalcBars()
{
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);

		m_pageDev = m_origPageDev;
		dc.LPtoDP((LPPOINT)&m_pageDev);

		m_lineDev = m_origLineDev;
		dc.LPtoDP((LPPOINT)&m_lineDev);
	}

	// make sure of the range
	if( m_pageDev.cy < 0 )
		m_pageDev.cy = -m_pageDev.cy;
	if( m_lineDev.cy < 0 )
		m_lineDev.cy = -m_lineDev.cy;

	// if none specified - use one tenth
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0 );

	if( m_pageDev.cx == 0 ) m_pageDev.cx = m_totalDev.cx / 10;
	if( m_pageDev.cy == 0 ) m_pageDev.cy = m_totalDev.cy / 10;
	if( m_lineDev.cx == 0 ) m_lineDev.cx = m_pageDev.cx / 10;
	if( m_lineDev.cy == 0 ) m_lineDev.cy = m_pageDev.cy / 10;

	// Now update the scrollbars
	if( m_hWnd != NULL )
	{
		UpdateBars();
		Invalidate(TRUE);
	}
}


// CZoomView 描画

void CZoomView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 描画コードをここに追加してください。
	CDC dc;
	CDC* pDrawDC = pDC;
	CBitmap bitmap;
	CBitmap* pOldBitmap = 0;

	CRect client;
	pDC->GetClipBox(client);
	CRect rect = client;
	DocToClient(rect);

	// Draw Off-Screen
	if( !pDC->IsPrinting() )
	{
		if( dc.CreateCompatibleDC(pDC) )
		{
			if( bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()))
			{
				OnPrepareDC(&dc, NULL);

				pDrawDC = &dc;

				//dc.SetGraphicsMode(GM_ADVANCED);
				dc.SetBkMode(TRANSPARENT);
				// offset origin more because bitmap is just piece of the whole drawing
				dc.OffsetViewportOrg(-rect.left, -rect.top);
				pOldBitmap = dc.SelectObject(&bitmap);

				//dc.SetBrushOrg(rect.left % 8, rect.top % 8);

				// might as well clip to the same rectangle
				dc.IntersectClipRect(client);
			}
		}
	}
	else
	{
		OnPrepareDC(pDrawDC);
	}

	// Clear Background
	pDrawDC->FillSolidRect(client, ::GetSysColor(COLOR_WINDOW));

	// Call Drawing
	Draw(pDrawDC);

	if( pDrawDC != pDC )
	{
		pDC->SetViewportOrg(0, 0);
		pDC->SetWindowOrg(0, 0);
		pDC->SetMapMode(MM_TEXT);
		dc.SetViewportOrg(0, 0);
		dc.SetWindowOrg(0, 0);
		dc.SetMapMode(MM_TEXT);

		double scale = GetZoomScale();
		int off = roundd(scale) + PICKMARGIN;

		int nWidth = rect.Width() + off;
		int nHeight = rect.Height() + off;

		pDC->SetStretchBltMode(STRETCH_DELETESCANS);

//		pDC->BitBlt(0, 0, nWidth, nHeight, &dc, 0, 0, SRCCOPY);

		pDC->StretchBlt(0, 0, nWidth, nHeight, &dc, 0, 0, rect.Width(), rect.Height(), SRCCOPY);

		pDC->SetMapMode(MM_ANISOTROPIC);

		if( pOldBitmap )
			dc.SelectObject(pOldBitmap);
		dc.DeleteDC();
	}
}


// CZoomView 診断

#ifdef _DEBUG
void CZoomView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CZoomView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


// CZoomView メッセージ ハンドラー
int CZoomView::ZoomIn(CRect& rect)
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	CRect zoomRect = rect;
	NormalizeRect(zoomRect);

	// Get the center of rect
	CPoint ptCenter;
	ptCenter.x = ((zoomRect.left + zoomRect.right) / 2);
	ptCenter.y = ((zoomRect.top + zoomRect.bottom) / 2);

	// Set if the rect is small enough for a point zoom
	CRect rectDP = zoomRect;

	ViewLPtoDP((LPPOINT)&rectDP, 2);

	BOOL bPointZoom = (max(rectDP.Width(), rectDP.Height()) < PICKMARGIN);
	if( bPointZoom )
	{
		return ZoomIn(&ptCenter);
	}

	CRect clientRect;
	GetClientRect(&clientRect);

	// Calculate the new zoom scale.
	float scaleH = (float) (clientRect.right) / (float) zoomRect.Width();
	float scaleV = (float) (clientRect.bottom) / (float) zoomRect.Height();
	
	// Keep the scale Isotropic
	m_zoomScale = min(scaleH, scaleV);
	if( m_zoomScale > m_maxScale )
		m_zoomScale = m_maxScale;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	CalcBars();

	// Set the current center point.
	CenterOnLogicalPoint(ptCenter);

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

int CZoomView::ZoomTo(CPoint *point, float scale)
{
	CPoint ptCenter;

	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	// Save the current center point.
	if (!point)
	{
		ptCenter = GetLogicalCenterPoint();
	}
	else
	{
		ptCenter = *point;
	}

	// Increase the zoom scale.
	m_zoomScale = scale;

	if( m_zoomScale < m_minScale )
		m_zoomScale = m_minScale;
	if( m_zoomScale > m_maxScale )
		m_zoomScale = m_maxScale;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	CalcBars();
	// Set the current center point.
	CenterOnLogicalPoint(ptCenter);

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

int  CZoomView::ZoomIn (CPoint *point, float  delta)
{
	CPoint ptCenter;

	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	// Save the current center point.
	if (!point)
	{
		ptCenter = GetLogicalCenterPoint();
	}
	else
	{
		ptCenter = *point;
	}

	// Increase the zoom scale.
	m_zoomScale *= delta;
	if( m_zoomScale > m_maxScale )
		m_zoomScale = m_maxScale;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	CalcBars();
	// Set the current center point.
	CenterOnLogicalPoint(ptCenter);

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}


int  CZoomView::ZoomOut(CPoint *point, float  delta)
{
	CPoint ptCenter;

	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	// Save the current center point.
	if (!point)
	{
		ptCenter = GetLogicalCenterPoint();
	}
	else
	{
		ptCenter = *point;
	}

	// Decrease the zoom scale.
	m_zoomScale /= delta;
	if( m_zoomScale < m_minScale )
		m_zoomScale = m_minScale;

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	CalcBars();

	// Set the current center point (logical coordinates.
	CenterOnLogicalPoint(ptCenter);

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

int CZoomView::ZoomHigh()
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	CRect rc;
	CPoint pt;
	CSize sizeSb;

	CRect clientRect;
	GetClientRect(&clientRect);
	
	int high = clientRect.Height();

	// Set the new zoom scale (could use cx or cy)
	m_zoomScale = ((float)high / (float)m_origTotalDev.cy);

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	// Fugure out scroll bar
	CalcBars();

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

int CZoomView::ZoomWide()
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	CRect rc;
	CSize sizeSb;

	CRect clientRect;
	GetClientRect(&clientRect);
	
	int wide = clientRect.Width();

	// Set the new zoom scale (could use cx or cy)
	m_zoomScale = ((float)wide / (float)m_origTotalDev.cx);

	// Modify the Viewport extent
	m_totalDev.cx = (int) ((float) m_origTotalDev.cx * m_zoomScale);
	m_totalDev.cy = (int) ((float) m_origTotalDev.cy * m_zoomScale);

	CalcBars();

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

int CZoomView::ZoomFull()
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	CRect rc;
	CPoint pt;
	CSize sizeSb;

	// Just set Viewport Extent to Client size for full fit
	GetTrueClientSize(m_totalDev, sizeSb);

	// Maintain origional ratio
	PersistRatio(m_totalLog, m_totalDev, pt);

	// Set the new zoom scale (could use cx or cy)
	m_zoomScale = ((float)m_totalDev.cx / (float)m_origTotalDev.cx);

	// Remove the scrollbars
	UpdateBars();

	// Complete redraw
	Invalidate(TRUE);

	// Notify the class that a new zoom scale was done
	NotifyZoom();

	return TRUE;
}

CZoomView::ZOOMMODE CZoomView::GetZoomMode()
{
	return m_zoomMode;
}

void CZoomView::SetZoomMode(ZOOMMODE mode)
{
	ASSERT(m_nMapMode == MM_ANISOTROPIC);

	if (mode != m_zoomMode)
	{
		m_zoomMode = mode;

		if( m_hCursor != NULL )
		{
			::DestroyCursor(m_hCursor);
			m_hCursor = NULL;
		}

		if( mode == MODE_ZOOMIN )
		{
			m_hCursor = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_ZOOMIN));
		}
		else if( mode == MODE_ZOOMOUT )
		{
			m_hCursor = ::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_ZOOMOUT));
		}

		// Force cursor change now
		OnSetCursor(NULL, HTCLIENT, 0);
	}
}


void CZoomView::CenterOnLogicalPoint(CPoint pt)
{
	// Convert the point to device coordinates
	ViewLPtoDP(&pt);
	// Account for scroll bar position
	ClientToDevice(pt);

	// Use CScrollView's function for device coordinates
	CScrollView::CenterOnPoint(pt);
}

CPoint CZoomView::GetLogicalCenterPoint()
{
	CPoint pt;
	CRect rect;
	// Get the center of screen
	GetClientRect(&rect);
	pt.x = (rect.Width()  / 2);
	pt.y = (rect.Height() / 2);

	// Convert the point to logical coordinates
	ViewDPtoLP(&pt);

	return pt;
}


void CZoomView::DrawBox (CDC& dc, CRect& rect, BOOL xor)
{
	CPen pen;
	// Save the device context
	dc.SaveDC();

	if (xor)
	{
		dc.SetROP2(R2_NOTXORPEN);
		pen.CreatePen(PS_DOT,  0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}
	else
	{
		pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}

	dc.SelectObject(&pen);

	// Draw the rect with lines (eliminate rect middle fill)
	dc.MoveTo(rect.left,  rect.top);
	dc.LineTo(rect.right, rect.top);
	dc.LineTo(rect.right, rect.bottom);
	dc.LineTo(rect.left,  rect.bottom);
	dc.LineTo(rect.left,  rect.top);

	// Clean up
	dc.RestoreDC(-1);
}

void CZoomView::DrawLine (CDC &dc, const int &x1, const int &y1, const int &x2, const int &y2, BOOL xor)
{	
	CPen pen;

	// Save the device context
	dc.SaveDC();

	if (xor)
	{
		dc.SetROP2(R2_NOTXORPEN);
		pen.CreatePen(PS_DASH,  0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}
	else
	{
		pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0)); // 0 width = 1 device unit
	}
	dc.SelectObject(&pen);

	// Draw the line
	dc.MoveTo(x1, y1);
	dc.LineTo(x2, y2);
	// Clean up
	dc.RestoreDC(-1);
}

void CZoomView::ViewDPtoLP(LPPOINT lpPoints, int nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	ASSERT(m_nMapMode > 0); // must be set

	CWindowDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(lpPoints, nCount);
}

void CZoomView::ViewLPtoDP(LPPOINT lpPoints, int nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	ASSERT(m_nMapMode > 0); // must be set

	CWindowDC dc(this);
	OnPrepareDC(&dc);
	dc.LPtoDP(lpPoints, nCount);
}

void CZoomView::ClientToDevice(CPoint &point)
{
	// Need to account for scrollbar position
	CPoint scrollPt = GetDeviceScrollPosition();
	point.x += scrollPt.x;
	point.y += scrollPt.y;
}

void CZoomView::NormalizeRect(CRect &rect)
{
	if (rect.left > rect.right)
	{
		int r = rect.right;
		rect.right = rect.left;
		rect.left = r;
	}

	if (rect.top > rect.bottom)
	{
		int b = rect.bottom;
		rect.bottom = rect.top;
		rect.top = b;
	}
}


void CZoomView::PersistRatio(const CSize& os, CSize& ds, CPoint& remainder)
{
	float ratio1 = (float) os.cx / os.cy;
	float ratio2 = (float) ds.cx / ds.cy;
	int   newSize;

	// Do nothing if they are the same
	if (ratio1 > ratio2)
	{
		// Shrink hieght
		newSize = (int)(ds.cx / ratio1);
		remainder.x = 0;
		remainder.y = ds.cy - newSize;
		ds.cy = newSize;
	}
	else if (ratio2 > ratio1)
	{
		// Shrink width
		newSize = (int)(ds.cy * ratio1);
		remainder.x = ds.cx - newSize;
		remainder.y = 0;
		ds.cx = newSize;
	}
}


BOOL CZoomView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest != HTCLIENT)
		return CScrollView::OnSetCursor(pWnd, nHitTest, message);

	switch (m_zoomMode) {
		case MODE_ZOOMOFF:
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			break;

		default:
			// All other zoom modes
			::SetCursor(m_hCursor);
			break;
	}

	return TRUE;
}


void CZoomView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnLButtonDown(nFlags, point);

	switch (m_zoomMode) {
		case MODE_ZOOMIN:
			if( m_zoomScale > m_maxScale )
				break;

			// Capture the mouse for zooming in
			m_bCaptured = TRUE;
			SetCapture();
			// Save the mouse down point for XOR rect
			ViewDPtoLP(&point);
			m_dragRect.SetRect(point.x, point.y, point.x, point.y);

			break;

		case MODE_ZOOMOFF:
			{
				BOOL bHScroll = (GetStyle() & WS_HSCROLL);
				BOOL bVScroll = (GetStyle() & WS_VSCROLL);
				if( !bHScroll && !bVScroll )
				{
					break;
				}
				CClientDC dc(this);
				OnPrepareDC(&dc);
				dc.DPtoLP(&point);

				m_bCaptured = TRUE;
				SetCapture();
				m_anchor = point;
				m_bMove = FALSE;

				::SetCursor(m_hHandOver);
			}
			break;

		default:
			// Do nothing.
			break;
	}
}


void CZoomView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CScrollView::OnLButtonUp(nFlags, point);

	switch (m_zoomMode) {
		case MODE_ZOOMIN:
			// Uncapture the mouse?
			if (m_bCaptured) {
				m_bCaptured = FALSE;
				ReleaseCapture();
				// Set back the cross cursor to the Z
				::SetCursor(m_hCursor);
				// Get the Device Context
				CClientDC dc(this);
				OnPrepareDC(&dc);
				// Erase the bounding box
				DrawBox(dc, m_dragRect);
				Invalidate();
				UpdateWindow();
				// Now Zoom in on logical rectangle
				ZoomIn(m_dragRect);
			}
			break;

		case MODE_ZOOMOUT:
			ViewDPtoLP(&point);
			ZoomOut(&point);
			break;

		case MODE_ZOOMOFF:
			if( m_bCaptured )
			{
				ReleaseCapture();
				m_bCaptured = FALSE;

				// Set back the cross cursor to the Z
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));

				m_bMove = FALSE;
			}
			break;
		default:
			// Do nothing.
			break;
	}
}


void CZoomView::OnMouseMove(UINT nFlags, CPoint point)
{
	CScrollView::OnMouseMove(nFlags, point);

	if (m_bCaptured) {
		// Get the Device Context
		CClientDC dc(this);
		OnPrepareDC(&dc);
	
		switch (m_zoomMode) {
			case MODE_ZOOMIN:
				// Draw the drag-rect
				// Erase last rect
				DrawBox(dc, m_dragRect);
				Invalidate();
				UpdateWindow();

				// Draw new rect
				dc.DPtoLP(&point);
				m_dragRect.BottomRight() = point;
				DrawBox(dc, m_dragRect);
				Invalidate();
				UpdateWindow();

				break;

			case MODE_ZOOMOFF:
				if( !m_bMove )
				{
					::SetCursor(m_hHandDrag);
					m_bMove = TRUE;
				}

				{
					CClientDC dc(this);
					OnPrepareDC(&dc);
					dc.DPtoLP(&point);

					BOOL bHScroll = (GetStyle() & WS_HSCROLL);
					BOOL bVScroll = (GetStyle() & WS_VSCROLL);
					if( bHScroll || bVScroll )
					{
						int offX = m_anchor.x - point.x;
						int offY = m_anchor.y - point.y;
						offX = (int)(offX * GetZoomScale());
						offY = (int)(offY * GetZoomScale());
						if( !bHScroll )
							offX = 0;
						if( !bVScroll )
							offY = 0;

						CPoint pt = GetScrollPosition();
						pt.x += offX;
						pt.y += offY;
						ScrollToPosition(pt);

						Invalidate();
					}
				}

				break;
			default:
				// Do nothing.
				break;
		}
	}
}


void CZoomView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonDown(nFlags, point);

	// See if currently captured
	if (m_bCaptured) {
		// Maintain current mode, just stop current drag
		m_bCaptured = FALSE;
		ReleaseCapture();
		// Get the Device Context
		CClientDC dc(this);
		OnPrepareDC(&dc);

		switch (m_zoomMode)
		{
			case MODE_ZOOMIN:
				// Erase last rect
				DrawBox(dc, m_dragRect);
				Invalidate();
				UpdateWindow();
				break;

			default:
				// Do nothing.
				break;
		}
	} else {
		// Cancel current mode
		m_zoomMode = MODE_ZOOMOFF;
	}
}


BOOL CZoomView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if( MK_CONTROL & nFlags )
	{
		if( zDelta >0 )
		{
			ZoomIn();
		}
		else
		{
			ZoomOut();
		}
		Invalidate();

		return FALSE;
	}
	Invalidate();

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}


BOOL CZoomView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	return FALSE;
	//return CScrollView::OnEraseBkgnd(pDC);
}


void CZoomView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	LockWindowUpdate();

	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。
	switch( nChar )
	{
	case VK_HOME:
		OnVScroll(SB_TOP, 0, NULL);
		OnHScroll(SB_LEFT, 0, NULL);
		break;
	case VK_END:
		OnVScroll(SB_BOTTOM, 0, NULL);
		OnHScroll(SB_RIGHT, 0, NULL);
		break;
	case VK_UP:
		OnVScroll(SB_LINEUP, 0, NULL);
		break;
	case VK_DOWN:
		OnVScroll(SB_LINEDOWN, 0, NULL);
		break;
	case VK_PRIOR:
		OnVScroll(SB_PAGEUP, 0, NULL);
		break;
	case VK_NEXT:
		OnVScroll(SB_PAGEDOWN, 0, NULL);
		break;
	case VK_LEFT:
		OnHScroll(SB_LINELEFT, 0, NULL);
		break;
	case VK_RIGHT:
		OnHScroll(SB_LINERIGHT, 0, NULL);
		break;
	default:
		break;
	}
	UnlockWindowUpdate();

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CZoomView::DocToClient(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(&point);
}

void CZoomView::DocToClient(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(rect);
	rect.NormalizeRect();
}

void CZoomView::ClientToDoc(CPoint& point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(&point);
}

void CZoomView::ClientToDoc(CRect& rect)
{
	CClientDC dc(this);
	OnPrepareDC(&dc, NULL);
	dc.DPtoLP(rect);
	//ASSERT(rect.left <= rect.right);
	//ASSERT(rect.bottom >= rect.top);
}

BOOL CZoomView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= (WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	if( !CScrollView::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

