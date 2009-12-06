#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CMainDbgDlg dialog

class CMainDbgDlg : public CDialog
{
	DECLARE_DYNAMIC(CMainDbgDlg)
private:
	BOOL m_bInitialized;

public:
	CMainDbgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMainDbgDlg();

// Dialog Data
	enum { IDD = IDD_MAIN_DBG_DLG };

protected:
	void RepositionControls(int cx, int cy);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	// 함수 목록 컨트롤
	CListCtrl m_ctrlFuncList;
	// 레지스터 & 스택 값 리스트
	CListCtrl m_ctrlRegStackList;
	// 콜스택 표시 컨트롤
	CListBox m_ctrlCallstack;

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnKeydownListFunctions(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnClose();
	CStatic m_lblMainTextFunc;
};
