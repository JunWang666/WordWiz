#pragma once

#include "WordDetails.g.h"

namespace winrt::WordWiz::implementation
{
    struct WordDetails : WordDetailsT<WordDetails>
    {
        WordDetails()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordDetails : WordDetailsT<WordDetails, implementation::WordDetails>
    {
    };
}
