#ifndef __TOOLTIPS_H__
#define __TOOLTIPS_H__
#pragma once


// CTextToolTips

class CTextToolTips : public CWnd
{
	DECLARE_DYNAMIC(CTextToolTips)

public:
	CTextToolTips();
	virtual ~CTextToolTips();

	BOOL Create(CWnd* pParentWnd);
	void SetText(const CString& pszTitle, const CString& pszText);
	BOOL Show(const CPoint& pt);
	void Close();
private:
	void DisplayToolTip(const CPoint& pt);

private:
	CWnd*	m_pParentWnd;
	CPoint	m_pt;
	CFont	m_font;
	BOOL    m_bShowStatus;
	CString m_szText;
	CString m_szTitle;
protected:
	//{{AFX_MSG(CTextToolTips)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif
