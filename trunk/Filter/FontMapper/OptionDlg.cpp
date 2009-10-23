// Windows 헤더 파일:
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <string>
#include <map>
#include "../../Common/DefStruct.h"		// AralTrans 함수 및 구조체가 정의된 헤더 파일 Include
#include "FontMapper.h"
#include "OptionDlg.h"
#include "resource.h"

using namespace std;

map<wstring,int> g_mapCodePages;
HWND g_hwndOptionDlg = NULL;
extern FONT_MAPPER_OPTION g_sTempOption;

void OptionDialogInit(HWND hwndDlg)
{
	g_hwndOptionDlg = hwndDlg;
	
	// Temporary
	g_sTempOption.nCodePageFrom = 932;
	g_sTempOption.nCodePageTo	= 949;
	::EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_FROM), FALSE);
	::EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_TO), FALSE);
	

	// Program Locale
	g_mapCodePages.insert(pair<wstring,int>(L"Unified Hangul Code (949)", 949));
	g_mapCodePages.insert(pair<wstring,int>(L"Simplified Chinese (936)", 936));
	g_mapCodePages.insert(pair<wstring,int>(L"Japanese Shift-JIS (932)", 932));
	g_mapCodePages.insert(pair<wstring,int>(L"Thai (874)", 874));

	for(map<wstring,int>::iterator iter = g_mapCodePages.begin();
		iter != g_mapCodePages.end();
		iter++)
	{
		int nIdx = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_FROM), CB_ADDSTRING, 0, (LPARAM)iter->first.c_str());
		if(g_sTempOption.nCodePageFrom == iter->second)
			SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_FROM), CB_SETCURSEL, (WPARAM)nIdx, 0);
	}
	
	// User-defined Locale
	for(map<wstring,int>::iterator iter = g_mapCodePages.begin();
		iter != g_mapCodePages.end();
		iter++)
	{
		int nIdx = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_TO), CB_ADDSTRING, 0, (LPARAM)iter->first.c_str());
		if(g_sTempOption.nCodePageTo == iter->second)
			SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_LOCALE_TO), CB_SETCURSEL, (WPARAM)nIdx, 0);
	}

	// Character Encoding 
	SendMessage(GetDlgItem(hwndDlg, IDC_CHK_ENCODE), BM_SETCHECK, g_sTempOption.bEncoding, NULL);

	// Font face name
	SetDlgItemText(hwndDlg, IDC_EDIT_FONT_NAME, g_sTempOption.wszFontFaceName);
	
	// Font loading level
	SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETRANGE, TRUE, MAKELONG(0, 4));
	SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETTICFREQ, 1, NULL);
	SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_SETPOS, TRUE, g_sTempOption.nFontLoadLevel/5);
	OnFontLoadingLevelChanged();
}

void OnFontLoadingLevelChanged()
{
	int nLoadingLevel = SendMessage(GetDlgItem(g_hwndOptionDlg, IDC_SLIDER1), TBM_GETPOS, NULL, NULL) * 5;
	
	BOOL bEnabled;
	if(nLoadingLevel) bEnabled = TRUE;
	else bEnabled = FALSE;

	::EnableWindow(GetDlgItem(g_hwndOptionDlg, IDC_EDIT_FONT_NAME), bEnabled);
	::EnableWindow(GetDlgItem(g_hwndOptionDlg, IDC_BTN_FONT_LOAD), bEnabled);
}

BOOL CALLBACK OptionDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			OptionDialogInit(hwndDlg);
			return TRUE;

		case WM_HSCROLL:
			if(GetDlgItem(hwndDlg, IDC_SLIDER1) == (HWND)lParam)
			{
				OnFontLoadingLevelChanged();
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTN_FONT_LOAD:
				{
					// 현재 폰트 노드를 가지고 LOGFONT 구조체 값 세팅					
					LOGFONTW logfont;
					ZeroMemory(&logfont, sizeof(LOGFONT));
					GetDlgItemText(hwndDlg, IDC_EDIT_FONT_NAME, logfont.lfFaceName, LF_FACESIZE);
					//wcscpy_s(logfont.lfFaceName, LF_FACESIZE, g_sTempOption.wszFontFaceName);
					//logfont.lfHeight = (long)g_sTempOption.nFixedFontSize;
					
					// Setting CHOOSEFONT structure
					CHOOSEFONTW cf;
					ZeroMemory(&cf, sizeof(CHOOSEFONTW));
					cf.lStructSize = sizeof(CHOOSEFONTW);
					cf.hwndOwner = hwndDlg;
					cf.lpLogFont = &logfont;
					cf.Flags = CF_EFFECTS | CF_SCREENFONTS;

					if(::ChooseFontW(&cf))
					{
						//wcscpy_s(g_sTempOption.wszFontFaceName, LF_FACESIZE, cf.lpLogFont->lfFaceName);						
						SetDlgItemText(hwndDlg, IDC_EDIT_FONT_NAME, cf.lpLogFont->lfFaceName);
					}
					break;
				}

				case IDOK:
				{
					// Program Locale
					wchar_t wszLocaleName[64];
					ZeroMemory(wszLocaleName, 64);
					GetDlgItemText(hwndDlg, IDC_COMBO_LOCALE_FROM, wszLocaleName, 64);
					map<wstring,int>::iterator iter = g_mapCodePages.find(wszLocaleName);
					if(iter != g_mapCodePages.end())
					{
						g_sTempOption.nCodePageFrom = iter->second;
					}
					else
					{
						g_sTempOption.nCodePageFrom = 0;
					}

					// User-defined Locale
					ZeroMemory(wszLocaleName, 64);
					GetDlgItemText(hwndDlg, IDC_COMBO_LOCALE_TO, wszLocaleName, 64);
					iter = g_mapCodePages.find(wszLocaleName);
					if(iter != g_mapCodePages.end())
					{
						g_sTempOption.nCodePageTo = iter->second;
					}
					else
					{
						g_sTempOption.nCodePageTo = 0;
					}

					// Character Encoding 
					g_sTempOption.bEncoding = SendMessage(GetDlgItem(hwndDlg, IDC_CHK_ENCODE), BM_GETCHECK, NULL, NULL);

					// Font loading level
					g_sTempOption.nFontLoadLevel = SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER1), TBM_GETPOS, NULL, NULL) * 5;
					
					// Font face name
					GetDlgItemText(hwndDlg, IDC_EDIT_FONT_NAME, g_sTempOption.wszFontFaceName, LF_FACESIZE);
					if(g_sTempOption.nFontLoadLevel > 0 && g_sTempOption.wszFontFaceName[0] == L'\0')
					{
						wcscpy_s(g_sTempOption.wszFontFaceName, LF_FACESIZE, L"궁서");
					}

				}
				case IDCANCEL:
					EndDialog(hwndDlg, LOWORD(wParam));
					return TRUE;

				default:;
			}
			break;

		default:;

	}
	//return ::DefWindowProc(hwndDlg, uMsg, wParam, lParam);
	return FALSE;
}