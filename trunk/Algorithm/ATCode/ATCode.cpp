// ATCode.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ATCode.h"
#include "ATCodeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CATCodeMgr g_objATCodeMgr;

extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
{
	BOOL bRetVal = FALSE;

	if(pPluginInfo && pPluginInfo->cch >= sizeof(PLUGIN_INFO))
	{
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"AT Code for AralTrans 0.3");
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Algorithm");
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Algorithm/ATCode.zip");
		bRetVal = TRUE;
	}
	return bRetVal;
}


extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption)
{
	return g_objATCodeMgr.Init(hAralWnd, wszPluginOption);
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return g_objATCodeMgr.Option();
}

extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return g_objATCodeMgr.Close();
}



/*
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

	if( g_objATCodeMgr.m_sTextFunc.pfnOrigWideCharToMultiByte )
	{
		nRetVal = g_objATCodeMgr.m_sTextFunc.pfnOrigWideCharToMultiByte(
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

	if( g_objATCodeMgr.m_sTextFunc.pfnOrigMultiByteToWideChar )
	{
		nRetVal = g_objATCodeMgr.m_sTextFunc.pfnOrigMultiByteToWideChar(
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
*/
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CATCodeApp

BEGIN_MESSAGE_MAP(CATCodeApp, CWinApp)
END_MESSAGE_MAP()


// CATCodeApp construction

CATCodeApp::CATCodeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CATCodeApp object

CATCodeApp theApp;


// CATCodeApp initialization

BOOL CATCodeApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CATCodeApp::ExitInstance()
{
	TRACE(_T("CATCodeApp::ExitInstance(() \n"));

	g_objATCodeMgr.Close();

	return CWinApp::ExitInstance();
}
