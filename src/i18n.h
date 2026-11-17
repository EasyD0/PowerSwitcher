#pragma once

enum class Str : int {
    AppName = 0,
    OpenSettings,
    Exit,
    SettingsTitle,
    AutoSwitch,
    OnAc,
    OnBattery,
    ShowOnlyDefault,
    LeftClick,
    LeftClickNone,
    LeftClickMenu,
    Language,
    Ok,
    Cancel,
    NoPlans,
    Count
};

// 0 = English, 1 = Chinese.
int DetectSystemLanguage();

const wchar_t* T(Str string);
