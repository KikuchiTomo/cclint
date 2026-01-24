# ソースファイルを定義
set(CCLINT_SOURCES
    main.cpp
    cli/argument_parser.cpp
    cli/help_formatter.cpp
    config/config_loader.cpp
    config/yaml_config.cpp
    compiler/wrapper.cpp
    compiler/detector.cpp
    parser/ast.cpp
    parser/lexer.cpp
    parser/simple_parser.cpp
    diagnostic/diagnostic.cpp
    diagnostic/fixer.cpp
    output/formatter.cpp
    output/text_formatter.cpp
    output/json_formatter.cpp
    output/xml_formatter.cpp
    output/formatter_factory.cpp
    utils/file_utils.cpp
    utils/string_utils.cpp
    utils/logger.cpp
    rules/rule_base.cpp
    rules/rule_registry.cpp
    rules/rule_executor.cpp
    rules/builtin/naming_convention.cpp
    rules/builtin/header_guard.cpp
    rules/builtin/max_line_length.cpp
    rules/builtin/function_complexity.cpp
    engine/analysis_engine.cpp
    engine/incremental.cpp
    engine/dependency_tracker.cpp
    lua/lua_engine.cpp
    lua/lua_bridge.cpp
    lua/lua_rule.cpp
    cache/file_cache.cpp
    parallel/thread_pool.cpp
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
