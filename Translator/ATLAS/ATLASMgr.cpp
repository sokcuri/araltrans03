#pragma warning(disable:4996)

#include "stdafx.h"
#include "ATLASMgr.h"
#include <process.h>

CATLASMgr::CATLASMgr(void)
	: m_hMapFile(NULL)
	, m_hTransReadyEvent(NULL)
	, m_hRequestEvent(NULL)
	, m_hResponseEvent(NULL)
	, m_hTransThread(NULL)
	
	, m_hWndATLCLIP(NULL)
	, m_hWndToolbar(NULL)
	, m_hWndOrigTextbox(NULL)
	, m_hWndTransTextbox(NULL)
{
}

CATLASMgr::~CATLASMgr(void)
{
}

BOOL CATLASMgr::InitATLASMgr()
{
	BOOL bRetVal = FALSE;
	
	try
	{
		//////////////////////////////////////////////////////////////////////////
		// Get control handles
		//////////////////////////////////////////////////////////////////////////
		// ATLCLIP Main Window
		TCHAR szClassName[MAX_PATH] = {0,};

		m_hWndATLCLIP = FindWindow(NULL, _T("ATLAS Clipboard Translation"));
		if(::IsWindow(m_hWndATLCLIP) == FALSE) throw FALSE;
		/*
		CWnd* pMainWnd = ::AfxGetMainWnd();
		if(NULL == pMainWnd || ::IsWindow(pMainWnd->m_hWnd) == FALSE) throw FALSE;
		m_hWndATLCLIP = pMainWnd->m_hWnd;
		*/
		//::ShowWindow(m_hWndATLCLIP, SW_HIDE);

		// 1st Static Window (client area)
		HWND hStaticWnd = NULL;
		for(HWND hTmpWnd = ::GetWindow(m_hWndATLCLIP, GW_CHILD);
			NULL != hTmpWnd;
			hTmpWnd = ::GetWindow(hTmpWnd, GW_HWNDNEXT))
		{
			::GetClassName(hTmpWnd, szClassName, MAX_PATH);
			if(_tcscmp(szClassName, _T("Static")) == 0)
			{
				hStaticWnd = hTmpWnd;
			}
			else if(_tcscmp(szClassName, _T("ToolbarWindow32")) == 0)
			{
				m_hWndToolbar = hTmpWnd;
			}
		}
		if(NULL == hStaticWnd || ::IsWindow(hStaticWnd) == FALSE) throw FALSE;						
		if(NULL == m_hWndToolbar || ::IsWindow(m_hWndToolbar) == FALSE) throw FALSE;						

		// 2st Static Window (seperator panel)
		for(HWND hTmpWnd = ::GetWindow(hStaticWnd, GW_CHILD);
			NULL != hTmpWnd;
			hTmpWnd = ::GetWindow(hTmpWnd, GW_HWNDNEXT))
		{
			::GetClassName(hTmpWnd, szClassName, MAX_PATH);
			if(_tcscmp(szClassName, _T("Static")) == 0)
			{
				hStaticWnd = hTmpWnd;
			}
		}
		if(NULL == hStaticWnd || ::IsWindow(hStaticWnd) == FALSE) throw FALSE;						
		
		// RichEdit20A
		HWND hEditWnd = ::GetWindow(hStaticWnd, GW_CHILD);
		if(NULL == hEditWnd || ::IsWindow(hEditWnd) == FALSE) throw FALSE;
		m_hWndTransTextbox = hEditWnd;
		hEditWnd = ::GetWindow(hEditWnd, GW_HWNDNEXT);
		if(NULL == hEditWnd || ::IsWindow(hEditWnd) == FALSE) throw FALSE;
		m_hWndOrigTextbox = hEditWnd;


		// 동기화 객체 생성
		m_hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,		// use paging file
			NULL,						// default security 
			PAGE_READWRITE,				// read/write access
			0,							// max. object size 
			sizeof(ATLAS_TRANS_BUFFER),	// buffer size  
			_T("ATBufferATLCLIP"));		// name of mapping object
		m_hTransReadyEvent = ::CreateEvent(NULL, FALSE, FALSE, _T("ATReadyATLCLIP"));
		m_hRequestEvent = ::CreateEvent(NULL, FALSE, FALSE, _T("ATRequestATLCLIP"));
		m_hResponseEvent = ::CreateEvent(NULL, FALSE, FALSE, _T("ATResponseTLCLIP"));
		if (m_hMapFile == NULL) throw FALSE;

		CString strMsg;
		strMsg.Format(_T("ATLAS hWnd : 0x%p \r\nOriginal Text hWnd : 0x%p \r\nTranslated Text hWnd : 0x%p \r\nRequest Event Handle : 0x%p \r\nResponse Event Handle : 0x%p"), 
			m_hWndATLCLIP, m_hWndOrigTextbox, m_hWndTransTextbox, m_hRequestEvent, m_hResponseEvent);

		// Start translation thread
		m_hTransThread = (HANDLE)_beginthreadex(NULL, 0, TransThreadFunc, this, 0, NULL);
		if(NULL == m_hTransThread) throw FALSE;
		//if(::WaitForSingleObject(m_hTransReadyEvent, 3000) != WAIT_OBJECT_0) throw FALSE;
		
		// Send WM_COMMAND message
		::SetWindowTextA(m_hWndOrigTextbox, "");
		::SetWindowTextA(m_hWndTransTextbox, "");
		::PostMessage(m_hWndATLCLIP, WM_COMMAND, (WPARAM)32772, (LPARAM)m_hWndToolbar);
		::PostMessage(m_hWndATLCLIP, WM_COMMAND, (WPARAM)32772, (LPARAM)m_hWndToolbar);
		::PostMessage(m_hWndATLCLIP, WM_COMMAND, (WPARAM)32773, (LPARAM)m_hWndToolbar);

		//MessageBox(NULL, strMsg, _T("Plugin Debugging Message"), MB_OK | MB_TOPMOST);

		bRetVal = TRUE;
	}
	catch (BOOL bRes)
	{
		CloseATLASMgr();
		bRetVal = bRes;
	}	

	return bRetVal;
}

void CATLASMgr::CloseATLASMgr()
{
	// 번역 쓰레드 정지
	if(m_hTransThread)
	{
		/*
		HANDLE hTmp = m_hTransThread;
		m_hTransThread = NULL;
		::SetEvent(m_hRequestEvent);
		if(::WaitForSingleObject(hTmp, 3000) == WAIT_TIMEOUT)
		{
			::TerminateThread(hTmp, 0);
		}
		*/
		::TerminateThread(m_hTransThread, 0);
		m_hTransThread = NULL;
	}
	
	// 동기화 객체 말기화
	if(m_hMapFile)
	{
		::CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}	
	if(m_hTransReadyEvent)
	{
		::CloseHandle(m_hTransReadyEvent);
		m_hTransReadyEvent = NULL;
	}	
	if(m_hRequestEvent)
	{
		::CloseHandle(m_hRequestEvent);
		m_hRequestEvent = NULL;
	}	
	if(m_hResponseEvent)
	{
		::CloseHandle(m_hResponseEvent);
		m_hResponseEvent = NULL;
	}

	// Initialize control handles
	m_hWndATLCLIP = NULL;
	m_hWndToolbar = NULL;
	m_hWndOrigTextbox = NULL;
	m_hWndTransTextbox = NULL;
}

int CATLASMgr::TranslationLoop()
{
	typedef int (* PROC_BufferTranslate2)(LPCSTR, LPCSTR, LPSTR*, int, int, int, int);
	typedef int (* PROC_ATLCRT_Free)(void*);

	HMODULE hAtleCont = GetModuleHandle(_T("AtleCont.dll"));

	PROC_BufferTranslate2 BufferTranslate2 = 
		(PROC_BufferTranslate2)GetProcAddress(hAtleCont, "BufferTranslate2");
	PROC_ATLCRT_Free ATLCRT_Free = 
		(PROC_ATLCRT_Free)GetProcAddress(hAtleCont, "ATLCRT_Free");

	while(m_hTransThread)
	{
		// Set status Ready
		::SetEvent(m_hTransReadyEvent);
		
		// Wating translation request...
		DWORD dwTransWait = WaitForSingleObject(m_hRequestEvent, INFINITE);
		if(m_hTransThread == NULL)
		{
			TRACE(_T("[aral1] Exit (pThis->m_hTransThread == NULL)"));
			break;
		}

		TRACE(_T("[aral1] Received Request of Trans Event (dwTransWait : %d)"), dwTransWait);

		// Open shared memory
		ATLAS_TRANS_BUFFER* pTransBuf = 
			(ATLAS_TRANS_BUFFER*) MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ATLAS_TRANS_BUFFER));

		if (pTransBuf)
		{
			pTransBuf->bResult = FALSE;

			// Translate
			if(strlen(pTransBuf->szOriginalText) < TRANS_BUF_SIZE)
			{
				pTransBuf->szTranslatedText[0] = '\0';

				if('\0' != pTransBuf->szOriginalText[0])
				{
					/*
					// Set OrigTextbox
					::SetWindowTextA(m_hWndOrigTextbox, pTransBuf->szOriginalText);
					//wchar_t tmpBuf[TRANS_BUF_SIZE];
					//::MultiByteToWideChar(932, 0, pTransBuf->szOriginalText, -1, tmpBuf, TRANS_BUF_SIZE);
					//::SetWindowTextW(m_hWndOrigTextbox, tmpBuf);

					
					// Send WM_COMMAND message
					::SendMessage(m_hWndATLCLIP, WM_COMMAND, (WPARAM)32773, (LPARAM)m_hWndToolbar);

					// Get Translated text from TransTextbox
					::GetWindowTextA(m_hWndTransTextbox, pTransBuf->szTranslatedText, TRANS_BUF_SIZE);
					*/
					
					LPSTR szTranslatedText = NULL;
					int rr = BufferTranslate2("JE", pTransBuf->szOriginalText, &szTranslatedText, 1, 1, 3, 0);
					if(szTranslatedText)
					{
						size_t nTranslatedLen = strlen(szTranslatedText);
						
						/*
						// Remove CR, LF
						if((nTranslatedLen > 0) && ('\n' == szTranslatedText[nTranslatedLen-1]))
							szTranslatedText[--nTranslatedLen] = '\0';
						if((nTranslatedLen > 0) && ('\r' == szTranslatedText[nTranslatedLen-1]))
							szTranslatedText[--nTranslatedLen] = '\0';
						if((nTranslatedLen > 0) && (' ' == szTranslatedText[nTranslatedLen-1]))
							szTranslatedText[--nTranslatedLen] = '\0';
						*/

						// Check boundary
						nTranslatedLen = min(nTranslatedLen, TRANS_BUF_SIZE-1);
						
						memcpy(pTransBuf->szTranslatedText, szTranslatedText, nTranslatedLen);
						pTransBuf->szTranslatedText[nTranslatedLen] = '\0';
						ATLCRT_Free(szTranslatedText);
					}


				}

				pTransBuf->bResult = TRUE;
			}

			// Close shared memory
			UnmapViewOfFile(pTransBuf);
		}

		// Set the response event
		::SetEvent(m_hResponseEvent);

	}

	return 0;
}
unsigned int __stdcall CATLASMgr::TransThreadFunc(void* pParam)
{
	if(NULL==pParam) return 0;
	CATLASMgr* pThis = (CATLASMgr*)pParam;

	return pThis->TranslationLoop();
}
	
