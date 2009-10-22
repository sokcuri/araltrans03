#pragma once

#include <vector>
using namespace std;

class COptionNode
{
private:
	CString m_strValue;
	COptionNode* m_pParent;
	vector<COptionNode*> m_vecChildren;

public:
	COptionNode();
	~COptionNode();	

	BOOL ParseChildren(LPCTSTR cszOptionString);
	void ClearChildren();

	CString	GetValue();
	void	SetValue(LPCTSTR cszValue);

	COptionNode* CreateChild();
	int	GetChildCount();

	void DeleteChild(int idx);
	void DeleteChild(LPCTSTR cszValue);
	void DeleteChild(COptionNode* pNode);

	COptionNode* GetChild(int idx);
	COptionNode* GetChild(LPCTSTR cszValue);

	CString ChildrenToString();
	CString ToString();
};
