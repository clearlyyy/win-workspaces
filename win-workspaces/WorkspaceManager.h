#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include "Workspace.hpp"


class WorkspaceManager {
private:
    std::vector<std::vector<Workspace>> Workspaces;
    HWND focusedWindow;
    std::vector<size_t> currentWorkspace;
    std::vector<HMONITOR> monitors;
    HWINEVENTHOOK hwEventHook;

public:
    WorkspaceManager();
    ~WorkspaceManager();

    void Update();
    void OnWindowCreated(HWND hwnd);

    void DumpWorkspaces() const;

private:
    HMONITOR GetCursorMonitor();
    int GetMonitorIndex(HMONITOR hMonitor);
    bool IsWindowTracked(HWND hwnd);
    int GetWorkspaceMonitorIndex(HWND hwnd);
    void DumpAllWorkspaces();

    // Static callback for WinAPI
    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD dwEventThread,
        DWORD dwmsEventTime
    );

    static BOOL CALLBACK MonitorEnumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData);
    static WorkspaceManager* instance;

private:
    bool keyDown[10] = { false };
};