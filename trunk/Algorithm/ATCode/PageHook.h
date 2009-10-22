#pragma once
#include "afxwin.h"

class COptionNode;

// CPageHook dialog

class CPageHook : public CDialog
{
	DECLARE_DYNAMIC(CPageHook)
private:
	COptionNode*	m_pHookNode;

public:
	CPageHook(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPageHook();
	BOOL InitFromHookNode(COptionNode* pHookNode);
	void ClearCtrlValues();
	void SetChildNodeFromCheckbox(COptionNode* pParentNode, LPCTSTR cszChildName, CButton& checkbox);

// Dialog Data
	enum { IDD = IDD_PAGE_HOOK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comboTransArgs;
	CButton m_chkUnicode;
	CButton m_chkAllSameText;
	CButton m_chkClipJpn;
	CButton m_chkClipKor;
	CButton m_btnDelArg;

public:

	int m_iTransMethod;
	CButton m_chkIgnore;

	afx_msg void OnCbnSelchangeComboTransArgs();
	afx_msg void OnBnClickedChkUnicode();
	afx_msg void OnBnClickedChkAllsametext();
	afx_msg void OnBnClickedBtnArgAdd();
	afx_msg void OnBnClickedBtnArgDel();
	afx_msg void OnBnClickedBtnDelHook();
	afx_msg void OnBnClickedChkClipJpn();
	afx_msg void OnBnClickedChkClipKor();
	afx_msg void OnBnClickedRadio();
	afx_msg void OnBnClickedChkIgnore();

};
