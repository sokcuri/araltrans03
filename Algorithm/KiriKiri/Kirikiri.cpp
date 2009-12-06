// Kirikiri.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

//#pragma warning(disable:4312)
//#pragma warning(disable:4313)
//#pragma warning(disable:4996)
//#pragma warning(disable:4101)

#include "stdafx.h"
#include "Kirikiri.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "BinaryPatternSearcher.h"
#include "KAGScriptMgr.h"
#include "TransProgressDlg.h"
#include "KirikiriOptionDlg.h"
#include <Psapi.h>

#pragma comment (lib, "psapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// The one and only CKirikiriApp object

CKirikiriApp theApp;


//////////////////////////////////////////////////////////////////////////
//
// GetPluginInfo
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo)
{
	BOOL bRetVal = FALSE;

	if(pPluginInfo && pPluginInfo->cch >= sizeof(PLUGIN_INFO))
	{
		wcscpy_s(pPluginInfo->wszPluginName, 64, L"KiriKiri");
		wcscpy_s(pPluginInfo->wszPluginType, 16, L"Algorithm");
		wcscpy_s(pPluginInfo->wszDownloadUrl, 256, L"http://www.aralgood.com/update_files_AT3/Plugin/Algorithm/Kirikiri.zip");
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
	return theApp.Init(hAralWnd, wszPluginOption);
}

//////////////////////////////////////////////////////////////////////////
//
// OnPluginClose
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose()
{
	return theApp.Close();
}

//////////////////////////////////////////////////////////////////////////
//
// OnPluginOption
//
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption()
{
	return theApp.Option();
}


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

	if( theApp.m_sTextFunc.pfnOrigWideCharToMultiByte )
	{
		nRetVal = theApp.m_sTextFunc.pfnOrigWideCharToMultiByte(
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

	if( theApp.m_sTextFunc.pfnOrigMultiByteToWideChar )
	{
		nRetVal = theApp.m_sTextFunc.pfnOrigMultiByteToWideChar(
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


// CKirikiriApp

BEGIN_MESSAGE_MAP(CKirikiriApp, CWinApp)
END_MESSAGE_MAP()


// CKirikiriApp construction
CKirikiriApp::CKirikiriApp()
	: m_hContainer(NULL), 
	m_hContainerWnd(NULL), 
	m_wszOptionString(NULL),
	m_pCodePoint(NULL),
	m_byteRegister(0x00),
	m_cwszOrigScript(NULL),
	m_wszScriptBuf(NULL),
	m_bUseCodePoint2(FALSE),
	m_nCodePoint2Type(0),
	m_pCodePoint2(NULL)

{
	ZeroMemory(&m_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
}

CKirikiriApp::~CKirikiriApp(void)
{
	Close();
}

// CKirikiriApp initialization
BOOL CKirikiriApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

BOOL CKirikiriApp::ExitInstance()
{
	TRACE(_T("CKirikiriApp::ExitInstance() \n"));

	Close();

	return CWinApp::ExitInstance();
}


BOOL CKirikiriApp::Init( HWND hAralWnd, LPWSTR wszPluginOption )
{
	Close();

	BOOL bRetVal = FALSE;

	// 부모 윈도우 핸들 저장
	if(NULL==hAralWnd) return FALSE;
	m_hContainerWnd = hAralWnd;

	// 컨테이너 함수 포인터 얻어오기
	m_hContainer = GetModuleHandle(_T("ATCTNR3.DLL"));
	if(m_hContainer && INVALID_HANDLE_VALUE != m_hContainer)
	{
		ZeroMemory(&m_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
		m_sATCTNR3.procHookWin32Api			= (PROC_HookWin32Api) GetProcAddress(m_hContainer, "HookWin32Api");
		m_sATCTNR3.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(m_hContainer, "UnhookWin32Api");
		m_sATCTNR3.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(m_hContainer, "HookCodePoint");
		m_sATCTNR3.procUnhookCodePoint		= (PROC_UnhookCodePoint) GetProcAddress(m_hContainer, "UnhookCodePoint");
		m_sATCTNR3.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(m_hContainer, "CreateTransCtx");
		m_sATCTNR3.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(m_hContainer, "DeleteTransCtx");
		m_sATCTNR3.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(m_hContainer, "TranslateUsingCtx");
		m_sATCTNR3.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(m_hContainer, "IsAppLocaleLoaded");
		m_sATCTNR3.procSuspendAllThread		= (PROC_SuspendAllThread) GetProcAddress(m_hContainer, "SuspendAllThread");
		m_sATCTNR3.procResumeAllThread		= (PROC_ResumeAllThread) GetProcAddress(m_hContainer, "ResumeAllThread");
		m_sATCTNR3.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(m_hContainer, "IsAllThreadSuspended");

	}

	if( m_sATCTNR3.procHookWin32Api && m_sATCTNR3.procUnhookWin32Api
		&& m_sATCTNR3.procHookCodePoint && m_sATCTNR3.procUnhookCodePoint
		&& m_sATCTNR3.procCreateTransCtx && m_sATCTNR3.procDeleteTransCtx
		&& m_sATCTNR3.procTranslateUsingCtx && m_sATCTNR3.procIsAppLocaleLoaded
		&& m_sATCTNR3.procSuspendAllThread && m_sATCTNR3.procResumeAllThread
		&& m_sATCTNR3.procIsAllThreadSuspended)
	{
		// 어플로케일 관련 함수
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));

		if( m_sTextFunc.pfnOrigMultiByteToWideChar && m_sTextFunc.pfnOrigWideCharToMultiByte )
		{
			
			// 스크립트 매니저
			if(m_ScriptMgr.Init() == TRUE)
			{
				// Create context
				if(m_sATCTNR3.procCreateTransCtx(L"KiriKiriContext") == TRUE)
				{
					// 기리기리 후킹
					bRetVal = HookKirikiri();
					if(FALSE == bRetVal)
					{
						MessageBox(hAralWnd, _T("기리기리 엔진 패턴을 찾을 수 없습니다."), _T("Aral Trans"), MB_OK);
					}

				}
			}

		}
	}

	if( bRetVal == TRUE )
	{

		// 옵션 먹이기
		m_wszOptionString = wszPluginOption;

		if(m_wszOptionString == NULL)
		{
			m_wszOptionString = new wchar_t[4096];
			ZeroMemory(m_wszOptionString, sizeof(wchar_t) * 4096);
		}

		CString strOptionString	= m_wszOptionString;		
		strOptionString = strOptionString.Trim();
		if(strOptionString.IsEmpty())
		{
			strOptionString = _T("CACHE(ZIP)");
		}

		if( m_optionRoot.ParseChildren(strOptionString) )
		{
			// 옵션 적용
			AdjustOption(&m_optionRoot);
		}
	}
	else
	{
		Close();
	}

	return bRetVal;
}


BOOL CKirikiriApp::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	// 기리기리 언훅
	if(m_pCodePoint)
	{
		m_sATCTNR3.procUnhookCodePoint((LPVOID)m_pCodePoint, PointCallback);
		m_pCodePoint = NULL;
	}
	if(m_pCodePoint2)
	{
		m_sATCTNR3.procUnhookCodePoint((LPVOID)m_pCodePoint2, PointCallback);
		m_pCodePoint2 = NULL;
	}

	// Delete context
	m_sATCTNR3.procDeleteTransCtx(L"KiriKiriContext");

	// 스크립트 매니저 말기화
	m_ScriptMgr.Close();

	// 옵션 객체 초기화
	ResetOption();
	m_optionRoot.ClearChildren();

	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));


	// 기타 변수 리셋
	m_hContainerWnd = NULL;
	m_wszOptionString = NULL;
	m_pCodePoint = NULL;
	m_byteRegister = 0x00;
	
	if(m_wszScriptBuf)
	{
		delete m_wszScriptBuf;
		m_wszScriptBuf = NULL;
	}

	return TRUE;
}

BOOL CKirikiriApp::Option()
{
	BOOL bRetVal = TRUE;

	CKirikiriOptionDlg od;
	
	if(m_ScriptMgr.m_bCacheToZIP && m_ScriptMgr.m_bCacheToFile) od.m_nCacheMode = 2;
	else if(m_ScriptMgr.m_bCacheToFile) od.m_nCacheMode = 1;
	else od.m_nCacheMode = 0;

	od.m_bAlsoSrc = m_ScriptMgr.m_bCacheSrc;

	od.m_bUseCP2 = m_bUseCodePoint2;

	od.m_nCP2Type = m_nCodePoint2Type;

	if( od.DoModal() == IDOK )
	{
		COptionNode tmpRoot;
		COptionNode* pChildNode = NULL;
		COptionNode* pChildChildNode = NULL;

		// 저장소 운용 모드
		pChildNode = tmpRoot.CreateChild();
		pChildNode->SetValue(_T("CACHE"));
		pChildChildNode = pChildNode->CreateChild();
		switch(od.m_nCacheMode)
		{
			case 1: pChildChildNode->SetValue(_T("FILE")); break;
			case 2: pChildChildNode->SetValue(_T("HYBRID")); break;
			default: pChildChildNode->SetValue(_T("ZIP"));
		}

		// 원문 저장 여부
		if(od.m_bAlsoSrc)
		{
			pChildChildNode = pChildNode->CreateChild();
			pChildChildNode->SetValue(_T("ALSOSRC"));
		}

		if (od.m_bUseCP2)
		{
			TCHAR szValue[10];
			pChildNode = tmpRoot.CreateChild();
			pChildNode->SetValue(_T("CP2"));
			pChildChildNode = pChildNode->CreateChild();
			wsprintf(szValue, _T("%d"), od.m_nCP2Type);
			pChildChildNode->SetValue(szValue);
		}

		ApplyOption(&tmpRoot);
	}

	return bRetVal;
}


BOOL CKirikiriApp::ApplyOption( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;

	CString strCurOptionString = m_optionRoot.ChildrenToString();

	// 작용시켜보고
	if( AdjustOption(pRootNode) == FALSE || m_optionRoot.ParseChildren( pRootNode->ChildrenToString() ) == FALSE )
	{
		// 실패면 원상복구
		m_optionRoot.ParseChildren( strCurOptionString );
		AdjustOption(&m_optionRoot);
	}
	else
	{
		// 적용 성공이면
		CString strOptionString = m_optionRoot.ChildrenToString();
		wcscpy_s(m_wszOptionString, MAX_OPTION_LEN, (LPCTSTR)strOptionString);
		bRetVal = TRUE;
	}

	return bRetVal;
}


void CKirikiriApp::ResetOption()
{
	// 설정값 초기화
	m_ScriptMgr.m_bCacheSrc = FALSE;
	m_ScriptMgr.m_bCacheToFile = FALSE;
	m_ScriptMgr.m_bCacheToZIP = FALSE;
}




//////////////////////////////////////////////////////////////////////////
//
// 이전 버전과의 호환을 위한 옵션 마이그레이션
//
//////////////////////////////////////////////////////////////////////////
BOOL CKirikiriApp::AdjustOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;

	ResetOption();

	BOOL bRetVal = TRUE;
	BOOL bUseCP2 = FALSE;
	int nCP2Type = 0;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();

		// CACHE 옵션
		if(strValue == _T("CACHE"))
		{
			if( pNode->GetChild(_T("ZIP")) ) 
				m_ScriptMgr.m_bCacheToZIP = TRUE;
			else if( pNode->GetChild(_T("FILE")) ) 
				m_ScriptMgr.m_bCacheToFile = TRUE;
			else if( pNode->GetChild(_T("HYBRID")) ) 
				m_ScriptMgr.m_bCacheToZIP = m_ScriptMgr.m_bCacheToFile = TRUE;
			
			if( pNode->GetChild(_T("ALSOSRC")) ) m_ScriptMgr.m_bCacheSrc = TRUE;			
			
		}
		// CP2 옵션
		else if(strValue == _T("CP2"))
		{
			bUseCP2=TRUE;

			COptionNode* pCP2Type = pNode->GetChild(0);
			
			if (pCP2Type)
				nCP2Type = _ttoi(pCP2Type->GetValue());
		}
	}

	if (bUseCP2)
	{
		m_bUseCodePoint2=TRUE;
		m_nCodePoint2Type=nCP2Type;

		if (!HookKirikiri2())
			bRetVal=FALSE;
	}
	else
	{
		m_bUseCodePoint2 = FALSE;
		m_nCodePoint2Type = 0;

		UnhookKiriKiri2();

	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
// PointCallback 후킹 함수
// 번역 포인트 (스크립트 로딩) 시 불린다
//////////////////////////////////////////////////////////////////////////
void CKirikiriApp::PointCallback( LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters )
{
	if(pHookedPoint == (LPVOID)theApp.m_pCodePoint)
	{
		theApp.OnScriptLoad(pRegisters);
	}
	else if(pHookedPoint == (LPVOID)theApp.m_pCodePoint2)
	{

		theApp.OnArrayLoad(pRegisters);
	}
}

// 스크립트를 읽어와 처리
void CKirikiriApp::OnScriptLoad( REGISTER_ENTRY* pRegisters )
{
	static int nFileSeq = 1;

	// OnScriptLoad 에서는 무조건 일반 모드
	m_ScriptMgr.SetTranslateMode(CKAGScriptMgr2::NORMAL);

	LPCWSTR cwszScriptData = NULL;

	switch (m_byteRegister)
	{
		// 1D:EBX, 0D:ECX, 15:EDX, 35:ESI, 3D:EDI, 2D:EBP, 25:ESP
		case 0x1D: cwszScriptData = (LPCWSTR)pRegisters->_EBX; break;
		case 0x0D: cwszScriptData = (LPCWSTR)pRegisters->_ECX; break;
		case 0x15: cwszScriptData = (LPCWSTR)pRegisters->_EDX; break;
		case 0x35: cwszScriptData = (LPCWSTR)pRegisters->_ESI; break;
		case 0x3D: cwszScriptData = (LPCWSTR)pRegisters->_EDI; break;
		case 0x2D: cwszScriptData = (LPCWSTR)pRegisters->_EBP; break;
		case 0x25: cwszScriptData = (LPCWSTR)pRegisters->_ESP; break;
	}


	if(NULL == cwszScriptData || L'\0' == cwszScriptData[0]) return;

	m_cwszOrigScript = cwszScriptData;
	size_t len = wcslen(cwszScriptData);
	if(m_wszScriptBuf)
	{
		delete m_wszScriptBuf;
	}

	// 버퍼 새로 할당 (2배 크기)
	m_wszScriptBuf = new wchar_t[len*2];

	BOOL bTrans = FALSE;
	CWnd* pParentWnd = CWnd::FromHandle( FindWindow(_T("TTVPWindowForm"), NULL) );	
	CTransProgressDlg trans_dlg(pParentWnd);
	if( trans_dlg.DoModal() == IDOK )
	{
		bTrans = TRUE;
	}
	TRACE(_T("[aral1] trans_dlg.DoModal() returned"));

	if( bTrans == TRUE )
	{
		// 레지스터의 값 바꿈
		switch (m_byteRegister)
		{
			// 1D:EBX, 0D:ECX, 15:EDX, 35:ESI, 3D:EDI, 2D:EBP, 25:ESP
			case 0x1D: pRegisters->_EBX = (DWORD)m_wszScriptBuf; break;
			case 0x0D: pRegisters->_ECX = (DWORD)m_wszScriptBuf; break;
			case 0x15: pRegisters->_EDX = (DWORD)m_wszScriptBuf; break;
			case 0x35: pRegisters->_ESI = (DWORD)m_wszScriptBuf; break;
			case 0x3D: pRegisters->_EDI = (DWORD)m_wszScriptBuf; break;
			case 0x2D: pRegisters->_EBP = (DWORD)m_wszScriptBuf; break;
			case 0x25: pRegisters->_ESP = (DWORD)m_wszScriptBuf; break;
		}
	}

	nFileSeq++;
}

// Array형 스크립트를 읽어와 처리
void CKirikiriApp::OnArrayLoad(REGISTER_ENTRY* pRegisters)
{
	LPCWSTR cwszScriptData = NULL;

	// OnArrayLoad 에서는 때에 따라 다름
	m_ScriptMgr.SetTranslateMode((CKAGScriptMgr2::TRANS_MODE)m_nCodePoint2Type);
	
	cwszScriptData = (LPCWSTR)pRegisters->_EBX;

	if(NULL == cwszScriptData || L'\0' == cwszScriptData[0]) return;

	m_cwszOrigScript = cwszScriptData;
	size_t len = wcslen(cwszScriptData);
	if(m_wszScriptBuf)
	{
		delete m_wszScriptBuf;
	}

	// 버퍼 새로 할당 (2배 크기)
	m_wszScriptBuf = new wchar_t[len*2];

	BOOL bTrans = FALSE;
	CWnd* pParentWnd = CWnd::FromHandle( FindWindow(_T("TTVPWindowForm"), NULL) );	
	CTransProgressDlg trans_dlg(pParentWnd);
	if( trans_dlg.DoModal() == IDOK )
	{
		bTrans = TRUE;
	}
	TRACE(_T("[aral1] trans_dlg.DoModal() returned"));

	if( bTrans == TRUE )
	{
		// 레지스터의 값 바꿈
		pRegisters->_EBX = (DWORD)m_wszScriptBuf;
		pRegisters->_ESI = (DWORD)m_wszScriptBuf;
		
	}
}

//////////////////////////////////////////////////////////////////////////
// HookKirikiri 함수
// 번역 포인트 (스크립트 로딩) 를 후킹한다.
//////////////////////////////////////////////////////////////////////////
BOOL CKirikiriApp::HookKirikiri()
{
	// tTVPScenarioCacheItem::LoadScenario() (KAGParser.cpp) 의 일부
	/*
		//					// krkr.exe ver. 2.30.2.416에서 참조
		33 C0				xor eax, eax
		89 45 C8			mov dword ptr [ebp-38], eax
		66 C7 45 E4 08 00	mov [ebp-1C], 0008h						// cszPattern1 시작
		66 C7 45 E4 14 00	mov [ebp-1C], 0014h
		B8 D8 6D 6E 00		mov eax, 006E6DD8
		E8 5C BB E2 FF		call -001D44A3h
		89 45 F8			mov dword ptr [ebp-08], eax
		FF 45 F0			inc [ebp-10]
		8D 55 F8			lea edx, dword ptr [ebp-08]
		8B 45 CC			mov eax, dword ptr [ebp-34]
		E8 CF D2 01 00		call +0001D2CFh
		89 45 C8			mov dword ptr [ebp-38], eax				// cszPattern1 끝 (89)
		FF 4D F0			dec [ebp-10]
		8B 45 F8			mov eax, dword ptr [ebp-08]
		85 C0				test eax, eax
		74 05				je +05h
		E8 05 B8 E2 FF		call +1D47FAh
		66 C7 45 E4 08 00	mov [ebp-1C], 0008h
		66 C7 45 E4 2C 00	mov [ebp-1C], 002Ch
		33 D2				xor edx, edx
		8D 4D FC			lea ecx, dword ptr [ebp-04]
		89 55 FC			mov dword ptr [ebp-04], edx
		FF 45 F0			inc [ebp-10]
		66 C7 45 E4 38 00	mov [ebp-1C], 0038h
		6A 00				push 00000000h
		51					push ecx
		8B 45 C8			mov eax, dword ptr [ebp-38]
		50					push eax
		8B 10				mov edx, dword ptr [eax]
		FF 12				call dword ptr [edx]
		83 C4 0C			add esp, 0Ch
		83 7D FC 00			cmp dword ptr [ebp-04], 00000000h		// cszPattern2 시작
		74 1B				je +1Bh
		8B 45 FC			mov eax, dword ptr [ebp-04]
		85 C0				test eax, eax
		75 04				jne +04h
		33 DB				xor ebx, ebx
		EB 16				jmp +16h
		83 78 04 00			cmp dword ptr [eax+04], 00000000h
		74 05				je +05h
		8B 58 04			mov ebx, dword ptr [eax+04]
		EB 0B				jmp +0Bh
		8D 58 08			lea ebx, dword ptr [eax+08]
		EB 06				jmp +06h
		8B 1D 44 93 6A 00	mov ebx, dword ptr [006A9344h]			// cszPattern2 끝 (8B). 로드된 스크립트를 레지스터에 저장한다.
		//															// 여기서 2번째 바이트 (1D) 가 저장되는 레지스터 (ebx) 를 결정한다.
		8B 45 D0			mov esi, dword ptr [eax]				// 여기가 후킹 포인트 (m_pCodePoint)

	*/
	// cszPattern1, cszPattern2
	LPCTSTR cszPattern1 = _T("66 C7 45 xx xx 00 66 C7 45 xx xx 00 B8 xx xx xx 00 E8 xx xx xx FF 89 45 xx FF 45 xx 8D 55 xx 8B 45 xx E8 xx xx xx 00 89");
	LPCTSTR cszPattern2 = _T("83 7D xx 00 74 1B 8B 45 xx 85 C0 75 04 33 xx EB 16 83 78 04 00 74 05 8B xx 04 EB 0B 8D xx 08 EB 06 8B");

	BOOL bRetVal = FALSE;

	try
	{
		// 실행 파일 이미지 크기 구하기
		MODULEINFO mi;
		if( GetModuleInformation( ::GetCurrentProcess(), (HMODULE)0x00400000, &mi, sizeof(MODULEINFO) ) == FALSE ) throw -1;

		// 잘못된 EntryPoint 의 경우 검색이 제대로 안 되는 문제를 수정
		if( (UINT_PTR)mi.EntryPoint - (UINT_PTR)mi.lpBaseOfDll > 0x2000)
		{
			mi.EntryPoint = (LPVOID)((UINT_PTR)mi.lpBaseOfDll + 0x1000);
		}

		// 루프
		UINT nSize = mi.SizeOfImage;
		int nRes = (int)((UINT_PTR)mi.EntryPoint - (UINT_PTR)mi.lpBaseOfDll);
		
		while(nRes != -1)
		{
			// 패턴1 검색
			nRes = CBinaryPatternSearcher::Search( (LPBYTE)mi.lpBaseOfDll, mi.SizeOfImage, cszPattern1, nRes);

			// 패턴1이 검색되었다면 
			if(nRes != -1)
			{
				// 패턴2 검색
				int nRes2 = CBinaryPatternSearcher::Search( (LPBYTE)mi.lpBaseOfDll, mi.SizeOfImage, cszPattern2, nRes);

				// 패턴2가 검색되었고, 패턴1과 패턴2의 거리가 200바이트 미만이라면
				if(nRes2 != -1 && (nRes2 - nRes) < 200)
				{
					// 후킹할 지점은 패턴2에서 +39바이트 지점
					UINT_PTR pCodePoint = (UINT_PTR)mi.lpBaseOfDll + (UINT_PTR)nRes2 + 39;

					// 번역할 인자는 패턴2에서 +34바이트 지점의 값에 의해 정해짐
					BYTE byteRegister = *(BYTE*)((UINT_PTR)mi.lpBaseOfDll + (UINT_PTR)nRes2 + 34);

					// 후킹
					if(m_sATCTNR3.procHookCodePoint((LPVOID)pCodePoint, PointCallback, 10) == TRUE)
					{
						m_pCodePoint = pCodePoint;
						m_byteRegister = byteRegister;

						// 루프탈출
						bRetVal = TRUE;
						break;
					}

				}

				nRes++;
			}


		}
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}


	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// HookKirikiri2 함수
// 추가 번역 포인트 (Array 로딩) 를 후킹한다.
//////////////////////////////////////////////////////////////////////////
BOOL CKirikiriApp::HookKirikiri2()
{
	if (m_pCodePoint2)
		return TRUE;

	m_sATCTNR3.procSuspendAllThread();

	// TJS_BEGIN_NATIVE_METHOD_DECL(/* func. name */load) 의 일부
	/*
		//					// krkr.exe ver. 2.30.2.416에서 참조
		8B 1D 44 93 6A 00	mov ebx, dword ptr [006A9344]			// 로드된 스크립트를 레지스터에 저장한다.
		8B F3				mov esi, ebx							// esi = ebx. 후킹 후 esi 와 ebx 레지스터를 같게 만들어주자.
		33 FF				xor edi, edi
		66 C7 45 D0 68 00	mov [ebp-30], 0068h
		EB 78				jmp +78h								// cszPattern3 시작, 후킹포인트 (m_pCodePoint2)
		66 8B 03			mov ax, word ptr [ebx]
		66 83 F8 0D			cmp ax, 000Dh							// ax == L'\r' ?
		74 06				je +06h
		66 83 F8 0A			cmp ax, 000Ah							// ax == L'\n' ?
		75 66				jne +66h								// cszPattern3 끝
		8B D3				mov edx, ebx
	*/
	// cszPattern3
	LPCTSTR cszPattern3 = _T("EB 78 66 8B 03 66 83 F8 0D 74 06 66 83 F8 0A 75 66");

	BOOL bRetVal = FALSE;

	try
	{
		// 실행 파일 이미지 크기 구하기
		MODULEINFO mi;
		if( GetModuleInformation( ::GetCurrentProcess(), (HMODULE)0x00400000, &mi, sizeof(MODULEINFO) ) == FALSE ) throw -1;

		// 잘못된 EntryPoint 의 경우 검색이 제대로 안 되는 문제를 수정
		if( (UINT_PTR)mi.EntryPoint - (UINT_PTR)mi.lpBaseOfDll > 0x2000)
		{
			mi.EntryPoint = (LPVOID)((UINT_PTR)mi.lpBaseOfDll + 0x1000);
		}

		UINT nSize = mi.SizeOfImage;
		int nRes = (int)((UINT_PTR)mi.EntryPoint - (UINT_PTR)mi.lpBaseOfDll);

		// 패턴 3 검색
		nRes = CBinaryPatternSearcher::Search( (LPBYTE)mi.lpBaseOfDll, mi.SizeOfImage, cszPattern3, nRes);

		if (nRes != -1)
		{
			
			UINT_PTR pCodePoint2 =  (UINT_PTR)mi.lpBaseOfDll + (UINT_PTR)nRes;
			if (m_sATCTNR3.procHookCodePoint((LPVOID)pCodePoint2, PointCallback, 10))
			{
				m_pCodePoint2 = pCodePoint2;
				bRetVal = TRUE;
			}
		}
		else
		{
			MessageBox(NULL, _T("기리기리 추가 패턴을 찾을 수 없습니다."), _T("Aral Trans"), MB_OK);
			bRetVal = FALSE;
		}

	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode;
	}
	m_sATCTNR3.procResumeAllThread();

	return bRetVal;
}

void CKirikiriApp::UnhookKiriKiri2()
{
	if(m_pCodePoint2)
	{
		m_sATCTNR3.procSuspendAllThread();
		m_sATCTNR3.procUnhookCodePoint((LPVOID)m_pCodePoint2, PointCallback);
		m_pCodePoint2 = NULL;
		m_sATCTNR3.procResumeAllThread();
	}
}

BOOL CKirikiriApp::ClearCache()
{
	return m_ScriptMgr.ClearCache();
}
