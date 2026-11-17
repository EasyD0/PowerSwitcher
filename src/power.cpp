#include "power.h"

#include <objbase.h>
#include <powrprof.h>

#include <cwchar>
#include <utility>

#ifdef _MSC_VER
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace {

std::wstring ReadFriendlyName(const GUID& guid)
{
    DWORD size = 0;
    DWORD result = PowerReadFriendlyName(nullptr, &guid, nullptr, nullptr, nullptr, &size);
    if (result != ERROR_SUCCESS || size == 0)
        return {};

    std::wstring name(size / sizeof(wchar_t) + 1, L'\0');
    result = PowerReadFriendlyName(nullptr, &guid, nullptr, nullptr,
                                   reinterpret_cast<PUCHAR>(&name[0]), &size);
    if (result != ERROR_SUCCESS)
        return {};

    name.resize(std::wcslen(name.c_str()));
    return name;
}

} // namespace

std::vector<PowerPlan> EnumeratePowerPlans()
{
    std::vector<PowerPlan> plans;

    for (DWORD index = 0; index < 256; ++index) {
        GUID guid{};
        DWORD size = sizeof(guid);
        const DWORD result = PowerEnumerate(nullptr, nullptr, nullptr, ACCESS_SCHEME, index,
                                            reinterpret_cast<UCHAR*>(&guid), &size);
        if (result == ERROR_NO_MORE_ITEMS || result != ERROR_SUCCESS)
            break;

        PowerPlan plan;
        plan.guid = guid;
        plan.name = ReadFriendlyName(guid);
        if (plan.name.empty())
            plan.name = GuidToString(guid);

        plans.push_back(std::move(plan));
    }

    return plans;
}

bool GetActivePowerPlan(GUID& guid)
{
    GUID* active = nullptr;
    const DWORD result = PowerGetActiveScheme(nullptr, &active);
    if (result != ERROR_SUCCESS || active == nullptr)
        return false;

    guid = *active;
    LocalFree(active);
    return true;
}

bool SetActivePowerPlan(const GUID& guid)
{
    return PowerSetActiveScheme(nullptr, &guid) == ERROR_SUCCESS;
}

bool IsAcConnected()
{
    SYSTEM_POWER_STATUS status{};
    if (!GetSystemPowerStatus(&status))
        return true;
    return status.ACLineStatus == 1;
}

bool IsDefaultPowerPlan(const GUID& guid)
{
    return IsEqualGUID(guid, kPlanHighPerformance) ||
           IsEqualGUID(guid, kPlanBalanced) ||
           IsEqualGUID(guid, kPlanPowerSaver);
}

std::wstring GuidToString(const GUID& guid)
{
    wchar_t buffer[64]{};
    if (StringFromGUID2(guid, buffer, 64) <= 0)
        return {};
    return buffer;
}

bool GuidFromString(const std::wstring& text, GUID& guid)
{
    if (text.empty())
        return false;
    return SUCCEEDED(CLSIDFromString(text.c_str(), &guid));
}
