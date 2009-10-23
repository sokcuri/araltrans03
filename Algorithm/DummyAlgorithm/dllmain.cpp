// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.

#ifndef WINVER              
#define WINVER 0x0501 // WinXP
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // WinXP
#endif

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

// Windows 헤더 파일:
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "../../Common/DefStruct.h"	// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include


CONTAINER_PROC_ENTRY gATCTNR3 = {NULL, };
LPWSTR	g_wszPluginOption = NULL;
HWND	g_hAralWnd = NULL;


//////////////////////////////////////////////////////////////////////////
//
// DllMain
//
//////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// GetPluginInfo
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
{
	BOOL bRetVal = FALSE;

	if(sizeof(PLUGIN_INFO) == pPluginInfo->cch)
	{
		// Set Plugin Name
		wcscpy_s(pPluginInfo->wszPluginName, sizeof(pPluginInfo->wszPluginName)/sizeof(wchar_t), L"Dummy Algorithm");
		// Set Plugin Type (Algorithm, Translator)
		wcscpy_s(pPluginInfo->wszPluginType, sizeof(pPluginInfo->wszPluginType)/sizeof(wchar_t), L"Algorithm");

		bRetVal = TRUE;
	}

	return bRetVal;
}




//////////////////////////////////////////////////////////////////////////
//
// OnPluginInit
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption)
{
	BOOL bRetVal = FALSE;

	// Initializing Code
	g_hAralWnd = hAralWnd;
	g_wszPluginOption = wszPluginOption;

	HMODULE hATCTNR3 = GetModuleHandle(_T("ATCTNR3.DLL"));
	if(hATCTNR3 && INVALID_HANDLE_VALUE != hATCTNR3)
	{
		ZeroMemory(&gATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
		gATCTNR3.procHookWin32Api		= (PROC_HookWin32Api) GetProcAddress(hATCTNR3, "HookWin32Api");
		gATCTNR3.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(hATCTNR3, "UnhookWin32Api");
		gATCTNR3.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(hATCTNR3, "HookCodePoint");
		gATCTNR3.procUnhookCodePoint	= (PROC_UnhookCodePoint) GetProcAddress(hATCTNR3, "UnhookCodePoint");
		gATCTNR3.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(hATCTNR3, "CreateTransCtx");
		gATCTNR3.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(hATCTNR3, "DeleteTransCtx");
		gATCTNR3.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(hATCTNR3, "TranslateUsingCtx");
		gATCTNR3.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(hATCTNR3, "IsAppLocaleLoaded");
		gATCTNR3.procSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress(hATCTNR3, "SuspendAllThread");
		gATCTNR3.procResumeAllThread	= (PROC_ResumeAllThread) GetProcAddress(hATCTNR3, "ResumeAllThread");
		gATCTNR3.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(hATCTNR3, "IsAllThreadSuspended");

		// Example of creating a context
		if(gATCTNR3.procCreateTransCtx)
		{
			bRetVal = gATCTNR3.procCreateTransCtx(L"Dummy1");
		}


	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// OnPluginClose
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	BOOL bRetVal = FALSE;

	// Closing Code

	// Example of creating a context
	if(gATCTNR3.procDeleteTransCtx)
	{
		bRetVal = gATCTNR3.procDeleteTransCtx(L"Dummy1");
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
//
// OnPluginOption
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	BOOL bRetVal = FALSE;

	// Show option dialog and Interact with user.
	char arrSrc[256] = {0,};
	char arrTar[256] = {0,};
	strcpy_s(arrSrc, 256, "안녕하세요");
	if(gATCTNR3.procTranslateUsingCtx)
	{
		bRetVal = gATCTNR3.procTranslateUsingCtx(L"Dummy1", arrSrc, 256, arrTar, 256);
		if(bRetVal)
		{
			char arrMsg[1024] = {0,};
			sprintf_s(arrMsg, 1024, "Before : %s\r\nAfter : %s", arrSrc, arrTar);
			::MessageBoxA(g_hAralWnd, arrMsg, "Test OK", MB_OK);
		}
	}


	wcscpy_s(g_wszPluginOption, MAX_OPTION_LEN, L"");
	bRetVal = TRUE;

	return bRetVal;
}
