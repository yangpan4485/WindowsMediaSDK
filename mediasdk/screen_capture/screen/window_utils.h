#pragma once
#include <Windows.h>

#include <vector>
#include <string>
#include <cstdint>

#include "screen_common.h"

void GetChildWindows(HWND windowID, std::vector<HWND>& windows);
void EnumMonitor(std::vector<HMONITOR>& monitor_list);
BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM param);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
BOOL CALLBACK EnumMonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
bool IsWindowValidAndVisible(HWND window);
bool IsWindowOnCurrentDesktop(HWND hwnd);
bool IsWindowCloaked(HWND hwnd);
bool IsWindowsSysmtemWnd(HWND hwnd);
bool IsWindowResponding(HWND window);
bool IsWindowVisibleOnCurrentDesktop(HWND hwnd);
bool AeroEnabled();
bool IsRunOnIntelGPU(HRESULT& hr);
bool IsEqualRect(const WindowRect& rect1, const WindowRect& rect2);
WindowRect GetScreenRect(int screen_id);
std::vector<HWND> GetOverWindow(HWND target_hwnd);