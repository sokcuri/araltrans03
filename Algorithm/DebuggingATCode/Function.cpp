#include "stdafx.h"
#include "Function.h"

CFunction::CFunction(UINT_PTR ptrFunction)
{
	m_ptrFunction = ptrFunction;	// 함수포인터
	m_strLastJapaneseText = _T("");		// 히트시 마지막 일본어 텍스트
	m_strLastKoreanText = _T("");		// 히트시 마지막 일본어 텍스트
}

CFunction::~CFunction(void)
{
}
