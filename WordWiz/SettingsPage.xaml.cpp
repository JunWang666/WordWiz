#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "SettingsData.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    int32_t SettingsPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SettingsPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void SettingsPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // 1. 定义要操作的表名和键名
        const std::string tableName = "user_settings"; // 例如，使用 "user_settings" 作为表名
        const std::string clickKey = "test_click_value";

        // 2. 创建 SettingsManager 的实例，并指定表名
        //    构造函数会处理数据库的初始化（如果尚未初始化）和表的创建（如果不存在）
        ::WordWiz::Data::SettingsManager settingsDbManager(tableName);

        // 3. 检查 SettingsManager 是否成功初始化（通过其内部逻辑）
        //    SettingsManager 的 isInitialized() 方法现在是实例方法
        if (!settingsDbManager.isInitialized())
        {
            // 处理错误：SettingsManager 初始化失败
            if (auto button = myButton()) { // 假设 myButton() 返回按钮控件
                button.Content(winrt::box_value(L"错误: 数据库初始化失败"));
            }
            return;
        }

        // 4. 从数据库获取当前值
        std::string currentValue = settingsDbManager.getString(clickKey, ""); // 通过实例调用

        // 5. 在当前值后面追加字符 'A'
        currentValue += "A";

        // 6. 将新值保存回数据库
        settingsDbManager.setString(clickKey, currentValue); // 通过实例调用

        // 7. 更新按钮内容以显示新的字符串值
        winrt::hstring displayText = winrt::to_hstring(currentValue);
        if (auto button = myButton()) {
            button.Content(winrt::box_value(displayText));
        }
    }
}
