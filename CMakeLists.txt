cmake_minimum_required(VERSION 3.16)
project(cclint VERSION 0.1.0 LANGUAGES C CXX)

# CMake モジュールパスの設定
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# C++17を必須にする
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ビルドタイプのデフォルト設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# コンパイラ警告を有効化
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# オプション設定
option(USE_LLVM_CLANG "Use LLVM/Clang for advanced AST parsing" OFF)
option(USE_YAML_CPP "Use yaml-cpp for YAML configuration" ON)
option(USE_LUAJIT "Use LuaJIT for Lua scripting support" ON)
option(BUILD_TESTS "Build tests" OFF)

# External library detection

# 1. LLVM/Clang (Optional - for advanced AST parsing)
# NOTE: Currently disabled by default because builtin parser is sufficient
if(USE_LLVM_CLANG)
    message(STATUS "Attempting to enable LLVM/Clang support...")
    message(STATUS "")
    message(STATUS "NOTE: If this fails with Clang library errors, it's safe to ignore.")
    message(STATUS "      The builtin parser provides full functionality.")
    message(STATUS "      To skip this check, use: cmake -DUSE_LLVM_CLANG=OFF ..")
    message(STATUS "")

    set(LLVM_CLANG_USABLE OFF)

    # Check if we should skip Clang detection due to known broken installations
    set(SKIP_CLANG_DETECTION OFF)

    # Detect common broken Clang installations
    if(EXISTS "/usr/lib/cmake/clang-14/ClangConfig.cmake")
        if(NOT EXISTS "/usr/lib/llvm-14/lib/libclangBasic.a")
            message(WARNING "Detected broken Clang 14 installation (config exists but libraries missing)")
            message(STATUS "Skipping LLVM/Clang detection to avoid errors")
            set(SKIP_CLANG_DETECTION ON)
        endif()
    endif()

    if(NOT SKIP_CLANG_DETECTION)
        # Try to find LLVM
        find_package(LLVM QUIET CONFIG)

        if(LLVM_FOUND)
            message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

            # Check if Clang libraries actually exist
            find_library(CLANG_BASIC_LIB
                NAMES clangBasic libclangBasic
                HINTS ${LLVM_LIBRARY_DIRS}
                NO_DEFAULT_PATH
            )

            if(CLANG_BASIC_LIB)
                # Try to load Clang package (may fail with broken installations)
                find_package(Clang QUIET CONFIG)

                if(Clang_FOUND)
                    set(LLVM_CLANG_USABLE ON)
                    message(STATUS "LLVM/Clang libraries verified and loaded successfully")
                else()
                    message(WARNING "Clang package config failed to load (may be broken installation)")
                endif()
            else()
                message(WARNING "Clang libraries not found in ${LLVM_LIBRARY_DIRS}")
            endif()
        else()
            message(STATUS "LLVM package not found")
        endif()
    endif()

    if(LLVM_CLANG_USABLE)
        add_definitions(${LLVM_DEFINITIONS})
        include_directories(${LLVM_INCLUDE_DIRS})
        link_directories(${LLVM_LIBRARY_DIRS})
        add_definitions(-DHAVE_LLVM_CLANG)

        message(STATUS "✓ LLVM/Clang support enabled")
    else()
        message(STATUS "")
        message(STATUS "================================================")
        message(STATUS "LLVM/Clang not available - using builtin parser")
        message(STATUS "================================================")
        message(STATUS "This is the recommended configuration.")
        message(STATUS "The builtin parser provides full functionality.")
        message(STATUS "")
        set(USE_LLVM_CLANG OFF)
    endif()
else()
    message(STATUS "================================================")
    message(STATUS "Using builtin C++ parser (recommended)")
    message(STATUS "================================================")
    message(STATUS "All features including comprehensive AST support")
    message(STATUS "are available through the builtin parser.")
    message(STATUS "")
endif()

# 2. yaml-cpp (Optional - for YAML configuration)
if(USE_YAML_CPP)
    message(STATUS "Searching for yaml-cpp...")
    find_package(yaml-cpp QUIET CONFIG)

    if(yaml-cpp_FOUND)
        message(STATUS "Found yaml-cpp ${yaml-cpp_VERSION}")

        # Add yaml-cpp include directory and library path
        if(APPLE)
            include_directories(/opt/homebrew/opt/yaml-cpp/include)
            link_directories(/opt/homebrew/opt/yaml-cpp/lib)
        endif()

        add_definitions(-DHAVE_YAML_CPP)
        message(STATUS "yaml-cpp support enabled")
    else()
        message(WARNING "yaml-cpp not found. YAML config loading will be limited.")
        message(STATUS "To install yaml-cpp:")
        message(STATUS "  Ubuntu: sudo apt install libyaml-cpp-dev")
        message(STATUS "  macOS:  brew install yaml-cpp")
        set(USE_YAML_CPP OFF)
    endif()
else()
    message(STATUS "yaml-cpp support disabled. Using simple YAML parser.")
endif()

# 3. LuaJIT 2.1+ (Optional but recommended - for Lua scripting)
if(USE_LUAJIT)
    message(STATUS "Searching for LuaJIT...")
    find_package(LuaJIT)

    if(LUAJIT_FOUND)
        message(STATUS "Found LuaJIT ${LUAJIT_VERSION_STRING}")
        message(STATUS "LuaJIT include dirs: ${LUAJIT_INCLUDE_DIRS}")
        message(STATUS "LuaJIT libraries: ${LUAJIT_LIBRARIES}")

        include_directories(${LUAJIT_INCLUDE_DIRS})
        add_definitions(-DHAVE_LUAJIT)

        message(STATUS "LuaJIT support enabled")
    else()
        message(WARNING "LuaJIT not found. Lua scripting will be disabled.")
        message(STATUS "To install LuaJIT:")
        message(STATUS "  Ubuntu: sudo apt install libluajit-5.1-dev")
        message(STATUS "  macOS:  brew install luajit")
        message(STATUS "  Or build from source: https://luajit.org/download.html")
        set(USE_LUAJIT OFF)
    endif()
else()
    message(STATUS "LuaJIT support disabled. Lua scripting unavailable.")
endif()

# サブディレクトリを追加
add_subdirectory(src)

# テストの追加（オプション）
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# インストール設定
install(TARGETS cclint DESTINATION bin)

# 標準Luaスクリプトのインストール
install(DIRECTORY scripts/rules
        DESTINATION share/cclint/scripts
        FILES_MATCHING PATTERN "*.lua")
