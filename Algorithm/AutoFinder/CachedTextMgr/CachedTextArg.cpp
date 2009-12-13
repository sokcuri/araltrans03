
#include "../stdafx.h"
#include "../AutoFinder.h"
#include "CachedTextArg.h"

#pragma warning(disable:4996)


CCachedText::CCachedText(void)
{
	ResetCachedText();
}

CCachedText::~CCachedText(void)
{
}

void CCachedText::ResetCachedText()
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
BOOL CCachedText::IsJapaneseTextA(LPCSTR szJpnText)
{
	BOOL bRetVal = FALSE;

	// 길이 유효성 검사
	int nLen = lstrlenA(szJpnText);
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

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// 유효한 일본어 텍스트인가 평가 (유니코드용)
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedText::IsJapaneseTextW(LPCWSTR wszJpnText)
{
	BOOL bRetVal = FALSE;

	size_t nLen = wcslen(wszJpnText);
	if(1 < nLen && nLen < MAX_TEXT_LENGTH)
	{
		bRetVal = TRUE;
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// 텍스트 세팅
//
//////////////////////////////////////////////////////////////////////////
BOOL CCachedText::SetCachedText(LPVOID pSource, BOOL bWideChar)
{
	BOOL bRetVal = FALSE;

	ResetCachedText();

	if( pSource && MAINAPP && MAINAPP->m_sContainerFunc.procTranslateUsingCtx  )
	{	
		BOOL bIsJpnText = FALSE;
		
		// 유효성 검사 : 유니코드 문자열이 넘어온 경우
		if( bWideChar )
		{
			bIsJpnText = IsJapaneseTextW((LPCWSTR)pSource);

			if( bIsJpnText )
			{
				m_nJapaneseLen = wcslen((LPCWSTR)pSource);
				wcscpy(m_wszJapaneseText, (LPCWSTR)pSource);
				MyWideCharToMultiByte(932, 0, m_wszJapaneseText, -1, m_szJapaneseText, MAX_TEXT_LENGTH*2-1, NULL, NULL);
			}
		}
		// 멀티바이트 문자열이 넘어온 경우
		else
		{
			bIsJpnText = IsJapaneseTextA((LPCSTR)pSource);
			
			if(bIsJpnText)
			{
				m_nJapaneseLen = strlen((LPCSTR)pSource);
				strcpy(m_szJapaneseText, (LPCSTR)pSource);
				MyMultiByteToWideChar(932, 0, m_szJapaneseText, -1, m_wszJapaneseText, MAX_TEXT_LENGTH-1 );
			}
		}


		// 일본어 텍스트면 번역 처리
		if( bIsJpnText )
		{			
			// 번역기에 집어넣어라
			char szKorean[MAX_TEXT_LENGTH*2];

			BOOL bTrans = MAINAPP->m_sContainerFunc.procTranslateUsingCtx(
				L"AutoFinder",
				m_szJapaneseText,
				strlen(m_szJapaneseText) + 1,
				szKorean,
				MAX_TEXT_LENGTH*2
				);
			
			if( bTrans )
			{
				// 유니코드 한국어
				wchar_t wszKoreanText[MAX_TEXT_LENGTH];
				MyMultiByteToWideChar(949, 0, (LPCSTR)szKorean, -1, wszKoreanText, MAX_TEXT_LENGTH-1 );
				CString strKorean = wszKoreanText;
				strKorean.Replace(L" ", L"");
				wcscpy(wszKoreanText, strKorean);

				int j_len = (int)wcslen(m_wszJapaneseText);
				int k_len = (int)wcslen(wszKoreanText);
				int nReduceSize = k_len - j_len;
				if( nReduceSize > 0 )
				{
					j_len--;
					k_len--;
					while(j_len>=0)
					{
						// 일반 아스키 문자
						if( wszKoreanText[k_len] < 0x80 ) 
						{
							m_wszKoreanText[j_len] = wszKoreanText[k_len];
							j_len--;
						}
						// 한글 문자
						else
						{

						}

						k_len--;
						if( j_len >= k_len ) break;
					}

					if( j_len >= 0 )
					{
						memcpy( m_wszKoreanText, wszKoreanText, (j_len+1)*sizeof(wchar_t) );
					}
				}
				else
				{
					wcscpy( m_wszKoreanText, wszKoreanText );
				}

				// 멀티바이트 한국어
				MyWideCharToMultiByte(949, 0, m_wszKoreanText, -1, szKorean, MAX_TEXT_LENGTH, NULL, NULL);

				/*
				// 멀티바이트 인코딩
				int len = (int)strlen(szKorean);
				int i = 0;
				int j = 0;
				while( i<len )
				{
					if( 0x80 <= (BYTE)szKorean[i] )
					{
						char tmpbuf[3];
						tmpbuf[0] = szKorean[i];
						tmpbuf[1] = szKorean[i+1];
						tmpbuf[2] = '\0';

						CCharacterMapper::EncodeK2J(tmpbuf, &m_szKoreanText[j]);

						i+=2;
					}
					else
					{
						m_szKoreanText[j] = szKorean[i++];
						if( 0x80 <= (BYTE)szKorean[i] )
						{
							m_szKoreanText[j+1] = ' ';
						}
						else
						{
							m_szKoreanText[j+1] = szKorean[i++];
						}
					}

					j+=2;
				}
				m_szKoreanText[j] = '\0';
				*/
				strcpy(m_szKoreanText, szKorean);
												
				if(bWideChar) m_nKoreanLen = wcslen(m_wszKoreanText);
				else m_nKoreanLen = strlen(m_szKoreanText);
			}

			m_setSourcePtr.insert(pSource);
			m_bWideChar = bWideChar;
			
			bRetVal = TRUE;

			//wchar_t dbg[1024];
			//swprintf(dbg, L"[ aral1 ] '%s'->'%s' \n", m_wszJapaneseText, m_wszKoreanText);
			//OutputDebugStringW(dbg);

		}	// 일본어 텍스트면 번역 처리
	}

	if( FALSE == bRetVal) ResetCachedText();

	return bRetVal;
}


inline BOOL CCachedText::IsControlCharacter(wchar_t wch) const
{
	//return ( ( wch==0x0A || wch==0x0D ) ? TRUE : FALSE );
	return ( wch < 0x20 ? TRUE : FALSE );
}

int CCachedText::TestText(LPVOID pSource)
{
	if(NULL==pSource) return 0;

	int iRetVal = 0;
	
	if(m_bWideChar)
	{
		LPWSTR wszSource = (LPWSTR)pSource;
		
		// 일본어 텍스트가 일치한다면
		if( wcscmp(wszSource, m_wszJapaneseText) == 0 )
		{
			// 한국어 텍스트 덮어쓰기가 필요하면 덮기
			if(m_bTranslated)
			{
				//memcpy(wszSource, m_wszKoreanText, m_nJapaneseLen*sizeof(wchar_t));
				OverwriteTranslatedText(pSource);
			}

			iRetVal = 1;
		}
		// 번역된 한국어와 일치한다면
		else if( wcsstr(m_wszKoreanText, wszSource) )
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
			// 한국어 텍스트 덮어쓰기가 필요하면 덮기
			if(m_bTranslated)
			{
				BOOL bTransRes = OverwriteTranslatedText(pSource);
				TRACE( " [ aral1 ] Overwrite result %d [0x%p]('%s') \n", bTransRes, pSource, pSource );
			}

			iRetVal = 1;
		}
		// 번역된 한국어와 일치한다면
		//else if( strncmp(szSource, m_szKoreanText, min(m_nJapaneseLen, m_nKoreanLen)) == 0 )
		else
		{
			char tmpsrc[1024];
			ZeroMemory(tmpsrc, 1024);
			int i = (int)strlen(szSource) - 1;
			while( i >= 0 )
			{
				if(szSource[i]==0x20) tmpsrc[i] = '\0';
				else tmpsrc[i] = szSource[i];
				i--;
			}

			if( strstr(m_szKoreanText, tmpsrc) )
			{
				iRetVal = 1;
			}
		}
	}

	if(iRetVal)
	{
		m_setSourcePtr.insert(pSource);
	}

	return iRetVal;
}


int CCachedText::TestCharacter(wchar_t wch)
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

int CCachedText::GetHitCount()
{
	return m_nHitCnt;
}

BOOL CCachedText::IsWideCharacter() 
{
	return m_bWideChar;
}

BOOL CCachedText::IsTranslatable() 
{
	BOOL bRetVal = TRUE;
	size_t cnt, i,len;

	// 일본어 안전검사
	len = wcslen(m_wszJapaneseText);
	if(len<3)
	{
		bRetVal = FALSE;
	}
	else
	{
		cnt = 0;
		for(i = 0; i < len; i++ )
		{
			if(m_wszJapaneseText[i]==L'?') cnt++;
		}
		if( cnt > len/2 ) bRetVal = FALSE;
		//TRACE("[ aral1 ] 일본어에 '?' 함유량 : %d \n", cnt);

	}

	// 한국어 안전검사
	len = wcslen(m_wszKoreanText);
	if(len<3)
	{
		bRetVal = FALSE;
	}
	else
	{
		cnt = 0;
		for(i = 0; i < len; i++ )
		{
			if( L'A' <= (WORD)m_wszKoreanText[i] && (WORD)m_wszKoreanText[i] <= L'z' ) continue;

			if( (WORD)m_wszKoreanText[i] < 0xAC00 || (0xAC00+11172) < (WORD)m_wszKoreanText[i] ) cnt++;
		}
		if( cnt > len/2 ) bRetVal = FALSE;
		//TRACE("[ aral1 ] 한국어에 괴문자 수 : %d \n", cnt);
	}


	return bRetVal;
}

void CCachedText::Translate() 
{
	if( m_bTranslated ) return;
	
	if( IsTranslatable() )
	{
		for(set<void*>::iterator iter = m_setSourcePtr.begin();	iter != m_setSourcePtr.end(); iter++)
		{
			void* pDest = (*iter);
			OverwriteTranslatedText(pDest);
		}

		m_bTranslated = TRUE;
	}
}

BOOL CCachedText::OverwriteTranslatedText( LPVOID pDest )
{
	if(m_bWideChar)
	{
		LPWSTR wszDest = (LPWSTR)pDest;

		// 문자열 포인터가 잘못되었다면 리턴
		if( IsBadWritePtr(pDest,m_nJapaneseLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;

		// 일어 텍스트가 일치하지 않는다면 리턴
		if( wcslen(wszDest) > 1024 || wcscmp(wszDest, m_wszJapaneseText) ) return FALSE;

		// 텍스트 복사
		size_t len = min(m_nJapaneseLen, m_nKoreanLen);
		memcpy(wszDest, m_wszKoreanText, len*sizeof(wchar_t));
		while(len<m_nJapaneseLen)
		{
			wszDest[len] = L' ';
			len++;
		}
		
	}
	else
	{

		LPSTR szDest = (LPSTR)pDest;

		// 문자열 포인터가 잘못되었다면 리턴
		if( IsBadWritePtr(pDest,m_nJapaneseLen) || IsBadStringPtrA(szDest, 1024*1024*1024) ) return FALSE;

		// 일어 텍스트가 일치하지 않는다면 리턴
		if( strlen(szDest) > 1024 || strcmp(szDest, m_szJapaneseText) ) return FALSE;

		// 텍스트 복사
		//min(m_nJapaneseLen, m_nKoreanLen);
		
		size_t len = 0;
		while(len<m_nJapaneseLen && len<m_nKoreanLen)
		{
			size_t addval = 1;
			if( (BYTE)0x80 <= (BYTE)m_szKoreanText[len] ) addval = 2;

			if( len + addval > m_nJapaneseLen ) break;

			len += addval;
		}

		memcpy(szDest, m_szKoreanText, len);
		while(len<m_nJapaneseLen)
		{
			szDest[len] = ' ';
			len++;
		}

	}

	return TRUE;
}
wchar_t CCachedText::GetBestTranslatedCharacter() 
{
	wchar_t wch = L'\0';
	
	if( m_nKoreanLen > (size_t)m_nNextTestIdx-1 )
	{
		wch = m_wszKoreanText[m_nNextTestIdx-1];
	}

	/*
	if(m_bWideChar)
	{
		if( m_nKoreanLen > (size_t)m_nNextTestIdx-1 )
		{
			wch = m_wszKoreanText[m_nNextTestIdx-1];
		}
	}
	else
	{
		char tmpchar[4] = {0,};
		size_t idx = m_nNextTestIdx-1;
		
		// 2글자짜리라면
		if( 0 <= (int)idx - 1 && 0xE0 <= (BYTE)m_szKoreanText[idx-1] && (BYTE)m_szKoreanText[idx-1] <= 0xFC )
		{
			idx--;
			BOOL bDecode = CCharacterMapper::DecodeJ2K(&m_szKoreanText[idx], tmpchar);
			if(FALSE==bDecode) return wch;

		}
		// 1글자짜리라면
		{
			tmpchar[0] = m_szKoreanText[idx];
		}
		
		wchar_t tmp_wchar[4] = {0,};
		MyMultiByteToWideChar( 949, 0, tmpchar, -1, tmp_wchar, 3 );

		wch = tmp_wchar[0];
	}
	*/

	return wch;
}