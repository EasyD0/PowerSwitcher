#include "ui.h"

#include "app.h"
#include "config.h"
#include "i18n.h"
#include "power.h"

namespace {

const wchar_t* kSettingsClass = L"PowerSwitcherSettingsWindow";

enum ControlId : int {
    IDC_CHK_AUTO = 201,
    IDC_LBL_AC,
    IDC_CMB_AC,
    IDC_LBL_BATTERY,
    IDC_CMB_BATTERY,
    IDC_CHK_DEFAULT,
    IDC_LBL_LEFT_CLICK,
    IDC_CMB_LEFT_CLICK,
    IDC_LBL_LANG,
    IDC_CMB_LANG,
    IDC_BTN_OK,
    IDC_BTN_CANCEL
};

HWND g_chkAuto = nullptr;
HWND g_lblAc = nullptr;
HWND g_cmbAc = nullptr;
HWND g_lblBattery = nullptr;
HWND g_cmbBattery = nullptr;
HWND g_chkDefault = nullptr;
HWND g_lblLeftClick = nullptr;
HWND g_cmbLeftClick = nullptr;
HWND g_lblLang = nullptr;
HWND g_cmbLang = nullptr;
HWND g_btnOk = nullptr;
HWND g_btnCancel = nullptr;

void SetControlFont(HWND control)
{
    if (control && g_app.font)
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(g_app.font), TRUE);
}

HWND CreateControl(const wchar_t* className, const wchar_t* text, DWORD style,
                   DWORD exStyle, int id, HWND parent)
{
    HWND control = CreateWindowExW(
        exStyle, className, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_app.instance, nullptr);
    SetControlFont(control);
    return control;
}

void LayoutSettings(HWND hwnd)
{
    RECT client{};
    GetClientRect(hwnd, &client);

    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    const int margin = 16;
    const int labelWidth = 150;
    const int fieldX = 176;

    MoveWindow(g_chkAuto, margin, 16, width - margin * 2, 24, TRUE);
    MoveWindow(g_lblAc, margin, 56, labelWidth, 24, TRUE);
    MoveWindow(g_cmbAc, fieldX, 52, width - fieldX - margin, 200, TRUE);
    MoveWindow(g_lblBattery, margin, 92, labelWidth, 24, TRUE);
    MoveWindow(g_cmbBattery, fieldX, 88, width - fieldX - margin, 200, TRUE);
    MoveWindow(g_chkDefault, margin, 128, width - margin * 2, 24, TRUE);
    MoveWindow(g_lblLeftClick, margin, 168, labelWidth, 24, TRUE);
    MoveWindow(g_cmbLeftClick, fieldX, 164, 240, 200, TRUE);
    MoveWindow(g_lblLang, margin, 204, labelWidth, 24, TRUE);
    MoveWindow(g_cmbLang, fieldX, 200, 180, 200, TRUE);

    const int buttonWidth = 92;
    const int buttonHeight = 30;
    const int buttonY = height - margin - buttonHeight;
    MoveWindow(g_btnCancel, width - margin - buttonWidth, buttonY, buttonWidth,
               buttonHeight, TRUE);
    MoveWindow(g_btnOk, width - margin - buttonWidth * 2 - 12, buttonY, buttonWidth,
               buttonHeight, TRUE);
}

void FillPlanCombo(HWND combo, const GUID& selected)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);

    int selectedIndex = -1;
    for (size_t i = 0; i < g_app.plans.size(); ++i) {
        const PowerPlan& plan = g_app.plans[i];
        const LRESULT index =
            SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(plan.name.c_str()));
        if (index == CB_ERR)
            continue;
        SendMessageW(combo, CB_SETITEMDATA, index, static_cast<LPARAM>(i));
        if (IsEqualGUID(plan.guid, selected))
            selectedIndex = static_cast<int>(index);
    }

    if (selectedIndex < 0 && !g_app.plans.empty())
        selectedIndex = 0;
    if (selectedIndex >= 0)
        SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(selectedIndex), 0);
}

GUID ComboPlanGuid(HWND combo, const GUID& fallback)
{
    const LRESULT index = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR)
        return fallback;

    const LRESULT data = SendMessageW(combo, CB_GETITEMDATA, index, 0);
    if (data == CB_ERR || data < 0 || static_cast<size_t>(data) >= g_app.plans.size())
        return fallback;

    return g_app.plans[static_cast<size_t>(data)].guid;
}

void CreateSettingsControls(HWND hwnd)
{
    g_chkAuto = CreateControl(L"BUTTON", T(Str::AutoSwitch), BS_AUTOCHECKBOX | WS_TABSTOP,
                              0, IDC_CHK_AUTO, hwnd);
    g_lblAc = CreateControl(L"STATIC", T(Str::OnAc), SS_LEFT | SS_CENTERIMAGE, 0,
                            IDC_LBL_AC, hwnd);
    g_cmbAc = CreateControl(L"COMBOBOX", L"",
                            CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0, IDC_CMB_AC,
                            hwnd);
    g_lblBattery = CreateControl(L"STATIC", T(Str::OnBattery), SS_LEFT | SS_CENTERIMAGE,
                                 0, IDC_LBL_BATTERY, hwnd);
    g_cmbBattery = CreateControl(L"COMBOBOX", L"",
                                 CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0,
                                 IDC_CMB_BATTERY, hwnd);
    g_chkDefault = CreateControl(L"BUTTON", T(Str::ShowOnlyDefault),
                                 BS_AUTOCHECKBOX | WS_TABSTOP, 0, IDC_CHK_DEFAULT, hwnd);
    g_lblLeftClick = CreateControl(L"STATIC", T(Str::LeftClick),
                                   SS_LEFT | SS_CENTERIMAGE, 0, IDC_LBL_LEFT_CLICK, hwnd);
    g_cmbLeftClick = CreateControl(L"COMBOBOX", L"",
                                   CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0,
                                   IDC_CMB_LEFT_CLICK, hwnd);
    g_lblLang = CreateControl(L"STATIC", T(Str::Language), SS_LEFT | SS_CENTERIMAGE, 0,
                              IDC_LBL_LANG, hwnd);
    g_cmbLang = CreateControl(L"COMBOBOX", L"",
                              CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0, IDC_CMB_LANG,
                              hwnd);
    g_btnOk = CreateControl(L"BUTTON", T(Str::Ok), BS_PUSHBUTTON | WS_TABSTOP, 0,
                            IDC_BTN_OK, hwnd);
    g_btnCancel = CreateControl(L"BUTTON", T(Str::Cancel), BS_PUSHBUTTON | WS_TABSTOP, 0,
                                IDC_BTN_CANCEL, hwnd);

    SendMessageW(g_chkAuto, BM_SETCHECK,
                 g_app.settings.autoSwitch ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(g_chkDefault, BM_SETCHECK,
                 g_app.settings.showOnlyDefault ? BST_CHECKED : BST_UNCHECKED, 0);

    FillPlanCombo(g_cmbAc, g_app.settings.onAc);
    FillPlanCombo(g_cmbBattery, g_app.settings.onBattery);

    // Order must match LeftClickAction in config.h.
    SendMessageW(g_cmbLeftClick, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(T(Str::LeftClickNone)));
    SendMessageW(g_cmbLeftClick, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(T(Str::OpenSettings)));
    SendMessageW(g_cmbLeftClick, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(T(Str::LeftClickMenu)));
    int leftClick = g_app.settings.leftClickAction;
    if (leftClick < 0 || leftClick > 2)
        leftClick = 0;
    SendMessageW(g_cmbLeftClick, CB_SETCURSEL, static_cast<WPARAM>(leftClick), 0);

    SendMessageW(g_cmbLang, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
    SendMessageW(g_cmbLang, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"简体中文"));
    SendMessageW(g_cmbLang, CB_SETCURSEL, g_app.settings.language == 1 ? 1 : 0, 0);

    LayoutSettings(hwnd);
}

void SaveSettingsFromControls()
{
    g_app.settings.autoSwitch =
        SendMessageW(g_chkAuto, BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_app.settings.showOnlyDefault =
        SendMessageW(g_chkDefault, BM_GETCHECK, 0, 0) == BST_CHECKED;
    g_app.settings.onAc = ComboPlanGuid(g_cmbAc, g_app.settings.onAc);
    g_app.settings.onBattery = ComboPlanGuid(g_cmbBattery, g_app.settings.onBattery);

    const LRESULT leftClick = SendMessageW(g_cmbLeftClick, CB_GETCURSEL, 0, 0);
    g_app.settings.leftClickAction = (leftClick < 0 || leftClick > 2)
                                          ? 0
                                          : static_cast<int>(leftClick);

    const LRESULT language = SendMessageW(g_cmbLang, CB_GETCURSEL, 0, 0);
    g_app.settings.language = (language == 1) ? 1 : 0;

    SaveSettings(g_app.settings);

    if (g_app.settings.autoSwitch)
        ApplyAutoSwitch();

    // Keep the window open; the user closes it with the title bar button.
}

LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        CreateSettingsControls(hwnd);
        return 0;

    case WM_SIZE:
        LayoutSettings(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_OK:
            SaveSettingsFromControls();
            return 0;
        case IDC_BTN_CANCEL:
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        g_app.settingsWnd = nullptr;
        g_chkAuto = nullptr;
        g_lblAc = nullptr;
        g_cmbAc = nullptr;
        g_lblBattery = nullptr;
        g_cmbBattery = nullptr;
        g_chkDefault = nullptr;
        g_lblLeftClick = nullptr;
        g_cmbLeftClick = nullptr;
        g_lblLang = nullptr;
        g_cmbLang = nullptr;
        g_btnOk = nullptr;
        g_btnCancel = nullptr;
        return 0;

    case WM_CTLCOLORSTATIC:
        SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
        return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

} // namespace

void RegisterUiClasses(HINSTANCE instance)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SettingsWndProc;
    wc.hInstance = instance;
    wc.hIcon = g_app.icon;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kSettingsClass;
    RegisterClassExW(&wc);
}

void OpenSettingsWindow()
{
    if (g_app.settingsWnd) {
        ShowWindow(g_app.settingsWnd, SW_SHOW);
        SetForegroundWindow(g_app.settingsWnd);
        return;
    }

    LoadPlans();

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME, kSettingsClass, T(Str::SettingsTitle),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN, CW_USEDEFAULT,
        CW_USEDEFAULT, 540, 420, nullptr, nullptr, g_app.instance, nullptr);
    if (!hwnd)
        return;

    g_app.settingsWnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
}
