#pragma once

#include "../../Common/DefStruct.h"
#include "OptionMgr.h"
#include <list>
#include <map>
//#include "Win32UIMgr.h"
#include <Tlhelp32.h>

using namespace std;

typedef HMODULE (__stdcall * PROC_LoadLibrary)(LPCVOID);

/*
// 스트링관련 오리지널 함수 엔트리
typedef DWORD (__stdcall * PROC_GetGlyphOutline)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*);
typedef BOOL (__stdcall * PROC_TextOut)(HDC, int, int, LPCVOID, int);
typedef BOOL (__stdcall * PROC_ExtTextOut)(HDC, int, int, UINT, CONST RECT*, LPCVOID, UINT, CONST INT*);
typedef int (__stdcall * PROC_DrawText)(HDC, LPCVOID, int, LPRECT, UINT);
typedef int (__stdcall * PROC_DrawTextEx)(HDC, LPCVOID ,int ,LPRECT, UINT, LPDRAWTEXTPARAMS);

typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);

typedef HFONT (__stdcall * PROC_CreateFontIndirect)(LPVOID lpLogFont);
typedef HFONT (__stdcall * PROC_CreateFont)(
				 int nHeight,               // height of font
				 int nWidth,                // average character width
				 int nEscapement,           // angle of escapement
				 int nOrientation,          // base-line orientation angle
				 int fnWeight,              // font weight
				 DWORD fdwItalic,           // italic attribute option
				 DWORD fdwUnderline,        // underline attribute option
				 DWORD fdwStrikeOut,        // strikeout attribute option
				 DWORD fdwCharSet,          // character set identifier
				 DWORD fdwOutputPrecision,  // output precision
				 DWORD fdwClipPrecision,    // clipping precision
				 DWORD fdwQuality,          // output quality
				 DWORD fdwPitchAndFamily,   // pitch and family
				 LPVOID lpszFace           // typeface name
				 );

typedef struct _TEXT_FUNCTION_ENTRY
{

	PROC_GetGlyphOutline		pfnGetGlyphOutlineA;
	PROC_GetGlyphOutline		pfnGetGlyphOutlineW;
	PROC_TextOut				pfnTextOutA;
	PROC_TextOut				pfnTextOutW;
	PROC_ExtTextOut				pfnExtTextOutA;
	//PROC_ExtTextOut				pfnExtTextOutW;
	PROC_WideCharToMultiByte	pfnOrigWideCharToMultiByte;
	PROC_MultiByteToWideChar	pfnOrigMultiByteToWideChar;
	PROC_DrawText				pfnDrawTextA;
	PROC_DrawText				pfnDrawTextW;
	PROC_DrawTextEx				pfnDrawTextExA;
	PROC_DrawTextEx				pfnDrawTextExW;

} TEXT_FUNCTION_ENTRY, *PTEXT_FUNCTION_ENTRY;


typedef struct _FONT_FUNCTION_ENTRY
{

	PROC_CreateFont		pfnCreateFontA;
	PROC_CreateFont		pfnCreateFontW;
	PROC_CreateFontIndirect pfnCreateFontIndirectA;
	PROC_CreateFontIndirect pfnCreateFontIndirectW;

} FONT_FUNCTION_ENTRY, *PFONT_FUNCTION_ENTRY;
*/

//class CMainDbgDlg;
class CHookPoint;

class CATCodeMgr
{
private:
	static CATCodeMgr* _Inst;

	//PROC_LoadLibrary	m_pfnLoadLibraryA;
	//PROC_LoadLibrary	m_pfnLoadLibraryW;

	HMODULE				m_hContainer;
	HWND				m_hContainerWnd;
	LPWSTR				m_wszOptionString;	
	list<COptionNode*>	m_listRetryHook;
	COptionNode			m_optionRoot;

	// 클립보드 처리 관련
	BOOL				m_bRunClipboardThread;
	HANDLE				m_hClipboardThread;
	HANDLE				m_hClipTextChangeEvent;
	CRITICAL_SECTION	m_csClipText;
	CString				m_strClipText;

	void	ResetOption();
	BOOL	AdjustOption(COptionNode* pRootNode);
	BOOL	HookFromOptionNode(COptionNode* pRootNode);
	

	//////////////////////////////////////////////////////////////////////////
	// Static Functions

	static UINT __stdcall ClipboardThreadFunc(LPVOID pParam);

	static HMODULE __stdcall NewLoadLibraryA(LPCSTR lpFileName);
	static HMODULE __stdcall NewLoadLibraryW(LPCWSTR lpFileName);

public:

	CONTAINER_PROC_ENTRY	m_sContainerFunc;
	list<CHookPoint*>		m_listHookPoint;

	CATCodeMgr(void);
	~CATCodeMgr(void);

	static CATCodeMgr* GetInstance();
	static int GetAllLoadedModules(PMODULEENTRY32 pRetBuf, int maxCnt);

	BOOL MigrateOption(COptionNode* pRootNode);
	BOOL ApplyOption(COptionNode* pRootNode);
	BOOL SetClipboardText(LPCTSTR cszText);

	BOOL Init(HWND hAralWnd, LPWSTR wszPluginOption);
	BOOL Option();
	BOOL Close();

};
