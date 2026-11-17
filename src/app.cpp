#include "app.h"

AppState g_app;

HFONT CreateUiFont()
{
    HDC dc = GetDC(nullptr);
    const int height = -MulDiv(9, GetDeviceCaps(dc, LOGPIXELSY), 72);
    ReleaseDC(nullptr, dc);

    const wchar_t* face =
        (g_app.settings.language == 1) ? L"Microsoft YaHei UI" : L"Segoe UI";

    return CreateFontW(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       DEFAULT_PITCH | FF_DONTCARE, face);
}

void LoadPlans()
{
    g_app.plans = EnumeratePowerPlans();
    g_app.hasActiveGuid = GetActivePowerPlan(g_app.activeGuid);
}

void ApplyPlan(const GUID& guid)
{
    if (!SetActivePowerPlan(guid))
        return;

    g_app.activeGuid = guid;
    g_app.hasActiveGuid = true;
}

void ApplyAutoSwitch()
{
    const GUID target = IsAcConnected() ? g_app.settings.onAc : g_app.settings.onBattery;
    ApplyPlan(target);
}

void OnPowerStatusChanged()
{
    const int state = IsAcConnected() ? 1 : 0;
    if (state == g_app.lastAcState)
        return;

    g_app.lastAcState = state;

    if (g_app.settings.autoSwitch)
        ApplyAutoSwitch();
}
