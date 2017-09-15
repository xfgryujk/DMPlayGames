#pragma once
#include "PluginNative.h"

#include <map>
#include <string>


class PluginGBA final : public PluginNative
{
public:
	PluginGBA();

	virtual bool Start() override;
	virtual bool Stop() override;
	virtual void OnReceivedDanmaku(const std::wstring& text, const std::wstring& user) override;


private:
	// 弹幕指令 -> 虚拟键码
	std::map<std::wstring, UINT> m_virtualKeys;

	// 游戏相关

	HWND m_gameWindow = NULL;
};
