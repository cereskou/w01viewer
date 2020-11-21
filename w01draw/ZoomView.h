#ifndef __ZOOMVIEW_H__
#define __ZOOMVIEW_H__

#pragma once



// CZoomView ビュー

class CZoomView : public CScrollView
{
	DECLARE_DYNCREATE(CZoomView)

protected:
	CZoomView();           // 動的生成で使用される protected コンストラクター
	virtual ~CZoomView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	void SetZoomSizes(SIZE sizeTotal, const SIZE& sizePage = sizeDefault, const SIZE& sizeLine = sizeDefault);
	void CenterOnLogicalPoint(CPoint pt);
	CPoint GetLogicalCenterPoint();

	typedef enum {
		MODE_ZOOMOFF = 0,
		MODE_ZOOMIN,
		MODE_ZOOMOUT
	} ZOOMMODE;
	void SetZoomMode(ZOOMMODE mode);
	ZOOMMODE GetZoomMode();

	int ZoomTo(CPoint *point = NULL, float scale=1.0);
	int ZoomIn(CRect& rect);
	int ZoomIn(CPoint* point = NULL, float delta = 1.25);
	int ZoomOut(CPoint* point = NULL, float delta = 1.25);
	int ZoomFull();
	int ZoomWide();
	int ZoomHigh();

	virtual void NotifyZoom() {};

	// Zooming Utility functions
	void ViewDPtoLP(LPPOINT lpPoints, int nCount = 1);
	void ViewLPtoDP(LPPOINT lpPoints, int nCount = 1);

	void ClientToDevice(CPoint& point);
	void NormalizeRect(CRect& rect);
	void DrawBox(CDC& dc, CRect& rect, BOOL xor = TRUE);
	void DrawLine(CDC& dc, const int& x1, const int& y1, const int& x2, const int& y2, BOOL xor = TRUE);

	void DocToClient(CPoint& point);
	void DocToClient(CRect& rect);

	void ClientToDoc(CPoint& point);
	void ClientToDoc(CRect& rect);
	
	float GetMaxZoomScale() { return m_maxScale; }
	float GetMinZoomScale() { return m_minScale; }
	void SetMaxZoomScale(float fVal) { m_maxScale = fVal; }
	void SetMinZoomScale(float fVal) { m_minScale = fVal; }
	float GetZoomScale() { return m_zoomScale; }
	void SetZoomScale(float scale) { m_zoomScale = scale; }

protected:
	virtual void Draw(CDC* pDC) {};
	virtual void OnDraw(CDC* pDC);      // このビューを描画するためにオーバーライドされます。
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);

protected:
	// Internally called functions
	void PersistRatio(const CSize& os, CSize& ds, CPoint& remainder);
	void CalcBars();

	ZOOMMODE	m_zoomMode;
	BOOL		m_bCaptured;
	CRect		m_dragRect;
private:
	CSize		m_origTotalDev;
	CSize		m_origPageDev;
	CSize		m_origLineDev;
	HCURSOR		m_hCursor;

	HCURSOR		m_hHandOver;
	HCURSOR		m_hHandDrag;
	BOOL		m_bMove;

	CPoint		m_anchor;

	float		m_zoomScale;
	float		m_maxScale;
	float		m_minScale;
public:
	// Generated message map functions
	//{{AFX_MSG(CZoomView)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


#endif
