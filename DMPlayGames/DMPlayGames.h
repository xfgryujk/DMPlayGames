#pragma once


// 托管的Plugin类，用来和弹幕姬交互
public ref class PluginManaged sealed : public BilibiliDM_PluginFramework::DMPlugin
{
public:
	PluginManaged();

	void Inited() override;
	void DeInit() override;
	void Start() override;
	void Stop() override;

private:
	void OnReceivedDanmaku(Object^ sender, BilibiliDM_PluginFramework::ReceivedDanmakuArgs^ e);
};
