# Example Plugin Build Configuration

cmake_minimum_required(VERSION 3.14)

# Example plugin
add_library(cclint_example_plugin SHARED
    example_plugin.cpp
)

# Include directories
target_include_directories(cclint_example_plugin
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Set plugin properties
set_target_properties(cclint_example_plugin PROPERTIES
    PREFIX ""  # Don't add 'lib' prefix on Unix
    OUTPUT_NAME "example_plugin"
)

# Installation
install(TARGETS cclint_example_plugin
    LIBRARY DESTINATION lib/cclint/plugins
    RUNTIME DESTINATION lib/cclint/plugins
)
