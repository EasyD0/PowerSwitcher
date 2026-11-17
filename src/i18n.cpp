#include "i18n.h"

#include "app.h"

#include <windows.h>

namespace {

const wchar_t* const kEnglish[] = {
    L"PowerSwitcher",
    L"Open settings",
    L"Exit",
    L"PowerSwitcher - Settings",
    L"Automatically switch plans when the power source changes",
    L"When plugged in:",
    L"On battery:",
    L"Show only default power plans",
    L"Left-click the tray icon:",
    L"Do nothing",
    L"Show the tray menu",
    L"Language:",
    L"OK",
    L"Cancel",
    L"(No power plans found)",
};

const wchar_t* const kChinese[] = {
    L"PowerSwitcher",
    L"打开设置",
    L"退出",
    L"PowerSwitcher - 设置",
    L"电源变化时自动切换电源计划",
    L"接通交流电时：",
    L"使用电池时：",
    L"仅显示默认电源计划",
    L"左键单击托盘图标：",
    L"什么都不做",
    L"展开托盘菜单",
    L"语言：",
    L"确定",
    L"取消",
    L"（未找到电源计划）",
};

static_assert(sizeof(kEnglish) / sizeof(kEnglish[0]) == static_cast<size_t>(Str::Count),
              "English string table is out of sync with Str");
static_assert(sizeof(kChinese) / sizeof(kChinese[0]) == static_cast<size_t>(Str::Count),
              "Chinese string table is out of sync with Str");

} // namespace

int DetectSystemLanguage()
{
    const LANGID id = GetUserDefaultUILanguage();
    return PRIMARYLANGID(id) == LANG_CHINESE ? 1 : 0;
}

const wchar_t* T(Str string)
{
    const int index = static_cast<int>(string);
    if (index < 0 || index >= static_cast<int>(Str::Count))
        return L"";

    return (g_app.settings.language == 1 ? kChinese : kEnglish)[index];
}
