  // PageMain.cpp : implementation file
//

#pragma warning(disable:4996)
#pragma warning(disable:4101)

#include "stdafx.h"
#include "ATCode.h"
#include "PageMain.h"
#include "OptionMgr.h"
#include "OptionDlg.h"

// CPageMain dialog

IMPLEMENT_DYNAMIC(CPageMain, CDialog)

LPCTSTR _FONT_LOAD_DESC[] = {
	_T("한글 폰트를 로드하지 않습니다."),
	_T("문자 출력 함수에 한글폰트를 적용합니다."),
	_T("한글폰트 적용 후 복구하지 않습니다."),
	_T("폰트 로드 시 한글 폰트를 로드해줍니다."),
	_T("프로그램의 모든 폰트를 한글로 바꿉니다.")
};


CPageMain::CPageMain(CWnd* pParent /*=NULL*/)
	: CDialog(CPageMain::IDD, pParent), m_pRootNode(NULL)
{

}

CPageMain::~CPageMain()
{
}

void CPageMain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CPageMain::OnInitDialog()
{
	BOOL bRetVal = CDialog::OnInitDialog();
	return bRetVal;
}

BEGIN_MESSAGE_MAP(CPageMain, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_HOOK, &CPageMain::OnBnClickedBtnAddHook)
END_MESSAGE_MAP()


// CPageMain message handlers
BOOL CPageMain::InitFromRootNode( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;

	try
	{
		if(NULL==pRootNode) throw -1;

		// 컨트롤들 기본 상태로 세팅
		ClearCtrlValues();

		// 모든 노드 순회
		int cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();
		}

		bRetVal = TRUE;

		m_pRootNode = pRootNode;


	}
	catch (int nErr)
	{
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// 모든 UI 컨트롤 초기화
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::ClearCtrlValues()
{
}



//////////////////////////////////////////////////////////////////////////
//
// 새로운 후킹코드 추가
//
//////////////////////////////////////////////////////////////////////////
void CPageMain::SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox)
{
	if(NULL==pParentNode || NULL==cszChildName || _T('\0')==cszChildName[0]) return;

	COptionNode* pNode = pParentNode->GetChild(cszChildName);

	// 체크한 경우
	if(checkbox.GetCheck())
	{
		if(NULL==pNode)
		{
			pNode = pParentNode->CreateChild();
			pNode->SetValue(cszChildName);
		}
	}
	// 체크 해제 한 경우
	else
	{
		if(pNode)
		{
			pParentNode->DeleteChild(pNode);
		}
	}

	
	if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
	{
		COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);
	}
}


void CPageMain::OnBnClickedBtnAddHook()
{
	// TODO: Add your control notification handler code here
	COptionDlg::_Inst->OnBnClickedBtnAddHook();
}
