 #include "stdafx.h"
#include "OptionMgr.h"

COptionNode::COptionNode()
: m_pParent(NULL)
{
	m_strValue = _T("");
}

COptionNode::~COptionNode()
{
	ClearChildren();

	if(m_pParent)
	{
		for(vector<COptionNode*>::iterator iter = m_pParent->m_vecChildren.begin();
			iter != m_pParent->m_vecChildren.end();
			iter++)
		{
			if(this == (*iter))
			{
				m_pParent->m_vecChildren.erase(iter);
				break;
			}
		}
	}
}


BOOL COptionNode::ParseChildren(LPCTSTR cszOptionString)
{
	ClearChildren();

	if(cszOptionString == NULL) return FALSE;

	CString strOptionString = cszOptionString;
	CString strNodeName = _T("");
	CString strChildrenString = _T("");
	int len = strOptionString.GetLength();
	
	for (int i=0; i<len; i++)
	{
		TCHAR ch = strOptionString[i];
		if(ch==_T('('))
		{
			// 매칭되는 괄호의 인덱스 구하기
			i++;
			int nStartIdx = i;
			int nDepth = 1;
			while(i<len)
			{
				if(strOptionString[i] == _T('(')) nDepth++;
				if(strOptionString[i] == _T(')')) nDepth--;
				
				if(nDepth == 0) break;

				i++;
			}

			if(nDepth != 0)
			{
				ClearChildren();
				return FALSE;
			}

			// 차일드 옵션 문자열 추출
			strChildrenString = strOptionString.Mid(nStartIdx, i-nStartIdx);


		}
		else
		{
			strNodeName += ch;
		}

		if(ch==_T(',') || i+1 == len)
		{
			strNodeName.Replace(_T(" "), _T(""));
			strNodeName.Replace(_T(","), _T(""));

			// 차일드 생성
			COptionNode* pNode = CreateChild();
			if(pNode==NULL)
			{
				ClearChildren();
				return FALSE;
			}
			pNode->SetValue(strNodeName);
			if( pNode->ParseChildren(strChildrenString) == FALSE )
			{
				ClearChildren();
				return FALSE;
			}

			strNodeName = _T("");
			strChildrenString = _T("");
		}
	}

	return TRUE;
}

void COptionNode::ClearChildren()
{
	while(m_vecChildren.size()) delete m_vecChildren[0];
}


CString COptionNode::ToString()
{
	CString strChildrenString = ChildrenToString();

	if(strChildrenString.IsEmpty() == FALSE)
		strChildrenString = _T("(") + strChildrenString + _T(")");

	return m_strValue + strChildrenString;
}

CString COptionNode::ChildrenToString()
{
	CString strChildrenString = _T("");

	for(vector<COptionNode*>::iterator iter = m_vecChildren.begin();
		iter != m_vecChildren.end();
		iter++)
	{
		if(strChildrenString.IsEmpty() == FALSE) strChildrenString += _T(',');
		strChildrenString += (*iter)->ToString();
	}

	return strChildrenString;
}

CString	COptionNode::GetValue()
{
	return m_strValue;
}

void COptionNode::SetValue(LPCTSTR strValue)
{
	m_strValue = _T("");
	
	if(strValue)
	{
		m_strValue = strValue;
		m_strValue = m_strValue.Trim();
	}
}

COptionNode* COptionNode::CreateChild()
{
	COptionNode* pNode = new COptionNode();
	
	pNode->m_pParent = this;
	m_vecChildren.push_back(pNode);

	return pNode;
}

int	COptionNode::GetChildCount()
{
	return (int)m_vecChildren.size();
}

void COptionNode::DeleteChild(int idx)
{
	COptionNode* pNode = GetChild(idx);

	if(pNode)
	{
		delete pNode;
	}
}

void COptionNode::DeleteChild(LPCTSTR strValue)
{
	COptionNode* pNode = GetChild(strValue);

	if(pNode)
	{
		delete pNode;
	}
}

void COptionNode::DeleteChild(COptionNode* pNode)
{
	delete pNode;
}

COptionNode* COptionNode::GetChild(int idx)
{
	COptionNode* pNode = NULL;
	
	if( (size_t)idx < m_vecChildren.size() )
	{
		pNode = m_vecChildren[idx];
	}
	return pNode;
}

COptionNode* COptionNode::GetChild(LPCTSTR strValue)
{
	COptionNode* pNode = NULL;

	for(vector<COptionNode*>::iterator iter = m_vecChildren.begin();
		iter != m_vecChildren.end();
		iter++)
	{
		if( (*iter)->GetValue().CompareNoCase(strValue) == 0 )
		{
			pNode = (*iter);
			break;
		}
	}

	return pNode;
}
