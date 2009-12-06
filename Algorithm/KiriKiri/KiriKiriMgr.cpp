
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "CharacterMapper.h"
#include "OptionDlg.h"
#include <process.h>

CATCodeMgr*	CATCodeMgr::_Inst = NULL;



CATCodeMgr* CATCodeMgr::GetInstance()
{
	return _Inst;
}


CATCodeMgr::CATCodeMgr(void)
  : m_bRunning(FALSE), 
	m_nFontLoadLevel(0), 
	m_hContainer(NULL), 
	m_hContainerWnd(NULL), 
	m_szOptionString(NULL), 
	m_bEncodeKorean(FALSE),
	m_bRunClipboardThread(FALSE),
	m_hClipboardThread(NULL),
	m_hClipTextChangeEvent(NULL),
	m_pfnLoadLibraryA(NULL),
	m_pfnLoadLibraryW(NULL)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_FUNCTION_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
	ZeroMemory(&m_sFontFunc, sizeof(FONT_FUNCTION_ENTRY));
	InitializeCriticalSection(&m_csClipText);
}

CATCodeMgr::~CATCodeMgr(void)
{
	DeleteCriticalSection(&m_csClipText);
	Close();
	_Inst = NULL;
}

BOOL CATCodeMgr::Init( HWND hSettingWnd, LPSTR cszOptionStringBuffer )
{
	Close();

	BOOL bRetVal = FALSE;

	// 부모 윈도우 핸들 저장
	if(NULL==hSettingWnd) return FALSE;
	m_hContainerWnd = hSettingWnd;

	// 컨테이너 함수 포인터 얻어오기
	m_hContainer = GetModuleHandle(_T("ATCTNR.DLL"));
	if(m_hContainer)
	{
		m_sContainerFunc.pfnGetCurAlgorithm		= (PROC_GetCurAlgorithm) GetProcAddress( m_hContainer, "GetCurAlgorithm" );
		m_sContainerFunc.pfnGetCurTranslator	= (PROC_GetCurTranslator) GetProcAddress( m_hContainer, "GetCurTranslator" );
		m_sContainerFunc.pfnHookDllFunction		= (PROC_HookDllFunction) GetProcAddress( m_hContainer, "HookDllFunction" );
		m_sContainerFunc.pfnGetOrigDllFunction	= (PROC_GetOrigDllFunction) GetProcAddress( m_hContainer, "GetOrigDllFunction" );
		m_sContainerFunc.pfnUnhookDllFunction	= (PROC_UnhookDllFunction) GetProcAddress( m_hContainer, "UnhookDllFunction" );
		m_sContainerFunc.pfnHookCodePoint		= (PROC_HookCodePoint) GetProcAddress( m_hContainer, "HookCodePoint" );
		m_sContainerFunc.pfnUnhookCodePoint		= (PROC_UnhookCodePoint) GetProcAddress( m_hContainer, "UnhookCodePoint" );
		m_sContainerFunc.pfnTranslateText		= (PROC_TranslateText) GetProcAddress( m_hContainer, "TranslateText" );
		m_sContainerFunc.pfnIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress( m_hContainer, "IsAppLocaleLoaded" );
		m_sContainerFunc.pfnEnableAppLocale		= (PROC_EnableAppLocale) GetProcAddress( m_hContainer, "EnableAppLocale" );
	}

	if( m_sContainerFunc.pfnGetCurAlgorithm && m_sContainerFunc.pfnGetCurTranslator
		&& m_sContainerFunc.pfnHookDllFunction && m_sContainerFunc.pfnGetOrigDllFunction
		&& m_sContainerFunc.pfnUnhookDllFunction && m_sContainerFunc.pfnHookCodePoint
		&& m_sContainerFunc.pfnUnhookCodePoint && m_sContainerFunc.pfnTranslateText
		&& m_sContainerFunc.pfnIsAppLocaleLoaded && m_sContainerFunc.pfnEnableAppLocale )
	{
		
		// LoadLibrary 함수 후킹	 
		if( m_sContainerFunc.pfnHookDllFunction( "kernel32.dll", "LoadLibraryA", NewLoadLibraryA ) )
		{
			m_pfnLoadLibraryA = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryA");
		}

		if( m_sContainerFunc.pfnHookDllFunction( "kernel32.dll", "LoadLibraryW", NewLoadLibraryW ) )
		{
			m_pfnLoadLibraryW = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryW");
		}

		// 폰트 함수군 후킹
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.dll", "CreateFontA", NewCreateFontA ) )
		{
			m_sFontFunc.pfnCreateFontA = (PROC_CreateFont) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontA");
		}

		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.dll", "CreateFontW", NewCreateFontW ) )
		{
			m_sFontFunc.pfnCreateFontW = (PROC_CreateFont) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontW");
		}

		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.dll", "CreateFontIndirectA", NewCreateFontIndirectA ) )
		{
			m_sFontFunc.pfnCreateFontIndirectA = (PROC_CreateFontIndirect) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontIndirectA");
		}

		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.dll", "CreateFontIndirectW", NewCreateFontIndirectW ) )
		{
			m_sFontFunc.pfnCreateFontIndirectW = (PROC_CreateFontIndirect) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.dll", "CreateFontIndirectW");
		}


		// 텍스트 함수군 후킹

		// GetGlyphOutlineA
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "GetGlyphOutlineA", NewGetGlyphOutlineA ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineA = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineA");
		}

		// GetGlyphOutlineW
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "GetGlyphOutlineW", NewGetGlyphOutlineW ) )
		{
			m_sTextFunc.pfnGetGlyphOutlineW = 
				(PROC_GetGlyphOutline) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "GetGlyphOutlineW");
		}

		// TextOutA
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "TextOutA", NewTextOutA ) )
		{
			m_sTextFunc.pfnTextOutA = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutA");
		}

		// TextOutW
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "TextOutW", NewTextOutW ) )
		{
			m_sTextFunc.pfnTextOutW = 
				(PROC_TextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "TextOutW");
		}

		// ExtTextOutA
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "ExtTextOutA", NewExtTextOutA ) )
		{
			m_sTextFunc.pfnExtTextOutA = 
				(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutA");
		}

		// ExtTextOutW
		if( m_sContainerFunc.pfnHookDllFunction( "GDI32.DLL", "ExtTextOutW", NewExtTextOutW ) )
		{
			m_sTextFunc.pfnExtTextOutW = 
				(PROC_ExtTextOut) m_sContainerFunc.pfnGetOrigDllFunction("GDI32.DLL", "ExtTextOutW");
		}

		// DrawTextA
		if( m_sContainerFunc.pfnHookDllFunction( "USER32.DLL", "DrawTextA", NewDrawTextA ) )
		{
			m_sTextFunc.pfnDrawTextA = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextA");
		}

		// DrawTextW
		if( m_sContainerFunc.pfnHookDllFunction( "USER32.DLL", "DrawTextW", NewDrawTextW ) )
		{
			m_sTextFunc.pfnDrawTextW = 
				(PROC_DrawText) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextW");
		}

		// DrawTextExA
		if( m_sContainerFunc.pfnHookDllFunction( "USER32.DLL", "DrawTextExA", NewDrawTextExA ) )
		{
			m_sTextFunc.pfnDrawTextExA = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExA");
		}

		// DrawTextExW
		if( m_sContainerFunc.pfnHookDllFunction( "USER32.DLL", "DrawTextExW", NewDrawTextExW ) )
		{
			m_sTextFunc.pfnDrawTextExW = 
				(PROC_DrawTextEx) m_sContainerFunc.pfnGetOrigDllFunction("USER32.DLL", "DrawTextExW");
		}

		// 어플로케일 관련 함수
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));

		if( m_sTextFunc.pfnExtTextOutA && m_sTextFunc.pfnExtTextOutW
			&& m_sTextFunc.pfnGetGlyphOutlineA && m_sTextFunc.pfnGetGlyphOutlineW
			&& m_sTextFunc.pfnOrigMultiByteToWideChar && m_sTextFunc.pfnOrigWideCharToMultiByte
			&& m_sTextFunc.pfnTextOutA && m_sTextFunc.pfnTextOutW 
			&& m_sTextFunc.pfnDrawTextA	&& m_sTextFunc.pfnDrawTextW
			&& m_sTextFunc.pfnDrawTextExA && m_sTextFunc.pfnDrawTextExW
			
			&& m_pfnLoadLibraryA && m_pfnLoadLibraryW
			
			&& m_sFontFunc.pfnCreateFontA && m_sFontFunc.pfnCreateFontW
			&& m_sFontFunc.pfnCreateFontIndirectA && m_sFontFunc.pfnCreateFontIndirectW )
		{
			// 클립보드 관련
			m_bRunClipboardThread = TRUE;
			m_hClipTextChangeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hClipboardThread = (HANDLE)_beginthreadex(NULL, 0, ClipboardThreadFunc, NULL, 0, NULL);

			// 옵션 스트링 파싱
			m_szOptionString = cszOptionStringBuffer;

			if(m_szOptionString == NULL)
			{
				m_szOptionString = new char[4096];
				ZeroMemory(m_szOptionString, 4096);
			}

			if( m_szOptionString[0] == _T('\0') )
			{
				//strcpy(m_szOptionString, "FORCEFONT,ENCODEKOR,FONT(궁서,-24)");
			}

			//if( m_optionRoot.ParseChildren("FORCEFONT, HOOK( 0x00410320, TRANS([ESP+0x4],ANSI), TRANS([ESP+0xC],ANSI) ), ENCODEKOR") )

#ifdef UNICODE
			wchar_t wszTmpString[4096];
			MyMultiByteToWideChar(949, 0, m_szOptionString, -1, wszTmpString, 4096);
			CString strOptionString = wszTmpString;
#else
			CString strOptionString	= m_szOptionString;		
#endif

			if( m_optionRoot.ParseChildren(strOptionString) )
			{
				// 옵션 적용
				AdjustOption(&m_optionRoot);

				bRetVal = TRUE;
			}
			
		}
	}

	if( FALSE == bRetVal ) Close();

	return bRetVal;
}


BOOL CATCodeMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	ResetOption();

	// 클립보드 쓰레드 정지
	m_bRunClipboardThread = FALSE;
	if(m_hClipboardThread && m_hClipTextChangeEvent)
	{
		::SetEvent(m_hClipTextChangeEvent);
		::WaitForSingleObject(m_hClipboardThread, 3000);
		::CloseHandle(m_hClipboardThread);
		::CloseHandle(m_hClipTextChangeEvent);
	}
	m_hClipboardThread = NULL;
	m_hClipTextChangeEvent = NULL;
	
	// 옵션 객체 초기화
	m_optionRoot.ClearChildren();

	// GetGlyphOutlineA 언훅
	if( m_sTextFunc.pfnGetGlyphOutlineA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "GetGlyphOutlineA" );
		m_sTextFunc.pfnGetGlyphOutlineA = NULL;
	}

	// GetGlyphOutlineW 언훅
	if( m_sTextFunc.pfnGetGlyphOutlineW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "GetGlyphOutlineW" );
		m_sTextFunc.pfnGetGlyphOutlineW = NULL;
	}

	// TextOutA 언훅
	if( m_sTextFunc.pfnTextOutA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "TextOutA" );
		m_sTextFunc.pfnTextOutA = NULL;
	}

	// TextOutW 언훅
	if( m_sTextFunc.pfnTextOutW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "TextOutW" );
		m_sTextFunc.pfnTextOutW = NULL;
	}

	// ExtTextOutA 언훅
	if( m_sTextFunc.pfnExtTextOutA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "ExtTextOutA" );
		m_sTextFunc.pfnExtTextOutA = NULL;
	}

	// ExtTextOutW 언훅
	if( m_sTextFunc.pfnExtTextOutW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.DLL", "ExtTextOutW" );
		m_sTextFunc.pfnExtTextOutW = NULL;
	}

	// DrawTextA 언훅
	if( m_sTextFunc.pfnDrawTextA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextA" );
		m_sTextFunc.pfnDrawTextA = NULL;
	}

	// DrawTextW 언훅
	if( m_sTextFunc.pfnDrawTextW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextW" );
		m_sTextFunc.pfnDrawTextW = NULL;
	}

	// DrawTextExA 언훅
	if( m_sTextFunc.pfnDrawTextExA )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextExA" );
		m_sTextFunc.pfnDrawTextExA = NULL;
	}

	// DrawTextExW 언훅
	if( m_sTextFunc.pfnDrawTextExW )
	{
		m_sContainerFunc.pfnUnhookDllFunction( "USER32.DLL", "DrawTextExW" );
		m_sTextFunc.pfnDrawTextExW = NULL;
	}

	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));


	// 폰트 함수군 언훅
	if( m_sFontFunc.pfnCreateFontA )
	{
		TRACE(_T("GDI32.DLL!CreateFontA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontA" );
		m_sFontFunc.pfnCreateFontA = NULL;
	}
	if( m_sFontFunc.pfnCreateFontW )
	{
		TRACE(_T("GDI32.DLL!CreateFontW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontW" );
		m_sFontFunc.pfnCreateFontW = NULL;
	}
	if( m_sFontFunc.pfnCreateFontIndirectA )
	{
		TRACE(_T("GDI32.DLL!CreateFontIndirectA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontIndirectA" );
		m_sFontFunc.pfnCreateFontIndirectA = NULL;
	}
	if( m_sFontFunc.pfnCreateFontIndirectW )
	{
		TRACE(_T("GDI32.DLL!CreateFontIndirectW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "GDI32.dll", "CreateFontIndirectW" );
		m_sFontFunc.pfnCreateFontIndirectW = NULL;
	}

	// LoadLibrary 언훅
	if( m_pfnLoadLibraryA )
	{
		TRACE(_T("kernel32.DLL!LoadLibraryA Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "LoadLibraryA" );
		m_pfnLoadLibraryA = NULL;
	}

	if( m_pfnLoadLibraryW )
	{
		TRACE(_T("kernel32.DLL!LoadLibraryW Unhook... \n"));
		m_sContainerFunc.pfnUnhookDllFunction( "kernel32.dll", "LoadLibraryW" );
		m_pfnLoadLibraryW = NULL;
	}

	

	// 기타 변수 리셋
	m_hContainerWnd = NULL;
	m_szOptionString = NULL;

	return TRUE;
}

BOOL CATCodeMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CATCodeMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CATCodeMgr::Option()
{
	BOOL bRetVal = TRUE;

	CString strCurOptionString = m_optionRoot.ChildrenToString();
	
	COptionNode tmpRoot;
	if( tmpRoot.ParseChildren(strCurOptionString) == FALSE ) return FALSE;

	COptionDlg od;
	od.SetRootOptionNode(&tmpRoot);
	if( od.DoModal() == IDOK )
	{
		ApplyOption(&tmpRoot);
	}

	return bRetVal;
}

BOOL CATCodeMgr::ApplyOption( COptionNode* pRootNode )
{
	BOOL bRetVal = FALSE;
	
	CString strCurOptionString = m_optionRoot.ChildrenToString();

	// 새 옵션 루트에서 FORCEFONT가 없으면 FIXFONTSIZE, FONT 노드 제거
	if( pRootNode->GetChild(_T("FORCEFONT")) == NULL )
	{
		pRootNode->DeleteChild(_T("FIXFONTSIZE"));
		pRootNode->DeleteChild(_T("FONT"));
	}
	
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

#ifdef UNICODE
		MyWideCharToMultiByte(949, 0, (LPCWSTR)strOptionString, -1, m_szOptionString, 4096, NULL, NULL);
#else
		strcpy(m_szOptionString, (LPCTSTR)strOptionString);
#endif

		bRetVal = TRUE;
	}

	return bRetVal;
}

HFONT CATCodeMgr::CheckFont( HDC hdc )
{
	HFONT hRetVal = NULL;

	TEXTMETRIC tm;
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// 폰트 다시 로드
	if( bRes )//&& tm.tmHeight != lLastFontHeight )
	{
		
		HFONT font = NULL;
		long lFontSize = tm.tmHeight;
		
		if(m_strFontFace.IsEmpty())
		{
			m_strFontFace = _T("궁서");
		}

		// 폰트페이스명이 바뀌었으면 맵 초기화
		if(m_strLastFontFace.Compare(m_strFontFace))
		{
			for(map<long, HFONT>::iterator iter = m_mapFonts.begin();
				iter != m_mapFonts.end();
				iter++)
			{
				font = iter->second;
				DeleteObject(font);
			}
			m_mapFonts.clear();
		}
		
		// 폰트 크기 고정인 경우
		if(m_lFontSize !=0 && m_bFixedFontSize)
		{
			lFontSize = m_lFontSize;
		}

		// 이 크기에 해당하는 폰트가 없을 경우 폰트를 생성
		if( m_mapFonts.find(lFontSize) == m_mapFonts.end() )
		{
			font = CreateFont(lFontSize, 0, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
				HANGEUL_CHARSET,	//ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				DEFAULT_PITCH,		// | FF_SWISS,
				m_strFontFace);

			m_mapFonts[lFontSize] = font;

		}
		else
		{
			font = m_mapFonts[lFontSize];
		}

		hRetVal = (HFONT)SelectObject(hdc, font);

	}

	/*
	TEXTMETRIC tm;
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// 폰트 다시 로드
	if( bRes )
	{
		// 폰트 크기 고정인 경우
		if(m_lFontSize !=0 && m_bFixedFontSize)
		{
			tm.tmHeight = m_lFontSize;
		}

		HFONT font = InnerCreateFont(tm.tmHeight, 0, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
			HANGEUL_CHARSET,	//ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			DEFAULT_PITCH,		// | FF_SWISS,
			(LPWSTR)(LPCWSTR)m_strFontFace);

		if(font)
		{
			hRetVal = (HFONT)SelectObject(hdc, font);
		}
	}
	*/

	return hRetVal;

}

void CATCodeMgr::ResetOption()
{
	// 후킹 재시도 중인 목록을 클리어
	m_listRetryHook.clear();
	
	// 후킹한 ATCode들 해제
	for(list<CHookPoint*>::iterator iter = m_listHookPoint.begin();
		iter != m_listHookPoint.end();
		iter++)
	{
		CHookPoint* pPoint = (*iter);
		delete pPoint;
	}
	m_listHookPoint.clear();

	m_nFontLoadLevel = 0;
	m_bEncodeKorean = FALSE;	
	m_bFixedFontSize = FALSE;
	m_lFontSize = 0;
	m_strFontFace = "";

}


//////////////////////////////////////////////////////////////////////////
//
// 이전 버전과의 호환을 위한 옵션 마이그레이션
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::MigrateOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;

	BOOL bRetVal = TRUE;
	BOOL bNeedMigration = FALSE;

	BOOL bPtrCheat = FALSE;
	BOOL bRemoveSpace = FALSE;
	BOOL bTwoByte = FALSE;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();

		// FORCEFONT 옵션
		if(strValue == _T("FORCEFONT"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);

			// 과거 형식과 호환을 위해
			if(NULL==pLevelNode)
			{
				pLevelNode = pNode->CreateChild();
				pLevelNode->SetValue(_T("10"));
			}
		}
		
		// PTRCHEAT 옵션
		else if(strValue == _T("PTRCHEAT"))
		{
			bNeedMigration = TRUE;
			bPtrCheat = TRUE;
		}
		// REMOVESPACE 옵션
		else if(strValue == _T("REMOVESPACE"))
		{
			bNeedMigration = TRUE;
			bRemoveSpace = TRUE;
		}
		// TWOBYTE 옵션
		else if(strValue == _T("TWOBYTE"))
		{
			bNeedMigration = TRUE;
			bTwoByte = TRUE;
		}
	}	

	// 마이그레이션이 필요하면
	if(bNeedMigration)
	{
		// 텍스트 조작 방식 결정
		CString strTransMethod;
		if(bPtrCheat) strTransMethod = _T("PTRCHEAT");
		else strTransMethod = _T("OVERWRITE");

		// 필요없는 노드 삭제
		pRootNode->DeleteChild(_T("PTRCHEAT"));
		pRootNode->DeleteChild(_T("REMOVESPACE"));
		pRootNode->DeleteChild(_T("TWOBYTE"));
		
		// 루프를 돌며 모든 HOOK 노드에 적용
		cnt = pRootNode->GetChildCount();
		for(int i=0; i<cnt; i++)
		{
			COptionNode* pNode = pRootNode->GetChild(i);
			CString strValue = pNode->GetValue().MakeUpper();

			// HOOK 노드
			if(strValue == _T("HOOK"))
			{
				int cnt2 = pNode->GetChildCount();
				for(int j=0; j<cnt2; j++)
				{
					COptionNode* pTransNode = pNode->GetChild(j);
					strValue = pTransNode->GetValue().MakeUpper();

					// TRANS 노드
					if(strValue == _T("TRANS"))
					{
						pTransNode->DeleteChild(_T("NOP"));
						pTransNode->DeleteChild(_T("PTRCHEAT"));
						pTransNode->DeleteChild(_T("OVERWRITE"));
						pTransNode->DeleteChild(_T("REMOVESPACE"));
						pTransNode->DeleteChild(_T("TWOBYTE"));

						COptionNode* pChildNode = NULL;

						// 번역작업 방식
						pChildNode = pTransNode->CreateChild();
						pChildNode->SetValue(strTransMethod);
						
						// 공백제거
						if(bRemoveSpace)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("REMOVESPACE"));
						}

						// 1바이트를 2바이트로 변환
						if(bTwoByte)
						{
							pChildNode = pTransNode->CreateChild();
							pChildNode->SetValue(_T("TWOBYTE"));
						}

					}

				}
				
			} // HOOK 노드 끝
		}	
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 이전 버전과의 호환을 위한 옵션 마이그레이션
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::AdjustOption(COptionNode* pRootNode)
{
	if(NULL == pRootNode) return FALSE;
	
	ResetOption();
	MigrateOption(pRootNode);

	//FORCEFONT,HOOK(0x00434343,TRANS([ESP+0x4],ANSI,ALLSAMETEXT))
	BOOL bRetVal = TRUE;

	int cnt = pRootNode->GetChildCount();
	for(int i=0; i<cnt; i++)
	{
		COptionNode* pNode = pRootNode->GetChild(i);
		CString strValue = pNode->GetValue().MakeUpper();
		
		// FORCEFONT 옵션
		if(strValue == _T("FORCEFONT"))
		{
			COptionNode* pLevelNode = pNode->GetChild(0);
			m_nFontLoadLevel = _ttoi(pLevelNode->GetValue().Trim());
		}
		// FIXFONTSIZE 옵션
		else if(strValue == _T("FIXFONTSIZE"))
		{
			m_bFixedFontSize = TRUE;
		}
		// FONT 옵션
		else if(strValue == _T("FONT") && pNode->GetChildCount() == 2)
		{
			// 폰트 페이스명
			COptionNode* pFontFaceNode = pNode->GetChild(0);
			if(pFontFaceNode && m_strFontFace != pFontFaceNode->GetValue())
			{
				// 현재 모아놓은 폰트 객체들 해제
				for(map<long, HFONT>::iterator iter = m_mapFonts.begin();
					iter != m_mapFonts.end();
					iter++)
				{
					DeleteObject(iter->second);
				}
				m_mapFonts.clear();

				// 새로운 폰트 페이스 설정
				m_strFontFace = pFontFaceNode->GetValue();
			}

			// 폰트 사이즈
			COptionNode* pFontSizeNode = pNode->GetChild(1);
			if(pFontSizeNode)
			{
				m_lFontSize = (long)_ttoi(pFontSizeNode->GetValue());
			}
		}
		// ENCODEKOR 옵션
		else if(strValue == _T("ENCODEKOR"))
		{
			m_bEncodeKorean = TRUE;
		}
		// HOOK 노드
		else if(strValue == _T("HOOK"))
		{
			BOOL bHookRes = HookFromOptionNode(pNode);
			if(FALSE == bHookRes) m_listRetryHook.push_back(pNode);
		} // HOOK 노드 끝
	}

	return bRetVal;
}

BOOL CATCodeMgr::HookFromOptionNode(COptionNode* pNode)
{
	BOOL bRetVal = FALSE;
	
	try
	{
		// 후킹할 주소
		COptionNode* pAddrNode = pNode->GetChild(0);
		if(pAddrNode==NULL) throw -1;

		CHookPoint* pHookPoint = CHookPoint::CreateInstance(pAddrNode->GetValue());
		if(pHookPoint==NULL)
		{
			//MessageBox(m_hContainerWnd, _T("다음의 주소를 후킹하는데 실패했습니다 : ") + pAddrNode->GetValue(), _T("Hook error"), MB_OK);
			//continue;
			 throw -2;
		}
		m_listHookPoint.push_back(pHookPoint);

		// 이 주소에 대한 후킹 명령들 수집
		int cnt2 = pNode->GetChildCount();
		for(int j=1; j<cnt2; j++)
		{
			COptionNode* pNode2 = pNode->GetChild(j);
			CString strHookValue = pNode2->GetValue();

			// 번역 명령
			if(strHookValue.CompareNoCase(_T("TRANS"))==0)
			{
				// 인자 거리
				COptionNode* pDistNode = pNode2->GetChild(0);
				if(pDistNode==NULL) continue;

				/*
				int nDistFromESP = 0;
				CString strStorage = pDistNode->GetValue().MakeUpper();

				if(strStorage==_T("[ESP]")) nDistFromESP = 0x0;
				else if(strStorage==_T("EAX")) nDistFromESP = -0x4;
				else if(strStorage==_T("ECX")) nDistFromESP = -0x8;
				else if(strStorage==_T("EDX")) nDistFromESP = -0xC;
				else if(strStorage==_T("EBX")) nDistFromESP = -0x10;
				else if(strStorage==_T("ESP")) nDistFromESP = -0x14;
				else if(strStorage==_T("EBP")) nDistFromESP = -0x18;
				else if(strStorage==_T("ESI")) nDistFromESP = -0x1C;
				else if(strStorage==_T("EDI")) nDistFromESP = -0x20;
				else
				{

					_stscanf((LPCTSTR)strStorage, _T("[ESP+%x]"), &nDistFromESP);
					if(nDistFromESP == 0) continue;
				}

				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(nDistFromESP);
				*/
				CTransCommand* pTransCmd = pHookPoint->AddTransCmd(pDistNode->GetValue());
				
				// 번역 옵션들 수집
				int cnt3 = pNode2->GetChildCount();
				for(int k=1; k<cnt3; k++)
				{
					COptionNode* pNode3 = pNode2->GetChild(k);
					CString strTransOption = pNode3->GetValue().MakeUpper();

					// 번역방식
					if(strTransOption == _T("NOP"))
					{
						pTransCmd->SetTransMethod(0);
					}
					else if(strTransOption == _T("PTRCHEAT"))
					{
						pTransCmd->SetTransMethod(1);
					}
					else if(strTransOption == _T("OVERWRITE"))
					{
						pTransCmd->SetTransMethod(2);
					}
					
					// 멀티바이트 / 유니코드 지정
					else if(strTransOption == _T("ANSI"))
					{
						pTransCmd->SetUnicode(FALSE);
					}
					else if(strTransOption == _T("UNICODE"))
					{
						pTransCmd->SetUnicode(TRUE);
					}

					// 모든 일치하는 텍스트 번역
					else if(strTransOption == _T("ALLSAMETEXT"))
					{
						pTransCmd->SetAllSameText(TRUE);
					}

					// CLIPKOR 옵션
					else if(strTransOption == _T("CLIPKOR"))
					{
						pTransCmd->SetClipKor(TRUE);
					}

					// CLIPJPN 옵션
					else if(strTransOption == _T("CLIPJPN"))
					{
						pTransCmd->SetClipJpn(TRUE);
					}

					// REMOVESPACE 옵션
					else if(strTransOption == _T("REMOVESPACE"))
					{
						pTransCmd->SetRemoveSpace(TRUE);
					}

					// TWOBYTE 옵션
					else if(strTransOption == _T("TWOBYTE"))
					{
						pTransCmd->SetTwoByte(TRUE);
					}

					// SAFE 옵션
					else if(strTransOption == _T("SAFE"))
					{
						pTransCmd->SetSafe(TRUE);
					}
				}
			}

		}	// end of for ( 이 주소에 대한 후킹 명령들 수집 )

		bRetVal = TRUE;
	}
	catch (int nErrCode)
	{
		nErrCode = nErrCode; 
	}


	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 클립보드에 텍스트 세팅 작업만 하는 쓰레드
//
//////////////////////////////////////////////////////////////////////////
UINT __stdcall CATCodeMgr::ClipboardThreadFunc(LPVOID pParam)
{
	while(_Inst && _Inst->m_bRunClipboardThread)
	{
		DWORD dwRes = WaitForSingleObject(_Inst->m_hClipTextChangeEvent, 300);

		// 기다려도 변동 없을 때
		if(WAIT_TIMEOUT == dwRes)
		{
			EnterCriticalSection(&_Inst->m_csClipText);

			// 클립보드로 보내야 할 데이터가 있다면
			if(_Inst->m_strClipText.IsEmpty() == FALSE)
			{
				HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (_Inst->m_strClipText.GetLength() + 1) * sizeof(TCHAR));

				LPTSTR pGlobal = (LPTSTR)GlobalLock(hGlobal);
				
				if(pGlobal)
				{
					_tcscpy(pGlobal, (LPCTSTR)_Inst->m_strClipText);
					GlobalUnlock(hGlobal);

					OpenClipboard(NULL);
					EmptyClipboard();

#ifdef UNICODE
					SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
					SetClipboardData(CF_TEXT, hGlobal);
#endif
					
					CloseClipboard();

					//GlobalFree(hGlobal);
				}

				_Inst->m_strClipText.Empty();
			}

			LeaveCriticalSection(&_Inst->m_csClipText);
		}
		// 기다리는 중 텍스트가 추가 되었을 때
		else if(WAIT_OBJECT_0 == dwRes)
		{
			
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 클립보드에 텍스트 세팅
//
//////////////////////////////////////////////////////////////////////////
BOOL CATCodeMgr::SetClipboardText(LPCTSTR cszText)
{
	BOOL bRetVal = FALSE;

	EnterCriticalSection(&m_csClipText);

	m_strClipText += cszText;

	LeaveCriticalSection(&m_csClipText);

	::SetEvent(m_hClipTextChangeEvent);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 새로운 로드 라이브러리 함수 (A)
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryA(LPCSTR lpFileName)
{
	wchar_t wszTmp[MAX_PATH];
	MyMultiByteToWideChar(CP_ACP, 0, lpFileName, -1, wszTmp, MAX_PATH);
	TRACE(_T("[aral1] NewLoadLibraryA('%s') \n"), wszTmp);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		hModule = _Inst->m_pfnLoadLibraryA(lpFileName);

		// 실패했던 후킹들을 재시도
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );
			
			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


//////////////////////////////////////////////////////////////////////////
//
// 새로운 로드 라이브러리 함수 (W)
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryW(LPCWSTR lpFileName)
{
	TRACE(_T("[aral1] NewLoadLibraryW('%s') \n"), lpFileName);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		hModule = _Inst->m_pfnLoadLibraryW(lpFileName);

		// 실패했던 후킹들을 재시도
		for(list<COptionNode*>::iterator iter = _Inst->m_listRetryHook.begin();
			iter != _Inst->m_listRetryHook.end();)
		{
			BOOL bHookRes = _Inst->HookFromOptionNode( (*iter) );

			if(bHookRes) iter = _Inst->m_listRetryHook.erase(iter);
			else iter++;
		}

	}

	return hModule;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CATCodeMgr::NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{	
	HFONT hOrigFont = NULL;
	char chArray[10] = {0,};

	// char 배열에 문자 넣음
	size_t i,j;
	j = 0;
	for(i=sizeof(UINT); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}

	// 폰트 검사
	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	{
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning)
	{
		// 한글 인코딩 여부 검사		
		if(CATCodeMgr::_Inst->m_bEncodeKorean && 0x88 <= (BYTE)chArray[0] && (BYTE)chArray[0] <= 0xEE)
		{
			chArray[2] = '\0';
			char tmpbuf[10]  = {0,};

			if( CCharacterMapper::DecodeJ2K(chArray, tmpbuf) )
			{
				chArray[0] = tmpbuf[0];
				chArray[1] = tmpbuf[1];
				//uChar = (UINT)(MAKEWORD(tmpbuf[1], tmpbuf[0]));
			}

		}  // end of if(CATCodeMgr::_Inst->m_bEncodeKorean)
	}
	

	wchar_t wchArray[10];

	UINT nCodePage = 949;
	if(0x80 < (BYTE)chArray[0] && (BYTE)chArray[0] < 0xA0) nCodePage = 932;		
	MyMultiByteToWideChar(nCodePage, 0, chArray, sizeof(UINT), wchArray, 10 );
	
	DWORD dwRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnGetGlyphOutlineW(hdc, (UINT)wchArray[0], uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	
	// 폰트 복구
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall CATCodeMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	HFONT hOrigFont = NULL;

	if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	{
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	DWORD dwRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnGetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	// 폰트 복구
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return dwRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;


	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// 폰트 검사
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(lpString[a_idx] && a_idx<cbString)
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				//if(CATCodeMgr::_Inst->m_bEncodeKorean && 0xE0 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xFD )
				if(CATCodeMgr::_Inst->m_bEncodeKorean && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{

					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
				}

				UINT nCodePage = 949;
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		//TRACE(_T("[aral1] pfnGetGlyphOutlineW(%d) \n"), wchArray[0]);

		// TextOutW 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, wchArray, w_idx);
	}
	else
	{
		// 원래함수 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}


	// 폰트 복구
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	)
{
	HFONT hOrigFont = NULL;

	//if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	//{
	//	hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	//}

	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnTextOutW(hdc, nXStart, nYStart, lpString, cbString);

	// 폰트 복구
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	return CATCodeMgr::_Inst->m_sTextFunc.pfnExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall CATCodeMgr::NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	)
{
	
	return CATCodeMgr::_Inst->m_sTextFunc.pfnExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}


//////////////////////////////////////////////////////////////////////////
// DrawTextA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextA(
   HDC hDC,          // handle to DC
   LPCSTR lpString,  // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;


	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// 폰트 검사
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hDC);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(lpString[a_idx] && a_idx<nCount)
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				//if(CATCodeMgr::_Inst->m_bEncodeKorean && 0xE0 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xFD )
				if(CATCodeMgr::_Inst->m_bEncodeKorean && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{

					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
				}

				UINT nCodePage = 949;
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		//TRACE(_T("[aral1] pfnGetGlyphOutlineW(%d) \n"), wchArray[0]);

		// DrawTextW 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextW(hDC, wchArray, w_idx, lpRect, uFormat);
	}
	else
	{
		// 원래함수 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}


	// 폰트 복구
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// DrawTextW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextW(
   HDC hDC,          // handle to DC
   LPCWSTR lpString, // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
)
{
	HFONT hOrigFont = NULL;

	//if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	//{
	//	hOrigFont = CATCodeMgr::_Inst->CheckFont(hDC);
	//}

	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextW(hDC, lpString, nCount, lpRect, uFormat);

	// 폰트 복구
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	return bRetVal;

}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextExA(
	 HDC hDC,                     // handle to DC
	 LPSTR lpString,              // text to draw
	 int nCount,                 // length of text to draw
	 LPRECT lpRect,                 // rectangle coordinates
	 UINT uFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
)
{
	//TRACE(_T("[aral1] DrawTextExA( 0x%2x 0x%2x, %d ) \n", ), lpString[0], lpString[1], nCount);

	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;
	HFONT hOrigFont = NULL;


	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5 )
	{
		// 폰트 검사
		hOrigFont = CATCodeMgr::_Inst->CheckFont(hDC);
	}

	if( CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(lpString[a_idx] && a_idx<nCount)
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				//if(CATCodeMgr::_Inst->m_bEncodeKorean && 0xE0 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xFD )
				if(CATCodeMgr::_Inst->m_bEncodeKorean && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{

					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
				}

				UINT nCodePage = 949;
				if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		//TRACE(_T("[aral1] pfnDrawTextExW(%x, %d) \n", ), wchArray[0], w_idx);

		// DrawTextExW 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExW(hDC, wchArray, w_idx, lpRect, uFormat, lpDTParams);
		lpRect->right -= 10;
	}
	else
	{
		// 원래함수 호출
		bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}


	// 폰트 복구
	if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATCodeMgr::NewDrawTextExW(
	 HDC hdc,                     // handle to DC
	 LPWSTR lpchText,              // text to draw
	 int cchText,                 // length of text to draw
	 LPRECT lprc,                 // rectangle coordinates
	 UINT dwDTFormat,             // formatting options
	 LPDRAWTEXTPARAMS lpDTParams  // more formatting options
)
{
	HFONT hOrigFont = NULL;

	//if(CATCodeMgr::_Inst && CATCodeMgr::_Inst->m_bRunning && CATCodeMgr::_Inst->m_nFontLoadLevel >= 5)
	//{
	//	hOrigFont = CATCodeMgr::_Inst->CheckFont(hdc);
	//}

	BOOL bRetVal = CATCodeMgr::_Inst->m_sTextFunc.pfnDrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);

	// 폰트 복구
	//if(hOrigFont && CATCodeMgr::_Inst->m_nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}













int CATCodeMgr::GetAllLoadedModules( PMODULEENTRY32 pRetBuf, int maxCnt )
{
	int curCnt = 0;

	// 반환 버퍼 초기화
	ZeroMemory(pRetBuf, sizeof(PMODULEENTRY32)*maxCnt);
	
	// 프로세스 스냅샷 핸들을 생성
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(INVALID_HANDLE_VALUE == hModuleSnap) return 0;

	pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
	BOOL bExist = Module32First(hModuleSnap, &pRetBuf[curCnt]);

	while( bExist == TRUE && curCnt < maxCnt )
	{
		curCnt++;
		pRetBuf[curCnt].dwSize = sizeof(MODULEENTRY32);
		bExist = Module32Next(hModuleSnap, &pRetBuf[curCnt]);
	}

	CloseHandle (hModuleSnap);

	return curCnt;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontA( 
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
	LPSTR lpszFace )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		wchar_t wszFace[32] = {0,};
		if(lpszFace) MyMultiByteToWideChar(932, 0, lpszFace, 32, wszFace, 32);

		hFont = CATCodeMgr::_Inst->InnerCreateFont(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			wszFace );
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontA(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpszFace );
	}


	TRACE(_T("[aral1] NewCreateFontA returns 0x%p \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontW( 
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
	LPWSTR lpwszFace )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 20)
	{
		hFont = CATCodeMgr::_Inst->InnerCreateFont(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontW(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}


	TRACE(_T("[aral1] NewCreateFontW returns 0x%p \n"), hFont);

	return hFont;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontIndirectA( LOGFONTA* lplf )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		LOGFONTW lfWide;
		ZeroMemory(&lfWide, sizeof(LOGFONTW));
		lfWide.lfCharSet		= lplf->lfCharSet;
		lfWide.lfClipPrecision	= lplf->lfClipPrecision;
		lfWide.lfEscapement		= lplf->lfEscapement;
		lfWide.lfHeight			= lplf->lfHeight;
		lfWide.lfItalic			= lplf->lfItalic;
		lfWide.lfOrientation	= lplf->lfOrientation;
		lfWide.lfOutPrecision	= lplf->lfOutPrecision;
		lfWide.lfPitchAndFamily = lplf->lfPitchAndFamily;
		lfWide.lfQuality		= lplf->lfQuality;
		lfWide.lfStrikeOut		= lplf->lfStrikeOut;
		lfWide.lfUnderline		= lplf->lfUnderline;
		lfWide.lfWeight			= lplf->lfWeight;
		lfWide.lfWidth			= lplf->lfWidth;
		MyMultiByteToWideChar(932, 0, lplf->lfFaceName, 32, lfWide.lfFaceName, 32);

		hFont = CATCodeMgr::_Inst->InnerCreateFontIndirect(&lfWide);
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontIndirectA(lplf);
	}


	TRACE(_T("[aral1] NewCreateFontIndirectA returns 0x%p \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall CATCodeMgr::NewCreateFontIndirectW( LOGFONTW* lplf )
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 20)
	{
		hFont = CATCodeMgr::_Inst->InnerCreateFontIndirect(lplf);
	}
	else
	{
		hFont = CATCodeMgr::_Inst->m_sFontFunc.pfnCreateFontIndirectW(lplf);
	}


	TRACE(_T("[aral1] NewCreateFontIndirectW returns 0x%p \n"), hFont);

	return hFont;
}


HFONT CATCodeMgr::InnerCreateFont(
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
	)
{
	HFONT hFont = NULL;

	if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
	{
		fdwCharSet = HANGEUL_CHARSET;	//ANSI_CHARSET,
		fdwOutputPrecision = OUT_DEFAULT_PRECIS;
		fdwClipPrecision = CLIP_DEFAULT_PRECIS;
		fdwQuality = ANTIALIASED_QUALITY;
		fdwPitchAndFamily = DEFAULT_PITCH;		// | FF_SWISS,

		if(m_strFontFace.IsEmpty())
		{
			m_strFontFace = _T("Gungsuh");
		}

#ifdef UNICODE			
		if(lpszFace) wcscpy(lpszFace, (LPCWSTR)m_strFontFace);
#else
		if(lpszFace) MyMultiByteToWideChar(949, 0, m_strFontFace, -1, lpszFace, 32);
#endif


		// 폰트 크기 고정인 경우
		if(m_lFontSize !=0 && m_bFixedFontSize)
		{
			nHeight = m_lFontSize;
		}

	}


	hFont = m_sFontFunc.pfnCreateFontW(
		nHeight, 
		nWidth, 
		nEscapement, 
		nOrientation, 
		fnWeight, 
		fdwItalic, 
		fdwUnderline, 
		fdwStrikeOut, 
		fdwCharSet, 
		fdwOutputPrecision, 
		fdwClipPrecision, 
		fdwQuality, 
		fdwPitchAndFamily, 
		lpszFace );


	return hFont;	
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT CATCodeMgr::InnerCreateFontIndirect( LOGFONTW* lplf )
{
	//static CString strLastFontFace = _T("");

	HFONT hFont = NULL;

	if(lplf)
	{
		if(CATCodeMgr::_Inst->m_nFontLoadLevel >= 15)
		{
			lplf->lfCharSet = HANGEUL_CHARSET;	//ANSI_CHARSET,
			lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
			lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lplf->lfQuality = ANTIALIASED_QUALITY;
			lplf->lfPitchAndFamily = DEFAULT_PITCH;		// | FF_SWISS,

			if(m_strFontFace.IsEmpty())
			{
				m_strFontFace = _T("Gungsuh");
			}
			
#ifdef UNICODE			
			wcscpy(lplf->lfFaceName, (LPCWSTR)m_strFontFace);
#else
			MyMultiByteToWideChar(949, 0, m_strFontFace, -1, lplf->lfFaceName, 32);
#endif
		
			// 폰트 크기 고정인 경우
			if(m_lFontSize !=0 && m_bFixedFontSize)
			{
				lplf->lfHeight = m_lFontSize;
			}
		}

			

		hFont = m_sFontFunc.pfnCreateFontIndirectW(lplf);
		/*
		// 폰트페이스명이 바뀌었으면 맵 초기화
		if(strLastFontFace.CompareNoCase(lplf->lfFaceName))
		{
			m_mapFonts.clear();
		}
		
		// 이 크기에 해당하는 폰트가 없을 경우 폰트를 생성
		if( m_mapFonts.find(lplf->lfHeight) == m_mapFonts.end() )
		{
			hFont = m_sFontFunc.pfnCreateFontIndirectW(lplf);
			m_mapFonts[lplf->lfHeight] = hFont;
			
			CString strDbg;
			strDbg.Format(_T("[aral1] NewFont:%d \n"), lplf->lfHeight);
			TRACE((LPCTSTR)strDbg);
		}
		// 있으면 맵에서 꺼내줌
		else
		{
			hFont = m_mapFonts[lplf->lfHeight];
			
			CString strDbg;
			strDbg.Format(_T("[aral1] CachedFont:%d \n"), lplf->lfHeight);
			TRACE((LPCTSTR)strDbg);
		}

		strLastFontFace = lplf->lfFaceName;
		*/
		
	}


	//TRACE(_T("[aral1] NewCreateFontIndirectW returns 0x%p \n"), hFont);

	return hFont;	
}

