// DMPlayGames.h

#pragma once

#include <string>
#include <set>
#include <vector>
#include <memory>


namespace DMPlayGames
{
	// 托管的Plugin类，用来和弹幕姬交互
	public ref class Plugin sealed : public BilibiliDM_PluginFramework::DMPlugin
	{
	public:
		Plugin();

		void Inited() override;
		void DeInit() override;
		void Start() override;
		void Stop() override;

	private:
		void OnReceivedDanmaku(Object^ sender, BilibiliDM_PluginFramework::ReceivedDanmakuArgs^ e);
	};

	// 本机的Plugin类，实现主要逻辑
	class PluginNative final
	{
	public:
		PluginNative();

		bool Inited();
		bool DeInit();
		bool Start();
		bool Stop();
		void OnReceivedDanmaku(const std::wstring& text, const std::wstring& user);

		void MainThread();
	private:
		void StartVoting();
		void StopVoting();
		// 检测是否有选项
		bool CheckIsChoosing();
		// 选择投票最多的选项
		void ChooseOption();
		// 输出文字到OBS
		void OutputString(const std::wstring& str);


		// 插件使用的文件夹
		std::wstring CONFIG_DIR;

		// 游戏相关

		HWND m_gameWindow = NULL;
		struct HANDLEDeleter
		{
			typedef HANDLE pointer;
			void operator() (pointer p)
			{
				CloseHandle(p);
			}
		};
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
		int m_firstOptionTop = 0;
	};

	extern PluginNative g_pluginNative;
}
