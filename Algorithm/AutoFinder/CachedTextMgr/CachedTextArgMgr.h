#pragma once

#include "../../../Common/DefStruct.h"
#include "../ITextManager.h"
#include <set>
#include <map>

using namespace std;

// 후킹 예약 구조체
typedef struct _RESERVED_HOOK_POINT
{
	UINT_PTR	pHookPoint;
	size_t		nArgDist;
	BOOL		bWideChar;
} RESERVED_HOOK_POINT, *PRESERVED_HOOK_POINT;

class CCachedText;
class CFunction;

typedef set<RESERVED_HOOK_POINT*> CReservedHooks;
typedef set<CCachedText*> CCachedTextSet;
typedef map<UINT_PTR,CFunction*> CFunctionMap;
typedef pair<UINT_PTR,size_t> CArgInfo;
typedef map<UINT_PTR,size_t> CArgInfoMap;

//////////////////////////////////////////////////////////////////////////
//
class CCachedTextArgMgr : public ITextManager
{

private:
	static CCachedTextArgMgr* _Inst;

	CCachedTextSet			m_setActivatedArgs;			// 활성화된 텍스트 인자
	CCachedTextSet			m_setInactivatedArgs;		// 비활성화 되어 있는 인자들
	map<size_t,int>			m_mapHitDist;				// 적중한 ESP로 부터의 거리들
	CFunctionMap			m_mapFunc;
	CReservedHooks			m_setReservedHooks;			// 후킹 예약
	size_t					m_distBest;					// 이 인자는 무조건 번역
	pair<UINT_PTR,UINT_PTR> m_rangeExeModule;
	CArgInfoMap				m_mapArgInfoA;
	CArgInfoMap				m_mapArgInfoW;

	int		AddTextArg(LPVOID pText, BOOL bWideChar, BOOL bAutoTrans, UINT_PTR ptrFunc, size_t dist);	// 새로운 문자열 후보를 추가한다
	BOOL	TestCharacter(wchar_t wch, void* baseESP);					// 문자열 후보들 전체를 테스트한다. (더이상 일치하지 않는 후보는 바로 삭제)
	int		SearchStringA(INT_PTR ptrBegin, char ch1, char ch2);
	int		SearchStringW(INT_PTR ptrBegin, wchar_t wch);
	void	ModifyHitMap(CCachedText* pCachedText, void* baseESP, int increment);
	wchar_t GetBestTranslatedCharacter();
	void	FindBestDistAndClearHitMap();
	UINT_PTR GetFuncAddrFromReturnAddr(UINT_PTR pAddr);
	BOOL	IsAutoTransPoint(size_t dist);
	void	HookAllReservedPoints();

	static void ModifyValueA(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);
	static void ModifyValueW(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);

public:
	CONTAINER_PROC_ENTRY	m_sContainerFunc;

	CCachedTextArgMgr(void);

	//////////////////////////////////////////////////////////////////////////
	// Override Functions
	virtual ~CCachedTextArgMgr(void);

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
