#include "pch.h"
#include "WordSearchResultItem.xaml.h"
#if __has_include("WordSearchResultItem.g.cpp")
#include "WordSearchResultItem.g.cpp"
#endif
#include <logger.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    WordSearchResultItem::WordSearchResultItem()
    {
        
    }

    hstring WordSearchResultItem::Word()
    {
        return word;
    }

    void WordSearchResultItem::Word(hstring const& value)
    {
        word = value;
    }

    hstring WordSearchResultItem::Explanation()
    {
        return explanation;
    }

    void WordSearchResultItem::Explanation(hstring const& value)
    {
        explanation = value;
    }

    void WordSearchResultItem::OnItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
		WordWizServices::Log::LogMessage(L"WordSearchResultItem::OnItemClick");
    }

    int32_t WordSearchResultItem::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void WordSearchResultItem::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
