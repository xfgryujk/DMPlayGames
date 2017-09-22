// 这是主 DLL 文件。

#include "stdafx.h"
#include "DMPlayGames.h"
#include "PluginNative.h"

#include <Shlobj.h>
#include <Shlwapi.h>

#include <locale>
#include <fstream>

#include <msclr\marshal_cppstd.h>

using namespace BilibiliDM_PluginFramework;
using namespace std;
using namespace msclr::interop;


PluginManaged::PluginManaged()
{
	PluginAuth = L"xfgryujk";
	PluginName = L"弹幕玩游戏";
	PluginDesc = L"自制插件，用弹幕操作游戏";
	PluginCont = L"xfgryujk@126.com";
	PluginVer = L"1.0.0";

	ReceivedDanmaku += gcnew ReceivedDanmakuEvt(this, &PluginManaged::OnReceivedDanmaku);
}

void PluginManaged::Inited()
{
	DMPlugin::Inited();

	locale::global(locale(""));

	// 初始化配置目录
	CONFIG_DIR.resize(MAX_PATH);
	SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, &CONFIG_DIR.front());
	CONFIG_DIR.resize(wcslen(CONFIG_DIR.c_str()));
	CONFIG_DIR += L"\\弹幕姬\\Plugins\\弹幕玩游戏";
	CreateDirectoryW(CONFIG_DIR.c_str(), NULL);

	// 初始化配置
	wstring configPath = CONFIG_DIR + L"\\配置.ini";
	if (!PathFileExistsW(configPath.c_str()))
	{
		WritePrivateProfileStringW(L"DMPlayGames", L"GameName", L"", configPath.c_str());
	}

	// 输出支持的游戏
	wofstream f(CONFIG_DIR + L"\\支持的游戏.txt");
	if (f.is_open())
	{
		auto keys = PluginNativeFactory::GetInstance().GetKeys();
		for (const auto& i : keys)
			f << i << endl;
	}

	// 创建插件
	wstring key(1024, L'\0');
	GetPrivateProfileStringW(L"DMPlayGames", L"GameName", L"", &key.front(), (DWORD)key.size(), configPath.c_str());
	key.resize(wcslen(key.c_str()));
	g_pluginNative = PluginNativeFactory::GetInstance().Create(key);
}

void PluginManaged::DeInit()
{
	if (Status)
		Stop();
	DMPlugin::DeInit();
}

void PluginManaged::Start()
{
	if (g_pluginNative->Start())
		DMPlugin::Start();
}

void PluginManaged::Stop()
{
	if (g_pluginNative->Stop())
		DMPlugin::Stop();
}

void PluginManaged::OnReceivedDanmaku(Object^ sender, ReceivedDanmakuArgs^ e)
{
	if (e->Danmaku->MsgType == MsgTypeEnum::Comment)
	{
		g_pluginNative->OnReceivedDanmaku(marshal_as<wstring>(e->Danmaku->CommentText),
			marshal_as<wstring>(e->Danmaku->CommentUser));
	}
}
