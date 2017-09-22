#pragma once
#include <string>


struct HANDLEDeleter
{
	typedef HANDLE pointer;
	void operator() (pointer p)
	{
		CloseHandle(p);
	}
};

// 取其他进程的模块基址
DWORD GetRemoteModuleBase(DWORD pid, const std::wstring& moduleName);

// 寻找含有字符串的窗口
HWND FindWindowContainsString(LPCTSTR className, LPCTSTR title);

// 取窗口DPI缩放系数
float GetZoomScale(HWND hwnd);

// 模拟点击，使用消息
void ClickByMessage(HWND hwnd, const POINTS& pos);

// 模拟按键，使用信息
void PressKeyByMessage(HWND hwnd, UINT vk, DWORD holdTime = 100);

// 模拟按键，使用API
void PressKeyByAPI(UINT vk, DWORD holdTime);
