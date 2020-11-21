#pragma once

class CDirectoryToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CDirectoryTreeCtrl : public CMFCShellTreeCtrl
{
	HRESULT EnumObjects(HTREEITEM hParentItem, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent)
	{
		ASSERT_VALID(this);
		ASSERT_VALID(afxShellManager);

		LPENUMIDLIST pEnum = NULL;

		HRESULT hr = pParentFolder->EnumObjects(NULL, m_dwFlags, &pEnum);
		if (FAILED(hr) || pEnum == NULL)
		{
			return hr;
		}

		LPITEMIDLIST pidlTemp;
		DWORD dwFetched = 1;

		TCHAR szFullPath[_MAX_PATH] = {0};
		// Enumerate the item's PIDLs:
		while (SUCCEEDED(pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
		{
			TVITEM tvItem;
			ZeroMemory(&tvItem, sizeof(tvItem));

			// Fill in the TV_ITEM structure for this item:
			tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

			// AddRef the parent folder so it's pointer stays valid:
			pParentFolder->AddRef();

			// Put the private information in the lParam:
			LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
			ENSURE(pItem != NULL);

			pItem->pidlRel = pidlTemp;
			pItem->pidlFQ = afxShellManager->ConcatenateItem(pidlParent, pidlTemp);

			pItem->pParentFolder = pParentFolder;
			tvItem.lParam = (LPARAM)pItem;

			CString strItem = OnGetItemText(pItem);
			tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
			tvItem.iImage = OnGetItemIcon(pItem, FALSE);
			tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);

			// Determine if the item has children:
			//DWORD dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME | SFGAO_FILESYSANCESTOR;
			DWORD dwAttribs = SFGAO_CAPABILITYMASK | SFGAO_GHOSTED | SFGAO_LINK| SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_FILESYSANCESTOR;
			//DWORD dwAttribs = SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK;

			pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*) &pidlTemp, &dwAttribs);
			tvItem.cChildren = (dwAttribs & (SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR));

			// Determine if the item is shared:
			if (dwAttribs & SFGAO_SHARE)
			{
				tvItem.mask |= TVIF_STATE;
				tvItem.stateMask |= TVIS_OVERLAYMASK;
				tvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
			}

			// Fill in the TV_INSERTSTRUCT structure for this item:
			TVINSERTSTRUCT tvInsert;

			tvInsert.item = tvItem;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.hParent = hParentItem;

			if( dwAttribs & SFGAO_FOLDER )
			{
				InsertItem(&tvInsert);
			}
			else
			{
				if( ::SHGetPathFromIDList(pItem->pidlFQ, szFullPath) )
				{
					LPTSTR lpExt = ::PathFindExtension(szFullPath);
					if( lpExt && _tcsicmp(lpExt, _T(".w01")) == 0 )
					{
						InsertItem(&tvInsert);
					}
				}
			}

			dwFetched = 0;
		}

		pEnum->Release();
		return S_OK;
	}
};

// CDirectoryView
class CDirectoryView : public CDockablePane
{
	DECLARE_DYNAMIC(CDirectoryView)

public:
	CDirectoryView();
	virtual ~CDirectoryView();

protected:
	void AdjustLayout();
	CString& GetSelectedPath();

protected:
	CDirectoryToolBar	m_wndToolBar;
	CDirectoryTreeCtrl	m_wndShellTree;
	CString				m_szDocName;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnProperties();
	afx_msg void OnListExpand();
	afx_msg void OnFileListSelected(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


