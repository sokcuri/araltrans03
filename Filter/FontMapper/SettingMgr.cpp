#include <windows.h>
#include <tchar.h>
#include <string>
#include "FontMapper.h"
#include "SettingMgr.h"

#define MAINAPP CAralGoodHookApp::_gInst

using namespace std;

BOOL CSettingMgr::GetElemText(LPCWSTR cwszXml, LPCWSTR cwszElemName, LPWSTR wszBuf, size_t cch)
{
	BOOL bRetVal = FALSE;

	if(cwszXml && cwszElemName && wszBuf && cch > 0)
	{
		wstring strStartTag = L"<";
		strStartTag += cwszElemName;
		strStartTag += L">";
		
		wstring strEndTag = L"</";
		strEndTag += cwszElemName;
		strEndTag += L">";
		
		LPCWSTR pStartFound = wcsstr(cwszXml, strStartTag.c_str());
		LPCWSTR pEndFound = wcsstr(cwszXml, strEndTag.c_str());
		if(pStartFound && pEndFound)
		{
			size_t nCopyLen = pEndFound - (pStartFound+strStartTag.size());
			if(cch > nCopyLen)
			{
				memcpy(wszBuf, (pStartFound+strStartTag.size()), nCopyLen * sizeof(wchar_t));
				wszBuf[nCopyLen] = L'\0';
				bRetVal = TRUE;
			}
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// XmlToOption
//
//////////////////////////////////////////////////////////////////////////
BOOL CSettingMgr::XmlToOption(LPCTSTR cwszXml, FONT_MAPPER_OPTION* pOption)
{
	if(NULL == cwszXml || NULL == pOption) return FALSE;

	ZeroMemory(pOption, sizeof(FONT_MAPPER_OPTION));
	wchar_t wszTmpBuf[MAX_PATH];
	
	// ProgramCodePage
	ZeroMemory(wszTmpBuf, MAX_PATH*sizeof(wchar_t));
	if(GetElemText(cwszXml, L"ProgramCodePage", wszTmpBuf, MAX_PATH) == TRUE)
	{
		pOption->nCodePageFrom	= _wtoi(wszTmpBuf);
	}

	// UserDefinedCodePage
	ZeroMemory(wszTmpBuf, MAX_PATH*sizeof(wchar_t));
	if(GetElemText(cwszXml, L"UserDefinedCodePage", wszTmpBuf, MAX_PATH) == TRUE)
	{
		pOption->nCodePageTo	= _wtoi(wszTmpBuf);
	}

	// CharacterEncoding
	ZeroMemory(wszTmpBuf, MAX_PATH*sizeof(wchar_t));
	if(GetElemText(cwszXml, L"CharacterEncoding", wszTmpBuf, MAX_PATH) == TRUE)
	{
		if(_wcsicmp(wszTmpBuf, L"TRUE") == 0)
			pOption->bEncoding = TRUE;
		else
			pOption->bEncoding = FALSE;
	}

	// FontLoadingLevel
	ZeroMemory(wszTmpBuf, MAX_PATH*sizeof(wchar_t));
	if(GetElemText(cwszXml, L"FontLoadingLevel", wszTmpBuf, MAX_PATH) == TRUE)
	{
		pOption->nFontLoadLevel = _wtoi(wszTmpBuf);
	}

	// FontFaceName
	ZeroMemory(wszTmpBuf, MAX_PATH*sizeof(wchar_t));
	if(GetElemText(cwszXml, L"FontFaceName", wszTmpBuf, MAX_PATH) == TRUE)
	{
		wcscpy_s(pOption->wszFontFaceName, 32, wszTmpBuf);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// OptionToXml
//
//////////////////////////////////////////////////////////////////////////
BOOL CSettingMgr::OptionToXml(FONT_MAPPER_OPTION* pOption, LPWSTR wszBuf, size_t cch)
{
	if(NULL == pOption || NULL == wszBuf || 0 == cch) return FALSE;

	wsprintfW(wszBuf, L"<FontMapperSetting><ProgramCodePage>%d</ProgramCodePage><UserDefinedCodePage>%d</UserDefinedCodePage><CharacterEncoding>%s</CharacterEncoding><FontLoadingLevel>%d</FontLoadingLevel><FontFaceName>%s</FontFaceName></FontMapperSetting>", 
		pOption->nCodePageFrom,
		pOption->nCodePageTo,
		(pOption->bEncoding ? L"True" : L"False"),
		pOption->nFontLoadLevel,
		pOption->wszFontFaceName
	);

	return TRUE;
}


