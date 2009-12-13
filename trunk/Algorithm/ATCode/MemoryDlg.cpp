// MemoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ATCode.h"
#include "MemoryDlg.h"


// CMemoryDlg dialog

IMPLEMENT_DYNAMIC(CMemoryDlg, CDialog)

CMemoryDlg::CMemoryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoryDlg::IDD, pParent)
	, m_strSelectedArg(_T(""))
	, m_bCustom(FALSE)
	, m_strCustomMem(_T(""))
	, m_strMemName(_T(""))
{

}

CMemoryDlg::~CMemoryDlg()
{
}

void CMemoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listMemories);
	DDX_LBString(pDX, IDC_LIST1, m_strSelectedArg);
	DDX_Check(pDX, IDC_CHECK1, m_bCustom);
	DDX_Control(pDX, IDC_EDIT1, m_editCustomArg);
	DDX_Control(pDX, IDC_STATIC_CUSTOM, m_staticCustom);
	DDX_Text(pDX, IDC_EDIT1, m_strCustomMem);
	DDX_Text(pDX, IDC_EDIT2, m_strMemName);
}


BEGIN_MESSAGE_MAP(CMemoryDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST1, &CMemoryDlg::OnLbnDblclkList1)
	ON_BN_CLICKED(IDC_CHECK1, &CMemoryDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDOK, &CMemoryDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMemoryDlg message handlers
BOOL CMemoryDlg::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();

	// 리스트 박스 초기화
	m_listMemories.AddString(_T("EAX"));
	m_listMemories.AddString(_T("EBX"));
	m_listMemories.AddString(_T("ECX"));
	m_listMemories.AddString(_T("EDX"));
	m_listMemories.AddString(_T("ESI"));
	m_listMemories.AddString(_T("EDI"));
	m_listMemories.AddString(_T("EBP"));
	m_listMemories.AddString(_T("ESP"));
	m_listMemories.AddString(_T("[ESP]"));
	
	CString strMem;
	for(int i=4; i<=0x200; i+=4)
	{
		strMem.Format(_T("[ESP+0x%x]"), i);
		m_listMemories.AddString(strMem);
	}

	return bRetVal;
}

void CMemoryDlg::OnLbnDblclkList1()
{
	OnBnClickedOk();
}

void CMemoryDlg::OnBnClickedCheck1()
{
	UpdateData();

	if(m_bCustom)
	{
		m_listMemories.ShowWindow(SW_HIDE);
		m_editCustomArg.ShowWindow(SW_SHOW);
		m_staticCustom.ShowWindow(SW_SHOW);
	}
	else
	{
		m_listMemories.ShowWindow(SW_SHOW);
		m_editCustomArg.ShowWindow(SW_HIDE);
		m_staticCustom.ShowWindow(SW_HIDE);
	}
}

void CMemoryDlg::OnBnClickedOk()
{
	UpdateData();
	m_strMemName = m_strMemName.Trim();
	if(m_strMemName.IsEmpty())
	{
		MessageBox(_T("Please enter the context name."), _T("AT Code"), MB_OK);
		return;
	}
	
	OnOK();
}
