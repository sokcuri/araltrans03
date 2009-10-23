#pragma once

#define MAX_OPTION_LEN 10240

// PLUGIN_INFO 구조체 (OnPluginInfo 함수 인자로 사용)

struct PLUGIN_INFO
{
	int			cch;				// PLUGIN_INFO 구조체 사이즈
	int			nIconID;			// Icon Resource ID
	wchar_t		wszPluginType[16];	// 플러그인 타입
	wchar_t		wszPluginName[64];	// 플러그인 이름

};


// TRANSLATION_OBJECT 구조체 (번역 컨텍스트의 단위 구성요소)
struct TRANSLATION_OBJECT;
typedef BOOL (__stdcall * PROC_Translate)(TRANSLATION_OBJECT*);

struct TRANSLATION_OBJECT
{
	TRANSLATION_OBJECT* pPrevObject;
	TRANSLATION_OBJECT* pNextObject;
	HMODULE				hPlugin;
	PROC_Translate		procTranslate;
	LPWSTR				wszObjectOption;
	void*				pPreTransBuf;
	size_t				nPreTransBufLen;
	void*				pPostTransBuf;
	size_t				nPostTransBufLen;
	void*				pObjectExtention;

};


// 플러그인 DLL 함수포인터 구조체

typedef BOOL (__stdcall * PROC_GetPluginInfo)(PLUGIN_INFO*);
typedef BOOL (__stdcall * PROC_OnPluginInit)(HWND hAralWnd, LPWSTR wszPluginOption);
typedef BOOL (__stdcall * PROC_OnPluginClose)();
typedef BOOL (__stdcall * PROC_OnPluginOption)();

typedef BOOL (__stdcall * PROC_OnObjectInit)(TRANSLATION_OBJECT* pTransObj);
typedef BOOL (__stdcall * PROC_OnObjectClose)(TRANSLATION_OBJECT* pTransObj);
typedef BOOL (__stdcall * PROC_OnObjectMove)(TRANSLATION_OBJECT* pTransObj);
typedef BOOL (__stdcall * PROC_OnObjectOption)(TRANSLATION_OBJECT* pTransObj);


struct PLUGIN_PROC_ENTRY
{
	PROC_GetPluginInfo			procGetPluginInfo;
	PROC_OnPluginInit			procOnPluginInit;
	PROC_OnPluginClose			procOnPluginClose;
	PROC_OnPluginOption			procOnPluginOption;
	PROC_OnObjectInit			procOnObjectInit;
	PROC_OnObjectClose			procOnObjectClose;
	PROC_OnObjectMove			procOnObjectMove;
	PROC_OnObjectOption			procOnObjectOption;
};



// 콜백 함수형 정의
struct REGISTER_ENTRY
{

	DWORD _EAX;
	DWORD _EBX;
	DWORD _ECX;
	DWORD _EDX;
	DWORD _ESI;
	DWORD _EDI;
	DWORD _EBP;
	DWORD _ESP;
	DWORD _EFL;

};

typedef void (* PROC_HookCallback)(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);


// ATCTNR3 exported functions

typedef BOOL (__stdcall * PROC_HookWin32Api)(LPCWSTR cszDllName, LPCWSTR cszFuncName, LPVOID pfnNew, int nPriority);
typedef BOOL (__stdcall * PROC_UnhookWin32Api)(LPCWSTR cszDllName, LPCWSTR cszFuncName, LPVOID pfnNew);
typedef BOOL (__stdcall * PROC_HookCodePoint)(LPVOID pCodePoint, PROC_HookCallback procCallback, int nPriority);
typedef BOOL (__stdcall * PROC_UnhookCodePoint)(LPVOID pCodePoint, PROC_HookCallback procCallback);
typedef BOOL (__stdcall * PROC_CreateTransCtx)(LPCWSTR cwszContextName);
typedef BOOL (__stdcall * PROC_DeleteTransCtx)(LPCWSTR cwszContextName);
typedef BOOL (__stdcall * PROC_TranslateUsingCtx)(LPCWSTR cwszContextName, LPVOID pSrcData, int nSrcDataLen, LPVOID pTarBuf, int nTarBufLen);
typedef BOOL (__stdcall * PROC_IsAppLocaleLoaded)();
typedef BOOL (__stdcall * PROC_SuspendAllThread)();
typedef BOOL (__stdcall * PROC_ResumeAllThread)();
typedef BOOL (__stdcall * PROC_IsAllThreadSuspended)();

struct CONTAINER_PROC_ENTRY
{
	PROC_HookWin32Api			procHookWin32Api;
	PROC_UnhookWin32Api			procUnhookWin32Api;
	PROC_HookCodePoint			procHookCodePoint;
	PROC_UnhookCodePoint		procUnhookCodePoint;
	PROC_CreateTransCtx			procCreateTransCtx;
	PROC_DeleteTransCtx			procDeleteTransCtx;
	PROC_TranslateUsingCtx		procTranslateUsingCtx;
	PROC_IsAppLocaleLoaded		procIsAppLocaleLoaded;
	PROC_SuspendAllThread		procSuspendAllThread;
	PROC_ResumeAllThread		procResumeAllThread;
	PROC_IsAllThreadSuspended	procIsAllThreadSuspended;
};
