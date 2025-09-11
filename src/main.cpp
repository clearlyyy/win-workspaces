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
#include "WorkspaceManager.h"


int main()
{

    WorkspaceManager workSpaceManager;

    MSG msg;

    //D22222222222222222
    while (true) {
        // Run workspace Update
        workSpaceManager.Update();

        // Pump Windows messages for WinEvent hook
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Sleep to reduce CPU usage
        Sleep(10);
    }

    return 0;
}