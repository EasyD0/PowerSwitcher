#pragma once

#include "config.h"
#include "power.h"

#include <windows.h>

#include <vector>

struct AppState {
    HINSTANCE instance = nullptr;
    HWND trayWnd = nullptr;
    HWND settingsWnd = nullptr;
    HICON icon = nullptr;
    HFONT font = nullptr;

    Settings settings;
    std::vector<PowerPlan> plans;
    GUID activeGuid{};
    bool hasActiveGuid = false;
    int lastAcState = -1;
};

extern AppState g_app;

HFONT CreateUiFont();
void LoadPlans();
void ApplyPlan(const GUID& guid);
void ApplyAutoSwitch();
void OnPowerStatusChanged();
