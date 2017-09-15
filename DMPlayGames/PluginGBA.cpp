#include "stdafx.h"
#include "PluginGBA.h"
#include "Util.h"

#include <regex>

using namespace std;


REGISTER_PLUGIN(PluginGBA, "GBA");

PluginGBA::PluginGBA()
{
	for (auto& i : L"WSADJKUIVB")
	{
		if (i != L'\0')
			m_virtualKeys[wstring(1, i)] = i;
	}
}

bool PluginGBA::Start()
{
	m_gameWindow = FindWindow(_T("Afx:400000:0:0:900011:16060b"), NULL);
	if (m_gameWindow == NULL)
	{
		MessageBox(NULL, _T("找不到游戏窗口"), _T("弹幕玩游戏"), MB_ICONERROR);
		return false;
	}

	return PluginNative::Start();
}

bool PluginGBA::Stop()
{
	return PluginNative::Stop();
}

void PluginGBA::OnReceivedDanmaku(const std::wstring& text, const std::wstring& user)
{
	if (GetForegroundWindow() != m_gameWindow)
		return;

	static const wregex COMMAND_REG(LR"(^(\w+?)(\d*?\.?\d*?)$)", wregex::ECMAScript | wregex::icase);
	wsmatch match;
	if (regex_match(text, match, COMMAND_REG))
	{
		wstring command = match[1];
		transform(command.begin(), command.end(), command.begin(), toupper);
		auto it = m_virtualKeys.find(command);
		if (it == m_virtualKeys.end())
			return;

		float holdTime = 0.1f;
		try
		{
			if (match[2] != L"")
				holdTime = stof(match[2]);
		}
		catch (logic_error&)
		{
			return;
		}
		if (holdTime < 0)
			return;
		if (holdTime > 5) // 时间限制
			holdTime = 5;

		PressKeyByAPI(it->second, DWORD(holdTime * 1000));
	}
}
