# PowerSwitcher (native Win32)

A native C++ tray application that switches Windows power plans. No WPF, no
WinForms, no .NET runtime.

- Tray icon with a native popup menu: power plans, "Open settings", "Exit"
- The settings window is the main interface: AC/battery auto-switching,
  default-plan filtering, left-click behaviour and language
- English and Simplified Chinese UI, chosen at first launch from the system language
- Zero polling: AC changes come from `WM_POWERBROADCAST`, plans are read on demand
- Single instance: launching it again opens the existing settings window

## Behaviour

| Action | Result |
|---|---|
| Left click tray icon | Configured in settings: do nothing (default), open settings, or show the tray menu |
| Right click tray icon | Native menu (plans / open settings / exit) |
| Launching a second copy | Opens the settings window of the running instance |

Settings are stored in `%APPDATA%\PowerSwitcher\config.ini`.

> Note: with `Show only default power plans` enabled, only High performance,
> Balanced, Power saver (and the currently active plan) are listed.

## Build

Requirements: Windows 10/11, CMake 3.20+, and either

- MSVC (Visual Studio Build Tools, "Desktop development with C++"), or
- MSYS2 MinGW-w64 / UCRT64 (`mingw-w64-ucrt-x86_64-gcc`)

The easiest way:

```bat
build.bat
```

The script prefers MSVC and falls back to MSYS2 `ucrt64`, `mingw64`, `clang64`
or any `g++` on `PATH`. Output: `build\bin\PowerSwitcher.exe`.

Manual build:

```bat
cmake -S . -B build
cmake --build build --config Release
```

> Use the Visual Studio generator, not Ninja. CMake 4.3.2 with MSVC 14.50
> generates a broken RC rule for Ninja (the command starts with a literal
> `RC` and `CreateProcess` fails). `build.bat` uses the VS generator for MSVC.

MinGW:

```bat
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-mingw
```

## Layout

```
CMakeLists.txt      build definition
build.bat           toolchain auto-detection and build
src/
  main.cpp          entry point, tray icon, message loop, tray menu
  app.cpp/.h        application state, plan switching, power events
  power.cpp/.h      PowerEnumerate / PowerReadFriendlyName / PowerSetActiveScheme
  config.cpp/.h     INI configuration
  i18n.cpp/.h       English and Chinese string tables
  ui.cpp/.h         settings window (plain Win32 controls), the main interface
resources/
  app.ico           tray / window icon
  app.manifest      common controls v6, DPI awareness
  app.rc.in         resource script template
  resource.h        icon resource id
```
