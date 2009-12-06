#pragma once

#include <set>
#include <map>

using namespace std;


// 스트링관련 오리지널 함수 엔트리
//typedef DWORD (__stdcall * PROC_GetGlyphOutline)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*);
//typedef BOOL (__stdcall * PROC_TextOut)(HDC, int, int, LPCVOID, int);
//typedef BOOL (__stdcall * PROC_ExtTextOut)(HDC, int, int, UINT, CONST RECT*, LPCVOID, UINT, CONST INT*);
typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (_stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);
//typedef int (__stdcall * PROC_DrawText)(HDC, LPCVOID, int, LPRECT, UINT);
//typedef int (__stdcall * PROC_DrawTextEx)(HDC, LPCVOID ,int ,LPRECT, UINT, LPDRAWTEXTPARAMS);

typedef struct _TEXT_FUNCTION_ENTRY
{
	PROC_WideCharToMultiByte	pfnOrigWideCharToMultiByte;
	PROC_MultiByteToWideChar	pfnOrigMultiByteToWideChar;

} TEXT_FUNCTION_ENTRY, *PTEXT_FUNCTION_ENTRY;

class CATText;
class CFunction;
class CMainDbgDlg;

typedef set<UINT_PTR>	CHookedPoints;
typedef set<CATText*>	CATTextSet;
typedef map<UINT_PTR,CFunction*> CFunctionMap;

//////////////////////////////////////////////////////////////////////////
//
class CATTextArgMgr
{

private:

	enum
	{
		UseGetGlyphOutlineA = 0,
		UseGetGlyphOutlineW,
		UseTextOutA,
		UseTextOutW,
		UseExtTextOutA,
		UseExtTextOutW,
		UseDrawTextA,
		UseDrawTextW,
		UseDrawTextExA,
		UseDrawTextExW,
		TEXT_FUNC_CNT
	};

	static LPCTSTR m_arrTextFuncName[];

	static CATTextArgMgr* _Inst;

	BOOL					m_bApplocale;
	BOOL					m_bRunning;
	HMODULE					m_hContainer;
	CMainDbgDlg*			m_pMainDbgDlg;
	CWinThread*				m_pDlgThread;
	int						m_aTextFuncHit[TEXT_FUNC_CNT];

	CATTextSet				m_setActivatedArgs;			// 활성화된 텍스트 인자
	CATTextSet				m_setInactivatedArgs;		// 비활성화 되어 있는 인자들
	map<size_t,int>			m_mapHitDist;				// 적중한 ESP로 부터의 거리들

	CFunctionMap			m_mapFunc;
	CHookedPoints			m_setHookedPoints;
	HANDLE					m_hBreak;
	HANDLE					m_hResume;
	UINT_PTR				m_pCurBreakedPoint;

	int		AddTextArg(LPVOID pText, BOOL bWideChar, UINT_PTR ptrFunc, size_t dist);	// 새로운 문자열 후보를 추가한다
	BOOL	TestCharacter(wchar_t wch, void* baseESP);					// 문자열 후보들 전체를 테스트한다. (더이상 일치하지 않는 후보는 바로 삭제)
	int		SearchStringA(INT_PTR ptrBegin, char ch1, char ch2);
	int		SearchStringW(INT_PTR ptrBegin, wchar_t wch);
	void	ModifyHitMap(CATText* pATText, void* baseESP, int increment);
	int		GetMainTextFunction();
	UINT_PTR GetFuncAddrFromReturnAddr(UINT_PTR pAddr);
	void	AddRegAndStackDump(LPCTSTR cszStorage, UINT_PTR val, BOOL bWideChar);
	void	RefreshFunctionList();
	void	FillCallstackCtrl(UINT_PTR ptrBegin);
	
	static UINT MainDlgThreadFunc(LPVOID pParam);

	// 통보받을 시 작업
	void	OnBreakPoint(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);

	static void BreakRoutine(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);

	DWORD InnerGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	DWORD InnerGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	BOOL InnerTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	BOOL InnerTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	BOOL InnerExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	BOOL InnerExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	int InnerDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	int InnerDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	int InnerDrawTextExA(
		HDC hdc,                     // handle to DC
		LPSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	int InnerDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	//////////////////////////////////////////////////////////////////////////
	// Static Functions
	static DWORD __stdcall NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static DWORD __stdcall NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static BOOL __stdcall NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	static BOOL __stdcall NewExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	static int __stdcall NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	static int __stdcall NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	static int __stdcall NewDrawTextExA(
		HDC hdc,                     // handle to DC
		LPSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	static int __stdcall NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);
public:
	HWND						m_hContainerWnd;
	CONTAINER_PROC_ENTRY		m_sATCTNR3;
	TEXT_FUNCTION_ENTRY			m_sTextFunc;

	CATTextArgMgr(void);
	~CATTextArgMgr(void);

	BOOL Init(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL Start();
	BOOL Stop();
	BOOL Option();
	BOOL Close();

	void OnSetBreakPointOnFuncList(int nIdx);
	void OnResumeProgram();
	static CATTextArgMgr* GetInstance();

};
