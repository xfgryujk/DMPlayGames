#include "stdafx.h"
#include "PluginNative.h"

#include <Shlobj.h>

#include <locale>
#include <codecvt>
#include <fstream>

using namespace std;


shared_ptr<PluginNative> g_pluginNative = make_shared<PluginNative>();

PluginNative::PluginNative()
{
	CONFIG_DIR.resize(MAX_PATH);
	SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, &CONFIG_DIR.front());
	CONFIG_DIR.resize(wcslen(CONFIG_DIR.c_str()));
	CONFIG_DIR += L"\\弹幕姬\\Plugins\\弹幕玩游戏";
	CreateDirectoryW(CONFIG_DIR.c_str(), NULL);

	OutputString(L"");
}

bool PluginNative::Start()
{
	return true;
}

bool PluginNative::Stop()
{
	OutputString(L"");

	return true;
}

void PluginNative::OnReceivedDanmaku(const std::wstring& text, const std::wstring& user)
{

}

// 输出文字到OBS
void PluginNative::OutputString(const wstring& str)
{
	wofstream f(CONFIG_DIR + L"\\输出文本.txt");
	if (!f.is_open())
		return;
	f.imbue(locale(locale(), new codecvt_utf8_utf16<wchar_t>));
	f << str;
}
