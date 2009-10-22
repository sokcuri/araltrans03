// EZTransOptionDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ezTransXP.h"
#include "EZTransOptionDlg.h"


// CEZTransOptionDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CEZTransOptionDlg, CDialog)

CEZTransOptionDlg::CEZTransOptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEZTransOptionDlg::IDD, pParent)
	, m_bRemoveTrace(FALSE)
	, m_bRemoveDupSpace(FALSE)
{

}

CEZTransOptionDlg::~CEZTransOptionDlg()
{
}

void CEZTransOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_ALLOW_TRACE, m_bRemoveTrace);
	DDX_Check(pDX, IDC_CHK_ALLOW_DUPSPACE, m_bRemoveDupSpace);
}


BEGIN_MESSAGE_MAP(CEZTransOptionDlg, CDialog)
END_MESSAGE_MAP()


// CEZTransOptionDlg 메시지 처리기입니다.
