// OptionInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ATCode.h"
#include "OptionInputDlg.h"


// COptionInputDlg dialog

IMPLEMENT_DYNAMIC(COptionInputDlg, CDialog)

COptionInputDlg::COptionInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionInputDlg::IDD, pParent)
	, m_strInputString(_T(""))
{

}

COptionInputDlg::~COptionInputDlg()
{
}

void COptionInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strInputString);
}


BEGIN_MESSAGE_MAP(COptionInputDlg, CDialog)
END_MESSAGE_MAP()


// COptionInputDlg message handlers
