#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "SettingsData.h"
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <chrono>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
	// 静态成员定义
	winrt::Microsoft::UI::Xaml::Window App::s_mainWindow{nullptr};

	/// <summary>
	/// Initializes the singleton application object.  This is the first line of authored code
	/// executed, and as such is the logical equivalent of main() or WinMain().
	/// </summary>
	App::App()
	{
		// Xaml objects should not call InitializeComponent during construction.
		// See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
		UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
		{
			if (IsDebuggerPresent())
			{
				auto errorMessage = e.Message();
				__debugbreak();
			}
		});
#endif
	}

	/// <summary>    /// <summary>
	/// Invoked when the application is launched.
	/// </summary>
	/// <param name="e">Details about the launch request and process.</param>
	void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
	{
		window = make<MainWindow>();
		s_mainWindow = window; // 保存主窗口引用

		// 先激活窗口
		window.Activate();

		// 延迟加载主题设置，确保窗口完全初始化后再应用
		LoadAndApplySavedTheme();
	}

	// 加载并应用保存的主题设置
	void App::LoadAndApplySavedTheme()
	{
		try
		{
			// 添加延迟以确保窗口完全初始化
			::WordWiz::Data::SettingsManager settingsManager("app_settings");
			if (settingsManager.isInitialized())
			{
				std::string themeMode = settingsManager.getString("theme_mode", "light");
				bool isDarkMode = (themeMode == "dark");
				SetGlobalTheme(isDarkMode);
			}
			else
			{
				// 如果数据库未初始化，使用默认主题
				SetGlobalTheme(false);
			}
		}
		catch (const std::exception& e)
		{
			// 记录错误但不中断应用程序
			// 使用默认主题（浅色）
			SetGlobalTheme(false);
		}
		catch (...)
		{
			// 处理任何其他异常
			SetGlobalTheme(false);
		}
	}

	// 全局主题设置方法
	void App::SetGlobalTheme(bool isDarkMode)
	{
		try
		{
			if (s_mainWindow)
			{
				// 直接应用到主窗口内容
				if (auto content = s_mainWindow.Content())
				{
					if (auto rootElement = content.try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>())
					{
						rootElement.RequestedTheme(isDarkMode
							                           ? winrt::Microsoft::UI::Xaml::ElementTheme::Dark
							                           : winrt::Microsoft::UI::Xaml::ElementTheme::Light);
					}
				}
			}
		}
		catch (...)
		{
			// 忽略主题设置错误，继续运行
		}
	}

	// 获取主窗口方法
	winrt::Microsoft::UI::Xaml::Window App::GetMainWindow()
	{
		return s_mainWindow;
	}
}
