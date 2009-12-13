#pragma once
#include <Windows.h>
#include <map>

//struct FuncPtrLess : public binary_function<const CFunction*, const CFunction*, bool>
//{
//	bool operator()(const CFunction* pFunc1, const CFunction* pFunc2) const
//	{
//		return  ((CFunction*)pFunc1)->GetFuncPtr() < ((CFunction*)pFunc2)->GetFuncPtr();
//	}
//};


using namespace std;

class CFunction
{
	friend class CCachedTextArgMgr;

private:
	UINT_PTR		m_ptrFunction;				// 함수포인터
	map<size_t,int>	m_mapDistScores;			// 호출된 직후의 인자 점수들

public:
	CFunction(UINT_PTR ptrFunction);
	~CFunction(void);

};
