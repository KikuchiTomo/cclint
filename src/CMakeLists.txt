# ソースファイルを定義
set(CCLINT_SOURCES
    main.cpp
    cli/argument_parser.cpp
    cli/help_formatter.cpp
)

# 実行ファイルを作成
add_executable(cclint ${CCLINT_SOURCES})

# インクルードディレクトリを設定
target_include_directories(cclint
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# TODO: 依存関係の追加（Milestone 2以降）
# - LLVM/Clang (libtooling)
# - yaml-cpp
# - LuaJIT 2.1
