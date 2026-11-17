#include "app.h"
#include "config.h"
#include "i18n.h"
#include "power.h"
#include "resource.h"
#include "ui.h"

#include <commctrl.h>
#include <shellapi.h>

#include <windows.h>

#define WM_TRAYICON (WM_APP + 1)
#define WM_OPEN_SETTINGS (WM_APP + 2)

#define ID_TRAY_SETTINGS 9001
#define ID_TRAY_EXIT 9002
#define ID_TRAY_PLAN_BASE 10000

namespace {

const wchar_t* kTrayClass = L"PowerSwitcherTrayWindow";

NOTIFYICONDATAW g_nid{};
UINT g_taskbarCreatedMessage = 0;

void AddTrayIcon(HWND hwnd)
{
    g_nid = {};
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = g_app.icon ? g_app.icon : LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpynW(g_nid.szTip, T(Str::AppName), ARRAYSIZE(g_nid.szTip));

    Shell_NotifyIconW(NIM_ADD, &g_nid);
    g_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &g_nid);
}

void ShowTrayMenu(HWND hwnd)
{
    LoadPlans();

    HMENU menu = CreatePopupMenu();
    if (!menu)
        return;

    int shown = 0;
    for (size_t i = 0; i < g_app.plans.size(); ++i) {
        const PowerPlan& plan = g_app.plans[i];
        const bool isActive =
            g_app.hasActiveGuid && IsEqualGUID(plan.guid, g_app.activeGuid);

        if (g_app.settings.showOnlyDefault && !IsDefaultPowerPlan(plan.guid) && !isActive)
            continue;

        UINT flags = MF_STRING;
        if (isActive)
            flags |= MF_CHECKED;

        AppendMenuW(menu, flags, ID_TRAY_PLAN_BASE + static_cast<UINT>(i),
                    plan.name.c_str());
        ++shown;
    }

    if (shown == 0)
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, T(Str::NoPlans));

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_SETTINGS, T(Str::OpenSettings));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, T(Str::Exit));

    SetForegroundWindow(hwnd);

    POINT point{};
    GetCursorPos(&point);
    const UINT command = static_cast<UINT>(TrackPopupMenu(
        menu, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0,
        hwnd, nullptr));
    DestroyMenu(menu);
    PostMessageW(hwnd, WM_NULL, 0, 0);

    if (command >= ID_TRAY_PLAN_BASE &&
        command < ID_TRAY_PLAN_BASE + static_cast<UINT>(g_app.plans.size())) {
        ApplyPlan(g_app.plans[command - ID_TRAY_PLAN_BASE].guid);
    } else if (command == ID_TRAY_SETTINGS) {
        OpenSettingsWindow();
    } else if (command == ID_TRAY_EXIT) {
        DestroyWindow(hwnd);
    }
}

void HandleLeftClick(HWND hwnd)
{
    switch (g_app.settings.leftClickAction) {
    case kLeftClickSettings:
        OpenSettingsWindow();
        break;
    case kLeftClickMenu:
        ShowTrayMenu(hwnd);
        break;
    default:
        // kLeftClickNone: deliberately do nothing.
        break;
    }
}

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == g_taskbarCreatedMessage) {
        AddTrayIcon(hwnd);
        return 0;
    }

    switch (message) {
    case WM_TRAYICON: {
        const UINT event = LOWORD(lParam);
        if (event == WM_CONTEXTMENU) {
            ShowTrayMenu(hwnd);
        } else if (event == WM_LBUTTONUP || event == WM_LBUTTONDBLCLK ||
                   event == NIN_SELECT || event == NIN_KEYSELECT) {
            HandleLeftClick(hwnd);
        }
        return 0;
    }

    case WM_OPEN_SETTINGS:
        OpenSettingsWindow();
        return 0;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMPOWERSTATUSCHANGE)
            OnPowerStatusChanged();
        return TRUE;

    case WM_DESTROY:
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    g_app.instance = instance;

    LoadSettings(g_app.settings);
    if (g_app.settings.language != 0 && g_app.settings.language != 1) {
        g_app.settings.language = DetectSystemLanguage();
        SaveSettings(g_app.settings);
    }

    g_app.font = CreateUiFont();
    g_app.icon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APPICON));
    if (!g_app.icon)
        g_app.icon = LoadIconW(nullptr, IDI_APPLICATION);

    LoadPlans();

    INITCOMMONCONTROLSEX commonControls{};
    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&commonControls);

    g_taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");
    RegisterUiClasses(instance);

    // Single instance: open the settings window of the running copy instead of
    // starting twice.
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\PowerSwitcherNativeMutex");
    if (mutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(kTrayClass, nullptr);
        if (existing)
            PostMessageW(existing, WM_OPEN_SETTINGS, 0, 0);
        CloseHandle(mutex);
        return 0;
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = instance;
    wc.hIcon = g_app.icon;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kTrayClass;
    RegisterClassExW(&wc);

    HWND trayWindow = CreateWindowExW(WS_EX_TOOLWINDOW, kTrayClass, L"PowerSwitcher", 0,
                                      0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    if (!trayWindow)
        return 1;

    g_app.trayWnd = trayWindow;
    AddTrayIcon(trayWindow);

    g_app.lastAcState = IsAcConnected() ? 1 : 0;
    if (g_app.settings.autoSwitch)
        ApplyAutoSwitch();

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    Shell_NotifyIconW(NIM_DELETE, &g_nid);
    if (mutex) {
        ReleaseMutex(mutex);
        CloseHandle(mutex);
    }
    return static_cast<int>(message.wParam);
}
