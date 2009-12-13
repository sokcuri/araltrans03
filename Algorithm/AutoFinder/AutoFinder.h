// 다음 ifdef 블록은 DLL에서 내보내기하는 작업을 쉽게 해 주는 매크로를 만드는 
// 표준 방식입니다. 이 DLL에 들어 있는 파일은 모두 명령줄에 정의된 _EXPORTS 기호로
// 컴파일되며, 동일한 DLL을 사용하는 다른 프로젝트에서는 이 기호를 정의할 수 없습니다.
// 이렇게 하면 소스 파일에 이 파일이 들어 있는 다른 모든 프로젝트에서는 
// AUTOFINDER_API 함수를 DLL에서 가져오는 것으로 보고, 이 DLL은
// 이 DLL은 해당 매크로로 정의된 기호가 내보내지는 것으로 봅니다.
#include "resource.h"		// main symbols
#include "../../Common/DefStruct.h"


// Export Functions
/*
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
*/

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

// CATCodeApp
// See ATCode.cpp for the implementation of this class
//
class ITextManager;

class CAutoFinderApp : public CWinApp
{
private:
	HMODULE				m_hContainer;
	HWND				m_hContainerWnd;
	LPWSTR				m_wszOptionString;	
	ITextManager*		m_pTextMgr;

	//////////////////////////////////////////////////////////////////////////
	// GetGlyphOutlineA 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static DWORD __stdcall NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	//////////////////////////////////////////////////////////////////////////
	// GetGlyphOutlineW 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static DWORD __stdcall NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	//////////////////////////////////////////////////////////////////////////
	// NewTextOutA 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	//////////////////////////////////////////////////////////////////////////
	// NewTextOutW 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	//////////////////////////////////////////////////////////////////////////
	// ExtTextOutA 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewExtTextOutA(
		HDC hdc,			// handle to DC
		int nXStart,		// x-coordinate of reference point
		int nYStart,		// y-coordinate of reference point
		UINT fuOptions,		// text-output options
		CONST RECT* lprc,	// optional dimensions
		LPCSTR lpString,	// string
		UINT cbCount,		// number of characters in string
		CONST INT* lpDx		// array of spacing values
		);

	//////////////////////////////////////////////////////////////////////////
	// ExtTextOutW 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static BOOL __stdcall NewExtTextOutW(
		HDC hdc,          // handle to DC
		int nXStart,            // x-coordinate of reference point
		int nYStart,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextA 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextW 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextExA 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextExA(
		HDC hDC,                     // handle to DC
		LPSTR lpString,              // text to draw
		int nCount,                 // length of text to draw
		LPRECT lpRect,                 // rectangle coordinates
		UINT uFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	//////////////////////////////////////////////////////////////////////////
	// DrawTextExW 대체 함수
	//////////////////////////////////////////////////////////////////////////
	static int __stdcall NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,             // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

public:
	static CAutoFinderApp*	g_Inst;
	CONTAINER_PROC_ENTRY	m_sContainerFunc;
	CAutoFinderApp();

	// Overrides
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

public:
	BOOL Init(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL Option();
	BOOL Close();

	BOOL ApplyOption( LPCTSTR strCachingType );
	void ResetOption();
};

#define MAINAPP (CAutoFinderApp::g_Inst)
