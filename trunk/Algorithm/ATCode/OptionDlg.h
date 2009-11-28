#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "afxwin.h"

#define WM_DELETE_HOOK (WM_USER+1)

class COptionNode;
class CPageMain;
class CPageHook;

class COptionDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionDlg)
private:
	COptionNode*	m_pRootNode;
	CArray<CDialog*, CDialog*>	m_arrPage;

	BOOL InitFromRootNode(COptionNode* pRootNode);
	void ClearControls();

	int InitMainPage(COptionNode* pRootNode);
	int AddHookPage(COptionNode* pHookNode);

public:
	static COptionDlg* _Inst;
	static CString FormatAddress(LPCTSTR cszAddr);
	
	COptionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionDlg();

	void SetRootOptionNode(COptionNode* pRootNode);
	void ShowPage(int nPageIdx);

// Dialog Data
	enum { IDD = IDD_DIALOG_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL COptionDlg::OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg LRESULT OnDeleteHook(WPARAM wParam, LPARAM lParam);
public:
	CTabCtrl m_tabMain;
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedBtnCreateShortcut();
public:
	afx_msg void OnBnClickedApply();
public:
	CButton m_btnApply;
public:
	afx_msg void OnBnClickedBtnAddHook();
public:
	afx_msg void OnBnClickedBtnInputString();
};
