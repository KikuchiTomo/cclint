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
    rules/plugin_loader.cpp
    engine/analysis_engine.cpp
    engine/incremental.cpp
    engine/dependency_tracker.cpp
    engine/dataflow_analyzer.cpp
    lua/lua_engine.cpp
    lua/lua_bridge.cpp
    lua/lua_rule.cpp
    cache/file_cache.cpp
    parallel/thread_pool.cpp
)

# 実行ファイルを作成
add_executable(cclint ${CCLINT_SOURCES})

# LSPサーバー用のソースファイル
set(CCLINT_LSP_SOURCES
    lsp/lsp_server.cpp
    lsp/main_lsp.cpp
    utils/logger.cpp
    config/config_loader.cpp
    config/yaml_config.cpp
    engine/analysis_engine.cpp
    engine/incremental.cpp
    engine/dependency_tracker.cpp
    engine/dataflow_analyzer.cpp
    parallel/thread_pool.cpp
    cache/file_cache.cpp
    parser/ast.cpp
    parser/lexer.cpp
    parser/simple_parser.cpp
    rules/rule_base.cpp
    rules/rule_registry.cpp
    rules/rule_executor.cpp
    rules/plugin_loader.cpp
    diagnostic/diagnostic.cpp
    diagnostic/fixer.cpp
    lua/lua_engine.cpp
    lua/lua_bridge.cpp
    lua/lua_rule.cpp
    utils/file_utils.cpp
    utils/string_utils.cpp
)

# LSPサーバー実行ファイルを作成
add_executable(cclint-lsp ${CCLINT_LSP_SOURCES})

# インクルードディレクトリを設定
target_include_directories(cclint
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(cclint-lsp
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 依存関係のリンク

# LuaJIT
if(USE_LUAJIT AND LUAJIT_FOUND)
    target_link_libraries(cclint ${LUAJIT_LIBRARIES})
    target_include_directories(cclint PRIVATE ${LUAJIT_INCLUDE_DIRS})
    target_link_libraries(cclint-lsp ${LUAJIT_LIBRARIES})
    target_include_directories(cclint-lsp PRIVATE ${LUAJIT_INCLUDE_DIRS})
endif()

# yaml-cpp
if(USE_YAML_CPP AND yaml-cpp_FOUND)
    target_link_libraries(cclint yaml-cpp::yaml-cpp)
    target_link_libraries(cclint-lsp yaml-cpp::yaml-cpp)
endif()

# LLVM/Clang
if(USE_LLVM_CLANG AND LLVM_FOUND)
    # Clang libraries (依存関係は自動解決されるのでトップレベルのみ指定)
    target_link_libraries(cclint clangTooling)
    target_link_libraries(cclint-lsp clangTooling)
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(cclint dl pthread)
    target_link_libraries(cclint-lsp dl pthread)
elseif(APPLE)
    target_link_libraries(cclint dl pthread)
    target_link_libraries(cclint-lsp dl pthread)
elseif(WIN32)
    # Windows doesn't need dl, but might need other libraries
endif()
