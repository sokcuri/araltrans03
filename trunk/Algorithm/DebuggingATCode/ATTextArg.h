#pragma once
#include <set>

#define MAX_TEXT_LENGTH 1024

using namespace std;

class ITranslator;

class CATText
{
	friend class CATTextArgMgr;

private:
	BOOL		m_bWideChar;
	BOOL		m_bTranslated;
	BOOL		m_bTranslatedTestMode;
	int			m_nNextTestIdx;						// 문자열의 비교할 위치
	int			m_nHitCnt;							// 히트 카운트
	int			m_nFaultCnt;						// 실패 카운트
	set<void*>	m_setSourcePtr;
	set<pair<UINT_PTR,size_t>> m_setFuncArg;		// 당시 호출시 함수 포인터와 ESP로부터의 거리

	char		m_szJapaneseText[MAX_TEXT_LENGTH];	// 일본어 텍스트 버퍼 (멀티바이트)
	wchar_t		m_wszJapaneseText[MAX_TEXT_LENGTH];	// 일본어 텍스트 버퍼 (유니코드)
	size_t		m_nJapaneseLen;						// 일본어 텍스트 길이

	char		m_szKoreanText[MAX_TEXT_LENGTH];	// 한국어 텍스트 버퍼 (멀티바이트)
	wchar_t		m_wszKoreanText[MAX_TEXT_LENGTH];	// 한국어 텍스트 버퍼 (유니코드)
	size_t		m_nKoreanLen;						// 한국어 텍스트 길이

	BOOL		OverwriteTranslatedText(LPVOID pDest);
	BOOL		IsJapaneseTextA(LPCSTR szJpnText);
	BOOL		IsJapaneseTextW(LPCWSTR wszJpnText);

public:
	CATText(void);
	~CATText(void);

	BOOL	SetATText(LPVOID pSource, BOOL bWideChar);
	void	ResetATText();
	BOOL	IsWideCharacter();
	
	//////////////////////////////////////////////////////////////////////////
	//
	// <반환값>
	// 0 : 전혀 연관성 없음
	// 양수 : 중복된 문자열. 적절한 처리를 하였음.
	int		TestText(LPVOID pSource);

	//////////////////////////////////////////////////////////////////////////
	//
	// <반환값>
	// 0 : 적중되지 않음
	// 1 : 적중
	//////////////////////////////////////////////////////////////////////////
	int		TestCharacter(wchar_t wch);
	
	//////////////////////////////////////////////////////////////////////////
	//
	// <반환값>
	// 양수 : 연속 적중된 카운트
	// 음수 : 연속 실패한 카운트
	//////////////////////////////////////////////////////////////////////////
	int		GetHitCount();

};
