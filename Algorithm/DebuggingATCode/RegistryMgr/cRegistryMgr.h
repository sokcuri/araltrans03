// CRegistryMgr.h: interface for the CRegistryMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRegistryMgr_H__95BA5521_DC25_4615_813C_404228AA313E__INCLUDED_)
#define AFX_CRegistryMgr_H__95BA5521_DC25_4615_813C_404228AA313E__INCLUDED_

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>

class CRegistryMgr  
{
private:
	CRegistryMgr();
	virtual ~CRegistryMgr();

	static LONG		RegDeleteRecursive(HKEY hKey, LPCTSTR lpSubKey);
	
public:

	static CString	RegRead(CString rpath, CString name);
	static BOOL		RegWrite(CString rpath, CString name, CString value);

	static DWORD	RegReadDWORD(CString rpath, CString name);
	static BOOL		RegWriteDWORD(CString rpath, CString name, DWORD value);

	static BOOL		RegExist(CString rpath, CString name);
	static BOOL		RegDelete(CString rpath, CString name);

	static BOOL		RegistFileType(CString strExt, CString strAppPath, CString strIconPath);
	static BOOL		DeleteFileType(CString strExt);

};

#endif // !defined(AFX_CRegistryMgr_H__95BA5521_DC25_4615_813C_404228AA313E__INCLUDED_)
