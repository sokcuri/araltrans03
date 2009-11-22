
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include <process.h>
#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
//#include "RegistryMgr/cRegistryMgr.h"
//#include "CharacterMapper.h"
#include "OptionDlg.h"

CATCodeMgr*	CATCodeMgr::_Inst = NULL;

CATCodeMgr* CATCodeMgr::GetInstance()
{
	return _Inst;
}


CATCodeMgr::CATCodeMgr(void)
  : m_hContainer(NULL), 
	m_hContainerWnd(NULL), 
	m_wszOptionString(NULL), 
	m_bRunClipboardThread(FALSE),
	m_hClipboardThread(NULL),
	m_hClipTextChangeEvent(NULL)
	//m_pfnLoadLibraryA(NULL),
	//m_pfnLoadLibraryW(NULL)
{
	_Inst = this;
	ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_PROC_ENTRY));
	InitializeCriticalSection(&m_csClipText);
}

CATCodeMgr::~CATCodeMgr(void)
{
	DeleteCriticalSection(&m_csClipText);
	Close();
	_Inst = NULL;
}


BOOL CATCodeMgr::Init( HWND hAralWnd, LPWSTR wszPluginOption )
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
		ZeroMemory(&m_sContainerFunc, sizeof(CONTAINER_PROC_ENTRY));
		m_sContainerFunc.procHookWin32Api		= (PROC_HookWin32Api) GetProcAddress(m_hContainer, "HookWin32Api");
		m_sContainerFunc.procUnhookWin32Api		= (PROC_UnhookWin32Api) GetProcAddress(m_hContainer, "UnhookWin32Api");
		m_sContainerFunc.procHookCodePoint		= (PROC_HookCodePoint) GetProcAddress(m_hContainer, "HookCodePoint");
		m_sContainerFunc.procUnhookCodePoint	= (PROC_UnhookCodePoint) GetProcAddress(m_hContainer, "UnhookCodePoint");
		m_sContainerFunc.procCreateTransCtx		= (PROC_CreateTransCtx) GetProcAddress(m_hContainer, "CreateTransCtx");
		m_sContainerFunc.procDeleteTransCtx		= (PROC_DeleteTransCtx) GetProcAddress(m_hContainer, "DeleteTransCtx");
		m_sContainerFunc.procTranslateUsingCtx	= (PROC_TranslateUsingCtx) GetProcAddress(m_hContainer, "TranslateUsingCtx");
		m_sContainerFunc.procIsAppLocaleLoaded	= (PROC_IsAppLocaleLoaded) GetProcAddress(m_hContainer, "IsAppLocaleLoaded");
		m_sContainerFunc.procSuspendAllThread	= (PROC_SuspendAllThread) GetProcAddress(m_hContainer, "SuspendAllThread");
		m_sContainerFunc.procResumeAllThread	= (PROC_ResumeAllThread) GetProcAddress(m_hContainer, "ResumeAllThread");
		m_sContainerFunc.procIsAllThreadSuspended = (PROC_IsAllThreadSuspended) GetProcAddress(m_hContainer, "IsAllThreadSuspended");
	}

	if( m_sContainerFunc.procHookWin32Api && m_sContainerFunc.procUnhookWin32Api
		&& m_sContainerFunc.procHookCodePoint && m_sContainerFunc.procUnhookCodePoint
		&& m_sContainerFunc.procCreateTransCtx && m_sContainerFunc.procDeleteTransCtx
		&& m_sContainerFunc.procTranslateUsingCtx && m_sContainerFunc.procIsAppLocaleLoaded
		&& m_sContainerFunc.procSuspendAllThread && m_sContainerFunc.procResumeAllThread
		&& m_sContainerFunc.procIsAllThreadSuspended)
	{
		// 모든 쓰레드 정지
		m_sContainerFunc.procSuspendAllThread();

		// LoadLibrary 함수 후킹	 
		if( m_sContainerFunc.procHookWin32Api( L"kernel32.dll", L"LoadLibraryA", NewLoadLibraryA, 1 ) )
		{
			//m_pfnLoadLibraryA = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryA");
		}

		if( m_sContainerFunc.procHookWin32Api( L"kernel32.dll", L"LoadLibraryW", NewLoadLibraryW, 1 ) )
		{
			//m_pfnLoadLibraryW = (PROC_LoadLibrary) m_sContainerFunc.pfnGetOrigDllFunction("kernel32.dll", "LoadLibraryW");
		}

		// 클립보드 관련
		m_bRunClipboardThread = TRUE;
		m_hClipTextChangeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hClipboardThread = (HANDLE)_beginthreadex(NULL, 0, ClipboardThreadFunc, NULL, 0, NULL);

		// 옵션 스트링 파싱
		m_wszOptionString = wszPluginOption;

		if(m_wszOptionString == NULL)
		{
			m_wszOptionString = new wchar_t[MAX_OPTION_LEN];
			ZeroMemory(m_wszOptionString, MAX_OPTION_LEN);
		}

#ifdef UNICODE
		CString strOptionString	= m_wszOptionString;		
#else
		CString strOptionString;
		WideCharToMultiByte(CP_ACP, 0, m_wszOptionString, -1, strOptionString.GetBufferSetLength(MAX_OPTION_LEN), MAX_OPTION_LEN, NULL, NULL);
#endif

		if( m_optionRoot.ParseChildren(strOptionString) )
		{
			bRetVal = TRUE;
		}
			
		// 모든 쓰레드 재가동
		m_sContainerFunc.procResumeAllThread();

	}

	if( bRetVal == TRUE )
	{
		// 옵션 적용
		AdjustOption(&m_optionRoot);
	}
	else
	{
		Close();
	}

	return bRetVal;
}


BOOL CATCodeMgr::Close()
{
	if(NULL==m_hContainerWnd) return FALSE;

	// 클립보드 쓰레드 정지
	m_bRunClipboardThread = FALSE;
	if(m_hClipboardThread && m_hClipTextChangeEvent)
	{
		::SetEvent(m_hClipTextChangeEvent);
		//::WaitForSingleObject(m_hClipboardThread, 3000);
		::CloseHandle(m_hClipboardThread);
		::CloseHandle(m_hClipTextChangeEvent);
	}

	// 모든 쓰레드 정지
	m_sContainerFunc.procSuspendAllThread();

	ResetOption();

	m_hClipboardThread = NULL;
	m_hClipTextChangeEvent = NULL;
	
	// 옵션 객체 초기화
	m_optionRoot.ClearChildren();
	
	// LoadLibrary 언훅
	TRACE(_T("kernel32.DLL!LoadLibraryA Unhook... \n"));
	m_sContainerFunc.procUnhookWin32Api( L"kernel32.dll", L"LoadLibraryA", NewLoadLibraryA );
	TRACE(_T("kernel32.DLL!LoadLibraryW Unhook... \n"));
	m_sContainerFunc.procUnhookWin32Api( L"kernel32.dll", L"LoadLibraryW", NewLoadLibraryW );
	
	// 기타 변수 리셋
	m_hContainerWnd = NULL;
	m_wszOptionString = NULL;

	// 모든 쓰레드 재가동
	m_sContainerFunc.procResumeAllThread();

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

	// 적용시켜보고
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
		wcscpy_s(m_wszOptionString, MAX_OPTION_LEN, strOptionString);
#else
		CString strOptionString;
		WideCharToMultiByte(CP_ACP, 0, strOptionString, -1, m_wszOptionString, MAX_OPTION_LEN, NULL, NULL);
#endif

		bRetVal = TRUE;
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 새 옵션 적용 직전에 현재 옵션들을 모두 초기화해주는 함수
//
//////////////////////////////////////////////////////////////////////////
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
	BOOL bSOW = FALSE;
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
		// SOW 옵션
		else if(strValue == _T("SOW"))
		{
			bNeedMigration = TRUE;
			bSOW = TRUE;
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
		else if (bSOW) strTransMethod = _T("SOW");
		else strTransMethod = _T("OVERWRITE");

		// 필요없는 노드 삭제
		pRootNode->DeleteChild(_T("PTRCHEAT"));
		pRootNode->DeleteChild(_T("SOW"));
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
						pTransNode->DeleteChild(_T("SOW"));
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
// 옵션을 실제 프로그램에 적용
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
		
		// HOOK 노드
		if(strValue == _T("HOOK"))
		{
			BOOL bHookRes = HookFromOptionNode(pNode);
			if(FALSE == bHookRes) m_listRetryHook.push_back(pNode);
		} // HOOK 노드 끝
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// 위의 AdjustOption 에서 HOOK 노드를 만날경우 이 함수를 호출해서 적용한다.
//
//////////////////////////////////////////////////////////////////////////
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
						if(pNode3->GetChild(_T("IGNORE")) != NULL)
						{
							pTransCmd->SetIgnoreBufLen(TRUE);
						}
					}
					else if(strTransOption == _T("SOW"))
					{
						pTransCmd->SetTransMethod(3);
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
// AT코드가 DLL 영역인 경우, 그 DLL이 언제 로드될 지 모르므로
// 이곳에서 감시하고 있다가 로드되는 순간 후킹한다.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryA(LPCSTR lpFileName)
{
	wchar_t wszTmp[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, wszTmp, MAX_PATH);
	TRACE(_T("[aral1] NewLoadLibraryA('%s') \n"), wszTmp);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst)
	{
		hModule = LoadLibraryA(lpFileName);

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
// AT코드가 DLL 영역인 경우, 그 DLL이 언제 로드될 지 모르므로
// 이곳에서 감시하고 있다가 로드되는 순간 후킹한다.
//
//////////////////////////////////////////////////////////////////////////
HMODULE __stdcall CATCodeMgr::NewLoadLibraryW(LPCWSTR lpFileName)
{
	TRACE(_T("[aral1] NewLoadLibraryW('%s') \n"), lpFileName);

	HMODULE hModule = NULL;

	if(CATCodeMgr::_Inst)
	{
		hModule = LoadLibraryW(lpFileName);

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

