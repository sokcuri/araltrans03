#pragma once
#include "afxwin.h"


// CNewHookDlg dialog

class CNewHookDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewHookDlg)
	MODULEENTRY32 m_me32[200];
	int m_nModCnt;
public:
	CNewHookDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewHookDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_NEW_HOOK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL CNewHookDlg::OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CString m_strHookAddr;
	CString m_strModuleName;
	HMODULE m_hModule;

	CComboBox m_comboMods;
public:
	afx_msg void OnBnClickedOk();
};
