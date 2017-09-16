// 这是主 DLL 文件。

#include "stdafx.h"
#include "DMPlayGames.h"
#include "PluginNative.h"

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

	g_pluginNative = PluginNativeFactory::GetInstance().Create(L"FLOWERS 秋篇"); // 测试用
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
