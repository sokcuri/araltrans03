// Windows 헤더 파일:
#include <windows.h>
#include <tchar.h>
#include <string>
#include <map>
#include "../../Common/DefStruct.h"		// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include
#include "FontMapper.h"
#include "NewAPIs.h"
#include "CharacterMapper.h"
#include "resource.h"

#define MAX_TEXT_LENGTH 1024

extern PROC_WideCharToMultiByte g_pfnOrigWideCharToMultiByte;
extern PROC_MultiByteToWideChar g_pfnOrigMultiByteToWideChar;
extern FONT_MAPPER_OPTION g_sMainOption;

using namespace std;

wchar_t g_wszFontFace[LF_FACESIZE] = {0,};
map<long, HFONT> g_mapFonts;

int MyWideCharToMultiByte(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCWSTR lpWideCharStr, 
						  int cchWideChar, 
						  LPSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPCSTR lpDefaultChar, 
						  LPBOOL lpUsedDefaultChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigWideCharToMultiByte )
	{
		nRetVal = g_pfnOrigWideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}
	else
	{
		nRetVal = ::WideCharToMultiByte(
			CodePage, 
			dwFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpDefaultChar, 
			lpUsedDefaultChar 
			);
	}

	return nRetVal;
}

int MyMultiByteToWideChar(
						  UINT CodePage, 
						  DWORD dwFlags, 
						  LPCSTR lpMultiByteStr, 
						  int cbMultiByte, 
						  LPWSTR lpWideCharStr, 
						  int cchWideChar 
						  )
{
	int nRetVal = 0;

	if( g_pfnOrigMultiByteToWideChar )
	{
		nRetVal = g_pfnOrigMultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}
	else
	{
		nRetVal = ::MultiByteToWideChar(
			CodePage, 
			dwFlags, 
			lpMultiByteStr, 
			cbMultiByte, 
			lpWideCharStr, 
			cchWideChar 
			);
	}

	return nRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// DC에 한글 폰트를 먹이는 함수
//
//////////////////////////////////////////////////////////////////////////
HFONT CheckFont( HDC hdc )
{
	TRACE(_T("[CheckFont] Start! \n"));

	HFONT hRetVal = NULL;

	TEXTMETRIC tm;
	BOOL bRes = GetTextMetrics(hdc, &tm);

	// 폰트 다시 로드
	if( bRes )//&& tm.tmHeight != lLastFontHeight )
	{
		HFONT font = NULL;
		long lFontSize = tm.tmHeight;

		// 폰트페이스명이 바뀌었으면 맵 초기화
		if(_wcsicmp(g_sMainOption.wszFontFaceName, g_wszFontFace))
		{
			for(map<long, HFONT>::iterator iter = g_mapFonts.begin();
				iter != g_mapFonts.end();
				iter++)
			{
				font = iter->second;
				DeleteObject(font);
			}
			g_mapFonts.clear();
			wcscpy_s(g_wszFontFace, LF_FACESIZE, g_sMainOption.wszFontFaceName);
		}

		// 폰트 크기 고정인 경우
		if(g_sMainOption.nFixedFontSize)
		{
			lFontSize = g_sMainOption.nFixedFontSize;
		}

		// 이 크기에 해당하는 폰트가 없을 경우 폰트를 생성
		if( g_mapFonts.find(lFontSize) == g_mapFonts.end() )
		{
			font = CreateFont(lFontSize, 0, 0, 0, tm.tmWeight, tm.tmItalic, tm.tmUnderlined, tm.tmStruckOut,
				HANGEUL_CHARSET,	//ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,
				DEFAULT_PITCH,		// | FF_SWISS,
				g_wszFontFace);

			g_mapFonts[lFontSize] = font;

		}
		else
		{
			font = g_mapFonts[lFontSize];
		}

		hRetVal = (HFONT)SelectObject(hdc, font);

	}

	TRACE(_T("[CheckFont] End! \n"));

	return hRetVal;

}



//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineA 대체 함수
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{	
	// char 배열에 문자 넣음
	char chArray[10] = {0,};
	size_t i,j;
	j = 0;
	for(i=sizeof(/*UINT*/WCHAR); i>0; i--)
	{
		char one_ch = *( ((char*)&uChar) + i - 1 );
		if(one_ch)
		{
			chArray[j] = one_ch;
			j++;
		}
	}


	// 한글 인코딩 여부 검사		
	if(g_sMainOption.bEncoding
		&& 0x88 <= (BYTE)chArray[0] && (BYTE)chArray[0] <= 0xEE
		&& 0x00 != (BYTE)chArray[1])
	{
		chArray[2] = '\0';
		char tmpbuf[10]  = {0,};

		if( CCharacterMapper::DecodeJ2K(chArray, tmpbuf) )
		{
			chArray[0] = tmpbuf[0];
			chArray[1] = tmpbuf[1];
		}
	}


	wchar_t wchArray[10];

	UINT nCodePage = 949;
	if(0x80 < (BYTE)chArray[0] && (BYTE)chArray[0] < 0xA0) nCodePage = 932;		
	MyMultiByteToWideChar(nCodePage, 0, chArray, sizeof(UINT), wchArray, 10 );

	DWORD dwRetVal = GetGlyphOutlineW(hdc, (UINT)wchArray[0], uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	return dwRetVal;
}


//////////////////////////////////////////////////////////////////////////
// GetGlyphOutlineW 대체 함수
//////////////////////////////////////////////////////////////////////////
DWORD __stdcall NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	)
{
	HFONT hOrigFont = NULL;

	// 폰트 검사
	if(g_sMainOption.nFontLoadLevel >= 5)
	{
		hOrigFont = CheckFont(hdc);
	}

	DWORD dwRetVal = GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	// 폰트 복구
	if(hOrigFont && g_sMainOption.nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return dwRetVal;
}



//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall NewTextOutA(
									   HDC hdc,           // handle to DC
									   int nXStart,       // x-coordinate of starting position
									   int nYStart,       // y-coordinate of starting position
									   LPCSTR lpString,   // character string
									   int cbString       // number of characters
									   )
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;

	if(lpString && cbString > 0 )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(a_idx<cbString && lpString[a_idx])
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				UINT nCodePage;
				if(g_sMainOption.bEncoding && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{
					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
					nCodePage = 949;
				}
				else
				{
					nCodePage = 932;
				}

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}			
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// TextOutW 호출
		bRetVal = TextOutW(hdc, nXStart, nYStart, wchArray, w_idx);
	}
	else
	{
		// 원래함수 호출
		bRetVal = TextOutA(hdc, nXStart, nYStart, lpString, cbString);
	}


	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall NewTextOutW(
									   HDC hdc,           // handle to DC
									   int nXStart,       // x-coordinate of starting position
									   int nYStart,       // y-coordinate of starting position
									   LPCWSTR lpString,   // character string
									   int cbString       // number of characters
									   )
{
	HFONT hOrigFont = NULL;

	// 폰트 검사
	if(g_sMainOption.nFontLoadLevel >= 5)
	{
		hOrigFont = CheckFont(hdc);
	}

	BOOL bRetVal = TextOutW(hdc, nXStart, nYStart, lpString, cbString);

	// 폰트 복구
	if(hOrigFont && g_sMainOption.nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
BOOL __stdcall NewExtTextOutA(
	HDC hdc,			// handle to DC
	int nXStart,		// x-coordinate of reference point
	int nYStart,		// y-coordinate of reference point
	UINT fuOptions,		// text-output options
	CONST RECT* lprc,	// optional dimensions
	LPCSTR lpString,	// string
	UINT cbString,		// number of characters in string
	CONST INT* lpDx		// array of spacing values
	)
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;

	if(lpString && cbString > 0 )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		UINT a_idx = 0;
		UINT w_idx = 0;

		while(a_idx < cbString && lpString[a_idx])
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				UINT nCodePage;
				if(g_sMainOption.bEncoding && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{
					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
					nCodePage = 949;
				}
				else
				{
					nCodePage = 932;
				}

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// ExtTextOutW 호출
		bRetVal = ExtTextOutW(hdc, nXStart, nYStart, fuOptions, lprc, wchArray, w_idx, lpDx);
	}
	else
	{
		// 원래함수 호출
		bRetVal = ExtTextOutA(hdc, nXStart, nYStart, fuOptions, lprc, lpString, cbString, lpDx);
	}

	return bRetVal;
}



BOOL __stdcall NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
)
{

	HFONT hOrigFont = NULL;

	// 폰트 검사
	if(g_sMainOption.nFontLoadLevel >= 15)
	{
		hOrigFont = CheckFont(hdc);
	}

	BOOL bRetVal = ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);

	// 폰트 복구
	if(hOrigFont && g_sMainOption.nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// DrawTextA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall NewDrawTextA(
									   HDC hDC,          // handle to DC
									   LPCSTR lpString,  // text to draw
									   int nCount,       // text length
									   LPRECT lpRect,    // formatting dimensions
									   UINT uFormat      // text-drawing options
									   )
{
	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;

	if(lpString)	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		if(nCount == -1) nCount = strlen(lpString);

		while(a_idx<nCount && lpString[a_idx])
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				UINT nCodePage;
				if(g_sMainOption.bEncoding && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{
					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
					nCodePage = 949;
				}
				else
				{
					nCodePage = 932;
				}

				//if(0x80 < (BYTE)tmpbuf[0] && (BYTE)tmpbuf[0] < 0xA0) nCodePage = 932;		
				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// DrawTextW 호출
		bRetVal = DrawTextW(hDC, wchArray, w_idx, lpRect, uFormat);
	}
	else
	{
		// 원래함수 호출
		bRetVal = DrawTextA(hDC, lpString, nCount, lpRect, uFormat);
	}

	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
// DrawTextW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall NewDrawTextW(
	HDC hDC,          // handle to DC
	LPCWSTR lpString, // text to draw
	int nCount,       // text length
	LPRECT lpRect,    // formatting dimensions
	UINT uFormat      // text-drawing options
	)
{
	HFONT hOrigFont = NULL;

	// 폰트 검사
	if(g_sMainOption.nFontLoadLevel >= 5)
	{
		hOrigFont = CheckFont(hDC);
	}

	BOOL bRetVal = DrawTextW(hDC, lpString, nCount, lpRect, uFormat);

	// 폰트 복구
	if(hOrigFont && g_sMainOption.nFontLoadLevel < 10) SelectObject(hDC, hOrigFont);

	return bRetVal;

}

//////////////////////////////////////////////////////////////////////////
// DrawTextExA 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall NewDrawTextExA(
	HDC hDC,                     // handle to DC
	LPSTR lpString,              // text to draw
	int nCount,                 // length of text to draw
	LPRECT lpRect,                 // rectangle coordinates
	UINT uFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{

	BOOL bRetVal = FALSE;
	BOOL bDecoded = FALSE;

	if( lpString )	//&& (cbString<=2 || strlen(lpString)<=2) )
	{

		wchar_t wchArray[MAX_TEXT_LENGTH];
		int a_idx = 0;
		int w_idx = 0;

		while(a_idx<nCount && lpString[a_idx])
		{
			// 2바이트 문자면
			if(0x80 <= (BYTE)lpString[a_idx] && 0x00 != (BYTE)lpString[a_idx+1])
			{
				// char 배열에 문자 넣음
				char tmpbuf[8] = {0,};
				tmpbuf[0] = lpString[a_idx];
				tmpbuf[1] = lpString[a_idx+1];
				tmpbuf[2] = '\0';

				// 한글 인코딩 여부 검사		
				UINT nCodePage;
				if(g_sMainOption.bEncoding && 0x88 <= (BYTE)lpString[a_idx] && (BYTE)lpString[a_idx] <= 0xEE )
				{
					CCharacterMapper::DecodeJ2K(&lpString[a_idx], tmpbuf);
					nCodePage = 949;
				}
				else
				{
					nCodePage = 932;
				}

				MyMultiByteToWideChar(nCodePage, 0, tmpbuf, -1, &wchArray[w_idx], 2 );

				a_idx += 2;
			}
			// 1바이트 문자면
			else
			{
				wchArray[w_idx] = (wchar_t)lpString[a_idx];
				a_idx++;
			}

			w_idx++;

		}

		wchArray[w_idx] = L'\0';

		// DrawTextExW 호출
		bRetVal = DrawTextExW(hDC, wchArray, w_idx, lpRect, uFormat, lpDTParams);
		lpRect->right -= 10;
	}
	else
	{
		// 원래함수 호출
		bRetVal = DrawTextExA(hDC, lpString, nCount, lpRect, uFormat, lpDTParams);
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
// DrawTextExW 대체 함수
//////////////////////////////////////////////////////////////////////////
int __stdcall NewDrawTextExW(
	HDC hdc,                     // handle to DC
	LPWSTR lpchText,             // text to draw
	int cchText,                 // length of text to draw
	LPRECT lprc,                 // rectangle coordinates
	UINT dwDTFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	)
{
	HFONT hOrigFont = NULL;

	// 폰트 검사
	if(g_sMainOption.nFontLoadLevel >= 5)
	{
		hOrigFont = CheckFont(hdc);
	}

	// 원래함수 호출
	BOOL bRetVal = DrawTextExW(hdc, lpchText, cchText, lprc, dwDTFormat, lpDTParams);

	// 폰트 복구
	if(hOrigFont && g_sMainOption.nFontLoadLevel < 10) SelectObject(hdc, hOrigFont);

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall NewCreateFontA( 
	int nHeight, 
	int nWidth, 
	int nEscapement, 
	int nOrientation, 
	int fnWeight, 
	DWORD fdwItalic, 
	DWORD fdwUnderline, 
	DWORD fdwStrikeOut, 
	DWORD fdwCharSet, 
	DWORD fdwOutputPrecision, 
	DWORD fdwClipPrecision, 
	DWORD fdwQuality, 
	DWORD fdwPitchAndFamily, 
	LPSTR lpszFace )
{
	TRACE(_T("[NewCreateFontA] Start! \n"));

	HFONT hFont = NULL;

	wchar_t* lpwszFace = NULL;
	wchar_t wszFace[32] = {0,};
	if(lpszFace)
	{
		MyMultiByteToWideChar(g_sMainOption.nCodePageFrom, 0, lpszFace, 32, wszFace, 32);
		lpwszFace = wszFace;
	}

	hFont = CreateFontW(
		nHeight, 
		nWidth, 
		nEscapement, 
		nOrientation, 
		fnWeight, 
		fdwItalic, 
		fdwUnderline, 
		fdwStrikeOut, 
		fdwCharSet, 
		fdwOutputPrecision, 
		fdwClipPrecision, 
		fdwQuality, 
		fdwPitchAndFamily, 
		lpwszFace );


	TRACE(_T("[NewCreateFontA] End! (returns font handle 0x%p) \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall NewCreateFontW( 
	int nHeight, 
	int nWidth, 
	int nEscapement, 
	int nOrientation, 
	int fnWeight, 
	DWORD fdwItalic, 
	DWORD fdwUnderline, 
	DWORD fdwStrikeOut, 
	DWORD fdwCharSet, 
	DWORD fdwOutputPrecision, 
	DWORD fdwClipPrecision, 
	DWORD fdwQuality, 
	DWORD fdwPitchAndFamily, 
	LPWSTR lpwszFace )
{
	TRACE(_T("[NewCreateFontW] Start! \n"));

	HFONT hFont = NULL;

	if(g_sMainOption.nFontLoadLevel >= 15)
	{
		hFont = InnerCreateFont(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}
	else
	{
		hFont = CreateFontW(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpwszFace );
	}


	TRACE(_T("[NewCreateFontW] End! (returns font handle 0x%p) \n"), hFont);

	return hFont;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall NewCreateFontIndirectA( LOGFONTA* lplf )
{
	TRACE(_T("[NewCreateFontIndirectA] Start! \n"));

	HFONT hFont = NULL;

	LOGFONTW lfWide;
	ZeroMemory(&lfWide, sizeof(LOGFONTW));
	lfWide.lfCharSet		= lplf->lfCharSet;
	lfWide.lfClipPrecision	= lplf->lfClipPrecision;
	lfWide.lfEscapement		= lplf->lfEscapement;
	lfWide.lfHeight			= lplf->lfHeight;
	lfWide.lfItalic			= lplf->lfItalic;
	lfWide.lfOrientation	= lplf->lfOrientation;
	lfWide.lfOutPrecision	= lplf->lfOutPrecision;
	lfWide.lfPitchAndFamily = lplf->lfPitchAndFamily;
	lfWide.lfQuality		= lplf->lfQuality;
	lfWide.lfStrikeOut		= lplf->lfStrikeOut;
	lfWide.lfUnderline		= lplf->lfUnderline;
	lfWide.lfWeight			= lplf->lfWeight;
	lfWide.lfWidth			= lplf->lfWidth;
	MyMultiByteToWideChar(g_sMainOption.nCodePageFrom, 0, lplf->lfFaceName, 32, lfWide.lfFaceName, 32);

	hFont = CreateFontIndirectW(&lfWide);

	TRACE(_T("[NewCreateFontIndirectA] End! (returns font handle 0x%p) \n"), hFont);

	return hFont;

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT __stdcall NewCreateFontIndirectW( LOGFONTW* lplf )
{
	TRACE(_T("[NewCreateFontIndirectW] Start! \n"));

	HFONT hFont = NULL;

	if(g_sMainOption.nFontLoadLevel >= 15)
	{
		hFont = InnerCreateFontIndirect(lplf);
	}
	else
	{
		hFont = CreateFontIndirectW(lplf);
	}


	TRACE(_T("[NewCreateFontIndirectW] End! (returns font handle 0x%p) \n"), hFont);

	return hFont;
}


HFONT InnerCreateFont(
	int nHeight,               // height of font
	int nWidth,                // average character width
	int nEscapement,           // angle of escapement
	int nOrientation,          // base-line orientation angle
	int fnWeight,              // font weight
	DWORD fdwItalic,           // italic attribute option
	DWORD fdwUnderline,        // underline attribute option
	DWORD fdwStrikeOut,        // strikeout attribute option
	DWORD fdwCharSet,          // character set identifier
	DWORD fdwOutputPrecision,  // output precision
	DWORD fdwClipPrecision,    // clipping precision
	DWORD fdwQuality,          // output quality
	DWORD fdwPitchAndFamily,   // pitch and family
	LPWSTR lpszFace           // typeface name
	)
{
	TRACE(_T("[InnerCreateFont] Start! \n"));

	HFONT hFont = NULL;

	if(g_sMainOption.nFontLoadLevel >= 15)
	{
		DWORD fdwCharSet2 = HANGEUL_CHARSET;	//ANSI_CHARSET,
		DWORD fdwOutputPrecision2 = OUT_DEFAULT_PRECIS;
		DWORD fdwClipPrecision2 = CLIP_DEFAULT_PRECIS;
		DWORD fdwQuality2 = ANTIALIASED_QUALITY;
		DWORD fdwPitchAndFamily2 = DEFAULT_PITCH;		// | FF_SWISS,

		if(g_wszFontFace[0] == L'\0')
		{
			wcscpy_s(g_wszFontFace, LF_FACESIZE, L"Gungsuh");
		}

		//if(lpszFace) wcscpy_s(lpszFace, 32, g_wszFontFace);

		int nHeight2 = nHeight;
		int nWidth2 = nWidth;
		// 폰트 크기 고정인 경우
		if(g_sMainOption.nFixedFontSize !=0)
		{
			nHeight = g_sMainOption.nFixedFontSize;
			nWidth = 0;
		}

		hFont = CreateFontW(
			nHeight2, 
			nWidth2, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet2, 
			fdwOutputPrecision2, 
			fdwClipPrecision2, 
			fdwQuality2, 
			fdwPitchAndFamily2, 
			g_wszFontFace );

	}


	if(NULL == hFont)
	{
		hFont = CreateFontW(
			nHeight, 
			nWidth, 
			nEscapement, 
			nOrientation, 
			fnWeight, 
			fdwItalic, 
			fdwUnderline, 
			fdwStrikeOut, 
			fdwCharSet, 
			fdwOutputPrecision, 
			fdwClipPrecision, 
			fdwQuality, 
			fdwPitchAndFamily, 
			lpszFace );
	}

	TRACE(_T("[InnerCreateFont] End! (returns 0x%p) \n"), hFont);

	return hFont;	
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
HFONT InnerCreateFontIndirect( LOGFONTW* lplf )
{
	TRACE(_T("[InnerCreateFontIndirect] Start! \n"));

	HFONT hFont = NULL;

	if(lplf && g_sMainOption.nFontLoadLevel >= 15)
	{
		LOGFONTW lf2;
		memcpy(&lf2, lplf, sizeof(LOGFONTW));

		lf2.lfCharSet = HANGEUL_CHARSET;	//ANSI_CHARSET,
		lf2.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf2.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf2.lfQuality = ANTIALIASED_QUALITY;
		lf2.lfPitchAndFamily = DEFAULT_PITCH;		// | FF_SWISS,

		if(g_wszFontFace[0] == L'\0')
		{
			wcscpy_s(g_wszFontFace, LF_FACESIZE, L"Gungsuh");
		}

		wcscpy_s(lf2.lfFaceName, 32, g_wszFontFace);

		// 폰트 크기 고정인 경우
		if(g_sMainOption.nFixedFontSize !=0)
		{
			lf2.lfHeight = g_sMainOption.nFixedFontSize;
			lf2.lfHeight = 0;
		}
		
		hFont = CreateFontIndirectW(&lf2);
	}

	if(NULL == hFont)
	{
		hFont = CreateFontIndirectW(lplf);
	}

	TRACE(_T("[InnerCreateFontIndirect] End! (returns 0x%p) \n"), hFont);

	return hFont;	
}

