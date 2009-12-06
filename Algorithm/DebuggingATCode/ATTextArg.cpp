
#include "stdafx.h"
#include "ATTextArg.h"
#include "ATTextArgMgr.h"

#pragma warning(disable:4996)


CATText::CATText(void)
{
	ResetATText();
}

CATText::~CATText(void)
{
}

void CATText::ResetATText()
{
	m_bWideChar = FALSE;
	m_bTranslated = FALSE;
	m_bTranslatedTestMode = FALSE;
	m_nNextTestIdx = 0;						// 문자열의 비교할 위치
	m_nHitCnt = 0;							// 히트 카운트
	m_nFaultCnt = 0;						// 실패 카운트
	m_setSourcePtr.clear();
	m_setFuncArg.clear();

	m_nJapaneseLen = 0;						// 일본어 텍스트 길이
	m_nKoreanLen = 0;						// 한국어 텍스트 길이

	ZeroMemory(m_szJapaneseText, MAX_TEXT_LENGTH);					// 일본어 텍스트 버퍼 (멀티바이트)
	ZeroMemory(m_wszJapaneseText, MAX_TEXT_LENGTH*sizeof(wchar_t));	// 일본어 텍스트 버퍼 (유니코드)
	ZeroMemory(m_szKoreanText, MAX_TEXT_LENGTH);					// 한국어 텍스트 버퍼 (멀티바이트)
	ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH*sizeof(wchar_t));	// 한국어 텍스트 버퍼 (유니코드)
}


//////////////////////////////////////////////////////////////////////////
//
// 유효한 일본어 텍스트인가 평가 (멀티바이트용)
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::IsJapaneseTextA(LPCSTR szJpnText)
{
	BOOL bRetVal = FALSE;

	if( IsBadStringPtrA(szJpnText, 1024*1024*1024) == FALSE )
	{
		// 길이 유효성 검사
		int nLen = (int)strlen(szJpnText);
		if(1 < nLen && nLen < MAX_TEXT_LENGTH)
		{
			// 문자코드 유효성 검사
			int score = 0;
			for(int i=0; i<nLen; i++)
			{
				if((BYTE)szJpnText[i]>=0x80)
				{
					if(szJpnText[i+1]==0)
					{
						break;
					}
					else if( 0x81 <= (BYTE)szJpnText[i] && (BYTE)szJpnText[i] <= 0x9F )
					{
						score+=2;
					}

					i++;
				}
			}

			// 반이상 일본어 코드면 일본어로 인정
			if( score > nLen/2 ) bRetVal = TRUE;
		}
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// 유효한 일본어 텍스트인가 평가 (유니코드용)
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::IsJapaneseTextW(LPCWSTR wszJpnText)
{
	BOOL bRetVal = FALSE;

	if( IsBadStringPtrW(wszJpnText, 1024*1024*1024) == FALSE )
	{
		size_t nLen = wcslen(wszJpnText);
		if(1 < nLen && nLen < MAX_TEXT_LENGTH)
		{
			bRetVal = TRUE;
		}
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// 텍스트 세팅
//
//////////////////////////////////////////////////////////////////////////
BOOL CATText::SetATText(LPVOID pSource, BOOL bWideChar)
{
	BOOL bRetVal = FALSE;

	ResetATText();

	if( pSource )
	{	
		BOOL bIsJpnText = FALSE;
		int  nSrcLen = 0;
		
		// 유효성 검사 : 유니코드 문자열이 넘어온 경우
		if( bWideChar )
		{
			bIsJpnText = IsJapaneseTextW((LPCWSTR)pSource);

			if( bIsJpnText )
			{
				m_nJapaneseLen = wcslen((LPCWSTR)pSource);
				wcscpy(m_wszJapaneseText, (LPCWSTR)pSource);
				nSrcLen = MyWideCharToMultiByte(932, 0, m_wszJapaneseText, -1, m_szJapaneseText, MAX_TEXT_LENGTH*2-1, NULL, NULL);
			}
		}
		// 유효성 검사 : 멀티바이트 문자열이 넘어온 경우
		else
		{
			bIsJpnText = IsJapaneseTextA((LPCSTR)pSource);
			
			if(bIsJpnText)
			{
				m_nJapaneseLen = strlen((LPCSTR)pSource);
				nSrcLen = m_nJapaneseLen + 1;
				strcpy(m_szJapaneseText, (LPCSTR)pSource);
				MyMultiByteToWideChar(932, 0, m_szJapaneseText, -1, m_wszJapaneseText, MAX_TEXT_LENGTH-1 );
			}
		}


		// 일본어 텍스트면 번역 처리
		if( bIsJpnText )
		{			
			// 번역기에 집어넣어라
			BOOL bTrans = CATTextArgMgr::GetInstance()->m_sATCTNR3.procTranslateUsingCtx(L"DebuggingATCode", m_szJapaneseText, nSrcLen, m_szKoreanText, MAX_TEXT_LENGTH*2);
			if( bTrans )
			{
				MyMultiByteToWideChar(949, 0, m_szKoreanText, -1, m_wszKoreanText, MAX_TEXT_LENGTH-1 );
												
				if(bWideChar) m_nKoreanLen = wcslen(m_wszKoreanText);
				else m_nKoreanLen = strlen(m_szKoreanText);
			}

			m_setSourcePtr.insert(pSource);
			m_bWideChar = bWideChar;
			
			bRetVal = TRUE;

		}
	}

	if( FALSE == bRetVal) ResetATText();

	return bRetVal;
}


int CATText::TestText(LPVOID pSource)
{
	if(NULL==pSource) return 0;

	int iRetVal = 0;
	
	if(m_bWideChar)
	{
		LPWSTR wszSource = (LPWSTR)pSource;
		
		// 일본어 텍스트가 일치한다면
		if( wcscmp(wszSource, m_wszJapaneseText) == 0 )
		{
			iRetVal = 1;
		}
	}
	else
	{
		LPSTR szSource = (LPSTR)pSource;

		// 일본어 텍스트가 일치한다면
		if( strcmp(szSource, m_szJapaneseText) == 0 )
		{
			iRetVal = 1;
		}
	}

	if(iRetVal)
	{
		m_setSourcePtr.insert(pSource);
	}

	return iRetVal;
}


int CATText::TestCharacter(wchar_t wch)
{
	int nRetVal = 0;

	if( L'\0' != wch )
	{
		
		size_t tmpIdx = 0;
		
		// 일본어 검사
		if( m_bTranslatedTestMode == FALSE )
		{
			if( 0 < m_nNextTestIdx && m_wszJapaneseText[m_nNextTestIdx-1] == wch )
			{
				nRetVal = 3;
			}
			else
			{
				tmpIdx = m_nNextTestIdx;
				while(m_wszJapaneseText[tmpIdx])
				{
					if( m_wszJapaneseText[tmpIdx] == wch )
					{
						m_nNextTestIdx = (int)tmpIdx+1;
						m_nHitCnt++;
						m_nFaultCnt = 0;
						nRetVal = 1;
						break;
					}
					tmpIdx++;
				}
			}
		}

		// 한국어 검사
		if( 0==nRetVal)
		{
			if( 0 < m_nNextTestIdx && m_wszKoreanText[m_nNextTestIdx-1] == wch )
			{
				nRetVal = 3;
			}
			else
			{
				tmpIdx = m_nNextTestIdx;
				while(m_wszKoreanText[tmpIdx])
				{
					if( m_wszKoreanText[tmpIdx] == wch )
					{
						m_nNextTestIdx = (int)tmpIdx+1;
						m_nHitCnt++;
						m_nFaultCnt = 0;
						m_bTranslatedTestMode = TRUE;
						nRetVal = 1;
						break;
					}
					tmpIdx++;
				}

			}

		}


		//if(0==nRetVal && m_nFaultCnt<50)
		//{
		//	nRetVal = 4;		// 유예
		//	m_nHitCnt = 0;
		//	m_nFaultCnt++;
		//}

		if(0==nRetVal)
		{
			m_nHitCnt = 0;
			m_nFaultCnt++;
		}

		// 디버깅용
		//if(0==nRetVal && m_nNextTestIdx>2)
		//{
		//	wchar_t dbg[1024];
		//	swprintf(dbg, L"[ aral1 ] 탈락 : %d 번째에서 '%c'찾기시도, \"%s\"->\"%s\"  \n", m_nNextTestIdx, wch, m_wszJapaneseText, m_wszKoreanText);
		//	OutputDebugStringW(dbg);

		//}


	}

	return nRetVal;
}

int CATText::GetHitCount()
{
	return m_nHitCnt;
}

BOOL CATText::IsWideCharacter() 
{
	return m_bWideChar;
}
