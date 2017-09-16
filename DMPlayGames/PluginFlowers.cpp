#include "stdafx.h"
#include "PluginFlowers.h"

#include <algorithm>

using namespace std;


bool PluginFlowers::Start()
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
		((PluginFlowers*)thiz)->MainThread();
		return 0;
	}, this, 0, NULL));

	return PluginNative::Start();
}

bool PluginFlowers::Stop()
{
	m_stopFlag = true;
	if (m_mainThread)
		WaitForSingleObject(m_mainThread.get(), INFINITE);
	m_mainThread.reset(NULL);
	m_status = STOPPED;

	m_gameProcess.reset(NULL);

	return PluginNative::Stop();
}

void PluginFlowers::OnReceivedDanmaku(const std::wstring& text, const std::wstring& user)
{
	if (m_status != VOTING)
		return;

	int index;
	try
	{
		index = stoi(text);
	}
	catch (logic_error&)
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

void PluginFlowers::MainThread()
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
			constexpr int VOTE_TIME = 20; // 投票时间，秒
			if (GetTickCount() - m_voteStartTime > VOTE_TIME * 1000)
			{
				StopVoting();
				int index = int(max_element(m_voteCount.cbegin(), m_voteCount.cend()) - m_voteCount.cbegin());
				ChooseOption(index);
			}
			else
			{
				wstring output = L"投票中：\n";
				for (int i = 0; i < m_voteCount.size(); ++i)
					output += L"选项" + to_wstring(i + 1) + L"：" + to_wstring(m_voteCount[i]) + L"\n";
				output += L"发送选项号投票\n";
				output += L"剩余" + to_wstring(VOTE_TIME - (GetTickCount() - m_voteStartTime) / 1000) + L"秒\n";
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

void PluginFlowers::StartVoting()
{
	m_status = VOTING;
	m_voteStartTime = GetTickCount();
	m_votedUsers.clear();
	m_voteCount.clear();
	m_voteCount.resize(m_optionsCount);

	OutputString(L"开始投票！\n");
}

void PluginFlowers::StopVoting()
{
	m_status = NORMAL;

	OutputString(L"");
}

void PluginFlowers::ChooseOption(int index)
{
	float scale = GetZoomScale(m_gameWindow);
	// 第1个选项Y坐标为186，但第4个选项Y坐标为108
	POINTS pos = { SHORT(1280 / 2 * scale), 
				   SHORT(((m_optionsCount > 3 ? 108 : 186) + 20 + index * 78) * scale) };

	ClickByMessage(m_gameWindow, pos);
}


// 春篇 /////////////////////////////////////////////////////////////////////

REGISTER_PLUGIN(PluginFlowersSpring, "FLOWERS 春篇");

bool PluginFlowersSpring::CheckIsChoosing()
{
	// 游戏状态：BYTE PTR [[Script.dll + 0x4A71B0] + 0x1D00]
	// 选项数：BYTE PTR [[Script.dll + 0x4A71B0] + 0x1F14]
	DWORD base;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(m_scriptModuleBase + 0x4A71B0), &(base = 0), 4, NULL))
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


// 夏篇 /////////////////////////////////////////////////////////////////////

REGISTER_PLUGIN(PluginFlowersSummer, "FLOWERS 夏篇");

bool PluginFlowersSummer::CheckIsChoosing()
{
	// 游戏状态：BYTE PTR [[Script.dll + 0xA42654] + 0x1B28]
	// 选项数：BYTE PTR [[Script.dll + 0xA42654] + 0x1D3C]
	DWORD base;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(m_scriptModuleBase + 0xA42654), &(base = 0), 4, NULL))
		return false;
	DWORD gameStatus;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1B28), &(gameStatus = 0), 1, NULL))
		return false;
	if (gameStatus != 2) // 有选项
		return false;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1D3C), &(m_optionsCount = 0), 1, NULL))
		return false;
	return true;
}


// 秋篇 /////////////////////////////////////////////////////////////////////

REGISTER_PLUGIN(PluginFlowersAutumn, "FLOWERS 秋篇");

bool PluginFlowersAutumn::CheckIsChoosing()
{
	// 游戏状态：BYTE PTR [[Script.dll + 0xA436F8] + 0x1B2C]
	// 选项数：BYTE PTR [[Script.dll + 0xAA436F8] + 0x1D40]
	DWORD base;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(m_scriptModuleBase + 0xA436F8), &(base = 0), 4, NULL))
		return false;
	DWORD gameStatus;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1B2C), &(gameStatus = 0), 1, NULL))
		return false;
	if (gameStatus != 2) // 有选项
		return false;
	if (!ReadProcessMemory(m_gameProcess.get(), LPCVOID(base + 0x1D40), &(m_optionsCount = 0), 1, NULL))
		return false;
	return true;
}
