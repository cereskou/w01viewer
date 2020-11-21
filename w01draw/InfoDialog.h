#pragma once

#define PROP_TYPE_NORMAL	0
#define PROP_TYPE_BUTTON	1
#define PROP_TYPE_LIST		2
#define PROP_TYPE_LIST2		3

#include "CustomProp.h"

typedef CMap<DWORD, DWORD, CString, LPCTSTR>	CMapDWordToString;
typedef CMap<DWORD, DWORD, int, int>			CMapDWordToInt;

// CInfoDialog �_�C�A���O

class CInfoDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CInfoDialog)

public:
	CInfoDialog(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~CInfoDialog();

	void SetTitle(UINT nID);
	void SetTitle(LPCTSTR lpszTitle);

	void SetValue(UINT nID, LPCTSTR lpszVal, int nType = -1, CiEveryCallback* pCall = NULL);
	void SetValue(UINT nID, LPCTSTR lpszKey, LPCTSTR lpszVal, int nType = -1, CiEveryCallback* pCall = NULL);
	LPCTSTR GetValue(UINT nID);
	LPCTSTR GetKey(UINT nID);
	int GetType(UINT nID);
	CiEveryCallback* GetCallBack(UINT nID);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_INFODIALOG };
protected:
	UINT	m_nTitleID;
	CString	m_szTitle;
	CPropertyGridCtrlEx m_wndPropList;
	CFont	m_fntPropList;

	CArray<UINT, UINT&>	m_lstKey;

	CMapDWordToString	m_mapKey;
	CMapDWordToString	m_mapVal;
	CMapDWordToInt		m_mapType;
	CMapWordToPtr		m_mapCall;
protected:
	void AdjustLayout();
	void InitPropList(UINT nID);
	void SetPropListFont();

	void InitPropList_DocInf();		// ���[��{���
	void InitPropList_FntInf();		// �t�H���g���
	void InitPropList_UsrInf();		// ���[�U�[���

	BOOL AddSubItem(CPropertyGridPropertyEx* pGroup, UINT nID, UINT nTitleID = 0);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
