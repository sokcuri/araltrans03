
//#pragma warning(disable:4312)
//#pragma warning(disable:4313)
//#pragma warning(disable:4996)

#include "stdafx.h"
#include "Resource.h"
#include "ATTextArgMgr.h"
#include "ATTextArg.h"
#include "Function.h"
#include "RegistryMgr/cRegistryMgr.h"
#include "MainDbgDlg.h"

#define TEXT_ARG_POOL_SIZE 100

extern CATTextArgMgr g_objATTextArgMgr;
LPCTSTR CATTextArgMgr::m_arrTextFuncName[] = 
{
	_T("GetGlyphOutlineA"),
	_T("GetGlyphOutlineW"),
	_T("TextOutA"),
	_T("TextOutW"),
	_T("ExtTextOutA"),
	_T("ExtTextOutW"),
	_T("DrawTextA"),
	_T("DrawTextW"),
	_T("DrawTextExA"),
	_T("DrawTextExW")
};
CATTextArgMgr*	CATTextArgMgr::_Inst = NULL;


CATTextArgMgr* CATTextArgMgr::GetInstance()
{
	return _Inst;
}

CATTextArgMgr::CATTextArgMgr(void)
	: m_pMainDbgDlg(NULL), m_pDlgThread(NULL), m_pCurBreakedPoint(0)
{
	_Inst = this;
	ZeroMemory(&m_sATCTNR3, sizeof(CONTAINER_PROC_ENTRY));
	ZeroMemory(&m_sTextFunc, sizeof(TEXT_FUNCTION_ENTRY));
	ZeroMemory(m_aTextFuncHit, sizeof(int)*TEXT_FUNC_CNT);
}


CATTextArgMgr::~CATTextArgMgr(void)
{
	_Inst = NULL;
	Close();
}


UINT CATTextArgMgr::MainDlgThreadFunc( LPVOID pParam )
{
	CATTextArgMgr* pThis = (CATTextArgMgr*)pParam;
	pThis->m_pMainDbgDlg = new CMainDbgDlg();
	pThis->m_pMainDbgDlg->DoModal();
	delete pThis->m_pMainDbgDlg;
	pThis->m_pMainDbgDlg = NULL;

	return 0;
}

// 초기화
BOOL CATTextArgMgr::Init(HWND hAralWnd, LPWSTR wszPluginOption) 
{
	Close();

	BOOL bRetVal = FALSE;

	try
	{
		// 부모 윈도우 핸들 저장
		m_hContainerWnd = hAralWnd;
		if(NULL == m_hContainerWnd || INVALID_HANDLE_VALUE == m_hContainerWnd)
			throw _T("Invalid AralTrans Window Handle!");

		// 컨테이너 함수 포인터 얻어오기
		m_hContainer = GetModuleHandle(_T("ATCTNR3.DLL"));
		if(NULL == m_hContainer || INVALID_HANDLE_VALUE == m_hContainer)
			throw _T("Can not find ATCTNR3.DLL handle!");

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


		if(!(m_sATCTNR3.procHookWin32Api && m_sATCTNR3.procUnhookWin32Api
			&& m_sATCTNR3.procHookCodePoint && m_sATCTNR3.procUnhookCodePoint
			&& m_sATCTNR3.procCreateTransCtx && m_sATCTNR3.procDeleteTransCtx
			&& m_sATCTNR3.procTranslateUsingCtx && m_sATCTNR3.procIsAppLocaleLoaded
			&& m_sATCTNR3.procSuspendAllThread && m_sATCTNR3.procResumeAllThread
			&& m_sATCTNR3.procIsAllThreadSuspended))
			throw _T("Failed to get container procedures!");

		// Create context
		if(m_sATCTNR3.procCreateTransCtx(L"DebuggingATCode") == FALSE)
			throw _T("Failed to create a context 'Debugging'!");
		
		// 모든 쓰레드 정지
		m_sATCTNR3.procSuspendAllThread();

		// 텍스트 함수군 후킹		
		// GetGlyphOutlineA
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA, 1 ) == FALSE)
			throw _T("Failed to hook 'GetGlyphOutlineA'!");

		// GetGlyphOutlineW
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW, 1 ) == FALSE)
			throw _T("Failed to hook 'GetGlyphOutlineW'!");

		// TextOutA
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA, 1 ) == FALSE)
			throw _T("Failed to hook 'TextOutA'!");

		// TextOutW
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW, 1 ) == FALSE)
			throw _T("Failed to hook 'TextOutW'!");

		// ExtTextOutA
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA, 1 ) == FALSE)
			throw _T("Failed to hook 'ExtTextOutA'!");

		// ExtTextOutW
		if( m_sATCTNR3.procHookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW, 1 ) == FALSE)
			throw _T("Failed to hook 'ExtTextOutW'!");

		// DrawTextA
		if( m_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA, 1 ) == FALSE)
			throw _T("Failed to hook 'DrawTextA'!");

		// DrawTextW
		if( m_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW, 1 ) == FALSE)
			throw _T("Failed to hook 'DrawTextW'!");

		// DrawTextExA
		if( m_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA, 1 ) == FALSE)
			throw _T("Failed to hook 'DrawTextExA'!");

		// DrawTextExW
		if( m_sATCTNR3.procHookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW, 1 ) == FALSE)
			throw _T("Failed to hook 'DrawTextExW'!");

		// 어플로케일 관련 함수
		m_sTextFunc.pfnOrigMultiByteToWideChar =
			(PROC_MultiByteToWideChar) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("M2WAddr"));

		m_sTextFunc.pfnOrigWideCharToMultiByte =
			(PROC_WideCharToMultiByte) CRegistryMgr::RegReadDWORD(_T("HKEY_CURRENT_USER\\Software\\AralGood"), _T("W2MAddr"));

		// 어플로케일 검사
		m_bApplocale = m_sATCTNR3.procIsAppLocaleLoaded();

		// 텍스트 인자 풀 생성
		for(int i=0; i<TEXT_ARG_POOL_SIZE; i++)
		{
			m_setInactivatedArgs.insert(new CATText());
		}

		// 디버깅 윈도우 생성
		m_pDlgThread = AfxBeginThread(MainDlgThreadFunc, this);
		while(m_pMainDbgDlg == NULL) Sleep(100);

		// 이벤트 생성
		m_hBreak = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hResume = ::CreateEvent(NULL, TRUE, TRUE, NULL);

		// 모든 쓰레드 재가동
		m_sATCTNR3.procResumeAllThread();

		bRetVal = TRUE;
	}
	catch (LPCTSTR strErr)
	{
		::MessageBox(m_hContainerWnd, strErr, _T("Debuggin AT Code"), MB_OK | MB_TOPMOST);
		Close();
		bRetVal = FALSE;
	}

	return bRetVal;
}

// 말기화
BOOL CATTextArgMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	m_hContainerWnd = NULL;

	// 이벤트 제거
	if(m_hBreak)
	{
		CloseHandle(m_hBreak);
		m_hBreak = NULL;
	}

	if(m_hResume)
	{
		SetEvent(m_hResume);
		CloseHandle(m_hResume);
		m_hResume = NULL;
	}


	// 메인 디버깅 윈도우 파괴
	if(m_pMainDbgDlg && ::IsWindow(m_pMainDbgDlg->m_hWnd))
	{
		m_pMainDbgDlg->PostMessage(WM_CLOSE,0,0);
		for(int k=0; k<10 && NULL != m_pMainDbgDlg; k++) Sleep(300);
	}

	// 모든 쓰레드 정지
	m_sATCTNR3.procSuspendAllThread();

	// GetGlyphOutlineA 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineA", NewGetGlyphOutlineA );
	// GetGlyphOutlineW 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"GetGlyphOutlineW", NewGetGlyphOutlineW );
	// TextOutA 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"TextOutA", NewTextOutA );
	// TextOutW 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"TextOutW", NewTextOutW );
	// ExtTextOutA 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutA", NewExtTextOutA );
	// ExtTextOutW 언훅
	m_sATCTNR3.procUnhookWin32Api( L"GDI32.DLL", L"ExtTextOutW", NewExtTextOutW );
	// DrawTextA 언훅
	m_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextA", NewDrawTextA );
	// DrawTextW 언훅
	m_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextW", NewDrawTextW );
	// DrawTextExA 언훅
	m_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExA", NewDrawTextExA );
	// DrawTextExW 언훅
	m_sATCTNR3.procUnhookWin32Api( L"USER32.DLL", L"DrawTextExW", NewDrawTextExW );

	// Delete context
	m_sATCTNR3.procDeleteTransCtx(L"DebuggingATCode");

	// 활성화 텍스트 인스턴스 모두 삭제
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CATText* pATText = *(iter);
		delete pATText;
	}
	m_setActivatedArgs.clear();
	
	// 비활성화 텍스트 인스턴스 모두 삭제
	for(CATTextSet::iterator iter = m_setInactivatedArgs.begin();
		iter != m_setInactivatedArgs.end();
		iter++)
	{
		CATText* pATText = *(iter);
		delete pATText;
	}
	m_setInactivatedArgs.clear();

	// 적중한 거리들 삭제
	m_mapHitDist.clear();

	// 함수 목록 삭제
	for( CFunctionMap::iterator iter2 = m_mapFunc.begin();
		iter2 != m_mapFunc.end();
		iter2++)
	{
		CFunction* pFunc = iter2->second;
		delete pFunc;
	}
	m_mapFunc.clear();

	// 후킹한 함수들 해제
	for( CHookedPoints::iterator iter3 = m_setHookedPoints.begin();
		iter3 != m_setHookedPoints.end();
		iter3++)
	{
		m_sATCTNR3.procUnhookCodePoint( (LPVOID)(*iter3), BreakRoutine );
	}
	m_setHookedPoints.clear();
	

	// 기타 변수 리셋
	m_bApplocale = FALSE;
	ZeroMemory(m_aTextFuncHit, sizeof(int)*TEXT_FUNC_CNT);

	// 모든 쓰레드 재가동
	m_sATCTNR3.procResumeAllThread();

	return TRUE;

}



// 새로운 문자열 후보를 추가한다
int CATTextArgMgr::AddTextArg(LPVOID pText, BOOL bWideChar, UINT_PTR ptrFunc, size_t dist)
{
	BOOL nRetVal = 0;

	CATText* pATText = NULL;
	
	// 기존에 들어있는 문자열인지 검사 (활성화 텍스트 인스턴스 모두 순환)
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();
		iter++)
	{
		CATText* pTmpATText = *(iter);

		if( pTmpATText->IsWideCharacter() == bWideChar && pTmpATText->TestText(pText) )
		{
			pATText = pTmpATText;
			nRetVal = 2;
		}		
	}		


	// 삽입이 필요하면 빈노드 또는 최악의 노드를 가져다 추가한다
	if( NULL == pATText)
	{	
		if(!m_setInactivatedArgs.empty()) 
		{
			CATTextSet::iterator iter = m_setInactivatedArgs.begin();
			pATText = ( *iter );
			m_setInactivatedArgs.erase(pATText);
			m_setActivatedArgs.insert(pATText);
		}
		else
		{
			// 활성화 텍스트 인스턴스 모두 순환
			for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
				iter != m_setActivatedArgs.end();
				iter++)
			{
				CATText* pTmpATText = *(iter);

				if( NULL == pATText || (pATText->GetHitCount() > pTmpATText->GetHitCount()) )
				{
					pATText = pTmpATText;
				}		
			}		
		}
			

		if( pATText->SetATText(pText, bWideChar) )
		{
			nRetVal = 1;
		}
		else
		{
			m_setActivatedArgs.erase( pATText );
			m_setInactivatedArgs.insert(pATText);
			pATText = NULL;
		}
	}

	// 결과가 성공이면 
	if( nRetVal!=0 )
	{
		// 당시 스택 정보 추가
		if( ptrFunc && dist )
		{
			pATText->m_setFuncArg.insert( pair<UINT_PTR,size_t>(ptrFunc,dist) );
		}
	}

	return nRetVal;
}

// 문자열 후보들 전체를 테스트한다. (더이상 일치하지 않는 후보는 바로 삭제)
BOOL CATTextArgMgr::TestCharacter(wchar_t wch, void* baseESP)
{
	BOOL bRetVal = FALSE;

	// 활성화 텍스트 인스턴스를 모두 순회
	for(CATTextSet::iterator iter = m_setActivatedArgs.begin();
		iter != m_setActivatedArgs.end();)
	{
		CATText* pATText = (*iter);
		iter++;
		
		// 검사 수행
		int nRes = pATText->TestCharacter(wch);
		
		// 히트(유예)시
		if( 0 != nRes )
		{
			if( nRes & 0x01 )
			{
				ModifyHitMap(pATText, baseESP, +1);
				bRetVal = TRUE;

				for(set<pair<UINT_PTR,size_t>>::iterator iter = pATText->m_setFuncArg.begin();
					iter != pATText->m_setFuncArg.end();
					iter++)
				{
					CFunction* pFunc = m_mapFunc[iter->first];
					size_t distArg = iter->second;
					
					if(pFunc && distArg)
					{
						pFunc->m_mapDistScores[distArg]++;
#ifdef UNICODE
						pFunc->m_strLastJapaneseText = pATText->m_wszJapaneseText;
						pFunc->m_strLastKoreanText = pATText->m_wszKoreanText;
#else
						pFunc->m_strLastJapaneseText = pATText->m_szJapaneseText;
						pFunc->m_strLastKoreanText = pATText->m_szKoreanText;
#endif

						
						// 만일 특정수치 이상 적중되었으면 이부분 후킹
						//if( pFunc->m_mapDistScores[distArg] > 30 
						//	&& m_sATCTNR3.procHookCodePoint
						//	&& m_mapArgInfoA.find(pFunc->m_ptrFunction) == m_mapArgInfoA.end()
						//	&& m_mapArgInfoW.find(pFunc->m_ptrFunction) == m_mapArgInfoW.end() )
						//{
						//	HMODULE hExeMod = GetModuleHandle(NULL);
						//	HMODULE hHookMod = NULL;

						//	if( GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)pFunc->m_ptrFunction, &hHookMod)
						//		&& hExeMod == hHookMod )
						//	{
						//		PRESERVED_HOOK_POINT pRHP = new RESERVED_HOOK_POINT;
						//		pRHP->bWideChar	= pATText->m_bWideChar;
						//		pRHP->nArgDist	= distArg;
						//		pRHP->pHookPoint	= pFunc->m_ptrFunction;
						//		m_setReservedHooks.insert(pRHP);
						//		TRACE("[ aral1 ] Function 0x%p(+%d) was reserved for hook \n", pFunc->m_ptrFunction, distArg);
						//	}
						//}

					}
				}
			}
		}
		// 폐기시
		else
		{
			for(set<pair<UINT_PTR,size_t>>::iterator iter = pATText->m_setFuncArg.begin();
				iter != pATText->m_setFuncArg.end();
				iter++)
			{
				CFunction* pFunc = m_mapFunc[iter->first];
				size_t distArg = iter->second;

				if(pFunc && distArg)
				{
					pFunc->m_mapDistScores[distArg]--;
				}
			}

			ModifyHitMap(pATText, baseESP, -1);
			m_setActivatedArgs.erase(pATText);
			m_setInactivatedArgs.insert(pATText);
		}
	}	

	return bRetVal;
}


UINT_PTR CATTextArgMgr::GetFuncAddrFromReturnAddr(UINT_PTR pAddr)
{
	UINT_PTR funcAddr = NULL;

	__try
	{
		if( !IsBadReadPtr( (void*)pAddr, sizeof(void*) ) )
		{
			// 함수콜 모양인지 검사
			BYTE* pRetAddr = (BYTE*)pAddr;
			if( 0xE8 == *(pRetAddr-5) )	// call 코드 case 1
			{
				UINT_PTR func_dist = *( (UINT_PTR*)(pRetAddr-4) );		// 이동거리 구하기
				funcAddr = pAddr + func_dist;							// 구해진 함수주소
			}
			else if( 0xFF == *(pRetAddr-6) && 0x15 == *(pRetAddr-5) )	// call 코드 case 2
			{
				funcAddr = **( (UINT_PTR**)(pRetAddr-4) );				// 구해진 함수주소
			}


			// 함수 포인터 유효성 검사
			if( funcAddr && IsBadCodePtr((FARPROC)funcAddr) )
			{
				funcAddr = NULL;
			}

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	return funcAddr;

}


int CATTextArgMgr::SearchStringA(INT_PTR ptrBegin, char ch1, char ch2)
{
	int iRetVal = 0;
	
	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);
		LPVOID found = NULL;

		if( IsBadStringPtrA(*ppText, 1024*1024*1024)==FALSE && strlen(*ppText) < 1024 )
		{
			if('\0'==ch2)
			{
				found = (LPVOID)strchr(*ppText, ch1);
			}
			else
			{
				char tmp[4] = {0,};
				tmp[0] = ch1;
				tmp[1] = ch2;
				found = (LPVOID)strstr(*ppText, tmp);
			}
		}

		// 문자열이라면
		if(found)
		{
			int nAddRes = AddTextArg( *ppText, FALSE, (pCurFunc?pCurFunc->m_ptrFunction:NULL), arg_dist );
			if( nAddRes )
			{
				// 인자 점수 등록
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// 함수 리턴주소라면 현재 지정된 함수를 교체한다
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// 지금 리스트에 존재하면
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// 없으면 새로 생성 & 추가
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}
				
				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);
	}
	
	TRACE(" [ aral1 ] 찾은거리:0x%p~0x%p (%d bytes) \n", ptrBegin, ptrBegin+dist, dist);

	return iRetVal;
}

int CATTextArgMgr::SearchStringW(INT_PTR ptrBegin, wchar_t wch)
{
	int iRetVal = 0;

	size_t dist = 0;
	size_t arg_dist = 0;
	CFunction* pCurFunc = NULL;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// 문자열이라면
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE && wcslen(*ppText) < 1024 && NULL!=wcschr(*ppText, wch) )
		{

			int nAddRes = AddTextArg( *ppText, TRUE, pCurFunc->m_ptrFunction, arg_dist );
			if( nAddRes )
			{
				// 인자 점수 등록
				m_mapHitDist.insert( pair<size_t, int>(dist, 0) );
				iRetVal++;

			}
		}
		else
		{
			UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

			// 함수 리턴주소라면 현재 지정된 함수를 교체한다
			if( funcAddr )
			{
				CFunctionMap::iterator iter = m_mapFunc.find( funcAddr );
				// 지금 리스트에 존재하면
				if( iter != m_mapFunc.end() )
				{
					pCurFunc = iter->second;
				}
				// 없으면 새로 생성 & 추가
				else
				{
					pCurFunc = new CFunction(funcAddr);
					m_mapFunc[funcAddr] = pCurFunc;
				}

				arg_dist = 0;
			}
		}

		dist += sizeof(void*);
		arg_dist += sizeof(void*);

	}	

	return iRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// 인자 점수 증감 처리
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::ModifyHitMap( CATText* pATText, void* baseESP, int increment ) 
{
	// 인자 점수 증가
	for(map<size_t,int>::iterator iter = m_mapHitDist.begin();
		iter != m_mapHitDist.end();)
	{
		void** ppSource = (void**)( (UINT_PTR)baseESP + iter->first );
		
		// 텍스트인자에 해당 문자열 포인터가 있으면
		if( IsBadReadPtr(ppSource, sizeof(void*)) == FALSE
			 && pATText->m_setSourcePtr.find(*ppSource) != pATText->m_setSourcePtr.end() )
		{
			iter->second += increment;
		}

		if( iter->second > 10 )
		{
			iter->second = 10;
			iter++;
		}
		// 더이상 필요없으면 삭제
		else if( iter->second < 0 )
		{
			size_t key = iter->first;
			iter++;
			m_mapHitDist.erase(key);
		}
		// 아직 존재해야 한다면
		else
		{
			iter++;
		}
	}

}



//////////////////////////////////////////////////////////////////////////
//
// 브레이크 포인트의 콜백 함수
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::BreakRoutine(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters)
{
	if( CATTextArgMgr::_Inst )
	{
		CATTextArgMgr::_Inst->OnBreakPoint(pHookedPoint, pRegisters);
	}

}


//////////////////////////////////////////////////////////////////////////
//
// 브레이크 포인트 처리
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::OnBreakPoint( LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters )
{
	if( NULL==m_pMainDbgDlg || FALSE==IsWindow(m_pMainDbgDlg->GetSafeHwnd()) )
	{
		return;
	}

	// 현재 문자셋 종류
	int nMainTextFunc = GetMainTextFunction();

	BOOL bWideChar;

	if( nMainTextFunc==UseGetGlyphOutlineA
		|| nMainTextFunc==UseTextOutA 
		|| nMainTextFunc==UseExtTextOutA 
		|| nMainTextFunc==UseDrawTextA 
		|| nMainTextFunc==UseDrawTextExA )
	{
		bWideChar = FALSE;
	}
	else
	{
		bWideChar = TRUE;
	}

	// Refresh Function List
	m_pCurBreakedPoint = (UINT_PTR)pHookedPoint;
	RefreshFunctionList();


	// UI 갱신 : 레지스터&스택
	m_pMainDbgDlg->GetDlgItem(IDC_STATIC_REGNSTACK)->EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlRegStackList.EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlRegStackList.DeleteAllItems();
	
	AddRegAndStackDump(_T("EAX"), (UINT_PTR)pRegisters->_EAX, bWideChar);
	AddRegAndStackDump(_T("EBX"), (UINT_PTR)pRegisters->_EBX, bWideChar);
	AddRegAndStackDump(_T("ECX"), (UINT_PTR)pRegisters->_ECX, bWideChar);
	AddRegAndStackDump(_T("EDX"), (UINT_PTR)pRegisters->_EDX, bWideChar);
	AddRegAndStackDump(_T("ESI"), (UINT_PTR)pRegisters->_ESI, bWideChar);
	AddRegAndStackDump(_T("EDI"), (UINT_PTR)pRegisters->_EDI, bWideChar);
	AddRegAndStackDump(_T("EBP"), (UINT_PTR)pRegisters->_EBP, bWideChar);
	AddRegAndStackDump(_T("ESP"), (UINT_PTR)pRegisters->_ESP, bWideChar);
	
	for(int i = 0; i < 20; i++)
	{
		CString strAddr;
		strAddr.Format(_T("[ESP+%x]"), i*4 );
		AddRegAndStackDump(strAddr, *((UINT_PTR*)(pRegisters->_ESP + i*4)), bWideChar);
	}


	// 콜스택
	m_pMainDbgDlg->GetDlgItem(IDC_STATIC_CALLSTACK)->EnableWindow(TRUE);
	m_pMainDbgDlg->m_ctrlCallstack.EnableWindow(TRUE);
	FillCallstackCtrl(pRegisters->_ESP);
	m_pMainDbgDlg->SetForegroundWindow();
	m_pMainDbgDlg->SetFocus();

	::ResetEvent(m_hResume);
	::WaitForSingleObject(m_hResume, INFINITE);

}

//////////////////////////////////////////////////////////////////////////
//
// 브레이크 걸렸던 프로그램 재개
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::OnResumeProgram()
{
	if(::IsWindow(m_pMainDbgDlg->m_hWnd))
	{
		// Refresh Function List
		m_pCurBreakedPoint = 0;
		RefreshFunctionList();

		// UI 갱신 : 레지스터&스택
		m_pMainDbgDlg->GetDlgItem(IDC_STATIC_REGNSTACK)->EnableWindow(FALSE);
		m_pMainDbgDlg->m_ctrlRegStackList.EnableWindow(FALSE);

		// 콜스택
		m_pMainDbgDlg->GetDlgItem(IDC_STATIC_CALLSTACK)->EnableWindow(FALSE);
		m_pMainDbgDlg->m_ctrlCallstack.EnableWindow(FALSE);

	}
	
	::SetEvent(m_hResume);
}

//////////////////////////////////////////////////////////////////////////
//
// 레지스터&스택에 한 줄 추가
//
//////////////////////////////////////////////////////////////////////////
void CATTextArgMgr::AddRegAndStackDump( LPCTSTR cszStorage, UINT_PTR val, BOOL bWideChar )
{
	if(NULL==m_pMainDbgDlg || NULL==cszStorage) return;

	int nIdx = m_pMainDbgDlg->m_ctrlRegStackList.GetItemCount();
	CString strVal;
	strVal.Format(_T("%p"), val);
	m_pMainDbgDlg->m_ctrlRegStackList.InsertItem(nIdx, cszStorage);
	m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 1, strVal);
	
	CATText attext;
	if( attext.SetATText((LPVOID)val, bWideChar) )
	{
		CString strText;

#ifdef UNICODE
		LPCTSTR cszJpnText = attext.m_wszJapaneseText;
		LPCTSTR cszKorText = attext.m_wszKoreanText;
#else
		LPCTSTR cszJpnText = attext.m_szJapaneseText;
		LPCTSTR cszKorText = attext.m_szKoreanText;
#endif

		strText.Format(_T("%s\"%s\""), (bWideChar ? _T("L") : _T("")), cszJpnText);
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 2, (LPCTSTR)strText);
		
		strText.Format(_T("%s\"%s\""), (bWideChar ? _T("L") : _T("")), cszKorText);
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 3, (LPCTSTR)strText);
	}
	else
	{
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 2, _T("< Bad Text Pointer >"));
		m_pMainDbgDlg->m_ctrlRegStackList.SetItemText(nIdx, 3, _T("< Bad Text Pointer >"));
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






































////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	if(CATTextArgMgr::_Inst)
	{
		char chArray[10] = {0,};
		wchar_t wchArray[10] = {0,};
		wchar_t wch = L'\0';

		// 바로 앞 전 EBP를 구함
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

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

		MyMultiByteToWideChar(932, 0, chArray, sizeof(UINT), wchArray, 10 );

		wch = wchArray[0];

		// 검사중인 텍스트 포인터들 모두 순회
		BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

		// 적중된 포인터가 없다면 검색
		if( FALSE == bHitOnce && wch > 0x80)
		{			
			// 검색
			int iRes = CATTextArgMgr::_Inst->SearchStringA(_CUR_EBP, chArray[0], chArray[1]);
			if(iRes)
			{
				CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseGetGlyphOutlineA]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	return GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
DWORD CATTextArgMgr::NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	if(CATTextArgMgr::_Inst)
	{
		// 바로 앞 전 EBP를 구함
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		wchar_t wch = (wchar_t)uChar;

		// 검사중인 텍스트 포인터들 모두 순회
		BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

		// 적중된 포인터가 없다면 검색
		if( FALSE == bHitOnce && wch > 0x80 )
		{			

			// 검색
			int iRes = CATTextArgMgr::_Inst->SearchStringW(_CUR_EBP, wch);
			if(iRes)
			{
				CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
			}
		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseGetGlyphOutlineW]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	return GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewTextOutA(
   HDC hdc,           // handle to DC
   int nXStart,       // x-coordinate of starting position
   int nYStart,       // y-coordinate of starting position
   LPCSTR lpString,   // character string
   int cbString       // number of characters
   )
{
	if(CATTextArgMgr::_Inst)
	{
		// 바로 앞 전 EBP를 구해서
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		if( cbString<=2 || strlen(lpString)<=2 )
		{
			wchar_t wchArray[10] = {0,};
			MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

			wchar_t wch = wchArray[0];

			// 검사중인 텍스트 포인터들 모두 순회
			BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

			// 적중된 포인터가 없다면 검색
			if( FALSE == bHitOnce && wch > 0x80 )
			{			
				// 검색
				int iRes = CATTextArgMgr::_Inst->SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
				if(iRes)
				{
					CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
				}
			}

		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseTextOutA]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	// 원래함수 호출
	return TextOutA(hdc, nXStart, nYStart, lpString, cbString);
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewTextOutW(
   HDC hdc,           // handle to DC
   int nXStart,       // x-coordinate of starting position
   int nYStart,       // y-coordinate of starting position
   LPCWSTR lpString,   // character string
   int cbString       // number of characters
   )
{
	if(CATTextArgMgr::_Inst)
	{
		// 바로 앞 전 EBP를 구해서
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		if( cbString==1 || wcslen(lpString)==1 )
		{
			wchar_t wch = lpString[0];

			// 검사중인 텍스트 포인터들 모두 순회
			BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

			// 적중된 포인터가 없다면 검색
			if( FALSE == bHitOnce && wch > 0x80 )
			{			
				// 검색
				int iRes = CATTextArgMgr::_Inst->SearchStringW(_CUR_EBP, wch);
				if(iRes)
				{
					CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
				}
			}

		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseTextOutW]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	// 원래함수 호출
	return TextOutW(hdc, nXStart, nYStart, lpString, cbString);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewExtTextOutA(
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
	if(CATTextArgMgr::_Inst)
	{
		// 바로 앞 전 EBP를 구해서
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		if( cbCount<=2 || strlen(lpString)<=2 )
		{
			wchar_t wchArray[10] = {0,};
			MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

			wchar_t wch = wchArray[0];

			// 검사중인 텍스트 포인터들 모두 순회
			BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

			// 적중된 포인터가 없다면 검색
			if( FALSE == bHitOnce && wch > 0x80 )
			{			
				// 검색
				int iRes = CATTextArgMgr::_Inst->SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
				if(iRes)
				{
					CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
				}
			}

		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseExtTextOutA]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	// 원래함수 호출
	return ExtTextOutA(
		hdc,          // handle to DC
		X,            // x-coordinate of reference point
		Y,            // y-coordinate of reference point
		fuOptions,   // text-output options
		lprc, // optional dimensions
		lpString, // string
		cbCount,     // number of characters in string
		lpDx   // array of spacing values
		);
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL CATTextArgMgr::NewExtTextOutW(
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
	return ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}


//////////////////////////////////////////////////////////////////////////
// DrawTextA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextA(
   HDC hDC,          // handle to DC
   LPCSTR lpString,  // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	if(CATTextArgMgr::_Inst)
	{
		BOOL bRetVal = FALSE;

		// 바로 앞 전 EBP를 구해서
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		if( nCount<=2 || strlen(lpString)<=2 )
		{
			wchar_t wchArray[10] = {0,};
			MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

			wchar_t wch = wchArray[0];

			// 검사중인 텍스트 포인터들 모두 순회
			BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

			// 적중된 포인터가 없다면 검색
			if( FALSE == bHitOnce && wch > 0x80 )
			{			
				// 검색
				int iRes = CATTextArgMgr::_Inst->SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
				if(iRes)
				{
					CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
				}
			}

		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseDrawTextA]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	// 원래함수 호출
	return DrawTextA(hDC, lpString, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
// DrawTextW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextW(
   HDC hDC,          // handle to DC
   LPCWSTR lpString, // text to draw
   int nCount,       // text length
   LPRECT lpRect,    // formatting dimensions
   UINT uFormat      // text-drawing options
   )
{
	return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextExA(
	HDC hDC,                     // handle to DC
	LPSTR lpString,              // text to draw
	int nCount,                 // length of text to draw
	LPRECT lpRect,                 // rectangle coordinates
	UINT uFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{
	if(CATTextArgMgr::_Inst)
	{
		// 바로 앞 전 EBP를 구해서
		INT_PTR _CUR_EBP = NULL;
		_asm
		{
			mov eax, [ebp];
			mov _CUR_EBP, eax;
		}

		if( nCount<=2 || strlen(lpString)<=2 )
		{
			wchar_t wchArray[10] = {0,};
			MyMultiByteToWideChar(932, 0, lpString, sizeof(UINT), wchArray, 10 );

			wchar_t wch = wchArray[0];

			// 검사중인 텍스트 포인터들 모두 순회
			BOOL bHitOnce = CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);

			// 적중된 포인터가 없다면 검색
			if( FALSE == bHitOnce && wch > 0x80 )
			{			
				// 검색
				int iRes = CATTextArgMgr::_Inst->SearchStringA(_CUR_EBP, lpString[0], lpString[1]);
				if(iRes)
				{
					CATTextArgMgr::_Inst->TestCharacter(wch, (void*)_CUR_EBP);
				}
			}

		}

		CATTextArgMgr::_Inst->m_aTextFuncHit[UseDrawTextExA]++;
		CATTextArgMgr::_Inst->RefreshFunctionList();
	}

	// 원래함수 호출
	return DrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);

}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall CATTextArgMgr::NewDrawTextExW(
	HDC hDC,                     // handle to DC
	LPWSTR lpString,              // text to draw
	int nCount,                 // length of text to draw
	LPRECT lpRect,                 // rectangle coordinates
	UINT uFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{
	return DrawTextExW(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
}







BOOL CATTextArgMgr::Start()
{
	m_bRunning = TRUE;
	return TRUE;
}

BOOL CATTextArgMgr::Stop()
{
	m_bRunning = FALSE;
	return TRUE;
}

BOOL CATTextArgMgr::Option()
{
	//if(m_hContainerWnd && IsWindow(m_hContainerWnd))
	//{
	//	::MessageBox( m_hContainerWnd, "현재 지원되지 않는 기능입니다.", "Cached Plugin", MB_OK );
	//}

	if( m_pMainDbgDlg && IsWindow(m_pMainDbgDlg->GetSafeHwnd()) )
	{
		m_pMainDbgDlg->ShowWindow(SW_SHOW);
		m_pMainDbgDlg->SetFocus();
	}
	return TRUE;
}


int CATTextArgMgr::GetMainTextFunction()
{
	int nRetVal = 0;
	
	for(int i=1; i<TEXT_FUNC_CNT; i++)
	{
		if( m_aTextFuncHit[nRetVal] < m_aTextFuncHit[i] )
		{
			nRetVal = i;
		}
	}

	return nRetVal;
}

void CATTextArgMgr::RefreshFunctionList()
{
	if(m_pMainDbgDlg && IsWindow(m_pMainDbgDlg->m_ctrlFuncList.GetSafeHwnd()))
	{
		m_pMainDbgDlg->m_ctrlFuncList.DeleteAllItems();
		int nIdx = 0;
		CString strTemp;

		// EXE 모듈
		HMODULE hExeMod = GetModuleHandle(NULL);

		// 함수 목록 순회
		for( CFunctionMap::iterator iter = m_mapFunc.begin();
			iter != m_mapFunc.end();
			iter++)
		{
			CFunction* pFunc = iter->second;

			HMODULE hCurMod = NULL;
			BOOL bGetRes = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pFunc->m_ptrFunction, &hCurMod);

			if(FALSE == bGetRes || NULL == hCurMod) continue;

			// 함수주소
			if( hExeMod == hCurMod )
			{
				strTemp.Format(_T("0x%p"), pFunc->m_ptrFunction);
			}
			else
			{
				TCHAR szModPathName[MAX_PATH];
				GetModuleFileName(hCurMod, szModPathName, MAX_PATH);
				LPTSTR szModName = _tcsrchr(szModPathName, _T('\\'));
				if(szModName) szModName++;
				else szModName = szModPathName;
				strTemp.Format(_T("%s!0x%X"), szModName, pFunc->m_ptrFunction - (UINT_PTR)hCurMod);
			}

			// 후킹된 함수면 하이라이트
			if( m_setHookedPoints.find(pFunc->m_ptrFunction) != m_setHookedPoints.end() )
			{
				//m_pMainDbgDlg->m_ctrlFuncList.SetTextBkColor(RGB(255,0,0));

				if(m_pCurBreakedPoint == pFunc->m_ptrFunction) strTemp = _T("▶") + strTemp;
				else strTemp = _T("▷") + strTemp;
			}
			m_pMainDbgDlg->m_ctrlFuncList.InsertItem(nIdx, (LPCTSTR)strTemp);
			m_pMainDbgDlg->m_ctrlFuncList.SetItemData(nIdx, pFunc->m_ptrFunction);

			// 인자점수
			strTemp = _T("");
			for( map<size_t,int>::iterator iter2 = pFunc->m_mapDistScores.begin();
				iter2 != pFunc->m_mapDistScores.end();
				iter2++)
			{
				CString strTemp2;
				strTemp2.Format(_T("ESP+0x%x(%dhit)"), iter2->first, iter2->second);

				if(!strTemp.IsEmpty()) strTemp += _T(", ");
				strTemp += strTemp2;
			}
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 1, (LPCTSTR)strTemp);

			// 마지막 텍스트
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 2, (LPCTSTR)pFunc->m_strLastJapaneseText);
			m_pMainDbgDlg->m_ctrlFuncList.SetItemText(nIdx, 3, (LPCTSTR)pFunc->m_strLastKoreanText);

			// 주 출력함수(TextOutA같은..) 이름 표시
			// 2008.11.13 by sc.Choi
			CString strLblText;
			strLblText.Format(_T("Main Text Function : %s"), m_arrTextFuncName[GetMainTextFunction()]);
			m_pMainDbgDlg->m_lblMainTextFunc.SetWindowText(strLblText);

			nIdx++;
		}

	}
}

void CATTextArgMgr::OnSetBreakPointOnFuncList( int nIdx )
{
	if( NULL == m_pMainDbgDlg || NULL == m_sATCTNR3.procHookCodePoint ) return;
	
	UINT_PTR pFunc = m_pMainDbgDlg->m_ctrlFuncList.GetItemData(nIdx);

	if( pFunc )
	{
		// 후킹되어 있는 코드라면
		if( m_setHookedPoints.find(pFunc) != m_setHookedPoints.end() )
		{
			// 후킹 해제
			BOOL bUnhook = m_sATCTNR3.procUnhookCodePoint( (LPVOID)pFunc, BreakRoutine );
			m_setHookedPoints.erase(pFunc);
			TRACE(_T("[ aral1 ] Unhook the Code 0x%p Result : %d \n"), pFunc, bUnhook);
		}
		// 새로 후킹해야 할 코드라면
		else
		{
			// 후킹
			BOOL bHooked = m_sATCTNR3.procHookCodePoint( (LPVOID)pFunc, BreakRoutine, 1 );
			m_setHookedPoints.insert(pFunc);
			TRACE(_T("[ aral1 ] Hooking the Code 0x%p Result : %d \n"), pFunc, bHooked);
		}

		RefreshFunctionList();
	}
}

void CATTextArgMgr::FillCallstackCtrl( UINT_PTR ptrBegin )
{
	// Clear listbox control
	while(m_pMainDbgDlg->m_ctrlCallstack.GetCount())
	{
		m_pMainDbgDlg->m_ctrlCallstack.DeleteString(0);
	}

	// Create current callstack
	size_t dist = 0;

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{

		UINT_PTR funcAddr = GetFuncAddrFromReturnAddr( *((UINT_PTR*)(ptrBegin+dist)) );

		// 함수 리턴주소라면 현재 지정된 함수를 교체한다
		if( funcAddr )
		{
			CString strFuncAddr;
			strFuncAddr.Format(_T("0x%p"), funcAddr);
			m_pMainDbgDlg->m_ctrlCallstack.AddString(strFuncAddr);
		}

		dist += sizeof(void*);
	}

}
