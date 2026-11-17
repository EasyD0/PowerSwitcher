#include "config.h"

#include <windows.h>

namespace {

std::wstring ConfigPath()
{
    wchar_t buffer[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"APPDATA", buffer, MAX_PATH);

    std::wstring directory;
    if (length > 0 && length < MAX_PATH)
        directory.assign(buffer, length);
    else
        directory = L".";

    directory += L"\\PowerSwitcher";
    CreateDirectoryW(directory.c_str(), nullptr);
    return directory + L"\\config.ini";
}

std::wstring ReadValue(const std::wstring& path, const wchar_t* key, const wchar_t* fallback)
{
    wchar_t buffer[256]{};
    GetPrivateProfileStringW(L"General", key, fallback, buffer, 256, path.c_str());
    return buffer;
}

} // namespace

std::wstring GetConfigPath()
{
    return ConfigPath();
}

void LoadSettings(Settings& settings)
{
    const std::wstring path = ConfigPath();

    settings.autoSwitch =
        GetPrivateProfileIntW(L"General", L"AutoSwitch", 0, path.c_str()) != 0;
    settings.showOnlyDefault =
        GetPrivateProfileIntW(L"General", L"ShowOnlyDefault", 0, path.c_str()) != 0;

    const std::wstring language = ReadValue(path, L"Language", L"");
    if (language == L"zh")
        settings.language = 1;
    else if (language == L"en")
        settings.language = 0;
    else
        settings.language = -1;

    GUID guid{};
    const std::wstring onAc = ReadValue(path, L"OnAc", L"");
    if (GuidFromString(onAc, guid))
        settings.onAc = guid;

    const std::wstring onBattery = ReadValue(path, L"OnBattery", L"");
    if (GuidFromString(onBattery, guid))
        settings.onBattery = guid;

    const std::wstring leftClick = ReadValue(path, L"LeftClick", L"none");
    if (leftClick == L"settings")
        settings.leftClickAction = kLeftClickSettings;
    else if (leftClick == L"menu")
        settings.leftClickAction = kLeftClickMenu;
    else
        settings.leftClickAction = kLeftClickNone;
}

void SaveSettings(const Settings& settings)
{
    const std::wstring path = ConfigPath();

    WritePrivateProfileStringW(L"General", L"AutoSwitch",
                               settings.autoSwitch ? L"1" : L"0", path.c_str());
    WritePrivateProfileStringW(L"General", L"ShowOnlyDefault",
                               settings.showOnlyDefault ? L"1" : L"0", path.c_str());
    WritePrivateProfileStringW(L"General", L"Language",
                               settings.language == 1 ? L"zh" : L"en", path.c_str());

    const std::wstring onAc = GuidToString(settings.onAc);
    WritePrivateProfileStringW(L"General", L"OnAc", onAc.c_str(), path.c_str());

    const std::wstring onBattery = GuidToString(settings.onBattery);
    WritePrivateProfileStringW(L"General", L"OnBattery", onBattery.c_str(), path.c_str());

    const wchar_t* leftClick = L"none";
    if (settings.leftClickAction == kLeftClickSettings)
        leftClick = L"settings";
    else if (settings.leftClickAction == kLeftClickMenu)
        leftClick = L"menu";
    WritePrivateProfileStringW(L"General", L"LeftClick", leftClick, path.c_str());
}
