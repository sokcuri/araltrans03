#pragma once

void OptionDialogInit(HWND hwndDlg);
void OnFontLoadingLevelChanged();
BOOL CALLBACK OptionDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
