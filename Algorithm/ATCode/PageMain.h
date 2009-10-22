#pragma once

#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

class COptionNode;

// CPageMain dialog

class CPageMain : public CDialog
{
	DECLARE_DYNAMIC(CPageMain)
private:
	COptionNode*	m_pRootNode;

public:
	CPageMain(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPageMain();

	BOOL InitFromRootNode(COptionNode* pRootNode);
	void ClearCtrlValues();
	void SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox);

// Dialog Data
	enum { IDD = IDD_PAGE_MAIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnAddHook();
};
