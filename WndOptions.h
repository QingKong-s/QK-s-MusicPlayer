#pragma once
#include <Windows.h>

#define OPTIONSWNDTABCOUNT 4

void Settings_Read();
void Settings_Save();
INT_PTR CALLBACK DlgProc_OptLrc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Options(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);