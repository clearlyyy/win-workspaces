#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>

struct Workspace {
    size_t id;
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

    void MoveToWorkspace(Workspace &old, HWND window) {
        if (!old.RemoveFromWorkspace(window)) {
            std::cout << "Failed to move HWND to workspace: " << window << std::endl;
            return;
        }
        HideWindowInstant(window);
        this->AddToWorkspace(window);
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
        static void HideWindowInstant(HWND hwnd) {
            BOOL disable = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disable, sizeof(disable));
            ShowWindow(hwnd, SW_HIDE);
            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
        }
        
        static void ShowWindowInstant(HWND hwnd) {
            BOOL disable = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disable, sizeof(disable));
            ShowWindow(hwnd, SW_SHOWNA);
            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
};