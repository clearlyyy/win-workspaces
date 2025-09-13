#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <vector>
#include <algorithm>
#include <iostream>

struct Workspace {
    size_t id;
    int monitorId;
    std::vector<HWND> windows;
    bool isSelected = false;
    HWND selectedWindow;

    void Update() {
        if (!isSelected && windows.size() > 0) {
            for (size_t i = 0; i < windows.size(); i++) {
                HideWindowInstant(windows[i]);
            }
            return;
        }
    }


    bool RemoveFromWorkspace(HWND window) {
        auto newEnd = std::remove(windows.begin(), windows.end(), window);
        if (newEnd != windows.end()) {
            std::cout << "Found and Removed HWND: " << window << std::endl;
            windows.erase(newEnd, windows.end());
            return true;
        }
        std::cout << "Couldn't remove from workspace, no matching HWND found: " << window << std::endl;
        return false;
    }

    void AddToWorkspace(HWND window) {
        windows.push_back(window);
    }

    void MoveToWorkspace(Workspace& old, HWND window) {

        if (&old == this) {
            return;
        }

        if (!old.RemoveFromWorkspace(window)) {
            std::cout << "Failed to move HWND to workspace: " << window << std::endl;
            return;
        }

        // Always hide the window when moving it, regardless of monitor
        HideWindowInstant(window);

        this->AddToWorkspace(window);
        if (this->isSelected) {
            ShowWindowInstant(window);
        }
    }

    void HideWorkspace() {
        for (size_t i = 0; i < windows.size(); i++) {
            HideWindowInstant(windows[i]);
        }
    }

    void ShowWorkspace() {
        for (size_t i = 0; i < windows.size(); i++) {
            ShowWindowInstant(windows[i]);
        }
    }

private:

    // Hide and show windows, Disable animations without the fucked fade in/out, this required me to use visual studio and i hate it.
    static void HideWindowInstant(HWND hwnd) {
        BOOL disable = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disable, sizeof(disable));
        ShowWindow(hwnd, SW_HIDE);
    }

    static void ShowWindowInstant(HWND hwnd) {
        BOOL disable = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disable, sizeof(disable));
        ShowWindow(hwnd, SW_SHOWNA);
    }
};