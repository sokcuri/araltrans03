// MainDbgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DebuggingATCode.h"
#include "MainDbgDlg.h"
#include "ATTextArgMgr.h"


// CMainDbgDlg dialog

IMPLEMENT_DYNAMIC(CMainDbgDlg, CDialog)

CMainDbgDlg::CMainDbgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDbgDlg::IDD, pParent), m_bInitialized(FALSE)
{

}

CMainDbgDlg::~CMainDbgDlg()
{
}

void CMainDbgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FUNCTIONS, m_ctrlFuncList);
	DDX_Control(pDX, IDC_LIST_REGNSTACK, m_ctrlRegStackList);
	DDX_Control(pDX, IDC_LIST_CALLSTACK, m_ctrlCallstack);
	DDX_Control(pDX, IDC_STATIC_FUNCNAME, m_lblMainTextFunc);
}


BEGIN_MESSAGE_MAP(CMainDbgDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_FUNCTIONS, &CMainDbgDlg::OnLvnKeydownListFunctions)
	ON_WM_CLOSE()
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////////
//
// 다이얼로그 초기화
//
//////////////////////////////////////////////////////////////////////////
BOOL CMainDbgDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	// 함수목록 컬럼헤더 정의
	m_ctrlFuncList.InsertColumn(0, _T("Address"), LVCFMT_RIGHT, 100);
	m_ctrlFuncList.InsertColumn(1, _T("Score"), 0, 100);
	m_ctrlFuncList.InsertColumn(2, _T("Last Text (Japanese)"), 0, 200);
	m_ctrlFuncList.InsertColumn(3, _T("Last Text (Korean)"), 0, 200);
	ListView_SetExtendedListViewStyle( m_ctrlFuncList.GetSafeHwnd(), LVS_EX_FULLROWSELECT  );
	
	// 레지스터&스택 컬럼헤더 정의
	m_ctrlRegStackList.InsertColumn(0, _T("Storage"), 0, 100);
	m_ctrlRegStackList.InsertColumn(1, _T("Value"), 0, 100);
	m_ctrlRegStackList.InsertColumn(2, _T("Text Dump"), 0, 100);
	m_ctrlRegStackList.InsertColumn(3, _T("Text Dump (Korean)"), 0, 100);
	ListView_SetExtendedListViewStyle( m_ctrlRegStackList.GetSafeHwnd(), LVS_EX_FULLROWSELECT  );

	// 컨트롤 위치 & 크기 초기화
	m_bInitialized = TRUE;
	CRect rtClient;
	this->GetClientRect(rtClient);
	RepositionControls(rtClient.Width(), rtClient.Height());

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 사이즈 변동시 이벤트 핸들러
//
//////////////////////////////////////////////////////////////////////////
void CMainDbgDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(m_bInitialized)
	{
		RepositionControls(cx,cy);
	}
	
}



//////////////////////////////////////////////////////////////////////////
//
// 컨트롤 사이즈 위치 재조정
//
//////////////////////////////////////////////////////////////////////////
void CMainDbgDlg::RepositionControls(int cx, int cy)
{
	int nCallstackWidth = 150;
	int nCallstackHeight = 200;

	// 함수 목록
	m_ctrlFuncList.MoveWindow(0, 16, cx, cy-32-nCallstackHeight);

	// 레지스트리&스택 값
	this->GetDlgItem(IDC_STATIC_REGNSTACK)->MoveWindow(1, cy-12-nCallstackHeight, cx-nCallstackWidth-4, 16);
	m_ctrlRegStackList.MoveWindow(0, cy-nCallstackHeight+4, cx-nCallstackWidth-4, nCallstackHeight);

	// 콜스택
	this->GetDlgItem(IDC_STATIC_CALLSTACK)->MoveWindow(cx-nCallstackWidth, cy-12-nCallstackHeight, nCallstackWidth, 16);
	m_ctrlCallstack.MoveWindow(cx-nCallstackWidth, cy-nCallstackHeight+4, nCallstackWidth, nCallstackHeight);

}



void CMainDbgDlg::OnLvnKeydownListFunctions(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	if( VK_F2 == pLVKeyDow->wVKey )
	{
		int nIdx = m_ctrlFuncList.GetSelectionMark();
		
		if(nIdx>=0)
		{		
			CATTextArgMgr::GetInstance()->OnSetBreakPointOnFuncList(nIdx);
		}
	}

	*pResult = 0;
}


BOOL CMainDbgDlg::PreTranslateMessage( MSG* pMsg )
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if( pMsg->wParam == VK_F9 )
		{
			CATTextArgMgr::GetInstance()->OnResumeProgram();
		}

	}

	return CDialog::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////////////////
//
// 윈도우 닫을 시
//
//////////////////////////////////////////////////////////////////////////
void CMainDbgDlg::OnClose()
{
	// 브레이크 포인트 풀어줌
	CATTextArgMgr::GetInstance()->OnResumeProgram();

	// 상황에 따라 Hide / Close
	if(CATTextArgMgr::GetInstance()->m_hContainerWnd)
	{
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		CDialog::OnClose();
	}
}
