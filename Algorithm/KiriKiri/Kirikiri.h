// Kirikiri.h : Kirikiri DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "OptionMgr.h"
#include "KAGScriptMgr.h"
#include "../../Common/DefStruct.h"	// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include

using namespace std;

// 스트링관련 오리지널 함수 엔트리
typedef DWORD (__stdcall * PROC_GetGlyphOutline)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*);
typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);

typedef struct _TEXT_FUNCTION_ENTRY
{
	PROC_WideCharToMultiByte	pfnOrigWideCharToMultiByte;
	PROC_MultiByteToWideChar	pfnOrigMultiByteToWideChar;

} TEXT_FUNCTION_ENTRY, *PTEXT_FUNCTION_ENTRY;


// CKirikiriApp
// 이 클래스의 구현을 보려면 Kirikiri.cpp를 참조하십시오.
//

class CKirikiriApp : public CWinApp
{
private:
	HMODULE				m_hContainer;
	HWND				m_hContainerWnd;
	LPWSTR				m_wszOptionString;
	COptionNode			m_optionRoot;

	UINT_PTR			m_pCodePoint;
	BYTE				m_byteRegister;
	LPCWSTR				m_cwszOrigScript;
	LPWSTR				m_wszScriptBuf;
	CKAGScriptMgr2		m_ScriptMgr;
	
	BOOL				m_bUseCodePoint2;
	int					m_nCodePoint2Type;
	UINT_PTR			m_pCodePoint2;

	void ResetOption();
	BOOL AdjustOption(COptionNode* pRootNode);
	BOOL HookKirikiri();
	BOOL HookKirikiri2();
	void UnhookKiriKiri2();
	void OnScriptLoad(REGISTER_ENTRY* pRegisters);
	void OnArrayLoad(REGISTER_ENTRY* pRegisters);

	static void PointCallback(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);

public:
	CONTAINER_PROC_ENTRY		m_sATCTNR3;
	TEXT_FUNCTION_ENTRY			m_sTextFunc;

	CKirikiriApp();
	~CKirikiriApp();

	BOOL			ApplyOption( COptionNode* pRootNode );
	BOOL			ClearCache();
	CKAGScriptMgr*	GetKAGScriptMgr(){ return &m_ScriptMgr; };
	LPCWSTR			GetOriginalScript(){ return m_cwszOrigScript; };
	LPWSTR			GetScriptBuffer(){ return m_wszScriptBuf; };


	BOOL Init(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL Option();
	BOOL Close();

	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

	DECLARE_MESSAGE_MAP()
};
