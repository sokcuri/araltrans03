#pragma once

#include "../../Common/DefStruct.h"		// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include

struct FONT_MAPPER_OPTION
{
	int		nCodePageFrom;
	int		nCodePageTo;
	int		nFontLoadLevel;
	int		nFixedFontSize;
	BOOL	bEncoding;
	wchar_t wszFontFaceName[32];
};

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  );

extern "C" __declspec(dllexport) BOOL __stdcall GetPluginInfo(PLUGIN_INFO* pPluginInfo);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginInit(HWND hAralWnd, LPWSTR wszPluginOption);
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginClose();
extern "C" __declspec(dllexport) BOOL __stdcall OnPluginOption();
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectInit(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectClose(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectMove(TRANSLATION_OBJECT* pTransObj);
extern "C" __declspec(dllexport) BOOL __stdcall OnObjectOption(TRANSLATION_OBJECT* pTransObj);

BOOL __stdcall Encode(TRANSLATION_OBJECT* pTransObject);

BOOL EncodeMultiBytes( LPSTR szNewText );
BOOL ApplyOption(FONT_MAPPER_OPTION* pOption);
BOOL XmlToOption(LPCTSTR cwszXml, FONT_MAPPER_OPTION* pOption);
BOOL OptionToXml(FONT_MAPPER_OPTION* pOption, LPTSTR wszXml, int cch);

#ifdef _DEBUG
	void TmpTrace(LPCTSTR format, ...);
	#define TRACE TmpTrace
#else
	#define TRACE __noop
#endif


