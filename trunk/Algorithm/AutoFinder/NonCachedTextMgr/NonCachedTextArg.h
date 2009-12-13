#pragma once

#define MAX_TEXT_LENGTH 1024

class CNonCachedTextArg
{
	friend class CNonCachedTextArgMgr;

private:
	int		m_nNextTestIdx;						// 문자열의 비교할 위치
	int		m_nNextKorIdx;						// 다음 반환할 문자
	int		m_nHitCnt;							// 히트 카운트
	int		m_nDupCnt;							// 중복 카운트
	int		m_nFaultCnt;						// 실패 카운트
	wchar_t	m_wszJapaneseText[MAX_TEXT_LENGTH];	// 일본어 텍스트 버퍼
	size_t	m_nJapaneseLen;						// 일본어 텍스트 길이
	wchar_t	m_wszKoreanText[MAX_TEXT_LENGTH];	// 한국어 텍스트 버퍼
	size_t	m_nKoreanLen;						// 한국어 텍스트 길이

	inline BOOL IsControlCharacter(wchar_t wch) const;
		
public:
	CNonCachedTextArg(void);
	~CNonCachedTextArg(void);

	BOOL	SetNonCachedTextArg(LPCWSTR wszJapaneseText);
	void	ResetNonCachedTextArg();

	int		TestCharacter(wchar_t wch, UINT nPreperedDupCnt);
	int		GetHitCount();
	wchar_t	GetTranslatedCharacter();

	int		TestString(LPCWSTR wszJapaneseText, int nLen);
	int		GetTranslatedString(LPWSTR wszKoreanBuf, int nBufSize, int &nOrigLen);
};
