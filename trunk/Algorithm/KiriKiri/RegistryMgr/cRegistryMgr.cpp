// CRegistryMgr.cpp: implementation of the CRegistryMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "CRegistryMgr.h"
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistryMgr::CRegistryMgr()
{

}

CRegistryMgr::~CRegistryMgr()
{

}



//---------------------------------------------------------------------------
//	◎함수명 : RegRead
//	◎함수설명 : 레지스트리에서 데이터를 읽어온다
//	◎인자 : CString 레지스트리경로, CString 이름
//	◎반환값 : 위의 경로와 이름에 해당하는 레지스트리 데이터(CString 형식)
//---------------------------------------------------------------------------
CString CRegistryMgr::RegRead(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	CString retString = _T("");

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retString;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retString;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	
	// 키를 여는데 성공했다면
	if(lRet == ERROR_SUCCESS)
	{
		LPCTSTR pName = NULL;
		if(!name.IsEmpty()) pName = (LPCTSTR)name;

		DWORD size = 0;
		RegQueryValueEx(hKey, name, 0, &type, NULL, &size);

		BYTE* dir = new BYTE[size+2];

		lRet = RegQueryValueEx(hKey, name, 0, &type, dir, &size);
		if(lRet == ERROR_SUCCESS) retString = (LPCTSTR)dir;

		delete dir;

		RegCloseKey(hKey);
	}

	return retString;
}





//---------------------------------------------------------------------------
//	◎함수명 : RegWrite
//	◎함수설명 : 레지스트리에 데이터를 쓴다
//	◎인자 : CString 레지스트리경로, CString 이름, CString 값
//	◎반환값 : 성공시 TRUE, 실패시 FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegWrite(CString rpath, CString name, CString value)
{
	int strindex, count;
	CString temp = _T("");
	HKEY hCategoryKey;

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return FALSE;
	
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	rpath = rpath.Mid(strindex+1);
	count = rpath.GetLength();
	if(count>0){				// 레지스트리 디렉토리 생성
		TCHAR ch;
		temp = _T("");
		for(strindex=0; strindex<count; strindex++)
		{
			ch = rpath.GetAt(strindex);
			
			if(ch==_T('\\') || ch==_T('/'))
			{
				lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
				if(lRet == ERROR_SUCCESS) RegCloseKey(hKey);
			}

			temp += ch;
		}
	}
	else return FALSE;

	lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if(lRet != ERROR_SUCCESS) return FALSE;
	if(name.IsEmpty() || name.Compare(_T("0"))==0)
		RegSetValueEx( hKey, NULL, 0, REG_SZ, (LPBYTE)(LPCTSTR)value, (value.GetLength()+1)*sizeof(TCHAR) );
	else
		RegSetValueEx( hKey, name, 0, REG_SZ, (LPBYTE)(LPCTSTR)value, (value.GetLength()+1)*sizeof(TCHAR) );
	
	RegCloseKey(hKey);
	return TRUE;

}







BOOL CRegistryMgr::RegExist(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	BOOL retValue = FALSE;
	
	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retValue;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retValue;
	
	HKEY hKey;
	LONG lRet;
	DWORD type = REG_DWORD;
	DWORD size = MAX_PATH*2;
	//BYTE dir[MAX_PATH*2];
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS){		// 키를 여는데 성공했다면
		if(name.IsEmpty() || name.Compare(_T("0"))==0)
			lRet = RegQueryValueEx(hKey, NULL, 0, NULL, NULL, NULL);
		else
			lRet = RegQueryValueEx(hKey, name, 0, NULL, NULL, NULL);
		
		if(lRet == ERROR_SUCCESS)
		{
			retValue = TRUE;
		}
		RegCloseKey(hKey);
	}
	
	return retValue;
}





//---------------------------------------------------------------------------
//	◎함수명 : RegReadDWORD
//	◎함수설명 : 레지스트리에서 DWORD 데이터를 읽어온다
//	◎인자 : CString 레지스트리경로, CString 이름
//	◎반환값 : 위의 경로와 이름에 해당하는 레지스트리 데이터(DWORD 형식)
//---------------------------------------------------------------------------
DWORD CRegistryMgr::RegReadDWORD(CString rpath, CString name)
{
	int strindex;
	HKEY hCategoryKey;
	CString temp;
	DWORD retValue = 0;

	strindex = rpath.Find(_T('\\'));
	if(strindex<0) return retValue;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return retValue;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_DWORD;
	DWORD size = MAX_PATH*2;
	BYTE dir[MAX_PATH*2];
	rpath = rpath.Mid(strindex+1);
	lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS){		// 키를 여는데 성공했다면
		if(name.IsEmpty() || name.Compare(_T("0"))==0)
			lRet = RegQueryValueEx(hKey, NULL, 0, &type, (LPBYTE)&dir, &size);
		else
			lRet = RegQueryValueEx(hKey, name, 0, &type, (LPBYTE)&dir, &size);
		
		if(lRet == ERROR_SUCCESS)
		{
			memcpy( &retValue, &dir, sizeof(DWORD) );
		}
		RegCloseKey(hKey);
	}

	return retValue;
}





//---------------------------------------------------------------------------
//	◎함수명 : RegWriteDWORD
//	◎함수설명 : 레지스트리에 DWORD 데이터를 쓴다
//	◎인자 : CString 레지스트리경로, CString 이름, DWORD 값
//	◎반환값 : 성공시 TRUE, 실패시 FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegWriteDWORD(CString rpath, CString name, DWORD value)
{
	int strindex, count;
	CString temp = "";
	HKEY hCategoryKey;

	strindex = rpath.Find('\\');
	if(strindex<0) return FALSE;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	HKEY hKey;
	LONG lRet;
	DWORD type = REG_SZ;
	DWORD size = MAX_PATH;
	rpath = rpath.Mid(strindex+1);
	count = rpath.GetLength();
	if(count>0){				// 레지스트리 디렉토리 생성
		TCHAR ch;
		temp = "";
		for(strindex=0; strindex<count; strindex++){
			ch = rpath.GetAt(strindex);
			if(ch=='\\' || ch=='/'){
				lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
				if(lRet == ERROR_SUCCESS) RegCloseKey(hKey);
			}
			temp += ch;
		}
	}
	else return FALSE;

	lRet = RegCreateKeyEx(hCategoryKey, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if(lRet != ERROR_SUCCESS) return FALSE;
	if(name.IsEmpty() || name.Compare(_T("0"))==0)
		RegSetValueEx(hKey, NULL, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
	else
		RegSetValueEx(hKey, name, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
	
	RegCloseKey(hKey);
	return TRUE;

}


















//---------------------------------------------------------------------------
//	◎함수명 : RegDelete
//	◎함수설명 : 레지스트리의 특정 데이터를 삭제한다
//	◎인자 : CString 레지스트리경로, CString 삭제할이름
//	삭제할이름이 빈문자열이거나 "0"일 경우 해당 경로를 포함한 하위
//	서브키들까지 모조리 삭제한다.
//	◎반환값 : 성공시 TRUE, 실패시 FALSE
//---------------------------------------------------------------------------
BOOL CRegistryMgr::RegDelete(CString rpath, CString name)
{
	int strindex;
	CString temp = "";
	HKEY hCategoryKey;

	strindex = rpath.Find('\\');
	if(strindex<0) return FALSE;
	temp = rpath.Left(strindex);
	if(temp.CompareNoCase(_T("HKEY_CLASSES_ROOT"))==0) hCategoryKey = HKEY_CLASSES_ROOT;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_CONFIG"))==0) hCategoryKey = HKEY_CURRENT_CONFIG;
	else if(temp.CompareNoCase(_T("HKEY_CURRENT_USER"))==0) hCategoryKey = HKEY_CURRENT_USER;
	else if(temp.CompareNoCase(_T("HKEY_LOCAL_MACHINE"))==0) hCategoryKey = HKEY_LOCAL_MACHINE;
	else if(temp.CompareNoCase(_T("HKEY_USERS"))==0) hCategoryKey = HKEY_USERS;
	else return FALSE;

	
	rpath = rpath.Mid(strindex+1);
	long lRet;
	if(name.IsEmpty() || name.Compare(_T("0"))==0){		// 하위 경로까지 몽땅 지우기
		lRet = RegDeleteRecursive(hCategoryKey, rpath);
		if(lRet != ERROR_SUCCESS) return FALSE;
	}
	else{											// 특정키만 지우기
		HKEY hKey;
		lRet = RegOpenKeyEx(hCategoryKey, rpath, 0, KEY_ALL_ACCESS, &hKey);
		if(lRet == ERROR_SUCCESS) // 오픈 성공하면...
		{
			lRet = RegDeleteValue(hKey, name);
			if(lRet != ERROR_SUCCESS) return FALSE;
		}
		else return FALSE;
	}

	return TRUE;
}



// RegDeleteKey() 함수는 NT이후 서브키가 있으면 삭제가 안된다.
// 그래서 RegDelete를 보조해주는 함수를 만들었다. 직접 사용되진 않는다.
LONG CRegistryMgr::RegDeleteRecursive(HKEY hKey, LPCTSTR lpSubKey)
{
    HKEY newKey;
    TCHAR newSubKey[MAX_PATH];
    LONG Result;
    DWORD Size;
    FILETIME FileTime;

    RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &newKey);
    Result = ERROR_SUCCESS;
    while(TRUE) {
        Size = MAX_PATH;
        // 계속 키가 삭제 되므로 dwIndex는 항상 0을 넣어주어야 한다.
        // 만약 for문으로 i를 증가시며 사용하면 하나지우고 하나 뛰어넘어 반이 남는다.
        Result = RegEnumKeyEx(newKey, 0, newSubKey, &Size, NULL, NULL, NULL, &FileTime);
        if (Result != ERROR_SUCCESS) break;
        Result = RegDeleteRecursive(newKey, newSubKey);
        if (Result  != ERROR_SUCCESS) break;
    }
    RegCloseKey(newKey);
 
    return RegDeleteKey(hKey, lpSubKey);
}



//////////////////////////////////////////////////////////////////////////
//
// 레지스트리에 확장자를 등록하고 쉘에 통보
//
BOOL CRegistryMgr::RegistFileType(CString strExt, CString strAppPath, CString strIconPath)
{
	if(strExt.IsEmpty() || strAppPath.IsEmpty()) return FALSE;
	if(strIconPath.IsEmpty()) strIconPath = strAppPath;
	
	CString strKey = strExt + "ClassApp";
	
	// strExt에 '.' 붙임
	strExt = "." + strExt;

	// 확장자 기록
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strExt, 
		"", 
		strKey
	);

 	// 실행커맨드 기록
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strKey + "\\Shell\\Open\\Command", 
		"", 
		strAppPath + " \"%1\""
	);

 	// 확장자에 대한 아이콘 기록
	CRegistryMgr::RegWrite(
		"HKEY_CLASSES_ROOT\\" + strKey + "\\DefaultIcon", 
		"", 
		strIconPath
	);

 	// 쉘에 통보
 	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

	return TRUE;

}



//////////////////////////////////////////////////////////////////////////
//
// 레지스트리에 확장자를 등록 해지하고 쉘에 통보
//
BOOL CRegistryMgr::DeleteFileType(CString strExt)
{
	if(strExt.IsEmpty()) return FALSE;
	

	CString strKey = strExt + "ClassApp";
	
	// 연결 프로그램 삭제
	CRegistryMgr::RegDelete("HKEY_CLASSES_ROOT\\" + strKey, "");

	// strExt에 '.' 붙임
	strExt = "." + strExt;

	// 확장자 삭제
	CRegistryMgr::RegDelete("HKEY_CLASSES_ROOT\\" + strExt, "");

 	// 쉘에 통보
 	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);

	return TRUE;
}

