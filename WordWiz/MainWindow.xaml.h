#pragma once

#include "MainWindow.g.h"

namespace winrt::WordWiz::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    public:
        MainWindow();

        winrt::AppWindow MyAppWindow();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void NavigationView_SelectionChanged(const winrt::Microsoft::UI::Xaml::Controls::NavigationView, const winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        winrt::AppWindow GetAppWindowForCurrentWindow();

        winrt::AppWindow m_mainAppWindow{ nullptr };
        hstring m_windowTitle = L"WinUI Desktop C++ Sample App";
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
