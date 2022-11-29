#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <ctime>
#include "resource.h"

#define DEFFAULT_STR_SIZE 256

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

HWND hReload;
HWND hTerminate;
HWND hNew;
HWND hProcName;
HWND hProcList;
HWND hStatID;
HWND hTime;

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

void ShowProcList(HWND hList) {
	SendMessage(hProcList, LB_RESETCONTENT, 0, 0);
	EnableWindow(hTerminate, FALSE);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processInfo = { sizeof(PROCESSENTRY32) };

	if (Process32First(hSnapshot, &processInfo) == TRUE) {
		do {
			LRESULT index = SendMessage(
				hList, 
				LB_ADDSTRING, 
				0, 
				LPARAM(processInfo.szExeFile)
			);

			SendMessage(
				hList,
				LB_SETITEMDATA,
				WPARAM(index),
				LPARAM(processInfo.th32ProcessID)
			);
		} while (Process32Next(hSnapshot, &processInfo));
	}
	CloseHandle(hSnapshot);
}

DWORD WINAPI Thread(LPVOID lp)
{
	WCHAR buff[DEFFAULT_STR_SIZE]{};

	while (true)
	{
		ShowProcList(hProcList);

		_wstrtime_s(buff, DEFFAULT_STR_SIZE);
		wcscat_s(buff, DEFFAULT_STR_SIZE, TEXT(" "));
		_wstrdate_s(buff + wcslen(buff), DEFFAULT_STR_SIZE);


		SetWindowText(hTime, buff);

		Sleep(5000);
	}
}	
BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp)
{
	switch (mes)
	{
	case WM_INITDIALOG:
	{	
		hReload = GetDlgItem(hWnd, IDC_BUTTON1);
		hTerminate = GetDlgItem(hWnd, IDC_BUTTON2);
		hNew = GetDlgItem(hWnd, IDC_BUTTON3);
		hProcName = GetDlgItem(hWnd, IDC_EDIT1);
		hProcList = GetDlgItem(hWnd, IDC_LIST1);
		hStatID = GetDlgItem(hWnd, IDC_EDIT2);
		hTime = GetDlgItem(hWnd, IDC_OUTTIME);

		CreateThread(NULL, 0, Thread, hWnd, 0, NULL);
	}
	return TRUE;

	case WM_COMMAND:
	{
		if (LOWORD(wp) == IDC_BUTTON1) {
			ShowProcList(hProcList);

			FILE* file;
			_wfopen_s(&file, TEXT("BackUp.txt"), TEXT("w"));

			if (file != NULL)
			{
				WCHAR buff[DEFFAULT_STR_SIZE]{};

				const int COUNT = SendMessage(hProcList, LB_GETCOUNT, 0, 0);

				for (unsigned i = 0; i < COUNT; ++i)
				{
					SendMessage(hProcList, LB_GETTEXT, (WPARAM)i, (LPARAM)buff);
					wcscat_s(buff, DEFFAULT_STR_SIZE, TEXT("\n"));
					fwprintf(file, buff, DEFFAULT_STR_SIZE);
				}

				fclose(file);
			}
		}
		else if (LOWORD(wp) == IDC_BUTTON2) {
			LRESULT index = SendMessage(hProcList, LB_GETCURSEL, 0, 0);
			HANDLE procHandle = OpenProcess(
				PROCESS_TERMINATE, 
				TRUE, 
				SendMessage(hProcList, LB_GETITEMDATA, WPARAM(index), 0)
			);

			TerminateProcess(procHandle, NULL);
			ShowProcList(hProcList);
			CloseHandle(procHandle);		
		}
		else if (LOWORD(wp) == IDC_BUTTON3) 
		{
			wchar_t buf[DEFFAULT_STR_SIZE]{ 0 };
			GetWindowText(hProcName, buf, DEFFAULT_STR_SIZE);
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			if (CreateProcess(buf, buf, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
			{
				ShowProcList(hProcList);
			}
		}
		else if (HIWORD(wp) == LBN_SELCHANGE) 
		{
			EnableWindow(hTerminate, TRUE);
			LRESULT index = SendMessage(hProcList, LB_GETCURSEL, 0, 0);
			int procId = SendMessage(hProcList, LB_GETITEMDATA, WPARAM(index), 0);
			TCHAR buf[DEFFAULT_STR_SIZE];
			wsprintf(buf, TEXT("%i"), procId);
			SendMessage(hStatID, WM_SETTEXT, 0, LPARAM(buf));			
		}
	}
	break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		EndDialog(hWnd, 0);
		return TRUE;
	}
	return FALSE;
}