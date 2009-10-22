#pragma once
#include <windows.h>

class CCharacterMapper
{
	static BYTE _SHIFT_JIS_MAP[0x10000];
	static BOOL bMapInitialized;
	
	CCharacterMapper(void);
	~CCharacterMapper(void);
	static void InitializeShiftJISMap();
public:

	static BOOL EncodeK2J(LPCSTR cszKorCode, LPSTR szJpnCode);
	static BOOL DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode);
	static WORD Ascii2ShiftJIS(char chAscii);
	static WORD Ascii2KS5601(char chAscii);
	static BOOL IsShiftJISText(LPCSTR cszJpnText);
};
