// FontMapper.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//
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
#include "FontMapper.h"
#include "NewAPIs.h"
#include "OptionDlg.h"
#include "CharacterMapper.h"
#include "SettingMgr.h"
#include "resource.h"

HMODULE g_hThisModule = NULL;
HWND	g_hAralWnd = NULL;
LPWSTR	g_wszPluginOption = NULL;
FONT_MAPPER_OPTION g_sMainOption = {0,};
FONT_MAPPER_OPTION g_sTempOption = {0,};
CONTAINER_PROC_ENTRY g_sATCTNR3 = {NULL, };
PROC_WideCharToMultiByte g_pfnOrigWideCharToMultiByte = NULL;
PROC_MultiByteToWideChar g_pfnOrigMultiByteToWideChar = NULL;

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
		g_hThisModule = hModule;
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

	if(sizeof(PLUGIN_INFO) <= pPluginInfo->cch)
	{
		// Set Plugin Icon ID
		//pPluginInfo->nIconID = IDI_ICON2;
		// Set Plugin Name
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"Font Mapper");
		// Set Plugin Type (Algorithm, Translator)
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Filter");
		// Set Download URL
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Filter/FontMapper.zip");

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

	try
	{
		//////////////////////////////////////////////////////////////////////////
		// Initializing Code
		//////////////////////////////////////////////////////////////////////////
		
		g_hAralWnd = hAralWnd;
		if(NULL == g_hAralWnd || INVALID_HANDLE_VALUE == g_hAralWnd)
			throw _T("Invalid AralTrans Window Handle!");

		g_wszPluginOption = wszPluginOption;
		if(NULL == g_wszPluginOption)
			throw _T("Invalid Option String Pointer!");

		// Get AT container
		HMODULE hATCTNR3 = GetModuleHandle(_T("ATCTNR3.DLL"));
		if(NULL == hATCTNR3 || INVALID_HANDLE_VALUE == hATCTNR3)
			throw _T("Can not find ATCTNR3.DLL handle!");

		// Get AT container procedures
		ZeroMemory(&g_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
		g_sATCTNR3.procHookWin32Api			= (PROC_HookWin32Api) GetProcAddress(hATCTNR3, "HookWin32Api");
		g_sATCTNR3.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(hATCTNR3, "UnhookWin32Api");
		g_sATCTNR3.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(hATCTNR3, "HookCodePoint");
		g_sATCTNR3.procUnhookCodePoint		= (PROC_UnhookCodePoint) GetProcAddress(hATCTNR3, "UnhookCodePoint");
		g_sATCTNR3.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(hATCTNR3, "CreateTransCtx");
		g_sATCTNR3.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(hATCTNR3, "DeleteTransCtx");
		g_sATCTNR3.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(hATCTNR3, "TranslateUsingCtx");
		g_sATCTNR3.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(hATCTNR3, "IsAppLocaleLoaded");
		g_sATCTNR3.procSuspendAllThread		= (PROC_SuspendAllThread) GetProcAddress(hATCTNR3, "SuspendAllThread");
		g_sATCTNR3.procResumeAllThread		= (PROC_ResumeAllThread) GetProcAddress(hATCTNR3, "ResumeAllThread");
		g_sATCTNR3.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(hATCTNR3, "IsAllThreadSuspended");

		if(!(g_sATCTNR3.procHookWin32Api && g_sATCTNR3.procUnhookWin32Api
			&& g_sATCTNR3.procHookCodePoint && g_sATCTNR3.procUnhookCodePoint
			&& g_sATCTNR3.procCreateTransCtx && g_sATCTNR3.procDeleteTransCtx
			&& g_sATCTNR3.procTranslateUsingCtx && g_sATCTNR3.procIsAppLocaleLoaded
			&& g_sATCTNR3.procSuspendAllThread && g_sATCTNR3.procResumeAllThread
			&& g_sATCTNR3.procIsAllThreadSuspended))
			throw _T("Failed to get container procedures!");

		// Suspend All Threads
		g_sATCTNR3.procSuspendAllThread();

		// Hook Font API
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.dll", L"CreateFontA", NewCreateFontA, 10 ) == FALSE )
			throw _T("Failed to hook 'CreateFontA'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.dll", L"CreateFontW", NewCreateFontW, 10 ) == FALSE )
			throw _T("Failed to hook 'CreateFontW'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.dll", L"CreateFontIndirectA", NewCreateFontIndirectA, 10 ) == FALSE )
			throw _T("Failed to hook 'CreateFontIndirectA'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.dll", L"CreateFontIndirectW", NewCreateFontIndirectW, 10 ) == FALSE )
			throw _T("Failed to hook 'CreateFontIndirectW'!");

		// Hook Drawing API
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA, 10 ) == FALSE )
			throw _T("Failed to hook 'GetGlyphOutlineA'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW, 10 ) == FALSE )
			throw _T("Failed to hook 'GetGlyphOutlineW'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA, 10 ) == FALSE )
			throw _T("Failed to hook 'TextOutA'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW, 10 ) == FALSE )
			throw _T("Failed to hook 'TextOutW'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA, 10 ) == FALSE )
			throw _T("Failed to hook 'ExtTextOutA'!");
		if( g_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW, 10 ) == FALSE )
			throw _T("Failed to hook 'ExtTextOutW'!");

		if( g_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA, 10 ) == FALSE )
			throw _T("Failed to hook 'DrawTextA'!");
		if( g_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW, 10 ) == FALSE )
			throw _T("Failed to hook 'DrawTextW'!");
		if( g_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA, 10 ) == FALSE )
			throw _T("Failed to hook 'DrawTextExA'!");
		if( g_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW, 10 ) == FALSE )
			throw _T("Failed to hook 'DrawTextExW'!");

		// Prevent Applocale Redirection
		HKEY hCategoryKey = HKEY_CURRENT_USER;
		HKEY hKey = NULL;
		LONG lRet = RegOpenKeyEx(hCategoryKey, _T("Software\\AralGood"), 0, KEY_READ, &hKey);

		// 키를 여는데 성공했다면
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


		// 옵션 스트링 파싱
		g_wszPluginOption = wszPluginOption;

		// 모든 쓰레드 재가동
		g_sATCTNR3.procResumeAllThread();

		bRetVal = TRUE;

	}
	catch (LPCTSTR strErr)
	{
		::MessageBox(NULL, strErr, _T("Font Mapper"), MB_OK | MB_TOPMOST);
		OnPluginClose();
	}


	if( bRetVal == TRUE )
	{
		// for Test
		//wcscpy_s(g_wszPluginOption, MAX_OPTION_LEN, L"<?xml version=\"1.0\" standalone=yes><FontMapperSetting><ProgramCodePage>932</ProgramCodePage><CharacterEncoding>True</CharacterEncoding></FontMapperSetting>");

		// 옵션 적용
		if(CSettingMgr::XmlToOption(g_wszPluginOption, &g_sMainOption) == TRUE)
			ApplyOption(&g_sMainOption);
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
	if(NULL == g_hAralWnd) return FALSE;

	//////////////////////////////////////////////////////////////////////////
	// Closing Code
	//////////////////////////////////////////////////////////////////////////
	BOOL bRetVal = FALSE;

	// Suspend All Threads
	g_sATCTNR3.procSuspendAllThread();

	// Unhook Drawing API
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW );
	g_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA );
	g_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW );
	g_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA );
	g_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW );

	// Unhook Font API
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.dll", L"CreateFontA", NewCreateFontA );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.dll", L"CreateFontW", NewCreateFontW );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.dll", L"CreateFontIndirectA", NewCreateFontIndirectA );
	g_sATCTNR3.procUnhookWin32Api( L"GDI32.dll", L"CreateFontIndirectW", NewCreateFontIndirectW );

	// 기타 변수 리셋
	g_hAralWnd = NULL;
	g_wszPluginOption = NULL;

	// 모든 쓰레드 재가동
	g_sATCTNR3.procResumeAllThread();

	bRetVal = TRUE;

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
	if (g_hAralWnd && IsWindow(g_hAralWnd))
	{
		memcpy(&g_sTempOption, &g_sMainOption, sizeof(FONT_MAPPER_OPTION));
		if (DialogBox((HINSTANCE)g_hThisModule, MAKEINTRESOURCE(IDD_OPTION_DLG), g_hAralWnd, OptionDialogProc) == IDOK)
		{
			if(ApplyOption(&g_sTempOption) == TRUE)
			{
				memcpy(&g_sMainOption, &g_sTempOption, sizeof(FONT_MAPPER_OPTION));
				CSettingMgr::OptionToXml(&g_sMainOption, g_wszPluginOption, MAX_OPTION_LEN);
				bRetVal = TRUE;
			}
		}

	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// OnObjectInit
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj)
{
	if(NULL == pTransObj) return FALSE;

	// Set the translation function pointer
	pTransObj->procTranslate = Encode;

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
		delete [] (char*)pTransObj->pPreTransBuf;
		pTransObj->pPreTransBuf = NULL;
	}
	pTransObj->nPreTransBufLen = 0;

	// Delete Post-translation buffer
	if(pTransObj->pPostTransBuf)
	{
		delete [] (char*)pTransObj->pPostTransBuf;
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


//////////////////////////////////////////////////////////////////////////
//
// Main Translation Function
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall Encode(TRANSLATION_OBJECT* pTransObject)
{
	BOOL bRetVal = FALSE;

	// Check translation object is NULL
	if(NULL == pTransObject) return FALSE;

	TRANSLATION_OBJECT* pPrevObject = pTransObject->pPrevObject;
	TRANSLATION_OBJECT* pNextObject = pTransObject->pNextObject;

	// Check the previous object is NULL
	if(NULL == pPrevObject) return FALSE;
	// Check pre-translation buffer of the previous object is NULL
	if(NULL == pPrevObject->pPreTransBuf || 0 == pPrevObject->nPreTransBufLen) return FALSE;

	// Create text buffer as double size of pre-translation buffer
	if(pTransObject->pPreTransBuf)
	{
		delete [] (char*)pTransObject->pPreTransBuf;
		pTransObject->pPreTransBuf = NULL;
		pTransObject->nPreTransBufLen = 0;
	}

	pTransObject->pPreTransBuf = new char[pPrevObject->nPreTransBufLen];
	pTransObject->nPreTransBufLen = pPrevObject->nPreTransBufLen;

	//////////////////////////////////////////////////////////////////////////
	//
	// Pre-translation
	//
	memcpy_s(pTransObject->pPreTransBuf, 
		pTransObject->nPreTransBufLen, 
		pPrevObject->pPreTransBuf,
		pPrevObject->nPreTransBufLen);
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
	if(NULL == pNextObject->pPostTransBuf || 0 == pNextObject->nPostTransBufLen) return FALSE;

	// Create text buffer as double size of pre-translation buffer
	if(pTransObject->pPostTransBuf)
	{
		delete [] (char*)pTransObject->pPostTransBuf;
		pTransObject->pPostTransBuf = NULL;
		pTransObject->nPostTransBufLen = 0;
	}

	pTransObject->pPostTransBuf = new char[pNextObject->nPostTransBufLen];
	pTransObject->nPostTransBufLen = pNextObject->nPostTransBufLen;

	//////////////////////////////////////////////////////////////////////////
	//
	// Post-translation
	//
	memcpy_s(pTransObject->pPostTransBuf, 
		pTransObject->nPostTransBufLen, 
		pNextObject->pPostTransBuf,
		pNextObject->nPostTransBufLen);
	bRetVal = EncodeMultiBytes((LPSTR)pTransObject->pPostTransBuf);
	//
	// End of Post-translation
	//
	//////////////////////////////////////////////////////////////////////////


	return bRetVal;
}


BOOL EncodeMultiBytes( LPSTR szNewText )
{

	// 멀티바이트 인코딩
	int len = (int)strlen(szNewText);
	int i = 0;
	while( i<len )
	{
		if( 0x80 <= (BYTE)szNewText[i] )
		{
			char tmpbuf[3];
			tmpbuf[0] = szNewText[i];
			tmpbuf[1] = szNewText[i+1];
			tmpbuf[2] = '\0';

			CCharacterMapper::EncodeK2J(tmpbuf, &szNewText[i]);

			i++;
		}

		i++;
	}

	szNewText[i] = '\0';

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// ApplyOption
//
//////////////////////////////////////////////////////////////////////////
BOOL ApplyOption(FONT_MAPPER_OPTION* pOption)
{
	BOOL bRetVal = FALSE;

	if(pOption)
	{
		bRetVal = TRUE;
	}

	return bRetVal;
}


#ifdef _DEBUG
void TmpTrace(LPCTSTR format, ...)
{
	TCHAR MsgBuf[10240];
	va_list arglist;
	va_start(arglist, format);
	wsprintf(MsgBuf, format, arglist);
	va_end(arglist);

	OutputDebugString(MsgBuf);
}
#endif