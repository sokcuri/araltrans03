// PageHook.cpp : implementation file
//

#pragma warning(disable:4996)
#pragma comment (lib, "psapi.lib")

#include "stdafx.h"
#include "ATCode.h"
#include "PageHook.h"
#include "OptionMgr.h"
#include "OptionDlg.h"
#include "MemoryDlg.h"
#include "ModuleInfoDlg.h"


// CPageHook dialog

IMPLEMENT_DYNAMIC(CPageHook, CDialog)

CPageHook::CPageHook(CWnd* pParent /*=NULL*/)
	: CDialog(CPageHook::IDD, pParent), m_pHookNode(NULL)
	, m_iTransMethod(0)
{

}

CPageHook::~CPageHook()
{
}

void CPageHook::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TRANS_ARGS, m_comboTransArgs);

	DDX_Control(pDX, IDC_CHK_UNICODE, m_chkUnicode);
	DDX_Control(pDX, IDC_CHK_ALLSAMETEXT, m_chkAllSameText);
	DDX_Control(pDX, IDC_CHK_CLIP_JPN, m_chkClipJpn);
	DDX_Control(pDX, IDC_CHK_CLIP_KOR, m_chkClipKor);

	DDX_Control(pDX, IDC_BTN_ARG_DEL, m_btnDelArg);
	DDX_Radio(pDX, IDC_RADIO_NOP, m_iTransMethod);
	DDX_Control(pDX, IDC_CHK_IGNORE, m_chkIgnore);
	DDX_Control(pDX, IDC_EDIT_TRANS_ARG, m_editTransArg);
}


BEGIN_MESSAGE_MAP(CPageHook, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANS_ARGS, &CPageHook::OnCbnSelchangeComboTransArgs)
	ON_BN_CLICKED(IDC_BTN_ARG_ADD, &CPageHook::OnBnClickedBtnArgAdd)
	ON_BN_CLICKED(IDC_BTN_ARG_DEL, &CPageHook::OnBnClickedBtnArgDel)

	ON_BN_CLICKED(IDC_RADIO_NOP, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_PTRCHANGE, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_OVERWRITE, &CPageHook::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_SOW, &CPageHook::OnBnClickedRadio)

	ON_BN_CLICKED(IDC_CHK_UNICODE, &CPageHook::OnBnClickedChkUnicode)
	ON_BN_CLICKED(IDC_CHK_ALLSAMETEXT, &CPageHook::OnBnClickedChkAllsametext)
	ON_BN_CLICKED(IDC_CHK_CLIP_JPN, &CPageHook::OnBnClickedChkClipJpn)
	ON_BN_CLICKED(IDC_CHK_CLIP_KOR, &CPageHook::OnBnClickedChkClipKor)

	ON_BN_CLICKED(IDC_BTN_DEL_HOOK, &CPageHook::OnBnClickedBtnDelHook)

	ON_BN_CLICKED(IDC_CHK_IGNORE, &CPageHook::OnBnClickedChkIgnore)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
//
// 후킹 노드로 UI 구성
//
//////////////////////////////////////////////////////////////////////////
BOOL CPageHook::InitFromHookNode( COptionNode* pHookNode )
{
	BOOL bRetVal = FALSE;

	m_pHookNode = NULL;

	m_comboTransArgs.Clear();
	m_comboTransArgs.ResetContent();
	m_comboTransArgs.EnableWindow(FALSE);

	this->GetDlgItem(IDC_RADIO_NOP)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_PTRCHANGE)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_OVERWRITE)->EnableWindow(FALSE);
	this->GetDlgItem(IDC_RADIO_SOW)->EnableWindow(FALSE);
	
	m_chkIgnore.EnableWindow(FALSE);
	m_chkUnicode.EnableWindow(FALSE);
	m_chkAllSameText.EnableWindow(FALSE);
	m_chkClipKor.EnableWindow(FALSE);
	m_chkClipJpn.EnableWindow(FALSE);
	m_btnDelArg.EnableWindow(FALSE);

	ClearCtrlValues();

	try
	{
		if(NULL==pHookNode || pHookNode->GetValue().CompareNoCase(_T("HOOK"))) throw -1;

		// 이 주소가 속한 모듈의 정보
		COptionNode* pAddrNode = pHookNode->GetChild(0);
		if(pAddrNode==NULL) throw -2;

		CString strAddr = pAddrNode->GetValue();


		// 이 주소에 대한 후킹 명령들 수집
		int cnt2 = pHookNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode* pCmdNode = pHookNode->GetChild(j);
			CString strCmdValue = pCmdNode->GetValue();

			// 번역 명령
			if(strCmdValue.CompareNoCase(_T("TRANS"))==0)
			{
				// 인자 주소
				COptionNode* pDistNode = pCmdNode->GetChild(0);
				if(pDistNode==NULL) throw -4;

				// 메모리 별칭
				COptionNode* pNameNode = pCmdNode->GetChild(1);
				if(pNameNode==NULL) throw -4;

				CString strStorage = pNameNode->GetValue(); // pDistNode->GetValue().MakeUpper();
				int nComboIdx = m_comboTransArgs.AddString(strStorage);
				m_comboTransArgs.SetItemData(nComboIdx, (DWORD_PTR)pCmdNode);

			}

		}

		// 콤보박스에 하나라도 들어갔으면 활성화시킴
		if( m_comboTransArgs.GetCount() > 0 )
		{
			m_comboTransArgs.SetCurSel(0);
			m_comboTransArgs.EnableWindow(TRUE);

			this->GetDlgItem(IDC_RADIO_NOP)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_PTRCHANGE)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_OVERWRITE)->EnableWindow(TRUE);
			this->GetDlgItem(IDC_RADIO_SOW)->EnableWindow(TRUE);

			m_chkUnicode.EnableWindow(TRUE);
			m_chkAllSameText.EnableWindow(TRUE);
			m_chkClipKor.EnableWindow(TRUE);
			m_chkClipJpn.EnableWindow(TRUE);

			m_btnDelArg.EnableWindow(TRUE);
			OnCbnSelchangeComboTransArgs();
		}

		UpdateData(FALSE);
		m_pHookNode = pHookNode;
		bRetVal = TRUE;
	}
	catch (int nErr)
	{
		nErr = nErr;
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// UI 초기화
//
//////////////////////////////////////////////////////////////////////////
void CPageHook::ClearCtrlValues()
{
	m_iTransMethod = 0;

	m_chkUnicode.SetCheck(0);
	m_chkAllSameText.SetCheck(0);
	m_chkClipKor.SetCheck(0);
	m_chkClipJpn.SetCheck(0);
	m_editTransArg.SetWindowText(_T(""));
	
	UpdateData(FALSE);

}
// CPageHook message handlers



//////////////////////////////////////////////////////////////////////////
//
// 번역 인자가 바뀌었을 때
//
//////////////////////////////////////////////////////////////////////////
void CPageHook::OnCbnSelchangeComboTransArgs()
{
	ClearCtrlValues();

	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		// 번역 옵션들 수집
		int cnt3 = pTransCmdNode->GetChildCount();
		if(cnt3 < 2) return;

		m_editTransArg.SetWindowText(pTransCmdNode->GetChild(0)->GetValue());

		for(int k=2; k<cnt3; k++)
		{
			COptionNode* pNode3 = pTransCmdNode->GetChild(k);
			CString strTransOption = pNode3->GetValue().MakeUpper();

			if(strTransOption.IsEmpty())
			{
				continue;
			}

			// NOP 옵션 (아무작업 안함)
			else if(strTransOption == _T("NOP"))
			{
				m_iTransMethod = 0;
				UpdateData(FALSE);
			}
			// PTRCHEAT 옵션 (포인터 바꿔치기)
			else if(strTransOption == _T("PTRCHEAT"))
			{
				m_iTransMethod = 1;
				UpdateData(FALSE);
			}
			// OVERWRITE 옵션 (메모리 덮어쓰기)
			else if(strTransOption == _T("OVERWRITE"))
			{
				m_iTransMethod = 2;
				UpdateData(FALSE);
				
				m_chkIgnore.EnableWindow(TRUE);
				if(pNode3->GetChild(_T("IGNORE")) != NULL)
				{
					m_chkIgnore.SetCheck(1);
				}
			}
			// Script OverWrite 옵션 (스크립트 덮어쓰기)
			else if(strTransOption == _T("SOW"))
			{
				m_iTransMethod = 3;
				UpdateData(FALSE);
			}


			// 멀티바이트 / 유니코드 지정
			else if(strTransOption == _T("ANSI"))
			{
				m_chkUnicode.SetCheck(0);
			}
			else if(strTransOption == _T("UNICODE"))
			{
				m_chkUnicode.SetCheck(1);
			}

			// 모든 일치하는 텍스트 번역
			else if(strTransOption == _T("ALLSAMETEXT"))
			{
				m_chkAllSameText.SetCheck(1);
			}

			// 번역된 텍스트를 클립보드로
			else if(strTransOption == _T("CLIPKOR"))
			{
				m_chkClipKor.SetCheck(1);
			}

			// 원문 텍스트를 클립보드로
			else if(strTransOption == _T("CLIPJPN"))
			{
				m_chkClipJpn.SetCheck(1);
			}

		}
	}
}

void CPageHook::OnBnClickedRadio()
{	
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		// 기존 번역 방식 삭제
		pTransCmdNode->DeleteChild(_T("NOP"));
		pTransCmdNode->DeleteChild(_T("PTRCHEAT"));
		pTransCmdNode->DeleteChild(_T("OVERWRITE"));
		pTransCmdNode->DeleteChild(_T("SOW"));

		// 새 번역 방식 세팅
		UpdateData(TRUE);
		LPCTSTR TRANS_METHOS[] = {
			_T("NOP"),
			_T("PTRCHEAT"),
			_T("OVERWRITE"),
			_T("SOW")
		};
		
		COptionNode* pNode = pTransCmdNode->CreateChild();
		pNode->SetValue(TRANS_METHOS[m_iTransMethod]);
		
		if(2 == m_iTransMethod)
		{
			m_chkIgnore.EnableWindow(TRUE);
			OnBnClickedChkIgnore();
		}
		else
		{
			m_chkIgnore.EnableWindow(FALSE);
		}
	}

}

void CPageHook::OnBnClickedChkIgnore()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		COptionNode* pTransMethod = pTransCmdNode->GetChild(_T("OVERWRITE"));
		if(pTransMethod)
		{
			SetChildNodeFromCheckbox(pTransMethod, _T("IGNORE"), m_chkIgnore);
		}
	}
}


void CPageHook::OnBnClickedChkUnicode()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("UNICODE"), m_chkUnicode);
	}
}

void CPageHook::OnBnClickedChkAllsametext()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("ALLSAMETEXT"), m_chkAllSameText);
	}
}


void CPageHook::OnBnClickedChkClipJpn()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("CLIPJPN"), m_chkClipJpn);
	}
}

void CPageHook::OnBnClickedChkClipKor()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	COptionNode* pTransCmdNode = (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx);

	if(pTransCmdNode)
	{
		SetChildNodeFromCheckbox(pTransCmdNode, _T("CLIPKOR"), m_chkClipKor);
	}
}

void CPageHook::SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox)
{
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

void CPageHook::OnBnClickedBtnArgAdd()
{
	CMemoryDlg memdlg;

	if( memdlg.DoModal() == IDOK)
	{
		try
		{
			if( NULL == m_pHookNode ) throw -1;
			
			CString strMemName = _T("");
			CString strMem = _T("");

			if(memdlg.m_bCustom)
			{
				strMem = memdlg.m_strCustomMem;
			}
			else
			{
				strMem = memdlg.m_strSelectedArg;
			}
			strMem.Remove(_T(' '));


			// 인자 유효성 검사
			if(strMem.IsEmpty())
			{
				this->MessageBox(_T("번역할 메모리를 선택해 주십시오."), _T("경고"));
				throw -4;
			}
			// 별칭 검사
			strMemName = memdlg.m_strMemName.Trim();
			if(strMemName.IsEmpty())
			{
				this->MessageBox(_T("메모리 별칭을 지정해주세요."), _T("경고"));
				throw -5;
			}

			// TRANS 노드 생성
			COptionNode* pNode = m_pHookNode->CreateChild();
			if(NULL == pNode) throw -2;
			pNode->SetValue(_T("TRANS"));

			// 메모리 지정
			COptionNode* pMemNode = pNode->CreateChild();
			if(NULL == pMemNode) throw -3;
			pMemNode->SetValue(strMem);

			// 별칭 지정
			COptionNode* pNameNode = pNode->CreateChild();
			if(NULL == pNameNode) throw -3;
			pNameNode->SetValue(strMemName);

			// UI 갱신
			if( InitFromHookNode(m_pHookNode) )
			{
				int nComboIdx = m_comboTransArgs.FindString(-1, strMem);
				if(nComboIdx != CB_ERR)
				{
					m_comboTransArgs.SetCurSel(nComboIdx);
					OnCbnSelchangeComboTransArgs();
				}

				COptionDlg::_Inst->m_btnApply.EnableWindow(TRUE);				
			}
			else
			{
				m_pHookNode->DeleteChild(pNode);
				InitFromHookNode(m_pHookNode);
			}

		}
		catch (int nErrCode)
		{
			nErrCode = nErrCode;
		}
	}
}

void CPageHook::OnBnClickedBtnArgDel()
{
	int nSelIdx = m_comboTransArgs.GetCurSel();
	
	if(nSelIdx >= 0)
	{
		m_pHookNode->DeleteChild( (COptionNode*)m_comboTransArgs.GetItemData(nSelIdx) );
		InitFromHookNode(m_pHookNode);
	}

}


void CPageHook::OnBnClickedBtnDelHook()
{
	
	if( COptionDlg::_Inst && ::IsWindow(COptionDlg::_Inst->m_btnApply.m_hWnd))
	{
		CString msg = m_pHookNode->GetChild(0)->GetValue() + _T("를 삭제하시겠습니까?");
		if(MessageBox(msg, _T("Delete"), MB_YESNO) == IDYES)
		{
			COptionDlg::_Inst->PostMessage(WM_DELETE_HOOK, (WPARAM)m_pHookNode, 0);
		}
		
	}
}


