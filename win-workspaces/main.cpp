// Clearly - September 10th, 2025
// win-workspaces
// i3,dwm,hyprland like workspaces for windows
// ----------------------------------------------------
// ALT + 1,2,3...9 to switch workspaces
// ALT + SHIFT + 1,2,3...9 to move focused window between workspaces.


#include <windows.h>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "WorkspaceManager.h"


// This is to add the program to your startup programs.
// To remove from startup apps just follow the procedure below.
// Open Task Manager, go to Startup Apps, and just disable "win-workspaces".
bool AddToStartup(const std::wstring& appName)
{
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileName(NULL, exePath, MAX_PATH) == 0)
        return false;

    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS)
        return false;

    result = RegSetValueExW(hKey, appName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(exePath),
        (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);
    return (result == ERROR_SUCCESS);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    AddToStartup(L"win-workspaces");

    WorkspaceManager workSpaceManager;

    MSG msg;
    workSpaceManager.InitHotkeys();
    workSpaceManager.Run();

    while (true) {

        // Pump Windows messages for WinEvent hook
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Sleep(50);
    }
    return 0;
}
