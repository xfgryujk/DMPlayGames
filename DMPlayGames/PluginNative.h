#pragma once
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <vector>


// 插件使用的文件夹
extern std::wstring CONFIG_DIR;


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
};

extern std::shared_ptr<PluginNative> g_pluginNative;


class PluginNativeFactory final
{
private:
	PluginNativeFactory() = default;
	PluginNativeFactory(const PluginNativeFactory&) = delete;

	// key -> generator
	std::map<std::wstring, std::function<std::shared_ptr<PluginNative>()> > m_generators;

public:
	static PluginNativeFactory& GetInstance()
	{
		static PluginNativeFactory s_instance;
		return s_instance;
	}

	std::shared_ptr<PluginNative> Create(const std::wstring& key);
	std::vector<std::wstring> GetKeys();


	template<class T>
	class AddGenerator
	{
	public:
		AddGenerator(const std::wstring& key)
		{
			auto& generators = PluginNativeFactory::GetInstance().m_generators;
			if (generators.find(key) == generators.end())
				generators.emplace(key, [] { return std::make_shared<T>(); });
			else
				throw std::invalid_argument("Duplicated key");
		}
	};
};

#define REGISTER_PLUGIN(T, key) static PluginNativeFactory::AddGenerator<T> __RegisterPlugin##T##(L##key)
