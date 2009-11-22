// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.

#ifndef WINVER              
#define WINVER 0x0501		// WinXP
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // WinXP
#endif

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

// Windows 헤더 파일:
#include <windows.h>
#include <tchar.h>
#include "../../Common/DefStruct.h"		// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include
#include "resource.h"
#include <stdlib.h>

#define TRANS_BUF_LEN 1024

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
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		ul_reason_for_call = ul_reason_for_call;
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

	if(sizeof(PLUGIN_INFO) <= pPluginInfo->cch)
	{
		// Set Plugin Icon ID
		pPluginInfo->nIconID = IDI_ICON2;
		// Set Plugin Name
		wcscpy_s(pPluginInfo->wszPluginName, sizeof(pPluginInfo->wszPluginName)/sizeof(wchar_t), L"Dummy Filter");
		// Set Plugin Type (Algorithm, Translator)
		wcscpy_s(pPluginInfo->wszPluginType, sizeof(pPluginInfo->wszPluginType)/sizeof(wchar_t), L"Filter");

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

		bRetVal = TRUE;
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
	wcscpy_s(g_wszPluginOption, MAX_OPTION_LEN, L"");
	bRetVal = TRUE;

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// Main Translation Function
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall DummyTranslate(TRANSLATION_OBJECT* pTransObject)
{
	// Check the object is NULL
	if(NULL == pTransObject) return FALSE;
	// Check pre-translation buffer is NULL
	if(NULL == pTransObject->pPreTransBuf) return FALSE;

	TRANSLATION_OBJECT* pPrevObject = pTransObject->pPrevObject;
	TRANSLATION_OBJECT* pNextObject = pTransObject->pNextObject;
	
	// Check the previous object is NULL
	if(NULL == pPrevObject) return FALSE;
	// Check pre-translation buffer of the previous object is NULL
	if(NULL == pPrevObject->pPreTransBuf) return FALSE;
	// Check pre-translation buffer size is enough
	if(pTransObject->nPreTransBufLen < pPrevObject->nPreTransBufLen) return FALSE;


	//////////////////////////////////////////////////////////////////////////
	//
	// Pre-translation
	//
	ZeroMemory(pTransObject->pPreTransBuf, pTransObject->nPreTransBufLen);
	memcpy_s(pTransObject->pPreTransBuf, 
			pTransObject->nPreTransBufLen, 
			pPrevObject->pPreTransBuf,
			pPrevObject->nPreTransBufLen);
	strcat_s((char*)pTransObject->pPreTransBuf, pTransObject->nPreTransBufLen, ", pre");
	//
	// End of Pre-translation
	//
	//////////////////////////////////////////////////////////////////////////
	

	// Check the next object is NULL
	if(NULL == pNextObject) return FALSE;
	// Check the next function is available
	if(NULL == pNextObject->procTranslate) return FALSE;

	// * Call next translation function
	BOOL bRes = pNextObject->procTranslate(pTransObject->pNextObject);
	if(FALSE == bRes) return FALSE;

	// Check post-translation buffer of the next object is NULL
	if(NULL == pNextObject->pPostTransBuf) return FALSE;
	// Check post-translation buffer size is enough
	if(pTransObject->nPostTransBufLen < pNextObject->nPostTransBufLen) return FALSE;

	//////////////////////////////////////////////////////////////////////////
	//
	// Post-translation
	//
	ZeroMemory(pTransObject->pPostTransBuf, pTransObject->nPostTransBufLen);
	
	memcpy_s(pTransObject->pPostTransBuf, 
		pTransObject->nPostTransBufLen, 
		pNextObject->pPostTransBuf,
		pNextObject->nPostTransBufLen);
	strcat_s((char*)pTransObject->pPostTransBuf, pTransObject->nPostTransBufLen, ", post");

	//
	// End of Post-translation
	//
	//////////////////////////////////////////////////////////////////////////

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//
// OnObjectInit
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	if(NULL == pTransObj) return FALSE;

	// Create Pre-translation buffer
	pTransObj->pPreTransBuf = malloc(TRANS_BUF_LEN);
	pTransObj->nPreTransBufLen = TRANS_BUF_LEN;
	ZeroMemory(pTransObj->pPreTransBuf, pTransObj->nPreTransBufLen);

	// Create Post-translation buffer
	pTransObj->pPostTransBuf = malloc(TRANS_BUF_LEN);
	pTransObj->nPostTransBufLen = TRANS_BUF_LEN;
	ZeroMemory(pTransObj->pPostTransBuf, pTransObj->nPostTransBufLen);

	// Set the translation function pointer
	pTransObj->procTranslate = DummyTranslate;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// OnObjectClose
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj)
{
	if(NULL == pTransObj) return FALSE;

	// Delete Pre-translation buffer
	if(pTransObj->pPreTransBuf)
	{
		free(pTransObj->pPreTransBuf);
		pTransObj->pPreTransBuf = NULL;
	}
	pTransObj->nPreTransBufLen = 0;
	
	// Delete Post-translation buffer
	if(pTransObj->pPostTransBuf)
	{
		free(pTransObj->pPostTransBuf);
		pTransObj->pPostTransBuf = NULL;
	}
	pTransObj->nPostTransBufLen = 0;

	// Reset the translation function pointer
	pTransObj->procTranslate = NULL;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// OnObjectMove
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj)
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// OnObjectOption
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj)
{
	return TRUE;
}

