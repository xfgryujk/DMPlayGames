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

// 取窗口DPI缩放系数
float GetZoomScale(HWND hwnd);

// 模拟点击，使用消息
void ClickByMessage(HWND hwnd, const POINTS& pos);
