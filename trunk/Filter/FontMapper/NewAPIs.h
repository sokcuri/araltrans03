#pragma once

typedef int (__stdcall * PROC_WideCharToMultiByte)(UINT,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,LPBOOL);
typedef int (__stdcall * PROC_MultiByteToWideChar)(UINT,DWORD,LPCSTR,int,LPWSTR,int);

int MyWideCharToMultiByte(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCWSTR lpWideCharStr, 
	int cchWideChar, 
	LPSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPCSTR lpDefaultChar, 
	LPBOOL lpUsedDefaultChar 
	);

int MyMultiByteToWideChar(
	UINT CodePage, 
	DWORD dwFlags, 
	LPCSTR lpMultiByteStr, 
	int cbMultiByte, 
	LPWSTR lpWideCharStr, 
	int cchWideChar 
	);

DWORD __stdcall NewGetGlyphOutlineA(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	);

DWORD __stdcall NewGetGlyphOutlineW(
	HDC hdc,             // handle to device context
	UINT uChar,          // character to query
	UINT uFormat,        // format of data to return
	LPGLYPHMETRICS lpgm, // pointer to structure for metrics
	DWORD cbBuffer,      // size of buffer for data
	LPVOID lpvBuffer,    // pointer to buffer for data
	CONST MAT2 *lpmat2   // pointer to transformation matrix structure
	);

BOOL __stdcall NewTextOutA(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCSTR lpString,   // character string
	int cbString       // number of characters
	);

BOOL __stdcall NewTextOutW(
	HDC hdc,           // handle to DC
	int nXStart,       // x-coordinate of starting position
	int nYStart,       // y-coordinate of starting position
	LPCWSTR lpString,   // character string
	int cbString       // number of characters
	);

BOOL __stdcall NewExtTextOutA(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	);

BOOL __stdcall NewExtTextOutW(
	HDC hdc,          // handle to DC
	int X,            // x-coordinate of reference point
	int Y,            // y-coordinate of reference point
	UINT fuOptions,   // text-output options
	CONST RECT* lprc, // optional dimensions
	LPCWSTR lpString, // string
	UINT cbCount,     // number of characters in string
	CONST INT* lpDx   // array of spacing values
	);

int __stdcall NewDrawTextA(
	HDC hDC,          // handle to DC
	LPCSTR lpString,  // text to draw
	int nCount,       // text length
	LPRECT lpRect,    // formatting dimensions
	UINT uFormat      // text-drawing options
	);

int __stdcall NewDrawTextW(
	HDC hDC,          // handle to DC
	LPCWSTR lpString, // text to draw
	int nCount,       // text length
	LPRECT lpRect,    // formatting dimensions
	UINT uFormat      // text-drawing options
	);

int __stdcall NewDrawTextExA(
	HDC hdc,                     // handle to DC
	LPSTR lpchText,              // text to draw
	int cchText,                 // length of text to draw
	LPRECT lprc,                 // rectangle coordinates
	UINT dwDTFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	);

int __stdcall NewDrawTextExW(
	HDC hdc,                     // handle to DC
	LPWSTR lpchText,              // text to draw
	int cchText,                 // length of text to draw
	LPRECT lprc,                 // rectangle coordinates
	UINT dwDTFormat,             // formatting options
	LPDRAWTEXTPARAMS lpDTParams  // more formatting options
	);

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
	LPSTR lpszFace           
	);

HFONT __stdcall NewCreateFontW(
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
	);

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
	);

HFONT __stdcall NewCreateFontIndirectA(LOGFONTA* lplf);

HFONT __stdcall NewCreateFontIndirectW(LOGFONTW* lplf);

HFONT InnerCreateFontIndirect(LOGFONTW* lplf);
