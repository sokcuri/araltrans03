#pragma once

#define TRANS_BUF_SIZE 4096

typedef struct _ATLAS_TRANS_BUFFER
{
	BOOL bResult;
	char szOriginalText[TRANS_BUF_SIZE];
	char szTranslatedText[TRANS_BUF_SIZE];
} ATLAS_TRANS_BUFFER;

class CATLASMgr
{
private:

	// For communication with game process
	HANDLE	m_hMapFile;
	HANDLE	m_hTransReadyEvent;
	HANDLE	m_hRequestEvent;
	HANDLE	m_hResponseEvent;
	HANDLE	m_hTransThread;

	HWND m_hWndATLCLIP;
	HWND m_hWndToolbar;
	HWND m_hWndOrigTextbox;
	HWND m_hWndTransTextbox;

	static unsigned int __stdcall TransThreadFunc(void* pParam);
	int TranslationLoop();
	
public:
	CATLASMgr(void);
	~CATLASMgr(void);

	BOOL InitATLASMgr();
	void CloseATLASMgr();
};
