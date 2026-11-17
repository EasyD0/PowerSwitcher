#pragma once

#include "power.h"

#include <string>

// What a left click on the tray icon does.
enum LeftClickAction : int {
    kLeftClickNone = 0,     // do nothing (default)
    kLeftClickSettings = 1, // open the settings window
    kLeftClickMenu = 2,     // show the same menu as a right click
};

struct Settings {
    bool autoSwitch = false;
    bool showOnlyDefault = false;
    int language = -1; // 0 = English, 1 = Chinese, -1 = auto
    int leftClickAction = kLeftClickNone;
    GUID onAc = kPlanHighPerformance;
    GUID onBattery = kPlanPowerSaver;
};

std::wstring GetConfigPath();
void LoadSettings(Settings& settings);
void SaveSettings(const Settings& settings);
