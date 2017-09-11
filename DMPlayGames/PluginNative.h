#pragma once
#include <string>
#include <memory>


// 本机的Plugin类，实现主要逻辑
class PluginNative
{
public:
	PluginNative();
	virtual ~PluginNative() = default;

	virtual bool Start();
	virtual bool Stop();
	virtual void OnReceivedDanmaku(const std::wstring& text, const std::wstring& user);

protected:
	// 输出文字到OBS
	void OutputString(const std::wstring& str);


	// 插件使用的文件夹
	std::wstring CONFIG_DIR;
};

extern std::shared_ptr<PluginNative> g_pluginNative;
