# cclint Test Suite CMake Configuration

cmake_minimum_required(VERSION 3.16)

# Test build option (disabled by default, enable with -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build tests" OFF)

if(NOT BUILD_TESTS)
    message(STATUS "Tests are disabled. Use -DBUILD_TESTS=ON to enable.")
    return()
endif()

message(STATUS "Building tests...")

# Find or fetch Google Test
find_package(GTest QUIET)

if(NOT GTEST_FOUND)
    message(STATUS "Google Test not found locally. Fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Enable testing
enable_testing()
include(GoogleTest)

# Include project source directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create a library from source files (for testing)
set(CCLINT_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/cli/argument_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/cli/help_formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/config/config_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/config/yaml_config.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/wrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/detector.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/ast.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/simple_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/diagnostic/diagnostic.cpp
    ${CMAKE_SOURCE_DIR}/src/diagnostic/fixer.cpp
    ${CMAKE_SOURCE_DIR}/src/output/formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/output/text_formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/output/json_formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/output/xml_formatter.cpp
    ${CMAKE_SOURCE_DIR}/src/output/formatter_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/file_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/string_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/rule_base.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/rule_registry.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/rule_executor.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/plugin_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/builtin/naming_convention.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/builtin/header_guard.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/builtin/max_line_length.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/builtin/function_complexity.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/analysis_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/incremental.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/dependency_tracker.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/dataflow_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/lua/lua_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/lua/lua_bridge.cpp
    ${CMAKE_SOURCE_DIR}/src/lua/lua_rule.cpp
    ${CMAKE_SOURCE_DIR}/src/cache/file_cache.cpp
    ${CMAKE_SOURCE_DIR}/src/parallel/thread_pool.cpp
)

add_library(cclint_lib STATIC ${CCLINT_LIB_SOURCES})

# Unit tests subdirectory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unit)
    add_subdirectory(unit)
endif()

# Integration tests subdirectory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration)
    add_subdirectory(integration)
endif()

# Create simple unit test executable (if unit tests exist)
file(GLOB UNIT_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")
if(UNIT_TEST_FILES)
    add_executable(unit_tests ${UNIT_TEST_FILES})
    target_link_libraries(unit_tests
        cclint_lib
        gtest_main
        gmock_main
    )
    gtest_discover_tests(unit_tests)
endif()

# Create integration test executable (if integration tests exist)
file(GLOB INTEGRATION_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp")
if(INTEGRATION_TEST_FILES)
    add_executable(integration_tests ${INTEGRATION_TEST_FILES})
    target_link_libraries(integration_tests
        cclint_lib
        gtest_main
        gmock_main
    )
    gtest_discover_tests(integration_tests)
endif()

# Copy test samples to build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/samples)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/samples
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Add custom target to run all tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS $<$<BOOL:${UNIT_TEST_FILES}>:unit_tests> $<$<BOOL:${INTEGRATION_TEST_FILES}>:integration_tests>
    COMMENT "Running all tests..."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Test configuration complete")
message(STATUS "Run 'cmake --build . --target check' to execute all tests")
