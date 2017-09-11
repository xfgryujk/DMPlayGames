// IG社的FLOWERS系列

#pragma once
#include "PluginNative.h"
#include "Util.h"

#include <set>
#include <vector>


class PluginFlowers : public PluginNative
{
public:
	virtual ~PluginFlowers() = default;

	virtual bool Start() override;
	virtual bool Stop() override;
	virtual void OnReceivedDanmaku(const std::wstring& text, const std::wstring& user) override;

	void MainThread();
protected:
	void StartVoting();
	void StopVoting();
	// 检测是否有选项
	virtual bool CheckIsChoosing() = 0;
	// 选择选项
	virtual void ChooseOption(int index);


	// 游戏相关

	HWND m_gameWindow = NULL;
	std::unique_ptr<HANDLE, HANDLEDeleter> m_gameProcess;
	DWORD m_scriptModuleBase = 0;

	// 插件主线程

	bool m_stopFlag = false;
	std::unique_ptr<HANDLE, HANDLEDeleter> m_mainThread;

	enum Status { NORMAL, VOTING, STOPPED };
	Status m_status = STOPPED;

	// 投票相关

	DWORD m_voteStartTime = 0;
	std::set<std::wstring> m_votedUsers;
	int m_optionsCount = 0;
	std::vector<int> m_voteCount;
};

class PluginFlowersSpring final : public PluginFlowers
{
protected:
	virtual bool CheckIsChoosing();
};

class PluginFlowersSummer final : public PluginFlowers
{
protected:
	virtual bool CheckIsChoosing();
};
