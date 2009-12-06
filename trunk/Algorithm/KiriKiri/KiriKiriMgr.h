#pragma once

#include <list>
#include <map>
#include "OptionMgr.h"
#include <Tlhelp32.h>

using namespace std;


// 스트링관련 오리지널 함수 엔트리
typedef DWORD (_stdcall * PROC_GetGlyphOutline)(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2*);
typedef BOOL (_stdcall * PROC_TextOut)(HDC, int, int, LPCVOID, int);
typedef BOOL (_stdcall * PROC_ExtTextOut)(HDC, int, int, UINT, CONST RECT*, LPCVOID, UINT, CONST INT*);
typedef int (__stdcall * PROC_DrawText)(HDC, LPCVOID, int, LPRECT, UINT);
typedef int (__stdcall * PROC_DrawTextEx)(HDC, LPCVOID ,int ,LPRECT, UINT, LPDRAWTEXTPARAMS);

typedef int (_stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (_stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);
typedef HMODULE (_stdcall * PROC_LoadLibrary)(LPCVOID);

typedef HFONT (_stdcall * PROC_CreateFontIndirect)(LPVOID lpLogFont);
typedef HFONT (_stdcall * PROC_CreateFont)(
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
	PROC_ExtTextOut				pfnExtTextOutW;
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


class CMainDbgDlg;
class CHookPoint;

class CATCodeMgr
{
private:
	static CATCodeMgr* _Inst;

	BOOL				m_bRunning;
	HMODULE				m_hContainer;
	HWND				m_hContainerWnd;
	LPSTR				m_szOptionString;

	PROC_LoadLibrary	m_pfnLoadLibraryA;
	PROC_LoadLibrary	m_pfnLoadLibraryW;
	
	CString				m_strLastFontFace;
	map<long, HFONT>	m_mapFonts;
	list<COptionNode*>	m_listRetryHook;
	COptionNode			m_optionRoot;

	// 클립보드 처리 관련
	BOOL				m_bRunClipboardThread;
	HANDLE				m_hClipboardThread;
	HANDLE				m_hClipTextChangeEvent;
	CRITICAL_SECTION	m_csClipText;
	CString				m_strClipText;


	HFONT	CheckFont(HDC hdc);
	void	ResetOption();
	BOOL	AdjustOption(COptionNode* pRootNode);
	BOOL	HookFromOptionNode(COptionNode* pRootNode);
	
	HFONT	InnerCreateFontIndirect(LOGFONTW* lplf);
	HFONT	InnerCreateFont(
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
		LPWSTR lpszFace           // typeface name
		);
	

	//////////////////////////////////////////////////////////////////////////
	// Static Functions

	static UINT __stdcall ClipboardThreadFunc(LPVOID pParam);

	static HMODULE __stdcall NewLoadLibraryA(LPCSTR lpFileName);
	static HMODULE __stdcall NewLoadLibraryW(LPCWSTR lpFileName);
	
	static DWORD __stdcall NewGetGlyphOutlineA(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static DWORD __stdcall NewGetGlyphOutlineW(
		HDC hdc,             // handle to device context
		UINT uChar,          // character to query
		UINT uFormat,        // format of data to return
		LPGLYPHMETRICS lpgm, // pointer to structure for metrics
		DWORD cbBuffer,      // size of buffer for data
		LPVOID lpvBuffer,    // pointer to buffer for data
		CONST MAT2 *lpmat2   // pointer to transformation matrix structure
		);

	static BOOL __stdcall NewTextOutA(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewTextOutW(
		HDC hdc,           // handle to DC
		int nXStart,       // x-coordinate of starting position
		int nYStart,       // y-coordinate of starting position
		LPCWSTR lpString,   // character string
		int cbString       // number of characters
		);

	static BOOL __stdcall NewExtTextOutA(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	static BOOL __stdcall NewExtTextOutW(
		HDC hdc,          // handle to DC
		int X,            // x-coordinate of reference point
		int Y,            // y-coordinate of reference point
		UINT fuOptions,   // text-output options
		CONST RECT* lprc, // optional dimensions
		LPCWSTR lpString, // string
		UINT cbCount,     // number of characters in string
		CONST INT* lpDx   // array of spacing values
		);

	static int __stdcall NewDrawTextA(
		HDC hDC,          // handle to DC
		LPCSTR lpString,  // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	static int __stdcall NewDrawTextW(
		HDC hDC,          // handle to DC
		LPCWSTR lpString, // text to draw
		int nCount,       // text length
		LPRECT lpRect,    // formatting dimensions
		UINT uFormat      // text-drawing options
		);

	static int __stdcall NewDrawTextExA(
		HDC hdc,                     // handle to DC
		LPSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	static int __stdcall NewDrawTextExW(
		HDC hdc,                     // handle to DC
		LPWSTR lpchText,              // text to draw
		int cchText,                 // length of text to draw
		LPRECT lprc,                 // rectangle coordinates
		UINT dwDTFormat,             // formatting options
		LPDRAWTEXTPARAMS lpDTParams  // more formatting options
		);

	static HFONT __stdcall NewCreateFontA(
		int nHeight,               
		int nWidth,                
		int nEscapement,           
		int nOrientation,          
		int fnWeight,              
		DWORD fdwItalic,           
		DWORD fdwUnderline,        
		DWORD fdwStrikeOut,        
		DWORD fdwCharSet,          
		DWORD fdwOutputPrecision,  
		DWORD fdwClipPrecision,    
		DWORD fdwQuality,          
		DWORD fdwPitchAndFamily,   
		LPSTR lpszFace           
		);

	static HFONT __stdcall NewCreateFontW(
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
		LPWSTR lpszFace           // typeface name
		);

	static HFONT __stdcall NewCreateFontIndirectA(LOGFONTA* lplf);

	static HFONT __stdcall NewCreateFontIndirectW(LOGFONTW* lplf);

public:
	//BOOL				m_bForceFontLoad;
	int					m_nFontLoadLevel;
	BOOL				m_bFixedFontSize;
	long				m_lFontSize;
	CString				m_strFontFace;
	BOOL				m_bEncodeKorean;

	CONTAINER_FUNCTION_ENTRY	m_sContainerFunc;
	TEXT_FUNCTION_ENTRY			m_sTextFunc;
	FONT_FUNCTION_ENTRY			m_sFontFunc;
	list<CHookPoint*>			m_listHookPoint;

	CATCodeMgr(void);
	~CATCodeMgr(void);

	static CATCodeMgr* GetInstance();
	static int GetAllLoadedModules(PMODULEENTRY32 pRetBuf, int maxCnt);

	BOOL MigrateOption(COptionNode* pRootNode);
	BOOL ApplyOption(COptionNode* pRootNode);
	BOOL SetClipboardText(LPCTSTR cszText);

	BOOL Init(HWND hSettingWnd, LPSTR cszOptionStringBuffer);
	BOOL Start();
	BOOL Stop();
	BOOL Option();
	BOOL Close();

};
