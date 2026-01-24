# Building cclint from Source

This guide explains how to build cclint from source on Ubuntu and macOS.

## Prerequisites

### Build Requirements

- **CMake** 3.16 or later
- **C++ Compiler** with C++17 support:
  - GCC 7+ on Linux
  - Clang 10+ on macOS/Linux
  - Apple Clang (Xcode 10+) on macOS

### Optional Dependencies

The following dependencies are optional but enable additional features:

- **LuaJIT** 2.1 or later (for Lua scripting support) - **Recommended**
- **yaml-cpp** 0.7.0 or later (for enhanced YAML parsing) - Optional
- **LLVM/Clang** 14.0 or later (for advanced AST parsing) - Optional

### Installing Dependencies

Use the provided script to install dependencies automatically:

```bash
# Install all dependencies interactively
./scripts/install_dependencies.sh
```

Or install manually:

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libluajit-5.1-dev        # For Lua scripting
sudo apt install libyaml-cpp-dev          # For YAML support
sudo apt install llvm-14-dev libclang-14-dev  # For LLVM/Clang support
```

**macOS:**
```bash
brew install cmake git
brew install luajit           # For Lua scripting
brew install yaml-cpp         # For YAML support
brew install llvm             # For LLVM/Clang support
```

## Quick Start

### Using the Build Script (Recommended)

The easiest way to build cclint is using the provided build script:

```bash
# Clone the repository
git clone https://github.com/yourusername/cclint.git
cd cclint

# Build in Release mode
./build.sh

# Or build with options
./build.sh --clean --jobs 8
```

The build script automatically:
- Detects the number of CPU cores
- Configures CMake with appropriate settings
- Builds the project in parallel
- Shows build progress and status

### Build Script Options

```
./build.sh [OPTIONS]

Options:
  --debug         Build in Debug mode (default: Release)
  --clean         Clean build directory before building
  --install       Install after building (requires sudo)
  --jobs, -j N    Use N parallel jobs (default: auto-detect)
  --help, -h      Show this help message
```

### Manual Build (Alternative)

If you prefer to build manually:

```bash
# Create and enter build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build (using 8 parallel jobs)
make -j8

# Run cclint
./src/cclint --help
```

## Build Configurations

### Release Build (Default)

Optimized for performance:

```bash
./build.sh
# or
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Debug Build

Includes debug symbols and disables optimizations:

```bash
./build.sh --debug
# or
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## CMake Options

cclint supports several CMake options to enable or disable features:

### Feature Flags

Enable optional features with CMake options:

```bash
cmake -DUSE_LUAJIT=ON \
      -DUSE_YAML_CPP=ON \
      -DUSE_LLVM_CLANG=ON \
      -DBUILD_TESTS=ON \
      ..
```

Available options:

| Option | Default | Description |
|--------|---------|-------------|
| `USE_LUAJIT` | `ON` | Enable LuaJIT support for Lua scripting |
| `USE_YAML_CPP` | `OFF` | Use yaml-cpp for YAML parsing (vs simple parser) |
| `USE_LLVM_CLANG` | `OFF` | Use LLVM/Clang for advanced AST parsing |
| `BUILD_TESTS` | `OFF` | Build unit and integration tests |
| `CMAKE_BUILD_TYPE` | `Release` | Build type: `Release`, `Debug`, `RelWithDebInfo` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |

### Examples

**Minimal build** (no optional dependencies):
```bash
cmake -DUSE_LUAJIT=OFF -DUSE_YAML_CPP=OFF ..
make
```

**Full-featured build** (all optional features):
```bash
cmake -DUSE_LUAJIT=ON \
      -DUSE_YAML_CPP=ON \
      -DUSE_LLVM_CLANG=ON \
      -DBUILD_TESTS=ON \
      ..
make
```

**Build with tests**:
```bash
cmake -DBUILD_TESTS=ON ..
make
make check  # or: ctest
```

### Specifying Library Paths

If libraries are installed in non-standard locations:

```bash
# Specify LuaJIT location
cmake -DLUAJIT_INCLUDE_DIR=/opt/luajit/include \
      -DLUAJIT_LIBRARY=/opt/luajit/lib/libluajit.so \
      ..

# Specify LLVM location
cmake -DLLVM_DIR=/opt/llvm/lib/cmake/llvm ..

# Specify yaml-cpp location
cmake -Dyaml-cpp_DIR=/opt/yaml-cpp/lib/cmake/yaml-cpp ..
```

## Installation

### Using the Build Script

```bash
./build.sh --install
```

This will install:
- Binary: `/usr/local/bin/cclint`
- Lua scripts: `/usr/local/share/cclint/scripts/`

### Manual Installation

```bash
cd build
sudo make install
```

### Custom Installation Prefix

To install to a custom location:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/cclint ..
make -j$(nproc)
sudo make install
```

## Platform-Specific Instructions

### Ubuntu / Debian

```bash
# Install build dependencies
sudo apt update
sudo apt install -y cmake g++ make

# Build
./build.sh

# Install
sudo ./build.sh --install
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Or install CMake via Homebrew
brew install cmake

# Build
./build.sh

# Install
sudo ./build.sh --install
```

## Verifying the Build

After building, verify that cclint works:

```bash
# Check version
./build/src/cclint --version

# Show help
./build/src/cclint --help

# Test with a simple C++ file
echo '#include <iostream>
int main() { return 0; }' > test.cpp

./build/src/cclint g++ -std=c++17 test.cpp
```

Expected output:
```
[INFO] cclint starting...
[INFO] Using default configuration
[INFO] Detected compiler: g++ version ...
[INFO] Compiler exit code: 0
[INFO] Source files found:
[INFO]   - test.cpp
[INFO] cclint finished successfully
```

## Build Artifacts

After a successful build, you will find:

```
build/
├── src/
│   └── cclint              # Main executable (240KB)
└── CMakeFiles/             # CMake build artifacts
```

## Troubleshooting

### CMake Not Found

```bash
# Ubuntu/Debian
sudo apt install cmake

# macOS
brew install cmake
```

### Compiler Too Old

If you get C++17 feature errors:

```bash
# Ubuntu - Install newer GCC
sudo apt install g++-9
export CXX=g++-9

# macOS - Update Xcode
xcode-select --install
```

### Build Fails with "make: function definition file not found"

This can happen with certain shell configurations. Use the absolute path to make:

```bash
cd build
/usr/bin/make -j8
```

### Permission Denied During Installation

Use sudo for system-wide installation:

```bash
sudo make install
```

Or use a custom prefix you have write access to:

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install  # No sudo needed
```

## Clean Build

To start fresh:

```bash
# Using build script
./build.sh --clean

# Or manually
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Development Builds

For development, you may want to:

1. **Use Debug builds** for better error messages:
   ```bash
   ./build.sh --debug
   ```

2. **Enable compiler warnings** (already enabled by default):
   - The project uses `-Wall -Wextra -Wpedantic`

3. **Run static analysis**:
   ```bash
   # Using clang-tidy
   clang-tidy src/**/*.cpp -- -std=c++17

   # Using cppcheck
   cppcheck --enable=all src/
   ```

## Next Steps

- Read [usage.md](usage.md) for usage instructions
- Read [requirements.md](requirements.md) for feature details
- Check [TODO.md](TODO.md) for development status
- See [milestones.md](milestones.md) for the roadmap

## Current Implementation Status

**Note**: The current build (Milestone 1) includes:
- ✅ CLI argument parsing
- ✅ Configuration loading
- ✅ Compiler detection and wrapping
- ✅ Diagnostic engine
- ✅ Output formatting (text, JSON, XML)
- ✅ 100 Lua rule scripts (ready for integration)

**Not yet integrated** (requires external dependencies):
- ⏳ C++ AST parsing (requires LLVM/Clang)
- ⏳ Lua rule execution (requires LuaJIT)
- ⏳ YAML configuration parsing (requires yaml-cpp)

These features will be added in Milestones 2 and 3.
