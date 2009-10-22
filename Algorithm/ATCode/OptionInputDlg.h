#pragma once


// COptionInputDlg dialog

class COptionInputDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionInputDlg)

public:
	COptionInputDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionInputDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STRING_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strInputString;
};
