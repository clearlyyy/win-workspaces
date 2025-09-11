#include "WorkspaceManager.h"

WorkspaceManager* WorkspaceManager::instance = nullptr;

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
        EVENT_OBJECT_SHOW,
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
}
void WorkspaceManager::Update() {
    focusedWindow = GetForegroundWindow();
    bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000);
    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
    if (!alt) return;
    HMONITOR cursorMon = GetCursorMonitor();
    int monIndex = GetMonitorIndex(cursorMon);
    if (monIndex == -1) return;
    auto& workspaces = Workspaces[monIndex];
    for (int i = 0; i <= 9; i++) {
        if (GetAsyncKeyState('0' + i) & 0x8000) {
            int wsIndex = i - 1;
            if (!shift) {
                std::cout << "ALT + " << i << " pressed\n";
                for (auto& ws : workspaces)
                    ws.HideWorkspace();
                workspaces[wsIndex].isSelected = true;
                workspaces[wsIndex].ShowWorkspace();
                currentWorkspace[monIndex] = wsIndex;
            }
            else {
                if (focusedWindow) {
                    int curWs = currentWorkspace[monIndex];
                    workspaces[wsIndex].MoveToWorkspace(workspaces[curWs], focusedWindow);
                    workspaces[wsIndex].Update();
                }
            }
            Sleep(200);
            break;
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
    if (idObject != OBJID_WINDOW || !IsWindowVisible(hwnd)) return;
    if (!instance) return;

    if (instance->IsWindowTracked(hwnd)) return;

    instance->OnWindowCreated(hwnd);

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
    mgr->monitors.push_back(hMonitor);
    std::vector<Workspace> wsVec;
    for (int i = 0; i < 9; i++) {
        Workspace ws;
        ws.id = i + 1;
        wsVec.push_back(ws);
    }
    mgr->Workspaces.push_back(wsVec);
    return TRUE;
}