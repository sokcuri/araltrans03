#pragma once
#include "afxwin.h"


// CModuleInfoDlg 대화 상자입니다.

class CModuleInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CModuleInfoDlg)

public:
	CString m_strHookAddr;
	CModuleInfoDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CModuleInfoDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_MOD_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editModName;
	CEdit m_editModRange;
};
