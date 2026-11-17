#pragma once

#include <windows.h>

#include <string>
#include <vector>

struct PowerPlan {
    GUID guid{};
    std::wstring name;
};

// Well-known Windows power scheme GUIDs.
inline const GUID kPlanHighPerformance = {
    0x8c5e7fda, 0xe8bf, 0x4a96, {0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c}};
inline const GUID kPlanBalanced = {
    0x381b4222, 0xf694, 0x41f0, {0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e}};
inline const GUID kPlanPowerSaver = {
    0xa1841308, 0x3541, 0x4fab, {0xbc, 0x81, 0xf7, 0x15, 0x56, 0xf2, 0x0b, 0x4a}};

std::vector<PowerPlan> EnumeratePowerPlans();
bool GetActivePowerPlan(GUID& guid);
bool SetActivePowerPlan(const GUID& guid);
bool IsAcConnected();
bool IsDefaultPowerPlan(const GUID& guid);

std::wstring GuidToString(const GUID& guid);
bool GuidFromString(const std::wstring& text, GUID& guid);
