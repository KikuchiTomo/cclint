# Building cclint on Windows

This guide explains how to build and use cclint on Windows.

## Prerequisites

### Required Software

1. **Visual Studio 2019 or later** (Community Edition is fine)
   - Install "Desktop development with C++" workload
   - Make sure to include CMake and Git for Windows

2. **CMake 3.14 or later**
   - Download from https://cmake.org/download/
   - Add to PATH during installation

3. **Git for Windows**
   - Download from https://git-scm.com/download/win

### Optional Dependencies

- **LuaJIT 2.1+**: For Lua scripting support
  - Download from https://luajit.org/download.html
  - Or build from source

## Building from Source

### Using Visual Studio Developer Command Prompt

1. Open "Developer Command Prompt for VS 2019" (or your VS version)

2. Clone the repository:
```cmd
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint
```

3. Create build directory:
```cmd
mkdir build
cd build
```

4. Generate Visual Studio solution:
```cmd
cmake -G "Visual Studio 16 2019" -A x64 ..
```

For Visual Studio 2022, use:
```cmd
cmake -G "Visual Studio 17 2022" -A x64 ..
```

5. Build the project:
```cmd
cmake --build . --config Release
```

Or open `cclint.sln` in Visual Studio and build from there.

### Using MinGW-w64

1. Install MinGW-w64:
   - Download from https://www.mingw-w64.org/
   - Or use MSYS2: https://www.msys2.org/

2. Open MinGW terminal:
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
mingw32-make
```

## Installation

### System-wide Installation (Requires Admin)

```cmd
cmake --install build --prefix "C:\Program Files\cclint"
```

Add to PATH:
```cmd
setx PATH "%PATH%;C:\Program Files\cclint\bin"
```

### User Installation

```cmd
cmake --install build --prefix "%USERPROFILE%\cclint"
```

Add to user PATH:
```cmd
setx PATH "%PATH%;%USERPROFILE%\cclint\bin"
```

## Using cclint on Windows

### Basic Usage

```cmd
cclint cl test.cpp
cclint g++ test.cpp
```

### With Configuration File

Create `.cclint.yaml` in your project:

```yaml
version: 1.0
cpp_standard: "c++17"
lua_scripts:
  - C:\Users\YourName\cclint\scripts\rules\naming.lua
```

Run cclint:
```cmd
cclint --config=.cclint.yaml cl /EHsc test.cpp
```

### PowerShell Usage

```powershell
cclint --format=json g++ test.cpp | ConvertFrom-Json
```

## Building Plugins on Windows

### Using Visual Studio

1. Create plugin project:

```cpp
// my_plugin.cpp
#include "cclint/plugin_api.h"
#include "rules/rule_base.hpp"

// ... plugin implementation ...

extern "C" {
    __declspec(dllexport) const char* cclint_plugin_get_name() {
        return "my-plugin";
    }
    // ... other plugin functions ...
}
```

2. CMakeLists.txt for plugin:

```cmake
add_library(my_plugin SHARED my_plugin.cpp)
target_include_directories(my_plugin PRIVATE
    ${CCLINT_SOURCE_DIR}/src
    ${CCLINT_SOURCE_DIR}/include
)
```

3. Build:
```cmd
cmake --build . --config Release
```

This creates `my_plugin.dll`.

## Troubleshooting

### CMake can't find compiler

Make sure you're using the Visual Studio Developer Command Prompt, or add the compiler to PATH:

```cmd
set PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64
```

### LuaJIT not found

Set LuaJIT path manually:

```cmd
cmake -DLUAJIT_INCLUDE_DIR="C:\path\to\luajit\include" ^
      -DLUAJIT_LIBRARY="C:\path\to\luajit\lua51.lib" ^
      ..
```

### DLL not found at runtime

Add cclint bin directory to PATH:

```cmd
set PATH=%PATH%;C:\path\to\cclint\bin
```

Or copy required DLLs to the same directory as cclint.exe.

### Permission denied

Run Command Prompt as Administrator when installing system-wide.

## Windows-Specific Features

### MSVC Compiler Support

cclint supports Microsoft Visual C++ compiler:

```cmd
cclint cl /W4 /EHsc test.cpp
cclint cl /std:c++17 /W4 test.cpp
```

### Path Handling

cclint automatically handles Windows path separators (`\`):

```yaml
lua_scripts:
  - scripts\rules\naming.lua  # Backslash works
  - scripts/rules/style.lua   # Forward slash also works
```

### Console Colors

cclint supports colored output in:
- Windows Terminal
- ConEmu
- PowerShell 5.1+ (limited)
- CMD with ANSI support enabled (Windows 10+)

To enable ANSI colors in CMD:

```cmd
reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 1
```

## CI/CD Integration

### GitHub Actions

```yaml
name: Windows Build

on: [push, pull_request]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3

      - name: Configure CMake
        run: cmake -B build -G "Visual Studio 17 2022" -A x64

      - name: Build
        run: cmake --build build --config Release

      - name: Test
        run: |
          cd build
          ctest -C Release --output-on-failure
```

### AppVeyor

```yaml
version: 1.0.{build}
image: Visual Studio 2019
platform: x64

build_script:
  - mkdir build
  - cd build
  - cmake -G "Visual Studio 16 2019" -A x64 ..
  - cmake --build . --config Release

test_script:
  - ctest -C Release --output-on-failure
```

## Packaging for Distribution

### Creating Installer with NSIS

1. Install NSIS: https://nsis.sourceforge.io/

2. Create installer script `cclint.nsi`:

```nsis
!include "MUI2.nsh"

Name "cclint"
OutFile "cclint-installer.exe"
InstallDir "$PROGRAMFILES64\cclint"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    File /r "build\Release\*.*"
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd
```

3. Build installer:
```cmd
makensis cclint.nsi
```

### Creating MSI Package

Use WiX Toolset: https://wixtoolset.org/

## Performance Notes

### Build Times

- Debug build: ~2-3 minutes
- Release build: ~3-5 minutes
- Incremental build: ~30 seconds

### Runtime Performance

cclint on Windows has similar performance to Linux/macOS:
- Small projects (<100 files): <1 second
- Medium projects (100-1000 files): 1-10 seconds
- Large projects (1000+ files): 10-60 seconds

Use `--profile` to analyze performance:

```cmd
cclint --profile cl test.cpp
```

## See Also

- [Build Guide](build.md)
- [Installation Guide](installation.md)
- [Troubleshooting](troubleshooting.md)
