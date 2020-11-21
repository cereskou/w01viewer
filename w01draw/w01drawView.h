
// w01drawView.h : CW01DrawView �N���X�̃C���^�[�t�F�C�X
//

#pragma once
#include "ZoomView.h"
#include "TextToolTips.h"

class CW01DrawView : public CZoomView
{
protected: // �V���A��������̂ݍ쐬���܂��B
	CW01DrawView();
	DECLARE_DYNCREATE(CW01DrawView)

// ����
public:
	CW01DrawDoc* GetDocument() const;

// ����
public:
	void SetHighLightID(INT id);
// �I�[�o�[���C�h
public:
	virtual void Draw(CDC* pDC);  // ���̃r���[��`�悷�邽�߂ɃI�[�o�[���C�h����܂��B
protected:
	virtual void OnInitialUpdate(); // �\�z��ɏ��߂ČĂяo����܂��B
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	void NotifyZoom();
	void SetSelectedZoomScale(CComboBox* pBox, float scale);
	float GetSelectedZoomScale(CComboBox* pBox);

	void ClearViewClicked();

// ����
public:
	virtual ~CW01DrawView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CTextToolTips	m_toolTips;
	BOOL m_bShowTips;
	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewZoom();
	afx_msg void OnViewFull();
	afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewShowwide();
	afx_msg void OnUpdateViewZoomin(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewZoomout(CCmdUI *pCmdUI);
	afx_msg void OnViewShowinfo();
	afx_msg void OnPageFirst();
	afx_msg void OnUpdatePageFirst(CCmdUI *pCmdUI);
	afx_msg void OnPageLast();
	afx_msg void OnUpdatePageLast(CCmdUI *pCmdUI);
	afx_msg void OnPageNext();
	afx_msg void OnUpdatePageNext(CCmdUI *pCmdUI);
	afx_msg void OnPagePrev();
	afx_msg void OnUpdatePagePrev(CCmdUI *pCmdUI);
	afx_msg void OnViewText();
	afx_msg void OnUpdateViewText(CCmdUI *pCmdUI);
	afx_msg void OnViewOvly();
	afx_msg void OnUpdateViewOvly(CCmdUI *pCmdUI);
	afx_msg void OnViewImage();
	afx_msg void OnUpdateViewImage(CCmdUI *pCmdUI);
	afx_msg void OnViewLine();
	afx_msg void OnUpdateViewLine(CCmdUI *pCmdUI);
	afx_msg void OnViewTextXY();
	afx_msg void OnUpdateViewTextXY(CCmdUI *pCmdUI);
	afx_msg void OnViewCellDef();
	afx_msg void OnUpdateViewCellDef(CCmdUI *pCmdUI);
	afx_msg void OnViewHighLight();
	afx_msg void OnUpdateViewHighLight(CCmdUI *pCmdUI);
	afx_msg void OnViewTextDump();
	afx_msg void OnUpdateViewTextDump(CCmdUI *pCmdUI);
	afx_msg void OnViewSaveImage();
	afx_msg void OnUpdateViewSaveImage(CCmdUI *pCmdUI);

	afx_msg void OnViewWsi();
	afx_msg void OnUpdateViewWsi(CCmdUI *pCmdUI);

	afx_msg void OnViewTips();
	afx_msg void OnViewEscape();
	afx_msg void OnUpdateViewTips(CCmdUI *pCmdUI);
	afx_msg LRESULT OnDocRefresh(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocFontSync(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocGotoPage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocUpdateUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // w01drawView.cpp �̃f�o�b�O �o�[�W����
inline CW01DrawDoc* CW01DrawView::GetDocument() const
   { return reinterpret_cast<CW01DrawDoc*>(m_pDocument); }
#endif

