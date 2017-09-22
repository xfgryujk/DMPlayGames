#include "stdafx.h"
#include "PluginNative.h"

#include <locale>
#include <codecvt>
#include <fstream>

using namespace std;


wstring CONFIG_DIR;


shared_ptr<PluginNative> g_pluginNative = make_shared<PluginNative>();

PluginNative::PluginNative()
{
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


// PluginNativeFactory /////////////////////////////////////////////////////////////////////

shared_ptr<PluginNative> PluginNativeFactory::Create(const std::wstring& key)
{
	auto it = m_generators.find(key);
	if (it == m_generators.end())
		return make_shared<PluginNative>();
	return it->second();
}

vector<wstring> PluginNativeFactory::GetKeys()
{
	vector<wstring> keys;
	for (const auto& i : m_generators)
		keys.push_back(i.first);
	return move(keys);
}
