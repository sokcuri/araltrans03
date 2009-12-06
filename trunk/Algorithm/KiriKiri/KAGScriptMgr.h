#pragma once

//#include "unzip.h"
//#include "zip.h"
#include "ZipArchive/ZipArchive.h"

typedef int (CALLBACK * PROC_TransProgress)(void*, int, int);

class CKAGScriptMgr
{
protected:

	//BOOL m_bZipOpenedForWrite;
	PROC_TransProgress m_pfnCallback;
	void* m_pCallbackContext;
	UINT m_dwCurHash;
	BOOL m_bLogOpened;

	CString m_strGameDir;
	CZipArchive m_zip;
	CStdioFile m_fileLog;
	//CString m_strATScriptFile;

	int GetTotalLines(LPCWSTR cwszText);
	int GetHelperTextCommaIdx(LPCWSTR cwszToken);
	BOOL GetNextLine(LPWSTR& wszBuf, CString& strLine);
	BOOL GetNextToken(LPWSTR& wszLine, CString& strToken);

	BOOL SaveToZip(LPCTSTR strFileName, LPCWSTR cwszScript);
	BOOL LoadFromZip(LPCTSTR strFileName, LPWSTR wszScript);
	BOOL SaveToFile(LPCTSTR strFileName, LPCWSTR cwszScript);
	BOOL LoadFromFile(LPCTSTR strFileName, LPWSTR wszScript);

	virtual BOOL TranslateUsingTranslator(LPCWSTR cwszSrc, LPWSTR wszTar);
	BOOL TranslateUnicodeText(LPCWSTR cwszSrc, LPWSTR wszTar);
	CString TranslateOnlyTextToken( LPCWSTR cwszLine );
	void WriteLog( LPCTSTR cszLine );

public:
	BOOL m_bCacheToZIP;
	BOOL m_bCacheToFile;
	BOOL m_bCacheSrc;

	CKAGScriptMgr(void);
	~CKAGScriptMgr(void);

	BOOL Init();
	void Close();

	void SetProgressCallback(void* pContext, PROC_TransProgress pfnCallback);
	BOOL TranslateScript(LPCWSTR cwszSrc, LPWSTR wszTar);
	UINT GetCurrentScriptHash();
	BOOL ClearCache();

};

class CKAGScriptMgr2 : public CKAGScriptMgr
{
public:
	enum TRANS_MODE { NOTHING, NORMAL, SILVERHAWK };

protected:
	TRANS_MODE m_nMode;

public:
	CKAGScriptMgr2() : m_nMode(NOTHING) {}
	~CKAGScriptMgr2() {}

	void SetTranslateMode(const TRANS_MODE nMode) { m_nMode = nMode; }
	const TRANS_MODE GetTranslateMode () { return m_nMode; }

	virtual BOOL TranslateUsingTranslator(LPCWSTR cwszSrc, LPWSTR wszTar);

protected:
	BOOL TranslateSilverHawkScript(LPCWSTR cwszSrc, LPWSTR wszTar);

};