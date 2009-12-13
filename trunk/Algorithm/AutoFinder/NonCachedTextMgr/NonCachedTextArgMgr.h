#pragma once

#include "../ITextManager.h"
#include <set>
#include <map>

using namespace std;

class CNonCachedTextArg;
typedef set<CNonCachedTextArg*> CNonCachedTextArgSet;

//////////////////////////////////////////////////////////////////////////
//
class CNonCachedTextArgMgr : public ITextManager
{
private:
	static CNonCachedTextArgMgr* _inst;

	UINT					m_aDupCntTable[16];			// 중복 카운트 테이블
	CNonCachedTextArgSet	m_setActivatedArgs;			// 활성화된 텍스트 인자
	CNonCachedTextArgSet	m_setInactivatedArgs;		// 비활성화 되어 있는 인자들
	BOOL					m_bMatchLen;

	BOOL	AddTextArg(LPCWSTR wszText);		// 새로운 문자열 후보를 추가한다
	BOOL	TestCharacter(wchar_t wch);					// 문자열 후보들 전체를 테스트한다. (더이상 일치하지 않는 후보는 바로 삭제)
	BOOL	GetBestTranslatedCharacter(wchar_t* pTransResultBuf);		// 최고로 확률이 높은 번역 문자를 반환
	CNonCachedTextArg*	FindString(LPCWSTR pTestString, int nSize = -1);
	BOOL	GetTranslatedStringA(INT_PTR ptrBegin, LPCSTR szOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize);
	BOOL	GetTranslatedStringW(INT_PTR ptrBegin, LPCWSTR wszOrigString, int nOrigSize, wchar_t *pTransResultBuf, int nBufSize, int &nTransSize);
	BOOL	IsEmpty();									// 현재 활성화 된 텍스트 인자들이 하나도 없는가?
	int		SearchStringA(INT_PTR ptrBegin, char ch1, char ch2);
	int		SearchStringW(INT_PTR ptrBegin, wchar_t wch);

public:

	CNonCachedTextArgMgr(void);
	static CNonCachedTextArgMgr* GetInstance(){ return _inst; };
	BOOL IsMatchLen(){ return m_bMatchLen; };

	//////////////////////////////////////////////////////////////////////////
	// Override Functions
	virtual ~CNonCachedTextArgMgr(void);

	virtual BOOL Init();
	virtual void Close();

	virtual DWORD NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	virtual DWORD NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	virtual BOOL NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	virtual BOOL NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	virtual BOOL NewExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	virtual BOOL NewExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	virtual int NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	virtual int NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	virtual int NewDrawTextExA(
		HDC hdc,                     // handle to DC
		LPSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	virtual int NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);
};
