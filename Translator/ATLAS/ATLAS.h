// ATLAS.h : ATLAS DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// main symbols
#include "ATLASMgr.h"
#include <string>
#include <map>
#include "../../Common/DefStruct.h"

#define EZTR_INIT_STR "CSUSER123455"

using namespace std;

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


// CATLASApp
// See ezTransXP.cpp for the implementation of this class
//

class CATLASApp : public CWinApp
{
private:

	// For communication with ATLCLIP.exe process
	HANDLE	m_hMapFile;
	HANDLE	m_hTransReadyEvent;
	HANDLE	m_hRequestEvent;
	HANDLE	m_hResponseEvent;

	BOOL	m_bRemoveTrace;
	HANDLE	m_hATLCLIPProcess;
	HHOOK	m_hATLASHook;
	CString m_strProcessName;
	CString m_strHomeDir;
	CString	m_strErrorMsg;
	map<UINT, CTextElement*> m_mapCache;
	CTextElement m_CacheHead;
	CTextElement m_CacheTail;
	CRITICAL_SECTION m_csTrans;

	CATLASMgr m_ATALSMgr;		// used in ATLCLIP.exe process context

	CString GetATLASHomeDir();	// Get ATLAS directory
	BOOL	InitATLAS();
	void	CloseATLAS();
	static unsigned int __stdcall TransThreadFunc(void* pParam);
	void	EncodeTrace(LPCSTR cszJpnSrc, LPSTR szJpnTar);
	void	FilterTrace(LPCSTR cszKorSrc, LPSTR szKorTar);
	void	FilterDupSpaces(LPCSTR cszKorSrc, LPSTR szKorTar);
	void	DecodeTrace(LPCSTR cszKorSrc, LPSTR szKorTar);
	BOOL	TranslateUsingATLAS(LPCSTR cszJapanese, LPSTR szKorean, int nBufSize);

public:
	CATLASApp();

	BOOL OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL OnPluginOption();
	BOOL OnPluginClose();
	BOOL OnObjectInit(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectClose(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectMove(TRANSLATION_OBJECT* pTransObj);
	BOOL OnObjectOption(TRANSLATION_OBJECT* pTransObj);
	BOOL TranslateJ2E(TRANSLATION_OBJECT* pTransObject);

	// Overrides
public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();


	DECLARE_MESSAGE_MAP()
};

