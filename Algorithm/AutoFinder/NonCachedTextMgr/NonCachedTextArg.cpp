
#include "../stdafx.h"
#include "../AutoFinder.h"
#include "NonCachedTextArg.h"
#include "NonCachedTextArgMgr.h"
#include "Misc.h"

#pragma warning(disable:4996)

CNonCachedTextArg::CNonCachedTextArg(void)
{
	ResetNonCachedTextArg();
}

CNonCachedTextArg::~CNonCachedTextArg(void)
{
}


BOOL CNonCachedTextArg::SetNonCachedTextArg(LPCWSTR wszJapaneseText)
{
	BOOL bRetVal = FALSE;

	ResetNonCachedTextArg();

	if( wszJapaneseText && MAINAPP && MAINAPP->m_sContainerFunc.procTranslateUsingCtx )
	{
		size_t nSrcLen = wcslen(wszJapaneseText);

		if( nSrcLen < MAX_TEXT_LENGTH )
		{
			// 일본어 텍스트 복사			
			//wcscpy(m_wszJapaneseText, wszJapaneseText);
			//m_nJapaneseLen = nSrcLen;
			try
			{

				m_nJapaneseLen = 0;
				for(size_t i=0; i<nSrcLen; i++)
				{
					if( !IsControlCharacter(wszJapaneseText[i]) )
					{
						m_wszJapaneseText[m_nJapaneseLen] = wszJapaneseText[i];
						m_nJapaneseLen++;
					}
				}
				m_wszJapaneseText[m_nJapaneseLen] = L'\0';

				if (!IsJapaneseW(m_wszJapaneseText, m_nJapaneseLen))
					throw -1;

				// 번역기에 집어넣어라
				char szJapanese[MAX_TEXT_LENGTH*2];

				int nSrcLen = MyWideCharToMultiByte(932, 0, m_wszJapaneseText, -1, szJapanese, MAX_TEXT_LENGTH*2, NULL, NULL);
				if (nSrcLen == 0)
					throw -2;

				char szKorean[MAX_TEXT_LENGTH*2];
				BOOL bTrans = MAINAPP->m_sContainerFunc.procTranslateUsingCtx(
					L"AutoFinder",
					szJapanese,
					nSrcLen,
					szKorean, 
					MAX_TEXT_LENGTH*2
					);
				
				if (!bTrans)
					throw -3;


				ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH);
				if (!MyMultiByteToWideChar(949, 0, szKorean, -1, m_wszKoreanText, MAX_TEXT_LENGTH-1 ))
					throw -4;

				CString strKorean = m_wszKoreanText;
				strKorean.Replace(L" ", L"");

				wcscpy_s(m_wszKoreanText, MAX_TEXT_LENGTH, (LPCWSTR)strKorean);
				m_nKoreanLen = wcslen(m_wszKoreanText);

				while(m_nKoreanLen<m_nJapaneseLen)
				{
					m_wszKoreanText[m_nKoreanLen] = L' ';
					m_nKoreanLen++;
				}

				m_nNextTestIdx = 0;
				m_nNextKorIdx = 0;
				m_nHitCnt = 1;
				m_nFaultCnt = 0;

				bRetVal = TRUE;

			}
			catch(int e)
			{
				e = e;
				bRetVal = FALSE;
			}

			//wchar_t dbg[1024];
			//swprintf(dbg, L"[ aral1 ] '%s'->'%s' \n", m_wszJapaneseText, m_wszKoreanText);
			//OutputDebugStringW(dbg);

		}
	}

	if( FALSE == bRetVal) ResetNonCachedTextArg();

	return bRetVal;
}

void CNonCachedTextArg::ResetNonCachedTextArg()
{
	m_nNextTestIdx = 0;								// 문자열의 비교할 위치
	m_nNextKorIdx = 0;
	m_nHitCnt = 0;
	m_nDupCnt = 0;
	m_nFaultCnt = 0;
	m_nJapaneseLen = 0;								// 일본어 텍스트 길이
	m_nKoreanLen = 0;								// 한국어 텍스트 길이
	ZeroMemory(m_wszJapaneseText, MAX_TEXT_LENGTH);	// 일본어 텍스트 버퍼
	ZeroMemory(m_wszKoreanText, MAX_TEXT_LENGTH);	// 한국어 텍스트 버퍼
}

inline BOOL CNonCachedTextArg::IsControlCharacter(wchar_t wch) const
{
	return ( ( wch==0x0A || wch==0x0D ) ? TRUE : FALSE );
}

int CNonCachedTextArg::TestCharacter(wchar_t wch, UINT nPreperedDupCnt)
{
	int nRetVal = 0;

	if (m_nNextTestIdx == 0)
		m_nNextTestIdx = 1;

	if( L'\0' != wch )
	{
		// 중복 출력의 가능성이 농후하면
		if( nPreperedDupCnt && (UINT)m_nDupCnt<nPreperedDupCnt && m_wszJapaneseText[m_nNextTestIdx-1] == wch )
		{
			m_nDupCnt++;
			nRetVal = 3;			
		}
		// 일반 적중시
		else if( m_wszJapaneseText[m_nNextTestIdx] == wch )
		{
			m_nNextKorIdx++;
			m_nNextTestIdx++;
			m_nHitCnt++;
			m_nDupCnt = 0;
			m_nFaultCnt = 0;
			nRetVal = 1;
		}
		// 중복출력의 경우도 있으므로
		else if( m_wszJapaneseText[m_nNextTestIdx-1] == wch )
		{
			//m_nHitCnt = 0;
			//m_nFaultCnt++;
			m_nDupCnt++;
			nRetVal = 2;
		}
		// 건너뛰어 출력할 수 있으므로
		else if( m_wszJapaneseText[m_nNextTestIdx] != L'\0' )
		{
			size_t tmpIdx = m_nNextTestIdx;
			while(m_wszJapaneseText[tmpIdx])
			{
				if( m_wszJapaneseText[tmpIdx] == wch )
				{
					m_nNextKorIdx++;
					m_nNextTestIdx = (int)tmpIdx+1;
					m_nHitCnt++;
					m_nDupCnt = 0;
					m_nFaultCnt = 0;
					nRetVal = 1;
					break;
				}
				tmpIdx++;
			}

			if(0==nRetVal && m_nFaultCnt<50)
			{
				nRetVal = 4;		// 유예
				m_nHitCnt = 0;
				m_nFaultCnt++;
			}
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

int CNonCachedTextArg::GetHitCount()
{
	return m_nHitCnt;
}

wchar_t CNonCachedTextArg::GetTranslatedCharacter()
{
	wchar_t wchRetVal = L'\0';

	int idx = m_nNextKorIdx;

	if( 0 <= idx && (unsigned)idx < m_nKoreanLen)
	{
		wchRetVal = m_wszKoreanText[idx];

		// 디버깅용
		// wchar_t tmp[2];
		// tmp[0] = m_wszKoreanText[idx];
		// tmp[1] = L'\0';
		// wchar_t dbg[MAX_PATH];
		// swprintf(dbg, L"[ aral1 ] Best : '(%x)%s' , '%s'[%d] \n", 
		// 	tmp[0], tmp, m_wszKoreanText, m_nNextKorIdx);
		// OutputDebugStringW(dbg);
	}
	else if( 0 <= idx && (unsigned)idx < m_nJapaneseLen)
	{
		wchRetVal = m_wszJapaneseText[idx];
	}

	//wchar_t dbg[MAX_PATH];
	//swprintf(dbg, L"[ aral1 ] Best : '%c' , '%s'[%d] -> '%s'[%d] \n", 
	//	wchRetVal, m_wszJapaneseText, m_nNextTestIdx-1, m_wszKoreanText, m_nNextKorIdx);
	//OutputDebugStringW(dbg);

	return wchRetVal;
}

int	CNonCachedTextArg::TestString(LPCWSTR wszJapaneseText, int nLen)
{
	int nRetVal = 0;

	char szTemp[1024]= {0, };

	if (wszJapaneseText)
	{
		if (nLen < 0) nLen = lstrlenW(wszJapaneseText);

		// nLen 이 1인 경우 한문자 출력방식일 수도 있고
		// 가끔 특별한 이유 없이 테스트문장의 첫글자만 들어오는 경우가 있어
		// 특수처리를 해 줘야 한다..
		if (nLen == 1)
		{
			if (wszJapaneseText[0] == m_wszJapaneseText[0])
			{
				// 첫글자가 들어왔을 수 있으므로 그냥 Hit 시키지만 m_nTestIdx 는 그냥 놔둔다.
				m_nHitCnt++;
				m_nDupCnt++;

				return 3;
			}
		}


		// 1. 마지막 출력된 곳 다음부터 테스트
		if (!wcsncmp(m_wszJapaneseText + m_nNextTestIdx, wszJapaneseText, nLen))
		{
			// 두번째 이후의 일치(Hit!)
			// 문장의 둘째 이후의 줄의 첫 출력
			m_nNextKorIdx = m_nNextTestIdx;
			m_nNextTestIdx += nLen;
			m_nHitCnt++;
			m_nDupCnt = 0;
			m_nFaultCnt = 0;
			nRetVal = 1;

		}
		// 2. 마지막 출력된 곳 테스트
		else if (!wcsncmp(m_wszJapaneseText + m_nNextKorIdx, wszJapaneseText, nLen))
		{
			// 두번째 이후의 중복
			m_nDupCnt++;
			m_nNextTestIdx = m_nNextKorIdx+nLen;
			nRetVal=2;

		}
		// 3. 문장의 처음부터 테스트
		else if (!wcsncmp(m_wszJapaneseText, wszJapaneseText, nLen))
		{

			// 중복출력
			m_nNextKorIdx = 0;
			m_nNextTestIdx = nLen;
			m_nDupCnt++;
			nRetVal = 2;

		}

		else
		{
			// 어딘가에서의 중복
			// 혹은 전혀 상관없는 문장

			int i;

			// 어딘가에서 중복되었는지 체크
			for (i = m_nNextKorIdx; i < (int)m_nJapaneseLen; i+=nLen)
			{
				if (!wcsncmp(m_wszJapaneseText + i, wszJapaneseText, nLen))
				{
					m_nNextKorIdx = i;
					m_nNextTestIdx = i+nLen;
					m_nHitCnt++;
					nRetVal=1;

					break;
				}

			}
			if (i >= (int)m_nJapaneseLen)
			{
				for (i = 0; i < m_nNextKorIdx; i+=nLen)
				{
					m_nNextKorIdx = i;
					m_nNextTestIdx = i+nLen;
					m_nDupCnt++;
					nRetVal = 3;

					break;
				}
			}
		}

		if (nRetVal == 0 && m_nFaultCnt < 50)
		{
			// 유예
			nRetVal = 4;
			m_nHitCnt = 0;
			m_nFaultCnt ++;
		}
	}

	return nRetVal;
}

int	CNonCachedTextArg::GetTranslatedString(LPWSTR wszKoreanBuf, int nBufSize, int &nOrigLen)
{
	if (m_nKoreanLen < (size_t)m_nNextKorIdx)
	{
		// 더이상 출력할 한글이 없다.
		wszKoreanBuf[0]=L'\0';
		return 0;
	}

	// MATCHLEN 이 FALSE 이고 마지막 라인이면 가능한 최대 문자를 출력
	if (!CNonCachedTextArgMgr::GetInstance()->IsMatchLen() && (m_nNextKorIdx+nOrigLen >= (int)m_nJapaneseLen))
		nOrigLen = nBufSize - 1;


	if (nOrigLen > nBufSize)
		nOrigLen = nBufSize - 1;

	lstrcpynW(wszKoreanBuf, m_wszKoreanText+m_nNextKorIdx, nOrigLen + 1);

	nOrigLen = lstrlenW(wszKoreanBuf);

	return nOrigLen;
}