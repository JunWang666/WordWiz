#include "pch.h"
#include "WordDetails.xaml.h"
#if __has_include("WordDetails.g.cpp")
#include "WordDetails.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    int32_t WordDetails::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void WordDetails::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

}
