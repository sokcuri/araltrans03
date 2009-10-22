// ezTransXP.h : main header file for the ezTransXP DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "../../DefStruct.h"
#include <string>

#define EZTR_INIT_STR "CSUSER123455"

using namespace std;

// Export Functions
/*
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj);
*/

// CTextElement 클래스
class CTextElement
{
public:
	
	CTextElement*	pPrevLink;
	CTextElement*	pNextLink;
	UINT			dwHash;
	string			strTranslatedText;

	CTextElement() : pPrevLink(NULL), pNextLink(NULL), dwHash(0x00000000) {}
};


// CezTransXPApp
// See ezTransXP.cpp for the implementation of this class
//

class CezTransXPApp : public CWinApp
{
	struct
	{
		HMODULE hJKMod;
		int (__stdcall * J2K_InitializeEx)(const char* initStr, const char* homeDir);
		char* (__stdcall * J2K_TranslateMMNT)(int data0, const char *jpStr);
		int (__stdcall * J2K_FreeMem)(char *krStr);
		int (__stdcall * J2K_StopTranslation)(int data0);
		int (__stdcall * J2K_Terminate)(void);
	} m_EZDLL;

private:
	HWND	m_hAralWnd;
	LPWSTR	m_wszPluginOption;
	CONTAINER_PROC_ENTRY m_sATCTNR3;
	
	//BOOL	m_bRemoveTrace;
	//BOOL	m_bRemoveDupSpace;
	HANDLE	m_hTransThread;
	HANDLE	m_hRequestEvent;
	HANDLE	m_hResponseEvent;
	TRANSLATION_OBJECT* m_pCurTransObj;
	//LPCSTR	m_pJpnText;
	//LPSTR		m_pKorText;
	//int		m_nBufSize;
	CString m_strHomeDir;
	CString m_strErrorMsg;
	CRITICAL_SECTION m_csTrans;


	CString GetEZTransHomeDir();	// 이지트랜스 홈 디랙토리
	void	InitEZTrans();
	void	CloseEZTrans();
	static unsigned int __stdcall TransThreadFunc(void* pParam);
	void	EncodeTwoByte(LPCSTR cszJpnSrc, LPSTR szJpnTar);
	void	EncodeTrace(LPCSTR cszJpnSrc, LPSTR szJpnTar);
	void	FilterTrace(LPCSTR cszKorSrc, LPSTR szKorTar);
	void	FilterDupSpaces(LPCSTR cszKorSrc, LPSTR szKorTar);
	void	DecodeTrace(LPCSTR cszKorSrc, LPSTR szKorTar);
	void	DecodeTwoByte(LPCSTR cszKorSrc, LPSTR szKorTar);
	size_t	GetEncodedLen(LPCSTR cszBytes);

public:
	CezTransXPApp();

	BOOL OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL OnPluginOption();
	BOOL OnPluginClose();
	BOOL OnObjectInit(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectClose(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectMove(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectOption(TRANSLATION_OBJECT* pTransObj);
	BOOL TranslateJ2K(TRANSLATION_OBJECT* pTransObject);

	// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
