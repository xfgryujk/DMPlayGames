#include "stdafx.h"
#include "Util.h"

#include <tlhelp32.h>

#include <tuple>

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

// 寻找含有字符串的窗口
HWND FindWindowContainsString(LPCTSTR className, LPCTSTR title)
{
	HWND result = NULL;
	auto _param = tie(className, title, result);

	EnumWindows([](HWND hwnd, LPARAM pParam)->BOOL {
		auto& param = *(decltype(_param)*)pParam;
		TCHAR buffer[1024];

		if (get<0>(param) != NULL)
		{
			GetClassName(hwnd, buffer, _countof(buffer));
			if (_tcsstr(buffer, get<0>(param)) == NULL)
				return TRUE;
		}

		if (get<1>(param) != NULL)
		{
			GetWindowText(hwnd, buffer, _countof(buffer));
			if (_tcsstr(buffer, get<1>(param)) == NULL)
				return TRUE;
		}

		get<2>(param) = hwnd;
		return FALSE;
	}, (LPARAM)&_param);

	return result;
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
	Sleep(100);
}

// 模拟按键，使用信息
void PressKeyByMessage(HWND hwnd, UINT vk, DWORD holdTime)
{
	UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	SendMessage(hwnd, WM_KEYDOWN, vk, 1 | (scanCode << 16));
	Sleep(holdTime);
	SendMessage(hwnd, WM_KEYUP, vk, 1 | (scanCode << 16) | (1 << 30) | (1 << 31));
	Sleep(100);
}

// 模拟按键，使用API
void PressKeyByAPI(UINT vk, DWORD holdTime)
{
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;
	input.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	SendInput(1, &input, sizeof(INPUT));
	Sleep(holdTime);
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	Sleep(100);
}
