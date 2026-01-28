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
    set(CMAKE_BUILD_TYPE Release)
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
option(USE_LLVM_CLANG "Use LLVM/Clang for advanced AST parsing" ON)
option(USE_YAML_CPP "Use yaml-cpp for YAML configuration" ON)
option(USE_LUAJIT "Use LuaJIT for Lua scripting support" ON)
option(BUILD_TESTS "Build tests" OFF)

# External library detection

# 1. LLVM/Clang (Optional - for advanced AST parsing)
if(USE_LLVM_CLANG)
    message(STATUS "Searching for LLVM/Clang...")
    find_package(LLVM QUIET)
    find_package(Clang QUIET)

    if(LLVM_FOUND AND Clang_FOUND)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
        message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
        message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
        message(STATUS "LLVM library dirs: ${LLVM_LIBRARY_DIRS}")

        add_definitions(${LLVM_DEFINITIONS})
        include_directories(${LLVM_INCLUDE_DIRS})
        link_directories(${LLVM_LIBRARY_DIRS})

        # Define preprocessor macro
        add_definitions(-DHAVE_LLVM_CLANG)

        message(STATUS "LLVM/Clang support enabled")
    else()
        message(WARNING "LLVM/Clang not found. Using simple parser instead.")
        message(STATUS "To install LLVM/Clang:")
        message(STATUS "  Ubuntu: sudo apt install llvm-14-dev libclang-14-dev")
        message(STATUS "  macOS:  brew install llvm")
        set(USE_LLVM_CLANG OFF)
    endif()
else()
    message(STATUS "LLVM/Clang support disabled. Using simple parser.")
endif()

# 2. yaml-cpp (Optional - for YAML configuration)
if(USE_YAML_CPP)
    message(STATUS "Searching for yaml-cpp...")
    find_package(yaml-cpp QUIET)

    if(yaml-cpp_FOUND)
        message(STATUS "Found yaml-cpp ${yaml-cpp_VERSION}")
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
