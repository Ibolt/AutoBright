#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "dwmapi.lib")

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strsafe.h>
#include <unordered_map>
#include <vector>

#include <atlimage.h>
#include <dwmapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <physicalmonitorenumerationapi.h>
#include <windows.h>

#include "controllerexception.h"
#include "displaycontroller.h"


const int COLOUR_BITS = 32;
// Integral portion of Golden Ratio
const int HASH_CONST = 0x9e3779b9;

// Custom hash function implementation for 3-element unsigned char Arrays using Boost hash_combine implementation
struct ArrayHash {
    std::size_t operator()(const std::array<uchar, 3>& arr) const {
        std::size_t hash = 0;
        for (auto ele : arr) {
            hash ^= std::hash<int>{}(ele)+HASH_CONST + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

template<typename F, typename... Args>
BOOL callWinApi(F func, const char* errMsg, Args... args) {
    BOOL bCmdRes = func(args...);
    if (!bCmdRes) {
        throw ControllerException(errMsg);
    }
    return TRUE;
}

DisplayController::DisplayController() {
    this->dwPhysMonitors = 0;
    HWND hWindow = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hWindow, MONITOR_DEFAULTTOPRIMARY);
    callWinApi(GetNumberOfPhysicalMonitorsFromHMONITOR, "Could not retrieve number of monitors.", hMonitor, &dwPhysMonitors);

    this->pPhysMonitors = new PHYSICAL_MONITOR[dwPhysMonitors];
    callWinApi(GetPhysicalMonitorsFromHMONITOR, "No physical monitors found", hMonitor, dwPhysMonitors, pPhysMonitors);
    this->hPhysMonitor = pPhysMonitors[0].hPhysicalMonitor;

    DWORD dwMonitorCapabilities = 0;
    DWORD dwMonitorColourTemps = 0;
    callWinApi(GetMonitorCapabilities, "Monitor does not support DDC/CI.", hPhysMonitor, &dwMonitorCapabilities, &dwMonitorColourTemps);

    this->dwMinBrightness = 0;
    this->dwMaxBrightness = 0;
    this->dwCurrBrightness = 0;
}

DisplayController::~DisplayController() {
    DestroyPhysicalMonitors(dwPhysMonitors, pPhysMonitors);
    // TODO: Check if any memory leaks relating to window handle, may need to make it a class member so it can be free'd here
    // DestroyWindow(hw)
    delete[] pPhysMonitors;
}

DWORD DisplayController::getCurrBrightness() {
    callWinApi(GetMonitorBrightness, "Error getting current monitor brightness.", hPhysMonitor, &dwMinBrightness, &dwCurrBrightness, &dwMaxBrightness);
    return this->dwCurrBrightness;
}

void DisplayController::setCurrBrightness(DWORD val) {
    callWinApi(SetMonitorBrightness, "Error setting monitor brightness.", this->hPhysMonitor, val);
    this->dwCurrBrightness = val;
}

const std::array<uchar, 3> mostFreqPixel(const std::vector<std::array<uchar, 3>> bits) {
    size_t bitsCount = bits.size();
    std::unordered_map<std::array<uchar, 3>, int, ArrayHash> hashTable;
    int currFreq = 1;
    std::array<uchar, 3> res = { 0, 0, 0 };


    for (int i = 0; i < bitsCount; ++i) {
        if (hashTable.find(bits[i]) != hashTable.end()) {
            hashTable[bits[i]]++;

            if (currFreq <= hashTable[bits[i]]) {
                currFreq = hashTable[bits[i]];
                res = bits[i];
            }
        }
        else {
            hashTable[bits[i]] = 1;
        }
    }
    return res;

}

const std::array<uchar, 3> DisplayController::findMainColour() {
    cv::Mat img = cv::imread("screen.jpg");
    std::vector<std::array<uchar, 3>> colours;
    uchar r, g, b;

    for (int i = 0; i < img.rows; i++) {
        cv::Vec3b* pixel = img.ptr<cv::Vec3b>(i);

        for (int j = 0; j < img.cols; j++) {
            r = pixel[j][2];
            g = pixel[j][1];
            b = pixel[j][0];
            std::array <uchar, 3> colour = { { r, g, b } };
            colours.push_back(colour);
        }
    }
    return mostFreqPixel(colours);
}

void DisplayController::captureScreen() {
    auto start = std::chrono::steady_clock::now();

    int x1, y1, x2, y2, width, height;
    x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    width = x2 - x1;
    height = y2 - y1;

    // Capture screen as bitmap
    HDC hScreen = GetDC(NULL);
    HDC hDeviceContext = CreateCompatibleDC(hScreen);
    HBITMAP hCapBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ hOldBitmap = SelectObject(hDeviceContext, hCapBitmap);
    BOOL bRet = BitBlt(hDeviceContext, 0, 0, width, height, hScreen, x1, y1, SRCCOPY);

    // Save bitmap to image
    CImage image;
    image.Attach(hCapBitmap);
    image.Save(L"screen.jpg");

    SelectObject(hDeviceContext, hOldBitmap);
    DeleteDC(hDeviceContext);
    ReleaseDC(NULL, hScreen);
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
}
