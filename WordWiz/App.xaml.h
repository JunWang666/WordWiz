#pragma once

#include "App.xaml.g.h"

namespace winrt::WordWiz::implementation
{
	struct App : AppT<App>
	{
		App();

		void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
		// 全局主题管理方法
		static void SetGlobalTheme(bool isDarkMode);
		static winrt::Microsoft::UI::Xaml::Window GetMainWindow();
		static void LoadAndApplySavedTheme();

	private:
		winrt::Microsoft::UI::Xaml::Window window{nullptr};
		static winrt::Microsoft::UI::Xaml::Window s_mainWindow;
	};
}
