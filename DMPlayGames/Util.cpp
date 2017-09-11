#include "stdafx.h"
#include "Util.h"

#include <tlhelp32.h>

using namespace std;


// 取其他进程的模块基址
DWORD GetRemoteModuleBase(DWORD pid, const wstring& moduleName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	MODULEENTRY32W moduleentry;
	moduleentry.dwSize = sizeof(moduleentry);

	BOOL hasNext = Module32FirstW(snapshot, &moduleentry);
	DWORD moduleBase = 0;
	do
	{
		if (_wcsicmp(moduleentry.szModule, moduleName.c_str()) == 0)
		{
			moduleBase = (DWORD)moduleentry.modBaseAddr;
			break;
		}
		hasNext = Module32NextW(snapshot, &moduleentry);
	} while (hasNext);

	CloseHandle(snapshot);
	return moduleBase;
}

// 取窗口DPI缩放系数
float GetZoomScale(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	float scale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
	ReleaseDC(hwnd, hdc);
	return scale;
}

// 模拟点击，使用消息
void ClickByMessage(HWND hwnd, const POINTS& pos)
{
	SendMessage(hwnd, WM_MOUSEMOVE, 0, *(DWORD*)&pos);
	Sleep(100);
	SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, *(DWORD*)&pos);
	Sleep(100);
	SendMessage(hwnd, WM_LBUTTONUP, 0, *(DWORD*)&pos);
}
