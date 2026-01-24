# cclint Test Suite CMake Configuration

cmake_minimum_required(VERSION 3.16)

# Test build option (disabled by default, enable with -DBUILD_TESTS=ON)
option(BUILD_TESTS "Build tests" OFF)

if(NOT BUILD_TESTS)
    message(STATUS "Tests are disabled. Use -DBUILD_TESTS=ON to enable.")
    return()
endif()

message(STATUS "Building tests...")

# Find Google Test
find_package(GTest QUIET)

if(NOT GTEST_FOUND)
    message(WARNING "Google Test not found. Tests will not be built.")
    message(STATUS "To install Google Test:")
    message(STATUS "  Ubuntu: sudo apt install libgtest-dev")
    message(STATUS "  macOS:  brew install googletest")
    return()
endif()

# Include Google Test
include_directories(${GTEST_INCLUDE_DIRS})

# Include project source directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Test executable
# add_executable(cclint_tests
#     ${UNIT_TEST_SOURCES}
#     ${INTEGRATION_TEST_SOURCES}
# )

# Link against Google Test and cclint libraries
# target_link_libraries(cclint_tests
#     ${GTEST_LIBRARIES}
#     pthread
#     cclint_lib  # When we create a library target
# )

# Enable CTest
enable_testing()

# Add tests to CTest
# add_test(NAME cclint_tests COMMAND cclint_tests)

message(STATUS "Test configuration complete")
