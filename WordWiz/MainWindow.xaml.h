#pragma once

#include "MainWindow.g.h"

namespace winrt::WordWiz::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    public:
        MainWindow();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void NavigationView_SelectionChanged(const winrt::Microsoft::UI::Xaml::Controls::NavigationView, const winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs);
        void NavigationView_ItemInvoked(
            winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender,
            winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        void BackButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);

        void OnWindowLoaded(winrt::Windows::Foundation::IInspectable const& sender,winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        static winrt::Microsoft::UI::Xaml::Controls::Frame GetMainFrame();

    private:
        winrt::Microsoft::UI::Windowing::AppWindow GetAppWindowForCurrentWindow();
        static winrt::Microsoft::UI::Xaml::Controls::Frame mainFrame;
        winrt::Microsoft::UI::Windowing::AppWindow m_mainAppWindow{ nullptr };
        hstring m_windowTitle = L"WinUI Desktop C++ Sample App";
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
