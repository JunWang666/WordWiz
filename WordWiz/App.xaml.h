#pragma once

#include "App.xaml.g.h"

namespace winrt::WordWiz::implementation
{
	struct App : AppT<App>
	{
		App();

		void OnLaunched(const LaunchActivatedEventArgs&);
		// 全局主题管理方法
		static void SetGlobalTheme(bool isDarkMode);
		static Window GetMainWindow();
		static void LoadAndApplySavedTheme();

	private:
		Window window{nullptr};
		static Window s_mainWindow;
	};
}
