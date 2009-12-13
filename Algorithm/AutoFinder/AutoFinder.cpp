// AutoFinder.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "AutoFinder.h"
#include "OptionDlg.h"
#include "NonCachedTextMgr/NonCachedTextArgMgr.h"
#include "CachedTextMgr/CachedTextArgMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 유일한 응용 프로그램 개체입니다.

CAutoFinderApp* CAutoFinderApp::g_Inst = NULL;
CAutoFinderApp theApp;

using namespace std;

extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
{
	BOOL bRetVal = FALSE;

	if(pPluginInfo && pPluginInfo->cch >= sizeof(PLUGIN_INFO))
	{
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"Auto Text Finder");
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Algorithm");
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Algorithm/AutoFinder.zip");
		bRetVal = TRUE;
	}
	return bRetVal;
}


extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption)
{
	return theApp.Init(hAralWnd, wszPluginOption);
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return theApp.Option();
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return theApp.Close();
}

typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);
PROC_WideCharToMultiByte g_pfnOrigWideCharToMultiByte = NULL;
PROC_MultiByteToWideChar g_pfnOrigMultiByteToWideChar = NULL;

int MyWideCharToMultiByte(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCWSTR lpWideCharStr, 
						  int cchWideChar, 
						  LPSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPCSTR lpDefaultChar, 
						  LPBOOL lpUsedDefaultChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigWideCharToMultiByte )
	{
		nRetVal = g_pfnOrigWideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}
	else
	{
		nRetVal = ::WideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}

	return nRetVal;
}

int MyMultiByteToWideChar(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPWSTR lpWideCharStr, 
						  int cchWideChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigMultiByteToWideChar )
	{
		nRetVal = g_pfnOrigMultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}
	else
	{
		nRetVal = ::MultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}

	return nRetVal;
}

//////////////////////////////////////////////////////////////////////////
// CATCodeApp class
//////////////////////////////////////////////////////////////////////////

CAutoFinderApp::CAutoFinderApp(void)
: m_hContainer(NULL)
, m_hContainerWnd(NULL)
, m_wszOptionString(NULL)
, m_pTextMgr(NULL)
{

}

BOOL CAutoFinderApp::InitInstance()
{
	return CWinApp::InitInstance();
}

BOOL CAutoFinderApp::ExitInstance()
{
	TRACE(_T("CAutoFinderApp::ExitInstance(() \n"));

	Close();
	return CWinApp::ExitInstance();
}

BOOL CAutoFinderApp::Init( HWND hAralWnd, LPWSTR wszPluginOption )
{
	Close();

	BOOL bRetVal = FALSE;

	try
	{
		//////////////////////////////////////////////////////////////////////////
		// Initializing Code
		//////////////////////////////////////////////////////////////////////////

		m_hContainerWnd = hAralWnd;
		if(NULL == hAralWnd || INVALID_HANDLE_VALUE == hAralWnd)
			throw _T("Invalid AralTrans Window Handle!");

		m_wszOptionString = wszPluginOption;
		if(NULL == m_wszOptionString)
			throw _T("Invalid Option String Pointer!");

		// Get AT container
		m_hContainer = GetModuleHandle(_T("ATCTNR3.DLL"));
		if(NULL == m_hContainer || INVALID_HANDLE_VALUE == m_hContainer)
			throw _T("Can not find ATCTNR3.DLL handle!");

		// 컨테이너 함수 포인터 얻어오기
		m_hContainer = GetModuleHandle(_T("ATCTNR3.DLL"));
		if(m_hContainer && INVALID_HANDLE_VALUE != m_hContainer)
		{
			ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_PROC_ENTRY));
			m_sContainerFunc.procHookWin32Api		= (PROC_HookWin32Api) GetProcAddress(m_hContainer, "HookWin32Api");
			m_sContainerFunc.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(m_hContainer, "UnhookWin32Api");
			m_sContainerFunc.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(m_hContainer, "HookCodePoint");
			m_sContainerFunc.procUnhookCodePoint	= (PROC_UnhookCodePoint) GetProcAddress(m_hContainer, "UnhookCodePoint");
			m_sContainerFunc.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(m_hContainer, "CreateTransCtx");
			m_sContainerFunc.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(m_hContainer, "DeleteTransCtx");
			m_sContainerFunc.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(m_hContainer, "TranslateUsingCtx");
			m_sContainerFunc.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(m_hContainer, "IsAppLocaleLoaded");
			m_sContainerFunc.procSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress(m_hContainer, "SuspendAllThread");
			m_sContainerFunc.procResumeAllThread	= (PROC_ResumeAllThread) GetProcAddress(m_hContainer, "ResumeAllThread");
			m_sContainerFunc.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(m_hContainer, "IsAllThreadSuspended");
		}

		if( !(m_sContainerFunc.procHookWin32Api && m_sContainerFunc.procUnhookWin32Api
			&& m_sContainerFunc.procHookCodePoint && m_sContainerFunc.procUnhookCodePoint
			&& m_sContainerFunc.procCreateTransCtx && m_sContainerFunc.procDeleteTransCtx
			&& m_sContainerFunc.procTranslateUsingCtx && m_sContainerFunc.procIsAppLocaleLoaded
			&& m_sContainerFunc.procSuspendAllThread && m_sContainerFunc.procResumeAllThread
			&& m_sContainerFunc.procIsAllThreadSuspended) )
			throw _T("Failed to get container procedures!");

		// Suspend All Threads
		m_sContainerFunc.procSuspendAllThread();

		// Hook Text Functions
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA, 7 ) == FALSE )
			throw _T("Failed to hook 'GetGlyphOutlineA'!");
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW, 7 ) == FALSE )
			throw _T("Failed to hook 'GetGlyphOutlineW'!");
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA, 7 ) == FALSE )
			throw _T("Failed to hook 'TextOutA'!");
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW, 7 ) == FALSE )
			throw _T("Failed to hook 'TextOutW'!");
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA, 7 ) == FALSE )
			throw _T("Failed to hook 'ExtTextOutA'!");
		if( m_sContainerFunc.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW, 7 ) == FALSE )
			throw _T("Failed to hook 'ExtTextOutW'!");
		if( m_sContainerFunc.procHookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA, 7 ) == FALSE )
			throw _T("Failed to hook 'DrawTextA'!");
		if( m_sContainerFunc.procHookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW, 7 ) == FALSE )
			throw _T("Failed to hook 'DrawTextW'!");
		if( m_sContainerFunc.procHookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA, 7 ) == FALSE )
			throw _T("Failed to hook 'DrawTextExA'!");
		if( m_sContainerFunc.procHookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW, 7 ) == FALSE )
			throw _T("Failed to hook 'DrawTextExW'!");

		// Prevent Applocale Redirection
		HKEY hCategoryKey = HKEY_CURRENT_USER;
		HKEY hKey = NULL;
		LONG lRet = RegOpenKeyEx(hCategoryKey, _T("Software\\AralGood"), 0, KEY_READ, &hKey);

		if(lRet == ERROR_SUCCESS)
		{
			DWORD type = REG_DWORD;
			DWORD size = MAX_PATH*2;
			BYTE dir[MAX_PATH*2];

			if(RegQueryValueEx(hKey, _T("M2WAddr"), 0, &type, (LPBYTE)&dir, &size) == ERROR_SUCCESS)
				memcpy( &g_pfnOrigMultiByteToWideChar, &dir, sizeof(DWORD) );

			if(RegQueryValueEx(hKey, _T("W2MAddr"), 0, &type, (LPBYTE)&dir, &size) == ERROR_SUCCESS)
				memcpy( &g_pfnOrigWideCharToMultiByte, &dir, sizeof(DWORD) );
		}

		// Create context
		if(m_sContainerFunc.procCreateTransCtx(L"AutoFinder") == FALSE)
			throw _T("Failed to create the context 'AutoFinder'!");

		// Apply option
		if(ApplyOption(m_wszOptionString) == FALSE)
			throw _T("Failed to apply option!");

		CAutoFinderApp::g_Inst = this;

		bRetVal = TRUE;
	}
	catch (LPCTSTR strErr)
	{
		::MessageBox(NULL, strErr, _T("Auto Text Finder"), MB_OK | MB_TOPMOST);
		Close();
	}

	// Resume All Threads
	if(m_sContainerFunc.procResumeAllThread) m_sContainerFunc.procResumeAllThread();

	return bRetVal;
}

BOOL CAutoFinderApp::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	CAutoFinderApp::g_Inst = NULL;

	// Suspend All Threads
	m_sContainerFunc.procSuspendAllThread();

	// Unhook Drawing API
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA );
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW );
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA );
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW );
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA );
	m_sContainerFunc.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW );
	m_sContainerFunc.procUnhookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA );
	m_sContainerFunc.procUnhookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW );
	m_sContainerFunc.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA );
	m_sContainerFunc.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW );

	// Resume All Threads
	if(m_sContainerFunc.procResumeAllThread) m_sContainerFunc.procResumeAllThread();

	// Delete Current Text Manager
	if(m_pTextMgr)
	{
		m_pTextMgr->Close();
		delete m_pTextMgr;
		m_pTextMgr = NULL;
	}

	// Delete Context
	m_sContainerFunc.procDeleteTransCtx(L"AutoFinder");

	// 기타 변수 리셋
	m_hContainer = NULL;
	m_hContainerWnd = NULL;
	m_wszOptionString = NULL;

	return TRUE;
}

BOOL CAutoFinderApp::Option()
{
	BOOL bRetVal = TRUE;

	COptionDlg od;
	od.m_strCachingType = m_wszOptionString;
	if( od.DoModal() == IDOK )
	{
		ApplyOption(od.m_strCachingType);
	}

	return bRetVal;
}

BOOL CAutoFinderApp::ApplyOption( LPCTSTR cwszCachingType )
{
	BOOL bRetVal = FALSE;

	CString strCachingType = cwszCachingType;
	if( strCachingType.CompareNoCase(_T("NonCached")) != 0
		&& strCachingType.CompareNoCase(_T("Cached")) != 0 )
	{
		strCachingType = _T("NonCached");
	}

	// Create TextMgr
	if( strCachingType.CompareNoCase(_T("NonCached")) == 0
		&& (_tcsicmp(m_wszOptionString, _T("NonCached")) != 0 || NULL == m_pTextMgr) )
	{
		if(m_pTextMgr)
		{
			m_pTextMgr->Close();
			delete m_pTextMgr;
		}
		// NonCachedTextMgr
		m_pTextMgr = new CNonCachedTextArgMgr();
		m_pTextMgr->Init();

		bRetVal = TRUE;
	}
	else if( strCachingType.CompareNoCase(_T("Cached")) == 0
		&& (_tcsicmp(m_wszOptionString, _T("Cached")) != 0 || NULL == m_pTextMgr) )
	{
		if(m_pTextMgr)
		{
			m_pTextMgr->Close();
			delete m_pTextMgr;
		}
		// CachedTextMgr
		m_pTextMgr = new CCachedTextArgMgr();
		m_pTextMgr->Init();

		bRetVal = TRUE;
	}
	else
	{
		bRetVal = TRUE;
	}

	// 적용 성공이면
	if(bRetVal)
	{
		wcscpy_s(m_wszOptionString, MAX_OPTION_LEN, strCachingType);
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineA 대체 함수
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CAutoFinderApp::NewGetGlyphOutlineA(
									HDC hdc,             // handle to device context
									UINT uChar,          // character to query
									UINT uFormat,        // format of data to return
									LPGLYPHMETRICS lpgm, // pointer to structure for metrics
									DWORD cbBuffer,      // size of buffer for data
									LPVOID lpvBuffer,    // pointer to buffer for data
									CONST MAT2 *lpmat2   // pointer to transformation matrix structure
									)
{	
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	else
		return GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}


//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineW 대체 함수
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CAutoFinderApp::NewGetGlyphOutlineW(
									HDC hdc,             // handle to device context
									UINT uChar,          // character to query
									UINT uFormat,        // format of data to return
									LPGLYPHMETRICS lpgm, // pointer to structure for metrics
									DWORD cbBuffer,      // size of buffer for data
									LPVOID lpvBuffer,    // pointer to buffer for data
									CONST MAT2 *lpmat2   // pointer to transformation matrix structure
									)
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	else
		return GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

}



//////////////////////////////////////////////////////////////////////////
// NewTextOutA 대체 함수
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CAutoFinderApp::NewTextOutA(
						   HDC hdc,           // handle to DC
						   int nXStart,       // x-coordinate of starting position
						   int nYStart,       // y-coordinate of starting position
						   LPCSTR lpString,   // character string
						   int cbString       // number of characters
						   )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	else
		return TextOutA(hdc, nXStart, nYStart, lpString, cbString);
}


//////////////////////////////////////////////////////////////////////////
// NewTextOutW 대체 함수
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CAutoFinderApp::NewTextOutW(
						   HDC hdc,           // handle to DC
						   int nXStart,       // x-coordinate of starting position
						   int nYStart,       // y-coordinate of starting position
						   LPCWSTR lpString,   // character string
						   int cbString       // number of characters
						   )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewTextOutW(hdc, nXStart, nYStart, lpString, cbString);
	else
		return TextOutW(hdc, nXStart, nYStart, lpString, cbString);
}



//////////////////////////////////////////////////////////////////////////
// ExtTextOutA 대체 함수
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CAutoFinderApp::NewExtTextOutA(
							  HDC hdc,			// handle to DC
							  int nXStart,		// x-coordinate of reference point
							  int nYStart,		// y-coordinate of reference point
							  UINT fuOptions,		// text-output options
							  CONST RECT* lprc,	// optional dimensions
							  LPCSTR lpString,	// string
							  UINT cbCount,		// number of characters in string
							  CONST INT* lpDx		// array of spacing values
							  )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewExtTextOutA(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbCount, lpDx);
	else
		return ExtTextOutA(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbCount, lpDx);
}

//////////////////////////////////////////////////////////////////////////
// ExtTextOutW 대체 함수
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CAutoFinderApp::NewExtTextOutW(
							  HDC hdc,          // handle to DC
							  int nXStart,            // x-coordinate of reference point
							  int nYStart,            // y-coordinate of reference point
							  UINT fuOptions,   // text-output options
							  CONST RECT* lprc, // optional dimensions
							  LPCWSTR lpString, // string
							  UINT cbCount,     // number of characters in string
							  CONST INT* lpDx   // array of spacing values
							  )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbCount, lpDx);
	else
		return ExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbCount, lpDx);
}

//////////////////////////////////////////////////////////////////////////
// DrawTextA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CAutoFinderApp::NewDrawTextA(
						   HDC hDC,          // handle to DC
						   LPCSTR lpString,  // text to draw
						   int nCount,       // text length
						   LPRECT lpRect,    // formatting dimensions
						   UINT uFormat      // text-drawing options
						   )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	else
		return DrawTextA(hDC, lpString, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
// DrawTextW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CAutoFinderApp::NewDrawTextW(
						   HDC hDC,          // handle to DC
						   LPCWSTR lpString, // text to draw
						   int nCount,       // text length
						   LPRECT lpRect,    // formatting dimensions
						   UINT uFormat      // text-drawing options
						   )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewDrawTextW(hDC, lpString, nCount, lpRect, uFormat);
	else
		return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CAutoFinderApp::NewDrawTextExA(
							 HDC hDC,						// handle to DC
							 LPSTR lpString,				// text to draw
							 int nCount,					// length of text to draw
							 LPRECT lpRect,					// rectangle coordinates
							 UINT uFormat,					// formatting options
							 LPDRAWTEXTPARAMS lpDTParams	// more formatting options
							 )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	else
		return DrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CAutoFinderApp::NewDrawTextExW(
							 HDC hdc,                     // handle to DC
							 LPWSTR lpchText,             // text to draw
							 int cchText,                 // length of text to draw
							 LPRECT lprc,                 // rectangle coordinates
							 UINT dwDTFormat,             // formatting options
							 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
							 )
{
	if(g_Inst && g_Inst->m_pTextMgr)
		return g_Inst->m_pTextMgr->NewDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
	else
		return DrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);
}
