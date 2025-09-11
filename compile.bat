@echo off
REM -- Set up MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM -- Change to your project directory
cd /d C:\Users\clearly\Desktop\Code\win-workspaces

REM -- Compile your program
cl /EHsc /std:c++17 src\main.cpp src\WorkspaceManager.cpp /link dwmapi.lib user32.lib

pause

main.exe
