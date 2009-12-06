#pragma once
#include "afxcmn.h"
#include "PictureEx/PictureEx.h"
#include "afxwin.h"


// CTransProgressDlg 대화 상자입니다.

class CTransProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CTransProgressDlg)

public:
	CTransProgressDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CTransProgressDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_TRANS_PROGRESS };

protected:
	CWinThread* m_pThread;
	BOOL		m_bCancel;
	BOOL		m_bShowUI;
	CStdioFile	m_fileLog;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	static int CALLBACK ProgressCallback(void* pContext, int nTransed, int nTotal);
	static UINT MainThread(void *lParam);

public:

	CString m_strCacheFileName;
	LPCWSTR m_cwszTextSrc;

	CProgressCtrl m_progTrans;
	afx_msg void OnBnClickedCancel();

	CPictureEx m_picAniGif;
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
