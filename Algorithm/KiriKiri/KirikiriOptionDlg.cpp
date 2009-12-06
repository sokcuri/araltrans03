// KirikiriOptionDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Kirikiri.h"
#include "KirikiriOptionDlg.h"

extern CKirikiriApp theApp;

// CKirikiriOptionDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CKirikiriOptionDlg, CDialog)

CKirikiriOptionDlg::CKirikiriOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKirikiriOptionDlg::IDD, pParent)
	, m_nCacheMode(0)
	, m_bAlsoSrc(FALSE)
	, m_bUseCP2(FALSE)
	, m_nCP2Type(0)
{

}

CKirikiriOptionDlg::~CKirikiriOptionDlg()
{
}

void CKirikiriOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nCacheMode);
	DDX_Check(pDX, IDC_CHECK_ALSO_SRC, m_bAlsoSrc);
	DDX_Check(pDX, IDC_CHECK_USE_CODEPOINT2, m_bUseCP2);
	DDX_Control(pDX, IDC_COMBO_CODEPOINTTYPE, m_comboCP2Type);
}


BEGIN_MESSAGE_MAP(CKirikiriOptionDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_CLEAR_CACHE, &CKirikiriOptionDlg::OnBnClickedBtnClearCache)
	ON_BN_CLICKED(IDC_CHECK_USE_CODEPOINT2, &CKirikiriOptionDlg::OnBnClickedCheckUseCodepoint2)
	ON_CBN_SELCHANGE(IDC_COMBO_CODEPOINTTYPE, &CKirikiriOptionDlg::OnCbnSelchangeComboCodepointtype)
	ON_BN_CLICKED(IDOK, &CKirikiriOptionDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CKirikiriOptionDlg 메시지 처리기입니다.

void CKirikiriOptionDlg::OnBnClickedBtnClearCache()
{
	int nMsgRes = this->MessageBox(_T("지금까지 누적된 스크립트들(ZIP 파일, 텍스트 파일)을 \r\n모두 지우고 초기화 하시겠습니까?"), _T("Warning!"), MB_YESNO | MB_ICONWARNING);

	if(IDYES == nMsgRes)
	{
		if(theApp.ClearCache()) this->MessageBox(_T("캐시 저장소가 초기화되었습니다."), _T("Aral Trans"));
		else  this->MessageBox(_T("캐시 초기화에 실패했습니다."), _T("Aral Trans"));
	}
}

void CKirikiriOptionDlg::OnBnClickedCheckUseCodepoint2()
{
	UpdateData();

	if (m_bUseCP2)
	{
		m_comboCP2Type.EnableWindow(TRUE);
	}
	else
	{
		m_nCP2Type=0;
		m_comboCP2Type.SetCurSel(0);
		m_comboCP2Type.EnableWindow(FALSE);
	}
}

BOOL CKirikiriOptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_comboCP2Type.InsertString(0, _T("번역안함"));
	m_comboCP2Type.InsertString(1, _T("일반"));
	m_comboCP2Type.InsertString(2, _T("SilverHawk"));

	m_comboCP2Type.SetCurSel(m_nCP2Type);

	if (m_bUseCP2)
		m_comboCP2Type.EnableWindow(TRUE);
	else
		m_comboCP2Type.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CKirikiriOptionDlg::OnCbnSelchangeComboCodepointtype()
{
	UpdateData();
	m_nCP2Type = m_comboCP2Type.GetCurSel();
}

void CKirikiriOptionDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnOK();
}
