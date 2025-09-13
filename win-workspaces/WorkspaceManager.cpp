#include "WorkspaceManager.h"

WorkspaceManager* WorkspaceManager::instance = nullptr;

// TODO: MULTI MONITOR WINDOW MOVING TO OTHER WORKSPACES!


static bool IsRealWindow(HWND hwnd) {
    // Skip Invisible Windows
    if (!IsWindowVisible(hwnd)) return false;

    // Skip tool/utility windows
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) return false;

    return true;

}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<HWND>*>(lParam);
    if (IsRealWindow(hwnd)) {
        windows->push_back(hwnd);
    }
    return TRUE;
}

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
    LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfo(hMonitor, &mi)) {
        std::cout << "Monitor: "
            << "Left=" << mi.rcMonitor.bottom
            << " Top=" << mi.rcMonitor.top
            << " Right=" << mi.rcMonitor.right
            << " Bottom=" << mi.rcMonitor.bottom
            << std::endl;
    }
    return TRUE;
}

WorkspaceManager::WorkspaceManager() {

    instance = this;
    hwEventHook = SetWinEventHook(
        EVENT_OBJECT_CREATE,
        EVENT_OBJECT_LOCATIONCHANGE,
        NULL,
        WinEventProc,
        0, // all processes
        0, /// all threads
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
    );

    EnumDisplayMonitors(NULL, NULL, WorkspaceManager::MonitorEnumCallback, reinterpret_cast<LPARAM>(this));

    // Enumerate all windows
    std::vector<HWND> allWindows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&allWindows));
    // Assign each window to its monitor's first workspace.
    for (HWND hwnd : allWindows) {
        HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        int monIndex = GetMonitorIndex(mon);
        if (monIndex == -1) continue;
        Workspaces[monIndex][0].AddToWorkspace(hwnd);
    }
    // Mark first workspace to monitor as selected
    for (size_t i = 0; i < Workspaces.size(); i++) {
        Workspaces[i][0].isSelected = true;
        Workspaces[i][0].ShowWorkspace();
        currentWorkspace.push_back(0);
    }
}
WorkspaceManager::~WorkspaceManager() {
    if (hwEventHook)
        UnhookWinEvent(hwEventHook);
    instance = nullptr;

    // Cleanup and show all windows from other workspaces, so they dont get lost.
    // Edit: this seems to never even get ran anyways, but i'll keep it here incase one day it does.
    for (auto& mon : Workspaces) {
        for (auto& ws : mon) {
            for (auto& window : ws.windows) {
                ShowWindow(window, SW_SHOW);
            }
        }
    }

}

void WorkspaceManager::InitHotkeys() {
    for (int i = 1; i <= 9; i++) {
        if (!RegisterHotKey(NULL, i, MOD_ALT, '0' + i)) {
            std::cerr << "Failed to register Alt+" << i << std::endl;
        }
    }
    
    for (int i = 1; i <= 9; i++) {
        if (!RegisterHotKey(NULL, 100 + i, MOD_ALT | MOD_SHIFT, '0' + i)) {
            std::cerr << "Failed to register Alt+Shift+" << i << std::endl;
        }
    }
}

void WorkspaceManager::Run() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            int id = (int)msg.wParam;
            HandleHotkey(id);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WorkspaceManager::HandleHotkey(int id) {
    focusedWindow = GetForegroundWindow();
    HMONITOR cursorMon = GetCursorMonitor();
    int monIndex = GetMonitorIndex(cursorMon);
    if (monIndex == -1) return;

    auto& workspaces = Workspaces[monIndex];

    if (id >= 1 && id <= 9) {
        // ALT+number -> switch workspace
        int wsIndex = id - 1;
        std::cout << "ALT + " << id << " pressed\n";
        for (auto& ws : workspaces) {
            ws.isSelected = false;
            ws.HideWorkspace();
        }
        workspaces[wsIndex].isSelected = true;
        workspaces[wsIndex].ShowWorkspace();
        currentWorkspace[monIndex] = wsIndex;

        // If theres only one window on workspace, set that to foreground window
        if (workspaces[wsIndex].windows.size() == 1) {
            SetForegroundWindow(workspaces[wsIndex].windows[0]);
        }
        else { // Otherwise, just set the window the cursor is on to foreground.
            // Focus Window under cursor
            POINT pt;
            GetCursorPos(&pt);
            HWND hwnd = WindowFromPoint(pt);
            if (hwnd) {
                HWND top = GetAncestor(hwnd, GA_ROOT);
                SetForegroundWindow(top);
            }
        }
    }
    else if (id >= 101 && id <= 109) {
        // ALT+SHIFT+number -> move window
        int wsIndex = (id - 100) - 1;
        if (focusedWindow) {
            int curWs = currentWorkspace[monIndex];
            workspaces[wsIndex].MoveToWorkspace(workspaces[curWs], focusedWindow);
            workspaces[wsIndex].Update();
            DumpAllWorkspaces();
            POINT pt;
            GetCursorPos(&pt);
            HWND hwnd = WindowFromPoint(pt);
            if (hwnd) {
                HWND top = GetAncestor(hwnd, GA_ROOT);
                SetForegroundWindow(top);
            }
        }
    }
}

void WorkspaceManager::OnWindowCreated(HWND hwnd) {
    if (!IsRealWindow(hwnd)) return;
    // Determine monitor of the new window
    HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    int monIndex = GetMonitorIndex(mon);
    if (monIndex == -1) return;
    // Get Currently active workspace on that monitor.
    int activeWs = currentWorkspace[monIndex];
    Workspaces[monIndex][activeWs].AddToWorkspace(hwnd);
    if (Workspaces[monIndex][activeWs].isSelected) {
        ShowWindow(hwnd, SW_SHOW);
    }
    else {
        ShowWindow(hwnd, SW_HIDE);
    }
    std::cout << "New window added to workspace " << (activeWs + 1) << " on monitor " << monIndex << "\n";
}

void CALLBACK WorkspaceManager::WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime
) {

    if (!instance) return;
    if (idObject != OBJID_WINDOW) return;
    if (!IsWindow(hwnd) || !IsWindowVisible(hwnd)) return;

    // Handle new window creation
    if (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) {
		if (instance->IsWindowTracked(hwnd)) return;
        instance->OnWindowCreated(hwnd);
		return;
    }

    // Handle window movement
    if (event == EVENT_OBJECT_LOCATIONCHANGE) {
        HMONITOR newMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        int newMonIndex = instance->GetMonitorIndex(newMon);
        if (newMonIndex == -1) return;

        int oldMonIndex = instance->GetWorkspaceMonitorIndex(hwnd);
        if (oldMonIndex == -1) return;

        if (oldMonIndex != newMonIndex) {
            int oldWsIndex = instance->currentWorkspace[oldMonIndex];
            int newWsIndex = instance->currentWorkspace[newMonIndex];

            auto& oldWs = instance->Workspaces[oldMonIndex][oldWsIndex];
            auto& newWs = instance->Workspaces[newMonIndex][newWsIndex];

            std::cout << "Window " << hwnd << " moved from monitor "
                << oldMonIndex << " to monitor " << newMonIndex << "\n";

            newWs.MoveToWorkspace(oldWs, hwnd);
        }
    }

    //if (idObject != OBJID_WINDOW || !IsWindowVisible(hwnd)) return;
    //if (!instance) return;
    //
    //if (instance->IsWindowTracked(hwnd)) return;
    //
    //instance->OnWindowCreated(hwnd);

}

int WorkspaceManager::GetWorkspaceMonitorIndex(HWND hwnd) {
    for (size_t monIdx = 0; monIdx < Workspaces.size(); monIdx++) {
        for (size_t wsIdx = 0; wsIdx < Workspaces[monIdx].size(); wsIdx++) {
            auto& ws = Workspaces[monIdx][wsIdx];
            if (std::find(ws.windows.begin(), ws.windows.end(), hwnd) != ws.windows.end()) {
                return static_cast<int>(monIdx);
            }
        }
    }
    return -1;
}

bool WorkspaceManager::IsWindowTracked(HWND hwnd) {
    for (auto& wsList : Workspaces)
        for (auto& ws : wsList)
            if (std::find(ws.windows.begin(), ws.windows.end(), hwnd) != ws.windows.end())
                return true;
    return false;
}

HMONITOR WorkspaceManager::GetCursorMonitor() {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    return MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
}

int WorkspaceManager::GetMonitorIndex(HMONITOR hMonitor) {
    for (size_t i = 0; i < monitors.size(); i++)
        if (monitors[i] == hMonitor) return static_cast<int>(i);
    return -1; // Not Found
}

BOOL CALLBACK WorkspaceManager::MonitorEnumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) {
    WorkspaceManager* mgr = reinterpret_cast<WorkspaceManager*>(dwData);
    int monIndex = static_cast<int>(mgr->monitors.size());
    mgr->monitors.push_back(hMonitor);
    std::vector<Workspace> wsVec;
    for (int i = 0; i < 9; i++) {
        Workspace ws;
        ws.id = i + 1;
        ws.monitorId = monIndex;
        wsVec.push_back(ws);
    }
    mgr->Workspaces.push_back(wsVec);
    return TRUE;
}

void WorkspaceManager::DumpAllWorkspaces() {
    std::cout << "=== Workspace Dump (non-empty only) ===\n";

    // iterate monitors
    for (size_t mon = 0; mon < Workspaces.size(); ++mon) {
        const auto& wsVec = Workspaces[mon];
        // skip monitors that have no workspaces (shouldn't normally happen)
        if (wsVec.empty()) continue;

        // decide whether this monitor has any non-empty workspace; skip if none
        bool monitorHasWindows = false;
        for (const auto& ws : wsVec) {
            if (!ws.windows.empty()) { monitorHasWindows = true; break; }
        }
        if (!monitorHasWindows) continue;

        // print monitor header
        std::cout << "Monitor " << mon;
        if (mon < monitors.size()) {
            std::cout << " (hmon=" << monitors[mon] << ")";
        }
        // print current workspace index for this monitor (if available)
        if (mon < currentWorkspace.size()) {
            std::cout << " currentWs=" << currentWorkspace[mon];
        }
        std::cout << '\n';

        // print each non-empty workspace for this monitor
        for (size_t w = 0; w < wsVec.size(); ++w) {
            const Workspace& ws = wsVec[w];
            if (ws.windows.empty()) continue; // only show workspaces that actually have windows

            std::cout << "  WS[" << w << "] id=" << ws.id
                << (ws.isSelected ? " [ACTIVE]" : "")
                << " count=" << ws.windows.size() << '\n';

            for (HWND hwnd : ws.windows) {
                char title[256] = { 0 };
                // try to get window title; if empty that's okay
                GetWindowTextA(hwnd, title, sizeof(title));
                std::cout << "    HWND: " << hwnd << " Title: \"" << title << "\"\n";
            }
        }
    }

    std::cout << "=== End Dump ===\n";
}