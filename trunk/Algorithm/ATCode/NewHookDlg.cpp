// cpp : implementation file
//

#include "stdafx.h"
#include "ATCode.h"
#include "ATCodeMgr.h"
#include "NewHookDlg.h"
#include "OptionDlg.h"


// CNewHookDlg dialog

IMPLEMENT_DYNAMIC(CNewHookDlg, CDialog)

CNewHookDlg::CNewHookDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewHookDlg::IDD, pParent)
	, m_strHookAddr(_T(""))
	, m_nModCnt(0)
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

	ZeroMemory(m_me32, sizeof(MODULEENTRY32) * 200);
	m_nModCnt = CATCodeMgr::GetAllLoadedModules(m_me32, 200);

	int nIdx = m_comboMods.AddString(_T("[Absolute]"));
	if(nIdx != CB_ERR) m_comboMods.SetItemData(nIdx, NULL);
	m_comboMods.SetCurSel(0);

	for(int i=0; i<m_nModCnt; i++)
	{
		nIdx = m_comboMods.AddString(m_me32[i].szModule);
		if(nIdx != CB_ERR) m_comboMods.SetItemData(nIdx, (DWORD_PTR)m_me32[i].hModule);
	}

	return bRetVal;
}
void CNewHookDlg::OnBnClickedOk()
{
	int nCurSel = m_comboMods.GetCurSel();

	m_hModule = (HMODULE)m_comboMods.GetItemData(nCurSel);

	UpdateData();

	if(NULL == m_hModule)
	{
		m_strHookAddr = COptionDlg::FormatAddress(m_strHookAddr);
		if(!m_strHookAddr.IsEmpty())
		{
			m_strModuleName = _T("");
			UINT_PTR pCodePoint = NULL;
			_stscanf_s(m_strHookAddr, _T("%x"), &pCodePoint);

			for(int i=0; i<m_nModCnt; i++)
			{

				if((UINT_PTR)m_me32[i].modBaseAddr <= pCodePoint 
					&& pCodePoint <= (UINT_PTR)m_me32[i].modBaseAddr + (UINT_PTR)m_me32[i].modBaseSize )
				{
					pCodePoint -= (UINT_PTR)m_me32[i].hModule;
					m_strModuleName = m_me32[i].szModule;
					m_strHookAddr.Format(_T("%x"), pCodePoint);
					m_hModule = m_me32[i].hModule;
					break;
				}
			}

			if(m_strModuleName.IsEmpty() && 0x00400000 < pCodePoint && pCodePoint < 0x01000000)
			{
				pCodePoint -= 0x00400000;
				m_strModuleName = m_me32[0].szModule;
				m_strHookAddr.Format(_T("%x"), pCodePoint);
				m_hModule = m_me32[0].hModule;
			}

		}
	}
	else
	{
		m_comboMods.GetLBText(nCurSel, m_strModuleName);
	}

	UpdateData(FALSE);

	OnOK();
}
