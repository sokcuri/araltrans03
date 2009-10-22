// ModuleInfoDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ATCode.h"
#include "ModuleInfoDlg.h"
#include <Psapi.h>


// CModuleInfoDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CModuleInfoDlg, CDialog)

CModuleInfoDlg::CModuleInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModuleInfoDlg::IDD, pParent)
{


}

CModuleInfoDlg::~CModuleInfoDlg()
{
}

void CModuleInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MOD_NAME, m_editModName);
	DDX_Control(pDX, IDC_EDIT_MOD_RANGE, m_editModRange);
}

BOOL CModuleInfoDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	HMODULE hModule = (HMODULE)0x00400000;
	int nIdx = m_strHookAddr.ReverseFind(_T('!'));

	if(nIdx>=0)
	{
		CString strModuleName = m_strHookAddr.Left(nIdx);

		hModule = GetModuleHandle(strModuleName);
		if(NULL == hModule) throw -3;
	}

	TCHAR szModPath[MAX_PATH] = {0,};
	GetModuleFileName(hModule, szModPath, MAX_PATH);
	m_editModName.SetWindowText(szModPath);

	MODULEINFO mi;
	if( GetModuleInformation( ::GetCurrentProcess(), hModule, &mi, sizeof(MODULEINFO) ) == FALSE ) throw -5;
	CString strModRange;
	strModRange.Format(_T("0x%p~0x%p (Size:%d)"), mi.lpBaseOfDll, (UINT_PTR)mi.lpBaseOfDll+(UINT_PTR)mi.SizeOfImage, mi.SizeOfImage);
	m_editModRange.SetWindowText(strModRange);

	return bRetVal;
}

BEGIN_MESSAGE_MAP(CModuleInfoDlg, CDialog)
END_MESSAGE_MAP()


// CModuleInfoDlg 메시지 처리기입니다.
