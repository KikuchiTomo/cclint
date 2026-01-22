cmake_minimum_required(VERSION 3.16)
project(cclint VERSION 0.1.0 LANGUAGES CXX)

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

# サブディレクトリを追加
add_subdirectory(src)

# インストール設定
install(TARGETS cclint DESTINATION bin)

# 標準Luaスクリプトのインストール
install(DIRECTORY scripts/rules
        DESTINATION share/cclint/scripts
        FILES_MATCHING PATTERN "*.lua")
