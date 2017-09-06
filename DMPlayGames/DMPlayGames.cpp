// 这是主 DLL 文件。

#include "stdafx.h"

#include <msclr\marshal_cppstd.h>
using namespace msclr::interop;

#include "DMPlayGames.h"
using namespace BilibiliDM_PluginFramework;
using namespace std;

#include <locale>
#include <codecvt>
#include <fstream>
#include <algorithm>

#include <Shlobj.h>
#include <tlhelp32.h>


namespace DMPlayGames
{
	PluginNative g_pluginNative;


	Plugin::Plugin()
	{
		PluginAuth = L"xfgryujk";
		PluginName = L"弹幕玩游戏";
		PluginDesc = L"自制插件，用弹幕操作游戏";
		PluginCont = L"xfgryujk@126.com";
		PluginVer = L"1.0.0";

		ReceivedDanmaku += gcnew ReceivedDanmakuEvt(this, &Plugin::OnReceivedDanmaku);
	}

	void Plugin::Inited()
	{
		if (g_pluginNative.Inited())
			DMPlugin::Inited();
	}

	void Plugin::DeInit()
	{
		if (g_pluginNative.DeInit())
			DMPlugin::DeInit();
	}

	void Plugin::Start()
	{
		if (g_pluginNative.Start())
			DMPlugin::Start();
	}

	void Plugin::Stop()
	{
		if (g_pluginNative.Stop())
			DMPlugin::Stop();
	}

	void Plugin::OnReceivedDanmaku(Object^ sender, ReceivedDanmakuArgs^ e)
	{
		if (e->Danmaku->MsgType == MsgTypeEnum::Comment)
		{
			g_pluginNative.OnReceivedDanmaku(marshal_as<wstring>(e->Danmaku->CommentText),
				marshal_as<wstring>(e->Danmaku->CommentUser));
		}
	}


	PluginNative::PluginNative()
	{
		CONFIG_DIR.resize(MAX_PATH);
		SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, &CONFIG_DIR.front());
		CONFIG_DIR.resize(wcslen(CONFIG_DIR.c_str()));
		CONFIG_DIR += L"\\弹幕姬\\Plugins\\弹幕玩游戏";
		CreateDirectoryW(CONFIG_DIR.c_str(), NULL);

		OutputString(L"");
	}

	bool PluginNative::Inited()
	{
		return true;
	}

	bool PluginNative::DeInit()
	{
		return Stop();
	}

	static DWORD GetRemoteModuleBase(DWORD pid, const wstring& moduleName)
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

	bool PluginNative::Start()
	{
		m_gameWindow = FindWindow(_T("systemω"), NULL);
		if (m_gameWindow == NULL)
		{
			MessageBox(NULL, _T("找不到游戏窗口"), _T("弹幕玩游戏"), MB_ICONERROR);
			return false;
		}

		DWORD pid = 0;
		GetWindowThreadProcessId(m_gameWindow, &pid);
		m_gameProcess.reset(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid));
		if (m_gameProcess == NULL)
		{
			MessageBox(NULL, _T("无法打开游戏进程"), _T("弹幕玩游戏"), MB_ICONERROR);
			return false;
		}

		m_scriptModuleBase = GetRemoteModuleBase(pid, L"Script.dll");
		if (m_scriptModuleBase == 0)
		{
			MessageBox(NULL, _T("找不到Script.dll"), _T("弹幕玩游戏"), MB_ICONERROR);
			m_gameProcess.reset(NULL);
			return false;
		}

		m_status = NORMAL;
		m_stopFlag = false;
		m_mainThread.reset(CreateThread(NULL, 0, [](void* thiz)->DWORD {
			((PluginNative*)thiz)->MainThread();
			return 0;
		}, this, 0, NULL));

		return true;
	}

	bool PluginNative::Stop()
	{
		m_stopFlag = true;
		if (m_mainThread)
			WaitForSingleObject(m_mainThread.get(), INFINITE);
		m_mainThread.reset(NULL);
		m_status = STOPPED;

		m_gameProcess.reset(NULL);

		OutputString(L"");

		return true;
	}

	void PluginNative::OnReceivedDanmaku(const std::wstring& text, const std::wstring& user)
	{
		if (m_status != VOTING)
			return;

		int index;
		try
		{
			index = stoi(text);
		}
		catch (invalid_argument&)
		{
			return;
		}
		catch (out_of_range&)
		{
			return;
		}
		if (index < 1 || index > m_voteCount.size())
			return;
		index -= 1;

		if (m_votedUsers.find(user) == m_votedUsers.end())
		{
			m_votedUsers.insert(user);
			++m_voteCount[index];
		}
	}

	void PluginNative::MainThread()
	{
		while (!m_stopFlag)
		{
			switch (m_status)
			{
			case NORMAL: // 检测是否有选项
				if (CheckIsChoosing())
					StartVoting();
				break;

			case VOTING: // 投票
				constexpr int voteTime = 20; // 投票时间，秒
				if (GetTickCount() - m_voteStartTime > voteTime * 1000)
				{
					StopVoting();
					ChooseOption();
				}
				else
				{
					wstring output = L"投票中：\n";
					for (int i = 0; i < m_voteCount.size(); ++i)
						output += L"选项" + to_wstring(i + 1) + L"：" + to_wstring(m_voteCount[i]) + L"\n";
					output += L"发送选项号投票\n";
					output += L"剩余" + to_wstring(voteTime - (GetTickCount() - m_voteStartTime) / 1000) + L"秒\n";
					OutputString(output);
				}
				break;
			}

			// 等待1秒
			for (int i = 0; i < 10; ++i)
			{
				if (m_stopFlag)
					break;
				Sleep(100);
			}
		}
	}

	void PluginNative::StartVoting()
	{
		m_status = VOTING;
		m_voteStartTime = GetTickCount();
		m_votedUsers.clear();
		m_voteCount.clear();
		m_voteCount.resize(m_optionsCount);

		OutputString(L"开始投票！\n");
	}

	void PluginNative::StopVoting()
	{
		m_status = NORMAL;

		OutputString(L"");
	}

	// 检测是否有选项
	bool PluginNative::CheckIsChoosing()
	{
		// 游戏状态：BYTE PTR [[Script.dll + 0x4A71B0] + 0x1D00]
		// 选项数：BYTE PTR [[Script.dll + 0x4A71B0] + 0x1F14]
		DWORD base;
		if (!ReadProcessMemory(m_gameProcess.get() , LPCVOID(m_scriptModuleBase + 0x4A71B0), &(base = 0), 4, NULL))
			return false;
		DWORD gameStatus;
		if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1D00), &(gameStatus = 0), 1, NULL))
			return false;
		if (gameStatus != 2) // 有选项
			return false;
		if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1F14), &(m_optionsCount = 0), 1, NULL))
			return false;
		return true;
	}

	// 选择投票最多的选项
	void PluginNative::ChooseOption()
	{
		int index = int(max_element(m_voteCount.cbegin(), m_voteCount.cend()) - m_voteCount.cbegin());

		// 第1个选项Y坐标为186，但第4个选项Y坐标为108
		// 要考虑DPI缩放，偷懒直接硬编码了
		POINTS pos = { 1280 / 0.8 / 2, (m_optionsCount > 3 ? 186 : 108) / 0.8 + 20 + index * 100 };
		
		SendMessage(m_gameWindow, WM_MOUSEMOVE, 0, *(DWORD*)&pos);
		Sleep(100);
		SendMessage(m_gameWindow, WM_LBUTTONDOWN, MK_LBUTTON, *(DWORD*)&pos);
		Sleep(100);
		SendMessage(m_gameWindow, WM_LBUTTONUP, 0, *(DWORD*)&pos);
	}

	// 输出文字到OBS
	void PluginNative::OutputString(const wstring& str)
	{
		wofstream f(CONFIG_DIR + L"\\输出文本.txt");
		if (!f.is_open())
			return;
		f.imbue(locale(locale(""), new codecvt_utf8<wchar_t>));
		f << str;
	}
}
