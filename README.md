# WIN-WORKSPACES
<p align="center" justify="center">
  <img src="/git-assets/win-workspaces-logo.png" alt="Screenshot" width="300"/>
  <img src="/git-assets/win-workspaces-example.gif" alt="Screenshot" width="500"/>
</p>

Fast, simple, i3/dwm/hyprland - like workspaces for windows 10/11 with multi-monitor support and monitor independent workspaces in less than 500 lines of code. It is nothing more, and nothing less.

![Made by Clearly](https://img.shields.io/badge/Made%20by-Clearly-blue)
![Version](https://img.shields.io/badge/Version-1.0.0-blue)


## How it works:
  Using the WINAPI; win-workspaces virtually assigns windows to workspaces, modifying their visbility based on the workspace your on. It also disables window animations, making it feel super smooth and nice to use.
  There is no Tiling, no bullshit, just monitor-independent workspaces similar to many Linux Window Managers. 

  If using a multi-monitor setup, each monitor has its own set of 9 workspaces. So with a triple-monitor setup you have a total of 27 workspaces. Each monitors 9 workspaces are accessible using ```ALT + 1,2,3,4...9``` and the workspace you switch to depends on the monitor your cursor is on. 

## Guide: 
  #### 1. Install via Github [Releases](https://www.google.com)

  #### 2. Open the application, and your ready to go.
  
  #### 3. Switching Workspaces is simple, ```ALT + 1,2,3,4....9``` 
  
  #### 4. Moving windows to another workspace is also simple; ```ALT + SHIFT + 1,2,3,4....9```

  #### 5. Note: Moving a window to another workspace requires that window to be focused. 

## Building From Source: 
 As far as i know, you need to use the MSVC Compiler, because the project utilizes ```dwmapi.h```. I've tried to build via MinGW64 and had no luck.

 Open the win-workspaces.sln file, and build it in Visual Studio 2022. (Requires Windows SDK, which should come with the "C++ Desktop Development" package in the VS Installer)
