// TransProgressDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Kirikiri.h"
#include "TransProgressDlg.h"
#include <process.h>

extern CKirikiriApp theApp;

// CTransProgressDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CTransProgressDlg, CDialog)

CTransProgressDlg::CTransProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransProgressDlg::IDD, pParent), m_bCancel(FALSE), m_pThread(NULL), m_bShowUI(FALSE)
{
	
}

CTransProgressDlg::~CTransProgressDlg()
{

}



void CTransProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_progTrans);
	DDX_Control(pDX, IDC_ANI_GIF, m_picAniGif);
}


BEGIN_MESSAGE_MAP(CTransProgressDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CTransProgressDlg::OnBnClickedCancel)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CTransProgressDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	CenterWindow();

	// 번역 쓰레드 쓰레드 시작
	m_pThread = ::AfxBeginThread(MainThread, this);
	
	// 프로그래스바의 배경색과 바 색을 변경
	m_progTrans.SendMessage(PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(230,230,230) );
	
	m_progTrans.SendMessage(PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)(COLORREF)RGB(130,190,255) );

	// 애니메이션 GIF
	if (m_picAniGif.Load(MAKEINTRESOURCE(IDR_GIF_1), _T("GIF")))
		m_picAniGif.Draw();


	this->SetTimer(927, 1500, NULL);

	return bRetVal;  // return TRUE  unless you set the focus to a control	
}


UINT CTransProgressDlg::MainThread(void *lParam)
{
	UINT nRetVal = 0;

	CTransProgressDlg* pThisDlg = (CTransProgressDlg*)lParam;

	try
	{
		CKAGScriptMgr* pScriptMgr = theApp.GetKAGScriptMgr();
		LPCWSTR orig_script = theApp.GetOriginalScript();
		LPWSTR buf = theApp.GetScriptBuffer();
		if(NULL == pScriptMgr) throw -1;
		if(NULL == orig_script) throw -2;
		if(NULL == buf) throw -3;

		if(orig_script[0] == L'\0')
		{
			buf[0] = L'\0';
		}
		else
		{

			// 콜백 함수 등록
			pScriptMgr->SetProgressCallback(pThisDlg, ProgressCallback);

			// 번역
			BOOL bTrans = FALSE;
			for(int i=0; i<5; i++)
			{
				bTrans = pScriptMgr->TranslateScript(orig_script, buf);
				if(bTrans) break;
				Sleep(500);
			}

			if(FALSE == bTrans) throw -4;
		}

		pThisDlg->PostMessage(WM_COMMAND, IDOK);

	}
	catch (int nErrCode)
	{
		TRACE(_T("[aral1] TranslationThread catched exception %d"), nErrCode);
		nRetVal = nErrCode;
		pThisDlg->PostMessage(WM_COMMAND, IDCANCEL);
	}

	return nRetVal;
}

int CALLBACK CTransProgressDlg::ProgressCallback( void* pContext, int nTransed, int nTotal )
{
	CTransProgressDlg* pThisDlg = (CTransProgressDlg*)pContext;
	int nRetVal = 1;
	
	// 취소되었다면
	if(pThisDlg->m_bCancel)
	{
		nRetVal = 0;
	}
	// 진행중이라면
	else
	{
		// 첫 호출이면 프로그래스 범위 세팅
		if(0 == nTransed)
		{
			pThisDlg->m_progTrans.SetRange(0, nTotal);

			CWnd* pWnd = pThisDlg->GetDlgItem(IDC_STATIC_HASH);
			if(pWnd)
			{
				CString strHash;
				strHash.Format(_T("%p"), theApp.GetKAGScriptMgr()->GetCurrentScriptHash());
				pWnd->SetWindowText(strHash);
			}

		}


		CWnd* pWnd = pThisDlg->GetDlgItem(IDC_STATIC_SIZE);
		if(pWnd)
		{
			CString strSize;
			strSize.Format(_T("%d/%d Lines"), nTransed, nTotal);
			pWnd->SetWindowText(strSize);
		}

		pThisDlg->m_progTrans.SetPos(nTransed);

	}

	return nRetVal;
}

// CTransProgressDlg 메시지 처리기입니다.

void CTransProgressDlg::OnBnClickedCancel()
{
	if( MessageBox(_T("번역을 중단하시겠습니까?"), _T("Confirm"), MB_YESNO) == IDYES )
	{
		m_bCancel = TRUE;
		TRACE(_T("[aral1] m_bCancel = TRUE;"));
	}
}

//////////////////////////////////////////////////////////////////////////
//
// 윈도우 안보이게 생성
//
//////////////////////////////////////////////////////////////////////////
void CTransProgressDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{

	if(m_bShowUI)
	{
		lpwndpos->flags |= SWP_SHOWWINDOW;
	}
	else
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}

	CDialog::OnWindowPosChanging(lpwndpos);
}

void CTransProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(nIDEvent == 927)
	{
		this->KillTimer(nIDEvent);
		m_bShowUI = TRUE;
		this->ShowWindow(SW_NORMAL);
	}

	CDialog::OnTimer(nIDEvent);
}
