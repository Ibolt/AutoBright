#pragma once
#include <array>

#include <lowlevelmonitorconfigurationapi.h>
#include <opencv2/core.hpp>
#include <windows.h>

#include "displaycontroller.h"

class DisplayController {
    DWORD dwPhysMonitors;
    LPPHYSICAL_MONITOR pPhysMonitors;
    HANDLE hPhysMonitor;
    DWORD dwMinBrightness;
    DWORD dwMaxBrightness;
    DWORD dwCurrBrightness;

public:
    DisplayController();
    ~DisplayController();
    DWORD getCurrBrightness();
    void setCurrBrightness(DWORD val);
    void captureScreen();
    const std::array<uchar, 3> findMainColour();
};