
#pragma warning(disable:4312)
#pragma warning(disable:4313)
#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATCodeMgr.h"
#include "HookPoint.h"
//#include "CharacterMapper.h"

//////////////////////////////////////////////////////////////////////////
//
// SOW 방식의 메모리를 백업하고 번역문을 덮어씌웁니다
//
//////////////////////////////////////////////////////////////////////////
void CTransCommand3::DoBackupAndOverwrite(const PBYTE pBackupPoint, UINT nBackupSize)
{
	// 백업 복구가 되어있지 않으면 복구
	if (m_pBackupPoint)
		RestoreBackup();

	m_pBackupBuffer = new BYTE [nBackupSize];
	m_nBackupSize = nBackupSize;
	m_pBackupPoint = pBackupPoint;

	// 백업
	CopyMemory(m_pBackupBuffer, m_pBackupPoint, m_nBackupSize);

	// 덮어쓰기
	CopyMemory(m_pBackupPoint, m_pTransTextBuf, m_nBackupSize);

}

//////////////////////////////////////////////////////////////////////////
//
// SOW 방식의 백업 데이타를 복구합니다
//
//////////////////////////////////////////////////////////////////////////
void CTransCommand3::RestoreBackup()
{

	if (m_pBackupPoint)
	{
		// 원 메모리가 쓰기가능할 때만 백업
		if (!IsBadWritePtr(m_pBackupPoint, m_nBackupSize))
		{
			bool bIsMismatch = false;
			UINT i;

			// 원 메모리가 백업 이후 바뀌었는지 체크
			for(i=0; i<m_nBackupSize; i++)
			{
				if (*(m_pBackupPoint+i) != *(m_pTransTextBuf+i))
				{
					bIsMismatch=true;
					break;
				}
			}

			if (!bIsMismatch)
				CopyMemory(m_pBackupPoint, m_pBackupBuffer, m_nBackupSize);
		}
		delete[] m_pBackupBuffer;
		m_pBackupPoint=NULL;
	}

}


CHookPoint::CHookPoint(void)
: m_hModule(NULL), m_pCodePoint(0)
{
}

CHookPoint::~CHookPoint(void)
{
	// 번역 명령들 모두 삭제
	DeleteAllTransCmd();
	
	// Unhook 함수 준비
	PROC_UnhookCodePoint UnhookCodePoint = CATCodeMgr::GetInstance()->m_sContainerFunc.procUnhookCodePoint;
	if(UnhookCodePoint)
	{
		// Unhook
		UnhookCodePoint( (LPVOID)GetHookAddress(), PointCallback );
	}
}

CHookPoint* CHookPoint::CreateInstance(CString strAddr)
{
	CHookPoint* pInst = new CHookPoint();

	try
	{
		int nIdx = strAddr.ReverseFind(_T('!'));
		
		if(nIdx>=0)
		{
			pInst->m_strModuleName = strAddr.Left(nIdx);

			pInst->m_hModule = GetModuleHandle(pInst->m_strModuleName);
			if(NULL == pInst->m_hModule) throw -4;
			
			_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pInst->m_pCodePoint);
		}
		else
		{
			pInst->m_hModule = NULL;
			_stscanf(strAddr.Mid(nIdx+1), _T("%x"), &pInst->m_pCodePoint);
		}

		if(NULL == pInst->m_pCodePoint) throw -1;

		// 후킹 함수 준비
		PROC_HookCodePoint HookCodePoint = CATCodeMgr::GetInstance()->m_sContainerFunc.procHookCodePoint;
		if(HookCodePoint == NULL) throw -2;

		// 후킹 (DebuggingATCode 플러그인을 고려해 우선순위를 뒤로 미룬다)
		BOOL bHookRes = HookCodePoint((LPVOID)pInst->GetHookAddress(), PointCallback, 10);
		if(bHookRes == FALSE) throw -3;
		
		TRACE("[aral1] ★ %p Hooked \n", pInst->GetHookAddress());
	}
	catch(int nErrCode)
	{
		nErrCode = nErrCode;
		delete pInst;
		pInst = NULL;
	}

	return pInst;
}

//////////////////////////////////////////////////////////////////////////
//
// 이 인스턴스의 후킹 주소를 반환합니다.
//
//////////////////////////////////////////////////////////////////////////
UINT_PTR CHookPoint::GetHookAddress()
{
	return ( (UINT_PTR)m_hModule + m_pCodePoint );
}

//////////////////////////////////////////////////////////////////////////
//
// 이 인스턴스의 후킹 주소를 형식화된 문자열 형태로 반환합니다.
//
//////////////////////////////////////////////////////////////////////////
CString CHookPoint::GetHookAddressString()
{
	CString strRetVal;

	if(NULL == m_hModule)
	{
		strRetVal.Format(_T("0x%p"), m_pCodePoint);
	}
	else
	{
		strRetVal.Format(_T("%s!0x%p"), (LPCTSTR)m_strModuleName, m_pCodePoint);
	}
	
	return strRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 새로운 번역 명령을 생성합니다.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::AddTransCmd(CString strArgScript)
{
	strArgScript.Remove(_T(' '));
	CTransCommand* pCmd = FindTransCmd(strArgScript);
	
	if(pCmd == NULL)
	{
		CString strContextName = GetHookAddressString() + _T("(") + strArgScript + _T(")");
		if( CATCodeMgr::GetInstance()->m_sContainerFunc.procCreateTransCtx(strContextName) == TRUE )
		{
			//pCmd = new CTransCommand();
			pCmd = new CTransCommand3();
			pCmd->SetArgScript(strArgScript);
			pCmd->SetContextName(strContextName);
			m_vectorTransCmd.push_back(pCmd);
		}
	}

	return pCmd;
}

//////////////////////////////////////////////////////////////////////////
//
// 번역 명령을 찾습니다.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::FindTransCmd(CString strArgScript)
{
	CTransCommand* pCmd = NULL;

	strArgScript.Remove(_T(' '));
	for(vector<CTransCommand*>::iterator iter = m_vectorTransCmd.begin();
		iter != m_vectorTransCmd.end();
		iter++)
	{
		if((*iter)->GetArgScript().CompareNoCase(strArgScript) == 0)
		{
			pCmd = (*iter);
			break;
		}
	}

	return pCmd;
}


//////////////////////////////////////////////////////////////////////////
//
// 번역 명령의 총 개수를 리턴합니다.
//
//////////////////////////////////////////////////////////////////////////
int CHookPoint::GetTransCmdCount()
{
	return (int)m_vectorTransCmd.size();
}


//////////////////////////////////////////////////////////////////////////
//
// 특정번째의 번역 명령을 리턴합니다.
//
//////////////////////////////////////////////////////////////////////////
CTransCommand* CHookPoint::GetTransCmd(int nIdx)
{
	CTransCommand* pCmd = NULL;
	
	if((size_t)nIdx < m_vectorTransCmd.size())
	{
		pCmd = m_vectorTransCmd[nIdx];
	}

	return pCmd;
}


//////////////////////////////////////////////////////////////////////////
//
// 번역 명령을 삭제합니다.
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::DeleteTransCmd(int nIdx)
{
	if((size_t)nIdx < m_vectorTransCmd.size())
	{
		CTransCommand* pCmd = m_vectorTransCmd[nIdx];
		CATCodeMgr::GetInstance()->m_sContainerFunc.procDeleteTransCtx(pCmd->GetContextName());
		delete pCmd;
		m_vectorTransCmd.erase(m_vectorTransCmd.begin()+nIdx);
	}	
}

//////////////////////////////////////////////////////////////////////////
//
// 모든 번역 명령을 삭제합니다.
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::DeleteAllTransCmd()
{
	for(vector<CTransCommand*>::iterator iter = m_vectorTransCmd.begin();
		iter != m_vectorTransCmd.end();
		iter++)
	{
		CTransCommand* pCmd = (*iter);
		CATCodeMgr::GetInstance()->m_sContainerFunc.procDeleteTransCtx(pCmd->GetContextName());
		delete pCmd;
	}

	m_vectorTransCmd.clear();
}



//////////////////////////////////////////////////////////////////////////
//
// 특정지점 후킹의 콜백 함수 (static)
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::PointCallback(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters)
{
	
	CATCodeMgr* pATCodeMgr = CATCodeMgr::GetInstance();
	if(NULL == pATCodeMgr) return;

	bool bRestore=false;
	list<CHookPoint*>::iterator iter;
	CHookPoint* pPoint;

	// 훅맵에서 후킹정보 구하기
	for(iter = pATCodeMgr->m_listHookPoint.begin(); iter != pATCodeMgr->m_listHookPoint.end(); iter++)
	{
		pPoint = (*iter);

		if(pPoint->GetHookAddress() == (UINT_PTR)pHookedPoint)
		{
			if (pPoint->GetTransCmdCount() == 0)
			{
				bRestore=true;
				break;
			}

			// 번역 명령 수행
			pPoint->ExecuteTransCmds(pRegisters);
			break;
		}
	}

	if (bRestore) // 번역 명령이 없는 후킹코드
	{
		// 모든 SOW 백업 메모리 강제복구
		for(iter = pATCodeMgr->m_listHookPoint.begin(); iter != pATCodeMgr->m_listHookPoint.end(); iter++)
		{
			pPoint = (*iter);

			int cnt = pPoint->GetTransCmdCount();
			for(int i=0; i<cnt; i++)
			{
				CTransCommand *pCmd=pPoint->GetTransCmd(i);
				if (pCmd->GetTransMethod() == 3)
					((CTransCommand3 *) pCmd)->RestoreBackup();
			}

		}
	}


}

//////////////////////////////////////////////////////////////////////////
//
// 번역 작업 수행 (멀티바이트 / 유니코드 공용)
//
//////////////////////////////////////////////////////////////////////////
void CHookPoint::ExecuteTransCmds(REGISTER_ENTRY* pRegisters)
{
	int cnt = GetTransCmdCount();

	m_parser.SetRegisterValues(pRegisters);

	for(int i=0; i<cnt; i++)
	{
		CTransCommand* pCmd = GetTransCmd(i);		
		LPVOID pArgText = NULL;

		int nType;
		int* pRetVal = (int*)m_parser.GetValue(pCmd->GetArgScript(), &nType);
		if(pRetVal && 1 == nType)
		{
			pArgText = *(LPVOID*)pRetVal;
			delete pRetVal;
		}

		if(pArgText == NULL || IsBadReadPtr(pArgText, sizeof(LPVOID)) || (*(BYTE*)pArgText) == 0) continue;

		try
		{
			BOOL bTrans = FALSE;
			CString strClipboardText = _T("");

			// Script OverWrite 모드인 경우 이전 백업 데이타 복구
			if (pCmd->GetTransMethod() == 3)
				((CTransCommand3 *) pCmd)->RestoreBackup();

			// 유니코드 번역이라면
			if(pCmd->GetUnicode())
			{
				LPWSTR wszText = (LPWSTR)pArgText;
				if( IsBadStringPtrW(wszText, 1024*1024*1024) ) throw -3; // _T("잘못된 문자열 포인터입니다.");

				// 최근 번역된 한국어인가
				wchar_t wszPureText[MAX_TEXT_LENGTH];
				wcscpy(wszPureText, wszText);
				size_t wlen = wcslen(wszPureText);
				while(wlen>0 && wszPureText[wlen-1] == L' ')
				{
					wszPureText[wlen-1] = L'\0';
					wlen--;
				}
				if( wcslen((LPCWSTR)pCmd->m_pTransTextBuf) > 0 && wcsstr((LPCWSTR)pCmd->m_pTransTextBuf, wszPureText) )
				{
					throw -1; //_T("같은 위치를 중복 번역하려 합니다.");
				}

				// 최근 처리된 일본어가 아닌 새로운 문장이라면
				if( wcscmp(wszText, (LPCWSTR)pCmd->m_pOrigTextBuf) )
				{
					
					// 최근 일본어로 설정
					ZeroMemory(pCmd->m_pOrigTextBuf, sizeof(pCmd->m_pOrigTextBuf));
					wcscpy((LPWSTR)pCmd->m_pOrigTextBuf, wszText);

					// 멀티바이트로 변환
					char szAnsiJpn[MAX_TEXT_LENGTH*2];
					char szAnsiKor[MAX_TEXT_LENGTH*2];

					WideCharToMultiByte(932, 0, wszText, -1, szAnsiJpn, MAX_TEXT_LENGTH*2, NULL, NULL);

					// 일본어 대사 클립보드 여부 검사
					if( pCmd->GetClipJpn() )
					{
#ifdef UNICODE
						strClipboardText += wszText;
#else
						strClipboardText += szAnsiJpn;
#endif
					}

					// 번역기에 집어넣어라
					PROC_TranslateUsingCtx procTrans = CATCodeMgr::GetInstance()->m_sContainerFunc.procTranslateUsingCtx;
					if( procTrans == NULL ) throw -2; //_T("AT 컨테이너의 번역함수를 찾을 수 없습니다.");

					bTrans = procTrans(pCmd->GetContextName(), szAnsiJpn, (int)strlen(szAnsiJpn)+1, szAnsiKor, MAX_TEXT_LENGTH*2);			
					if( bTrans )
					{
#ifdef UNICODE
						wchar_t wszTmpKor[MAX_TEXT_LENGTH];
						MultiByteToWideChar(949, 0, szAnsiKor, -1, wszTmpKor, MAX_TEXT_LENGTH);
						CString strKorean = wszTmpKor;
#else
						CString strKorean = szAnsiKor;
#endif

						// 한국어 대사 클립보드 여부 검사
						if( pCmd->GetClipKor() )
						{
							strClipboardText += strKorean;
						}

						// 최근 한국어로 설정
						ZeroMemory(pCmd->m_pTransTextBuf, sizeof(pCmd->m_pTransTextBuf));
#ifdef UNICODE
						wcscpy((LPWSTR)pCmd->m_pTransTextBuf, (LPCWSTR)strKorean);
#else
						MyMultiByteToWideChar(949, 0, (LPCSTR)strKorean, -1, (LPWSTR)pCmd->m_pTransTextBuf, MAX_TEXT_LENGTH);
#endif
					}
					else // bTrans == FALSE
						continue;

				}			

			}
			// 멀티바이트 번역이라면
			else
			{

				LPSTR szText = (LPSTR)pArgText;
				if( IsBadStringPtrA(szText, 1024*1024*1024) )
				{
					throw -3; //_T("잘못된 문자열 포인터입니다.");
				}

				// 최근 번역된 한국어인가
				size_t nCmpCnt = strlen(szText);
				while(nCmpCnt > 0 && szText[nCmpCnt-1] == ' ') nCmpCnt--;
				if( strlen((LPCSTR)pCmd->m_pTransTextBuf) > 0 && strncmp(szText, (LPCSTR)pCmd->m_pTransTextBuf, nCmpCnt) == 0 )
				{
					throw -1; //_T("같은 위치를 중복 번역하려 합니다.");
				}
				
				// 최근 처리된 일본어가 아닌 새로운 문장이라면
				if( strcmp(szText, (LPCSTR)pCmd->m_pOrigTextBuf) )
				{
					// 최근 일본어로 설정
					ZeroMemory(pCmd->m_pOrigTextBuf, sizeof(pCmd->m_pOrigTextBuf));
					strcpy((LPSTR)pCmd->m_pOrigTextBuf, szText);

					// 일본어 대사 클립보드 여부 검사
					if( pCmd->GetClipJpn() )
					{
#ifdef UNICODE
						wchar_t wszTmpJpn[MAX_TEXT_LENGTH];
						MultiByteToWideChar(932, 0, szText, -1, wszTmpJpn, MAX_TEXT_LENGTH);
						strClipboardText += wszTmpJpn;
#else
						strClipboardText += szText;
#endif
					}

					// 번역기에 집어넣어라
					PROC_TranslateUsingCtx procTrans = CATCodeMgr::GetInstance()->m_sContainerFunc.procTranslateUsingCtx;
					if( procTrans == NULL )
					{
						throw -2; //"AT 컨테이너의 번역함수를 찾을 수 없습니다.";
					}

					bTrans = procTrans(pCmd->GetContextName(), szText, (int)strlen(szText)+1, (char*)pCmd->m_pTransTextBuf, MAX_TEXT_LENGTH*2);			
					if( bTrans )
					{
#ifdef UNICODE
						wchar_t wszTmpKor[MAX_TEXT_LENGTH];
						MultiByteToWideChar(949, 0, (LPCSTR)pCmd->m_pTransTextBuf, -1, wszTmpKor, MAX_TEXT_LENGTH);
						CString strKorean = wszTmpKor;
#else
						CString strKorean = (LPCSTR)pCmd->m_pTransTextBuf;
#endif

						// 한국어 대사 클립보드 여부 검사
						if( pCmd->GetClipKor() )
						{
							strClipboardText += strKorean;
						}

					}
					else // bTrans == FALSE
						continue;

				}
			}

			// 포인터 바꿔치기 방식
			if( pCmd->GetTransMethod() == 1 )
			{
				CString strScript = pCmd->GetArgScript();
				int nLen = strScript.GetLength();
				
				if(strScript.CompareNoCase(_T("EAX")) == 0) pRegisters->_EAX = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("EBX")) == 0) pRegisters->_EBX = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("ECX")) == 0) pRegisters->_ECX = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("EDX")) == 0) pRegisters->_EDX = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("ESI")) == 0) pRegisters->_ESI = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("EDI")) == 0) pRegisters->_EDI = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("EBP")) == 0) pRegisters->_EBP = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(strScript.CompareNoCase(_T("ESP")) == 0) pRegisters->_ESP = (DWORD)(UINT_PTR)pCmd->m_pTransTextBuf;
				else if(nLen >= 5 && _T('[') == strScript[0] && _T(']') == strScript[nLen-1])
				{
					int nType;
					int* pRetVal = (int*)m_parser.GetValue(strScript.Mid(1, nLen-2), &nType);
					if(pRetVal && 1 == nType)
					{
						LPBYTE* ppArgText = *(LPBYTE**)pRetVal;
						delete pRetVal;

						if(::IsBadWritePtr(ppArgText, sizeof(LPBYTE)) == FALSE)
						{
							*ppArgText = pCmd->m_pTransTextBuf;
						}

					}
				}
				
			}
			// 메모리 Overwrite 방식 (Default)
			else if( pCmd->GetTransMethod() == 2 )
			{
				list<LPVOID> listTexts;
				listTexts.push_back(pArgText);

				// 번역해야할 위치들 저장
				if(pCmd->GetAllSameText())
				{
					if(pCmd->GetUnicode())
					{
						SearchTextW((UINT_PTR)pRegisters->_EBP, (LPCWSTR)pArgText, &listTexts);
					}
					else
					{
						SearchTextA((UINT_PTR)pRegisters->_EBP, (LPCSTR)pArgText, &listTexts);
					}
				}


				// 위치들에 번역된 텍스트 Overwrite
				for(list<LPVOID>::iterator iter = listTexts.begin();
					iter != listTexts.end();
					iter++)
				{
					LPVOID pDest = (*iter);
					OverwriteTextBytes(pDest, pCmd->m_pTransTextBuf, pCmd->GetUnicode(), pCmd->GetIgnoreBufLen() );
				}
			}
			// Script OverWrite 방식
			else if( pCmd->GetTransMethod() == 3 )
			{
				CTransCommand3 *pCmd3 = (CTransCommand3 *)pCmd;
				int nOrigSize=0, nTransSize=0, nDelta=0;

				CString strScript = pCmd->GetArgScript();
				int nLen = strScript.GetLength();

				LPBYTE *ppArgText=NULL;

				// 길이를 구하고
				if (pCmd3->GetUnicode())
				{
					nOrigSize=wcslen((LPWSTR) pCmd3->m_pOrigTextBuf) *2;
					nTransSize=wcslen((LPWSTR) pCmd3->m_pTransTextBuf) *2;
				}
				else
				{
					nOrigSize=strlen((LPSTR) pCmd3->m_pOrigTextBuf);
					nTransSize=strlen((LPSTR) pCmd3->m_pTransTextBuf);
				}

				// 백업 및 덮어쓰기
				nDelta = nOrigSize - nTransSize;
				pCmd3->DoBackupAndOverwrite((LPBYTE)pArgText + nDelta, nTransSize);

				// 포인터 위치 보정
				if(strScript.Left(3).CompareNoCase(_T("EAX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EAX);
				else if(strScript.Left(3).CompareNoCase(_T("EBX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EBX);
				else if(strScript.Left(3).CompareNoCase(_T("ECX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ECX);
				else if(strScript.Left(3).CompareNoCase(_T("EDX")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EDX);
				else if(strScript.Left(3).CompareNoCase(_T("ESI")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ESI);
				else if(strScript.Left(3).CompareNoCase(_T("EDI")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EDI);
				else if(strScript.Left(3).CompareNoCase(_T("EBP")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_EBP);
				else if(strScript.Left(3).CompareNoCase(_T("ESP")) == 0) ppArgText=(LPBYTE *)&(pRegisters->_ESP);
				else if(nLen >= 5 && _T('[') == strScript[0])
				{
					int nType;
					int nBracketCount=0;
					int i;
					int *pRetVal;

					for(i=0; i < nLen; i++)
					{
						if (strScript[i] == _T('['))
							nBracketCount++;
						else if (strScript[i] == _T(']'))
							nBracketCount--;

						if (nBracketCount == 0)
						{
							i++;
							break;
						}
					}

					pRetVal = (int*)m_parser.GetValue(strScript.Mid(1, i-2), &nType);
					if(pRetVal && 1 == nType)
					{
						ppArgText = *(LPBYTE**)pRetVal;
						delete pRetVal;
					}
				}

				if (ppArgText && !IsBadWritePtr(ppArgText, sizeof(LPBYTE)))
					*ppArgText += nDelta;
					
			}


			// 클립보드로 복사
			if(!strClipboardText.IsEmpty())
			{
				CATCodeMgr::GetInstance()->SetClipboardText(strClipboardText);
			}

		}
		catch (int nErrCode)
		{
			nErrCode = nErrCode;
			TRACE(_T("ExecuteTransCmds exception code : %d \n"), nErrCode);
		}

	} // end of for
}


BOOL CHookPoint::OverwriteTextBytes( LPVOID pDest, LPVOID pSrc, BOOL bWideChar, BOOL bIgnoreBufLen )
{
	if(bWideChar)
	{
		LPWSTR wszSrc = (LPWSTR)pSrc;
		UINT_PTR nSrcLen =  wcslen(wszSrc);
		LPWSTR wszDest = (LPWSTR)pDest;
		UINT_PTR nDestLen = ( bIgnoreBufLen ? nSrcLen : wcslen(wszDest) );


		// 문자열 포인터가 잘못되었다면 리턴
		if( IsBadWritePtr(wszDest, nDestLen) || IsBadStringPtrW(wszDest, 1024*1024*1024) ) return FALSE;

		// 텍스트 복사
		size_t len = min(nDestLen, nSrcLen);
		memcpy(wszDest, wszSrc, (len+1)*sizeof(wchar_t));
		while(len<nDestLen)
		{
			wszDest[len] = L' ';
			len++;
		}

	}
	else
	{

		LPSTR szSrc = (LPSTR)pSrc;
		UINT_PTR nSrcLen =  strlen(szSrc);
		LPSTR szDest = (LPSTR)pDest;
		UINT_PTR nDestLen = ( bIgnoreBufLen ? nSrcLen : strlen(szDest) );

		// 문자열 포인터가 잘못되었다면 리턴
		if( IsBadWritePtr(szDest, nDestLen) || IsBadStringPtrA(szDest, 1024*1024*1024) ) return FALSE;

		// 텍스트 복사
		size_t len = 0;
		while(len<nDestLen && len<nSrcLen)
		{
			size_t addval = 1;
			if( (BYTE)0x80 <= (BYTE)szSrc[len] ) addval = 2;

			if( len + addval > nDestLen ) break;

			len += addval;
		}

		memcpy(szDest, szSrc, (len+1));
		while(len<nDestLen)
		{
			szDest[len] = ' ';
			len++;
		}

	}

	return TRUE;
}

BOOL CHookPoint::SearchTextA(UINT_PTR ptrBegin, LPCSTR cszText, list<LPVOID>* pTextList)
{
	BOOL bRetVal = FALSE;

	size_t dist = 0;
	size_t nOrigLen = strlen(cszText);

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPSTR* ppText = (LPSTR*)(ptrBegin+dist);

		// 일치한다면
		if( IsBadStringPtrA(*ppText, 1024*1024*1024)==FALSE 
			&& strlen(*ppText) == nOrigLen
			&& strcmp(*ppText, cszText) == 0 )
		{
			// 목록에 추가
			pTextList->push_back(*ppText);
			bRetVal = TRUE;
		}

		dist += sizeof(void*);
	}

	TRACE(_T(" [ aral1 ] SearchTextA 찾은거리:0x%p~0x%p (%d bytes) \n"), ptrBegin, ptrBegin+dist, dist);

	return bRetVal;
}

BOOL CHookPoint::SearchTextW(UINT_PTR ptrBegin, LPCWSTR cwszText, list<LPVOID>* pTextList)
{
	BOOL bRetVal = FALSE;

	size_t dist = 0;
	size_t nOrigLen = wcslen(cwszText);

	while( IsBadReadPtr((void*)(ptrBegin+dist), sizeof(void*)) == FALSE )
	{
		LPWSTR* ppText = (LPWSTR*)(ptrBegin+dist);

		// 일치한다면
		if( IsBadStringPtrW(*ppText, 1024*1024)==FALSE 
			&& wcslen(*ppText) == nOrigLen
			&& wcscmp(*ppText, cwszText) == 0 )
		{
			// 목록에 추가
			pTextList->push_back(*ppText);
			bRetVal = TRUE;
		}

		dist += sizeof(void*);

	}	

	TRACE(_T(" [ aral1 ] SearchTextW 찾은거리:0x%p~0x%p (%d bytes) \n"), ptrBegin, ptrBegin+dist, dist);

	return bRetVal;
}

