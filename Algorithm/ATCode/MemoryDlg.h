#pragma once
#include "afxwin.h"


// CMemoryDlg dialog

class CMemoryDlg : public CDialog
{
	DECLARE_DYNAMIC(CMemoryDlg)

public:
	CMemoryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMemoryDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MEMORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listMemories;
public:
	CString m_strSelectedArg;
public:
	afx_msg void OnLbnDblclkList1();
	BOOL m_bCustom;
	CEdit m_editCustomArg;
	CStatic m_staticCustom;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedOk();
	CString m_strCustomMem;
};
