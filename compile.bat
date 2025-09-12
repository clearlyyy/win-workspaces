-- This is useless

@echo off
REM 
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM 
cd /d C:\Users\clearly\Desktop\Code\win-workspaces

REM 
cl /EHsc /std:c++17 src\main.cpp src\WorkspaceManager.cpp /link dwmapi.lib user32.lib

pause

main.exe
