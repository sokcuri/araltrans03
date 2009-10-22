#pragma once
#include "afxwin.h"


// CEZTransOptionDlg 대화 상자입니다.

class CEZTransOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CEZTransOptionDlg)

public:
	CEZTransOptionDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CEZTransOptionDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRemoveTrace;
	BOOL m_bRemoveDupSpace;
};
