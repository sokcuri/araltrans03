
#include "../stdafx.h"
#include "../AutoFinder.h"
#include "Misc.h"


BOOL IsJapaneseW(LPCWSTR wszJapaneseText, int nJapaneseLen)
{
	BOOL bRet=FALSE;
	int i;

	if (nJapaneseLen < 0) 
		nJapaneseLen=lstrlenW(wszJapaneseText);



	for(i=0; i<nJapaneseLen; i++)
	{
		if (
			((0x2E80 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x2EFF)) ||	// 2E80 - 2EFF 한중일 부수 보충
			//((0x3000 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x303F)) ||	// 3000 - 303F 한중일 기호 및 구두점
			((0x31C0 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x31FF)) ||	// 31C0 - 31EF 한중일 한자 획
			//																		// 31F0 - 31FF 가타카나 음성 확장
			//((0x3200 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x32FF)) ||	// 3200 - 32FF 한중일 괄호 문자
			((0x3300 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x4DBF)) ||	// 3300 - 33FF 한중일 호환용
			//																		// 3400 - 4DBF 한중일 통합 한자 확장-A
			((0x4E00 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x9FBF)) ||	// 4E00 - 9FBF 한중일 통합 한자
			((0xF900 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFAFF)) ||	// FA00 - FAFF 한중일 호환용 한자
			((0xFE30 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFE4F)) ||	// FE30 - FE4F 한중일 호환 글꼴
			((0xFF66 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xFF9F))		// FF66 - FF9F 반각 가타카나
			)
		{
			bRet=TRUE;	// 일단 일어
		}
		else if ((0x3040 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0x30FF))	// 3040 - 309F 히라가나
			//																		// 30A0 - 30FF 가타카나
		{
			bRet=TRUE;	// 확실히 일어
			break;

		}
		else if ((0xAC00 <= wszJapaneseText[i]) && (wszJapaneseText[i] <= 0xD7AF))	// AC00 - D7AF 한글 글자
		{
			bRet=FALSE;	// 확실히 한글
			break;
		}
	}

/*
	if (bRet)	// debug
	{
		FILE *fp;
		char szTemp[1024]={0, };
		fp=fopen("c:\\noncached.txt", "a");
		fprintf(fp, "[JAP] ");

		MyWideCharToMultiByte(CP_UTF8, 0, wszJapaneseText, nJapaneseLen, szTemp, 1023, 0, 0);
		fprintf(fp, "%s\n", szTemp);

		if (i == nJapaneseLen)
		{
			fprintf(fp, "(len=%d) ", i);
			for (i=0; i < nJapaneseLen; i++)
			{
				fprintf(fp, "%04X ", wszJapaneseText[i]);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}
//*/
	return bRet;
}