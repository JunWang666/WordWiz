#pragma once

#include "MainWindow.g.h"

namespace winrt::WordWiz::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		int32_t MyProperty();
		void MyProperty(int32_t value);

		void NavigationView_SelectionChanged(Microsoft::UI::Xaml::Controls::NavigationView,
		                                     Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs);
		void NavigationView_ItemInvoked(
			const Microsoft::UI::Xaml::Controls::NavigationView& sender,
			const Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs& args);

		void BackButton_Click(const IInspectable&, const Microsoft::UI::Xaml::RoutedEventArgs&);
		void CustomPaneToggleButton_Click(const IInspectable&, const Microsoft::UI::Xaml::RoutedEventArgs&);
		void OnWindowLoaded(const Windows::Foundation::IInspectable& sender,
		                    const Microsoft::UI::Xaml::RoutedEventArgs& e);

		static Microsoft::UI::Xaml::Controls::Frame GetMainFrame();

	private:
		Microsoft::UI::Windowing::AppWindow GetAppWindowForCurrentWindow();
		static Microsoft::UI::Xaml::Controls::Frame mainFrame;
		Microsoft::UI::Windowing::AppWindow m_mainAppWindow{nullptr};
		hstring m_windowTitle = L"WinUI Desktop C++ Sample App";

		// 窗口最小尺寸处理
		static LRESULT CALLBACK WindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		                                           UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	public:
		void BackButton_RightTapped(const Windows::Foundation::IInspectable& sender,
		                            const Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs& e);
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
