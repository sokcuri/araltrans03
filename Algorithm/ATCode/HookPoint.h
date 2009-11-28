#pragma once

#include <vector>
#include <list>
#include "TransScriptParser.h"

using namespace std;

#define MAX_TEXT_LENGTH 1024


//////////////////////////////////////////////////////////////////////////
class CTransCommand
{
	friend class CHookPoint;

protected:

	int		m_nTransMethod;
	BOOL	m_bUnicode;
	BOOL	m_bAllSameText;
	BOOL	m_bClipKor;
	BOOL	m_bClipJpn;
	BOOL	m_bIgnoreBufLen;

	CString m_strArgScript;
	CString m_strContextName;

	CTransCommand() : 
		m_nTransMethod(0), m_bUnicode(FALSE), m_bAllSameText(FALSE), m_bClipKor(FALSE), 
		m_bClipJpn(FALSE), m_bIgnoreBufLen(FALSE)
	{
		m_strArgScript.Empty();
		m_strContextName.Empty();
		ZeroMemory(m_pOrigTextBuf, MAX_TEXT_LENGTH*2);
		ZeroMemory(m_pTransTextBuf, MAX_TEXT_LENGTH*2);
	};
	~CTransCommand() {};
public:
	BYTE	m_pOrigTextBuf[MAX_TEXT_LENGTH*2];
	BYTE	m_pTransTextBuf[MAX_TEXT_LENGTH*2];

	void SetArgScript(LPCTSTR strScript){ m_strArgScript = strScript; };
	void SetContextName(LPCTSTR strContextName){ m_strContextName = strContextName; };
	void SetTransMethod(int nMethod){ m_nTransMethod = nMethod; };
	void SetUnicode(BOOL bValue){ m_bUnicode = bValue; };
	void SetAllSameText(BOOL bAllSameText){ m_bAllSameText = bAllSameText; };
	void SetClipKor(BOOL bClipKor){ m_bClipKor = bClipKor; };
	void SetClipJpn(BOOL bClipJpn){ m_bClipJpn = bClipJpn; };
	void SetIgnoreBufLen(BOOL bEnable){ m_bIgnoreBufLen = bEnable; };

	CString	GetArgScript(){ return m_strArgScript; };
	CString	GetContextName(){ return m_strContextName; };
	int  GetTransMethod(){ return m_nTransMethod; };
	BOOL GetUnicode(){ return m_bUnicode; };
	BOOL GetAllSameText(){ return m_bAllSameText; };
	BOOL GetClipKor(){ return m_bClipKor; };
	BOOL GetClipJpn(){ return m_bClipJpn; };
	BOOL GetIgnoreBufLen(){ return m_bIgnoreBufLen; };
};

// CTransCommand3 : SOW 방식을 위한 CTransCommand 확장 클래스
class CTransCommand3 : public CTransCommand {
protected:
	BYTE *m_pBackupPoint;
	BYTE *m_pBackupBuffer;
	UINT m_nBackupSize;

protected:
	CTransCommand3() : CTransCommand(), m_pBackupPoint(NULL), m_pBackupBuffer(NULL), m_nBackupSize(0) {}
	~CTransCommand3() { RestoreBackup(); }

public:
	void DoBackupAndOverwrite(const PBYTE pBackupPoint, UINT nBackupSize);
	void RestoreBackup();

	friend class CHookPoint;
};


//////////////////////////////////////////////////////////////////////////
class CHookPoint
{
private:
	HMODULE		m_hModule;
	UINT_PTR	m_pCodePoint;
	CString		m_strModuleName;
	CTransScriptParser m_parser;
	vector<CTransCommand*> m_vectorTransCmd;

	CHookPoint();
	void ExecuteTransCmds(REGISTER_ENTRY* pRegisters);
	BOOL OverwriteTextBytes( LPVOID pDest, LPVOID pSrc, BOOL bWideChar, BOOL bIgnoreBufLen );
	BOOL SearchTextA(UINT_PTR ptrBegin, LPCSTR cszText, list<LPVOID>* pTextList);
	BOOL SearchTextW(UINT_PTR ptrBegin, LPCWSTR cwszText, list<LPVOID>* pTextList);
	static void PointCallback(LPVOID pHookedPoint, REGISTER_ENTRY* pRegisters);

public:
	static CHookPoint* CreateInstance(CString strAddr);
	~CHookPoint();

	UINT_PTR		GetHookAddress();
	CString			GetHookAddressString();
	
	CTransCommand*	AddTransCmd(CString strArgScript, CString strContextName);
	CTransCommand*	FindTransCmd(CString strArgScript);

	int				GetTransCmdCount();	
	CTransCommand*	GetTransCmd(int nIdx);
	void			DeleteTransCmd(int nIdx);
	void			DeleteAllTransCmd();
};
