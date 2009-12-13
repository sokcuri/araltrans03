
#pragma warning(disable:4312)
#pragma warning(disable:4996)

#include "../stdafx.h"
#include "NonCachedTextArgMgr.h"
#include "NonCachedTextArg.h"
#include "Misc.h"

#define TEXT_ARG_POOL_SIZE 100

CNonCachedTextArgMgr* CNonCachedTextArgMgr::_inst = NULL;


int MyWideCharToMultiByte(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCWSTR lpWideCharStr, 
	int cchWideChar, 
	LPSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPCSTR lpDefaultChar, 
	LPBOOL lpUsedDefaultChar 
);

int MyMultiByteToWideChar(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPWSTR lpWideCharStr, 
	int cchWideChar 
);

CNonCachedTextArgMgr::CNonCachedTextArgMgr(void)
: m_bMatchLen(FALSE)
{
	_inst = this;
}

CNonCachedTextArgMgr::~CNonCachedTextArgMgr(void)
{
	Close();
	_inst = NULL;
}

// 초기화
BOOL CNonCachedTextArgMgr::Init() 
{
	// 옵션 체크
	//if (!lstrcmpiA(m_pszOptionStringBuffer, "MATCHLEN"))
	//	m_bMatchLen=TRUE;
	//else
	m_bMatchLen=FALSE;

	// 중복 카운트 테이블 초기화
	ZeroMemory(m_aDupCntTable, sizeof(m_aDupCntTable));

	// 텍스트 인자 풀 생성
	for(int i=0; i<TEXT_ARG_POOL_SIZE; i++)
	{
		m_setInactivatedArgs.insert(new CNonCachedTextArg());
	}

	return TRUE;
}

// 말기화
void CNonCachedTextArgMgr::Close()
{
	// 활성화 텍스트 인스턴스 모두 삭제
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);
		delete pNonCachedTextArg;
	}
	m_setActivatedArgs.clear();
	
	// 비활성화 텍스트 인스턴스 모두 삭제
	for(CNonCachedTextArgSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);
		delete pNonCachedTextArg;
	}
	m_setInactivatedArgs.clear();

	// 중복 카운트 테이블 초기화
	ZeroMemory(m_aDupCntTable, sizeof(m_aDupCntTable));

}



// 새로운 문자열 후보를 추가한다
BOOL CNonCachedTextArgMgr::AddTextArg(LPCWSTR wszText)
{
	BOOL bRetVal = FALSE;

	CNonCachedTextArg* pNonCachedTextArg = NULL;
	if(!m_setInactivatedArgs.empty()) 
	{
		CNonCachedTextArgSet::iterator iter = m_setInactivatedArgs.begin();
		pNonCachedTextArg = ( *iter );
		m_setInactivatedArgs.erase(pNonCachedTextArg);
		m_setActivatedArgs.insert(pNonCachedTextArg);
	}
	else
	{
		// 활성화 텍스트 인스턴스 모두 순환
		for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
			iter != m_setActivatedArgs.end();
			iter++)
		{
			CNonCachedTextArg* pTmpNonCachedTextArg = *(iter);

			if( NULL == pNonCachedTextArg || (pNonCachedTextArg->m_nFaultCnt < pTmpNonCachedTextArg->m_nFaultCnt) )
			{
				pNonCachedTextArg = pTmpNonCachedTextArg;
			}		
		}		
	}
		

	if( pNonCachedTextArg->SetNonCachedTextArg(wszText) )
	{
		bRetVal = TRUE;
	}
	else
	{
		m_setActivatedArgs.erase(pNonCachedTextArg);
		m_setInactivatedArgs.insert(pNonCachedTextArg);
	}

	return bRetVal;
}

// 문자열 후보들 전체를 테스트한다. (더이상 일치하지 않는 후보는 바로 삭제)
BOOL CNonCachedTextArgMgr::TestCharacter(wchar_t wch)
{
	BOOL bRetVal = FALSE;

	// 활성화 텍스트 인스턴스를 모두 순회
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CNonCachedTextArg* pNonCachedTextArg = (*iter);
		iter++;

		// 예상되는 중복 횟수 구하기
		UINT nPreperedDupCnt = 0;
		for(UINT i=1; i<16; i++)
		{
			if( m_aDupCntTable[nPreperedDupCnt] < m_aDupCntTable[i] )
			{
				nPreperedDupCnt = i;
			}
		}
		
		// 검사 수행
		int nRes = pNonCachedTextArg->TestCharacter(wch, nPreperedDupCnt);
		
		// 유예시
		if( 0 != nRes )
		{
			if( nRes & 0x01 ) bRetVal = TRUE;
			if( (nRes & 0x03) != 0 )
			{
				int idx = pNonCachedTextArg->m_nDupCnt;
				if(idx<16) m_aDupCntTable[idx]++;
			}
		}
		// 폐기시
		else
		{
			m_setActivatedArgs.erase(pNonCachedTextArg);
			m_setInactivatedArgs.insert(pNonCachedTextArg);
		}
	}	

	return bRetVal;
}

// 최고로 확률이 높은 번역 문자를 반환
BOOL CNonCachedTextArgMgr::GetBestTranslatedCharacter(wchar_t* pTransResultBuf)
{
	//wchar_t wchRetVal = L'\0';
	CNonCachedTextArg* pBestArg = NULL;
	
	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	// 활성화 텍스트 인스턴스 모두 순환
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CNonCachedTextArg* pNonCachedTextArg = *(iter);

		if( NULL == pBestArg )
		{
			if( 0 < pNonCachedTextArg->m_nHitCnt ) pBestArg = pNonCachedTextArg;
		}		
		else if( pBestArg->m_nHitCnt < pNonCachedTextArg->m_nHitCnt )
		{
			pBestArg = pNonCachedTextArg;
		}
		else if(pBestArg->m_nHitCnt == pNonCachedTextArg->m_nHitCnt)
		{
			size_t nBestRemain = pBestArg->m_nJapaneseLen - pBestArg->m_nNextTestIdx;
			size_t nTempRemain = pNonCachedTextArg->m_nJapaneseLen - pNonCachedTextArg->m_nNextTestIdx;
			if( nBestRemain < nTempRemain )
			{
				pBestArg = pNonCachedTextArg;				
			}
			else if( nBestRemain == nTempRemain && pBestArg->m_nFaultCnt > pNonCachedTextArg->m_nFaultCnt )
			{
				pBestArg = pNonCachedTextArg;
			}
		}
	}

	if(pBestArg)
	{
		// 마지막 문자인 경우 다 반환
		if(pBestArg->m_nJapaneseLen == pBestArg->m_nNextTestIdx-1 && pBestArg->m_nKoreanLen)
		{
			wcscpy( pTransResultBuf, &pBestArg->m_wszKoreanText[pBestArg->m_nNextKorIdx] );
		}
		// 한문자
		else
		{
			pTransResultBuf[0] = pBestArg->GetTranslatedCharacter();
			pTransResultBuf[1] = L'\0';

		}

		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}

CNonCachedTextArg* CNonCachedTextArgMgr::FindString(LPCWSTR pTestString, int nSize)
{
	CNonCachedTextArg* pMatchArg=NULL;
	// 활성화 텍스트 인스턴스를 모두 순회
	for(CNonCachedTextArgSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CNonCachedTextArg* pNonCachedTextArg = (*iter);
		iter++;

		// 검사 수행
		int nRes = pNonCachedTextArg->TestString(pTestString, nSize);

		// 폐기시
		if (nRes == 0)
		{
			m_setActivatedArgs.erase(pNonCachedTextArg);
			m_setInactivatedArgs.insert(pNonCachedTextArg);
		}
		// 찾거나 중복시
		else if (nRes != 4)
		{
			if ( !pMatchArg || ( nSize == 1 && pMatchArg->m_nJapaneseLen == 1 && pNonCachedTextArg->m_nJapaneseLen > 1) )
				pMatchArg = pNonCachedTextArg;
		}

	}	

	return pMatchArg;
}
BOOL CNonCachedTextArgMgr::GetTranslatedStringA(INT_PTR ptrBegin, LPCSTR szOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize)
{
	CNonCachedTextArg* pBestArg = NULL;

	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	wchar_t wszOrigString[1024] = { L'\0', };
	int nLen=MyMultiByteToWideChar(932, 0, szOrigString, nOrigSize, wszOrigString, 1023 );

	pBestArg = FindString(wszOrigString, nLen);

	if (!pBestArg)
	{
		int iRes = SearchStringA(ptrBegin, szOrigString[0], szOrigString[1]);
		pBestArg = FindString(wszOrigString, nLen);
	}


	if(pBestArg)
	{
		nTransSize= nTransSize / 2;

		pBestArg->GetTranslatedString(pTransResultBuf, nBufSize, nTransSize);

		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::GetTranslatedStringW(INT_PTR ptrBegin, LPCWSTR wszOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize)
{
	CNonCachedTextArg* pBestArg = NULL;

	if( NULL == pTransResultBuf ) return FALSE;

	BOOL bRetVal = FALSE;

	pBestArg = FindString(wszOrigString, nOrigSize);

	if (!pBestArg)
	{
		int iRes = SearchStringW(ptrBegin, wszOrigString[0]);
		pBestArg = FindString(wszOrigString, nOrigSize);
	}


	if(pBestArg)
	{
		pBestArg->GetTranslatedString(pTransResultBuf, nBufSize, nTransSize);
		
		if(pTransResultBuf[0]) bRetVal = TRUE;
	}

	return bRetVal;
}




// 현재 활성화 된 텍스트 인자들이 하나도 없는가?
BOOL CNonCachedTextArgMgr::IsEmpty()
{
	return m_setActivatedArgs.empty();
}



int CNonCachedTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;

	size_t dist = 0;


	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);
		if( IsBadStringPtrA(*ppText, 1024)==FALSE && (*ppText)[0]==ch1
			&&  ('\0'==ch2 || (*ppText)[1]==ch2) && strlen(*ppText) < 1024 )
		{
			wchar_t wszTmp[1024];
			MyMultiByteToWideChar(932, 0, *ppText, -1, wszTmp, 1023 );
			AddTextArg(wszTmp);
			iRetVal++;
		}
		dist += sizeof(void*);

	}

	return iRetVal;
}

int CNonCachedTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	size_t dist = 0;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);
		if( IsBadStringPtrW(*ppText, 1024)==FALSE && **ppText == wch )
		{
			AddTextArg(*ppText);
			iRetVal++;
		}
		dist += sizeof(void*);
	}	

	return iRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// GetGlyphOutlineA
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	char chArray[5] = {0,};
	wchar_t wchArray[10] = {0,};
	size_t i,j;
	j = 0;
	for(i=sizeof(UINT); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}

	MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );

	wchar_t wch = wchArray[0];

	// 검사중인 텍스트 포인터들 모두 순회
	BOOL bHitOnce = TestCharacter(wch);

	// 적중된 포인터가 없다면 검색
	if( FALSE == bHitOnce )
	{
		int iRes = SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
		//iRes += SearchStringW(_CUR_EBP, wch);
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wchNewChar[1024];

	DWORD dwRetVal = 0;

	if( GetBestTranslatedCharacter(wchNewChar) )
	{
		uChar = wchNewChar[0];

		if(uChar<=0x20)
		{
			uFormat = GGO_NATIVE;
		}

		dwRetVal = GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

		if(dwRetVal == GDI_ERROR)
		{
			TRACE("[ aral1 ] GetGlyphOutlineA(0x%x) failed (ErrCode:%d) \n", uChar, GetLastError());
		}

	}
	else	// 원래함수 호출
	{
		dwRetVal = GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;	
}


//////////////////////////////////////////////////////////////////////////
//
// GetGlyphOutlineW
//
//////////////////////////////////////////////////////////////////////////
DWORD CNonCachedTextArgMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	/*
	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}


	wchar_t wch = (wchar_t)uChar;

	// 검사중인 텍스트 포인터들 모두 순회
	BOOL bHitOnce = TestCharacter(wch);

	// 적중된 포인터가 없다면 검색
	if( FALSE == bHitOnce )
	{
		int iRes = SearchStringW(_CUR_EBP, wch);
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wchNewChar[1024];

	if( GetBestTranslatedCharacter(wchNewChar) )
	{
		uChar = wchNewChar[0];
	}

	wchar_t dbg[MAX_PATH];
	swprintf(dbg, L"[ aral1 ] MyGetGlyphOutlineW : '%c'->'%s' \n", wch, wchNewChar);
	OutputDebugStringW(dbg);

	// 원래함수 호출
	DWORD dwRetVal = 0;
	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		dwRetVal = m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	}

	return dwRetVal;
	*/
	
	return GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

BOOL CNonCachedTextArgMgr::NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wszNewString[1024];
	int nLen=cbString;

	if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
	{
		bRetVal = TextOutW(hdc, nXStart, nYStart, wszNewString, nLen);
	}
	else	// 원래함수 호출
	{
		bRetVal = TextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	/*
	BOOL bRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen = cbString;

	if (IsJapaneseW(lpString, nLen))
	{

		// 최고의 번역 문자를 얻어옴
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{

			bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wszNewString, nLen);
			return bRetVal;
		}
	}
	// 원래함수 호출
	if( m_sTextFunc.pfnTextOutW )
	{
		bRetVal = m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	}

	return bRetVal;
	*/

	return TextOutW(hdc, nXStart, nYStart, lpString, cbString);
}

BOOL CNonCachedTextArgMgr::NewExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString,  // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	BOOL bRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wszNewString[1024];
	int nLen = (int)cbCount;

	if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
	{
		bRetVal = ExtTextOutW(hdc, X, Y, fuOptions, lprc, wszNewString, nLen, lpDx);
	}
	else	// 원래함수 호출
	{
		bRetVal = ExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
}

BOOL CNonCachedTextArgMgr::NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	/*
	BOOL bRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen = (int)cbCount;

	if (IsJapaneseW(lpString, nLen))
	{

		// 최고의 번역 문자를 얻어옴
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, wszNewString, nLen, lpDx);
			return bRetVal;
		}
	}
	// 원래함수 호출
	if( m_sTextFunc.pfnExtTextOutW )
	{
		bRetVal = m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
	}

	return bRetVal;
	*/

	return ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}

int CNonCachedTextArgMgr::NewDrawTextA(
				   HDC hDC,          // handle to DC
				   LPCSTR lpString,  // text to draw
				   int nCount,       // text length
				   LPRECT lpRect,    // formatting dimensions
				   UINT uFormat      // text-drawing options
				   )
{
	int nRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wszNewString[1024];
	int nLen=nCount;

	if( GetTranslatedStringA(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
	{
		nRetVal = DrawTextW(hDC, wszNewString, nLen, lpRect, uFormat);
		return nRetVal;
	}
	else	// 원래함수 호출
	{
		nRetVal = DrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return nRetVal;
}

int CNonCachedTextArgMgr::NewDrawTextW(
				   HDC hDC,          // handle to DC
				   LPCWSTR lpString, // text to draw
				   int nCount,       // text length
				   LPRECT lpRect,    // formatting dimensions
				   UINT uFormat      // text-drawing options
				   )
{
	/*
	int nRetVal = 0;

	CheckFont(hDC);

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}
	int nLen=nCount;

	if (IsJapaneseW(lpString, nCount))
	{

		// 최고의 번역 문자를 얻어옴
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpString, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextW(hDC, wszNewString, nLen, lpRect, uFormat);
			return nRetVal;
		}
	}
	// 원래함수 호출
	if( m_sTextFunc.pfnDrawTextW )
	{
		nRetVal = m_sTextFunc.pfnDrawTextW(hDC, lpString, nLen, lpRect, uFormat);
	}

	return nRetVal;
	*/

	return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);
}

int CNonCachedTextArgMgr::NewDrawTextExA(
					 HDC hdc,                     // handle to DC
					 LPSTR lpchText,              // text to draw
					 int cchText,                 // length of text to draw
					 LPRECT lprc,                 // rectangle coordinates
					 UINT dwDTFormat,             // formatting options
					 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
					 )
{
	int nRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	// 최고의 번역 문자를 얻어옴
	wchar_t wszNewString[1024];
	int nLen = cchText;

	if( GetTranslatedStringA(_CUR_EBP, lpchText, nLen, wszNewString, 1024, nLen) )
	{
		nRetVal = DrawTextExW(hdc, wszNewString, nLen, lprc, dwDTFormat, lpDTParams);
	}
	else	// 원래함수 호출
	{
		nRetVal = DrawTextExA(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;
}

int CNonCachedTextArgMgr::NewDrawTextExW(
					 HDC hdc,                     // handle to DC
					 LPWSTR lpchText,             // text to draw
					 int cchText,                 // length of text to draw
					 LPRECT lprc,                 // rectangle coordinates
					 UINT dwDTFormat,             // formatting options
					 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
					 )
{
	/*
	int nRetVal = 0;

	// 바로 앞 전 EBP를 구해서
	INT_PTR _CUR_EBP = NULL;
	_asm
	{
		mov eax, [ebp];
		mov _CUR_EBP, eax;
	}

	int nLen=cchText;

	if (IsJapaneseW(lpchText, nLen))
	{

		// 최고의 번역 문자를 얻어옴
		wchar_t wszNewString[1024];

		if( GetTranslatedStringW(_CUR_EBP, lpchText, nLen, wszNewString, 1024, nLen) )
		{
			nRetVal = m_sTextFunc.pfnDrawTextExW(hdc, wszNewString, nLen, lprc, dwDTFormat, lpDTParams);
			return nRetVal;
		}
	}
	// 원래함수 호출
	if( m_sTextFunc.pfnDrawTextExW )
	{
		nRetVal = m_sTextFunc.pfnDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	}

	return nRetVal;
	*/

	return DrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
