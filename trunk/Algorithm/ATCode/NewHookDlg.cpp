// cpp : implementation file
//

#include "stdafx.h"
#include "ATCode.h"
#include "ATCodeMgr.h"
#include "NewHookDlg.h"


// CNewHookDlg dialog

IMPLEMENT_DYNAMIC(CNewHookDlg, CDialog)

CNewHookDlg::CNewHookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewHookDlg::IDD, pParent)
	, m_strHookAddr(_T(""))
{

}

CNewHookDlg::~CNewHookDlg()
{
}

void CNewHookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HOOK_ADDR, m_strHookAddr);
	DDX_Control(pDX, IDC_COMBO1, m_comboMods);
}


BEGIN_MESSAGE_MAP(CNewHookDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CNewHookDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CNewHookDlg message handlers

BOOL CNewHookDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	MODULEENTRY32 me32[200];
	int nModCnt = CATCodeMgr::GetAllLoadedModules(me32, 200);

	int nIdx = m_comboMods.AddString(_T("[Absolute]"));
	if(nIdx != CB_ERR) m_comboMods.SetItemData(nIdx, NULL);
	m_comboMods.SetCurSel(0);

	for(int i=0; i<nModCnt; i++)
	{
		nIdx = m_comboMods.AddString(me32[i].szModule);
		if(nIdx != CB_ERR) m_comboMods.SetItemData(nIdx, (DWORD_PTR)me32[i].hModule);
	}

	return bRetVal;
}
void CNewHookDlg::OnBnClickedOk()
{
	int nCurSel = m_comboMods.GetCurSel();

	m_hModule = (HMODULE)m_comboMods.GetItemData(nCurSel);

	if(NULL == m_hModule)
	{
		m_strModuleName = _T("");
	}
	else
	{
		m_comboMods.GetLBText(nCurSel, m_strModuleName);
	}

	OnOK();
}
