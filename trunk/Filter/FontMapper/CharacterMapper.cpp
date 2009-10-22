
#include <windows.h>
#include "CharacterMapper.h"


BYTE CCharacterMapper::_SHIFT_JIS_MAP[0x10000];
BOOL CCharacterMapper::bMapInitialized = FALSE;


CCharacterMapper::CCharacterMapper(void)
{
}

CCharacterMapper::~CCharacterMapper(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// ★ 한글코드를 SHIFTJIS 중 사용 빈도가 낮은 한자코드영역에 매핑
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::EncodeK2J(LPCSTR cszKorCode, LPSTR szJpnCode)
{
	BOOL bRetVal = FALSE;

	if(cszKorCode && cszKorCode && 0xA0 <= (BYTE)cszKorCode[0] && 0xA0 <= (BYTE)cszKorCode[1])
	{
		// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
		// 95~3074, 3119~4535, 16632~18622, 19089~19450
		BYTE hiKorByte = (BYTE)cszKorCode[0];
		BYTE loKorByte = (BYTE)cszKorCode[1];

		// 예외 ('아')
		if(hiKorByte==0xBE && loKorByte==0xC6){ hiKorByte = 0xA8; loKorByte = 0xA1; }
		// 예외 ('렇')
		if(hiKorByte==0xB7 && loKorByte==0xB8){ hiKorByte = 0xA8; loKorByte = 0xA2; }
		// 예외 ('먼')
		if(hiKorByte==0xB8 && loKorByte==0xD5){ hiKorByte = 0xA8; loKorByte = 0xA3; }

		
		WORD dwKorTmp = ( hiKorByte - 0xA0) * 0xA0;	// 96*96
		dwKorTmp += (loKorByte - 0xA0);		

		dwKorTmp += 95;
		if(dwKorTmp > 3074) dwKorTmp += 44;
		if(dwKorTmp > 4535) dwKorTmp += 12096;
		if(dwKorTmp > 18622) dwKorTmp += 466;
		if(dwKorTmp > 19450) return FALSE;
		
		BYTE hiByte = (dwKorTmp / 189) + 0x88;
		BYTE loByte = (dwKorTmp % 189) + 0x40;
		dwKorTmp = MAKEWORD( loByte, hiByte );

		//TRACE("[ aral1 ] EncodeK2J : '%s'(%p) -> (%p) \n", cszKorCode, MAKEWORD(cszKorCode[1], cszKorCode[0]), dwKorTmp );

		if( 0x889F <= dwKorTmp && dwKorTmp <= 0xEEEC )
		{
			szJpnCode[0] = HIBYTE(dwKorTmp);
			szJpnCode[1] = LOBYTE(dwKorTmp);

			bRetVal = TRUE;
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// ★ 매핑되었던 한글코드를 복구
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::DecodeJ2K(LPCSTR cszJpnCode, LPSTR szKorCode)
{
	BOOL bRetVal = FALSE;

	// 889F~9872, 989F~9FFC, E040~EAA4, ED40~EEEC
	// 95~3074, 3119~4535, 16632~18622, 19089~19450
	if(cszJpnCode && szKorCode && 0x88 <= (BYTE)cszJpnCode[0] && (BYTE)cszJpnCode[0] <= 0xEE)
	{
		WORD dwKorTmp = (WORD)( (BYTE)cszJpnCode[0] - 0x88 ) * 189;
		dwKorTmp += (WORD)( (BYTE)cszJpnCode[1] - 0x40 );

		if(dwKorTmp > 19450) return FALSE;
		if(dwKorTmp > 18622) dwKorTmp -= 466;
		if(dwKorTmp > 4535) dwKorTmp -= 12096;
		if(dwKorTmp > 3074) dwKorTmp -= 44;
		dwKorTmp -= 95;

		szKorCode[0] = (char)( ((UINT_PTR)dwKorTmp/(UINT_PTR)0xA0) + (UINT_PTR)0xA0);
		szKorCode[1] = (char)( ((UINT_PTR)dwKorTmp%(UINT_PTR)0xA0) + (UINT_PTR)0xA0);

		//TRACE("[ aral1 ] DecodeJ2K : (%p) -> (%p)'%s' \n", (UINT_PTR)(MAKEWORD(cszJpnCode[1], cszJpnCode[0])), (UINT_PTR)(MAKEWORD(szKorCode[1], szKorCode[0])), szKorCode);

		if( 0xA0 <= (BYTE)szKorCode[0] && 0xA0 <= (BYTE)szKorCode[1] )
		{
			// 예외 ('아')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA1 )
			{
				*((BYTE*)szKorCode) = 0xBE;
				*((BYTE*)szKorCode+1) = 0xC6;
			}

			// 예외 ('렇')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA2 )
			{
				*((BYTE*)szKorCode) = 0xB7;
				*((BYTE*)szKorCode+1) = 0xB8;
			}

			// 예외 ('먼')
			if( (BYTE)szKorCode[0]==0xA8 && (BYTE)szKorCode[1]==0xA3 )
			{
				*((BYTE*)szKorCode) = 0xB8;
				*((BYTE*)szKorCode+1) = 0xD5;
			}

			bRetVal = TRUE;
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////
//
// 이 함수는 머였더라...? ㅡ,.ㅡ;;
//
//////////////////////////////////////////////////////////////////////////
WORD CCharacterMapper::Ascii2KS5601( char chAscii )
{
	WORD wChar = 0;

	if( 0x20 <= (BYTE)chAscii && (BYTE)chAscii <= 0x7E )
	{	
		if(' ' == chAscii) wChar = 0xA1A1;
		else wChar = MAKEWORD( 0x80 + (BYTE)chAscii, 0xA3 );
	}

	return wChar;	
}


//////////////////////////////////////////////////////////////////////////
//
// _SHIFT_JIS_MAP 배열을 사용하여 주어진 문자열의 문자들이
// 모두 유효한 일본어인지 검사
// TRUE:유효, FALSE:비정상 문자 포함
//
//////////////////////////////////////////////////////////////////////////
BOOL CCharacterMapper::IsShiftJISText( LPCSTR cszJpnText )
{
	BOOL bRetVal = TRUE;

	if(FALSE == bMapInitialized)
	{
		InitializeShiftJISMap();
		bMapInitialized = TRUE;
	}

	int i=0;
	while(cszJpnText[i])
	{
		WORD chCode = 0;

		// 문자 코드 만들기
		if((BYTE)cszJpnText[i] >= 0x80 && cszJpnText[i+1] != '\0')
		{
			chCode = MAKEWORD((BYTE)cszJpnText[i+1], (BYTE)cszJpnText[i]);
			i++;
		}
		else
		{
			chCode = MAKEWORD((BYTE)cszJpnText[i], 0);
		}

		// 코드 맵에서 검사
		if(_SHIFT_JIS_MAP[chCode] == FALSE)
		{
			bRetVal = FALSE;
			break;
		}

		i++;
	}

	return bRetVal;
}



//////////////////////////////////////////////////////////////////////////
//
// SAFE 옵션을 위한 SHIFTJIS 코드 유효 테이블
// 1: 유효 문자, 0: 비정상 문자
//
//////////////////////////////////////////////////////////////////////////
void CCharacterMapper::InitializeShiftJISMap()
{
	ZeroMemory(_SHIFT_JIS_MAP, 0x10000);
	
	_SHIFT_JIS_MAP[0x0A] = 1;
	_SHIFT_JIS_MAP[0x0D] = 1;
	memset(&_SHIFT_JIS_MAP[0x20], 1, 0x7E-0x20+1);

	memset(&_SHIFT_JIS_MAP[0x8140], 1, 0x81AC-0x8140+1);
	memset(&_SHIFT_JIS_MAP[0x81B8], 1, 0x81BF-0x81B8+1);
	memset(&_SHIFT_JIS_MAP[0x81C8], 1, 0x81CE-0x81C8+1);
	memset(&_SHIFT_JIS_MAP[0x81DA], 1, 0x81E8-0x81DA+1);
	memset(&_SHIFT_JIS_MAP[0x81F0], 1, 0x81F7-0x81F0+1);
	_SHIFT_JIS_MAP[0x91FC] = 1;
	
	memset(&_SHIFT_JIS_MAP[0x824F], 1, 0x8258-0x824F+1);
	memset(&_SHIFT_JIS_MAP[0x8260], 1, 0x8279-0x8260+1);
	memset(&_SHIFT_JIS_MAP[0x8281], 1, 0x829A-0x8281+1);
	memset(&_SHIFT_JIS_MAP[0x829F], 1, 0x82F1-0x829F+1);

	memset(&_SHIFT_JIS_MAP[0x8340], 1, 0x8396-0x8340+1);
	memset(&_SHIFT_JIS_MAP[0x839F], 1, 0x83B6-0x839F+1);
	memset(&_SHIFT_JIS_MAP[0x83BF], 1, 0x83D6-0x83BF+1);

	memset(&_SHIFT_JIS_MAP[0x8440], 1, 0x8460-0x8440+1);
	memset(&_SHIFT_JIS_MAP[0x8470], 1, 0x8491-0x8470+1);
	memset(&_SHIFT_JIS_MAP[0x849F], 1, 0x84BE-0x849F+1);

	memset(&_SHIFT_JIS_MAP[0x8740], 1, 0x875D-0x8740+1);
	memset(&_SHIFT_JIS_MAP[0x875F], 1, 0x8775-0x875F+1);
	memset(&_SHIFT_JIS_MAP[0x877E], 1, 0x879C-0x877E+1);

	memset(&_SHIFT_JIS_MAP[0x889F], 1, 0x88FC-0x889F+1);
	for(BYTE hiByte = 0x89; hiByte <= 0xE9; hiByte++)
	{
		WORD idxBegin = MAKEWORD(0x40, hiByte);
		WORD idxEnd = MAKEWORD(0xFC, hiByte);
		memset(&_SHIFT_JIS_MAP[idxBegin], 1, idxEnd-idxBegin+1);		
	}
	memset(&_SHIFT_JIS_MAP[0xEA40], 1, 0xEAA4-0xEA40+1);

	memset(&_SHIFT_JIS_MAP[0xED40], 1, 0xEDFC-0xED40+1);
	memset(&_SHIFT_JIS_MAP[0xEE40], 1, 0xEEFC-0xEE40+1);

}