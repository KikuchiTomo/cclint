# FindLuaJIT.cmake - Find LuaJIT library
#
# This module defines:
#  LUAJIT_FOUND - system has LuaJIT
#  LUAJIT_INCLUDE_DIRS - the LuaJIT include directories
#  LUAJIT_LIBRARIES - link these to use LuaJIT
#  LUAJIT_VERSION_STRING - the version of LuaJIT found

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_LUAJIT QUIET luajit)
endif()

# Find include directory
find_path(LUAJIT_INCLUDE_DIR
    NAMES luajit.h
    HINTS
        ${PC_LUAJIT_INCLUDEDIR}
        ${PC_LUAJIT_INCLUDE_DIRS}
    PATHS
        /usr/include/luajit-2.1
        /usr/local/include/luajit-2.1
        /opt/homebrew/include/luajit-2.1
        /usr/include/luajit-2.0
        /usr/local/include/luajit-2.0
        /opt/homebrew/include/luajit-2.0
        ENV LUAJIT_DIR
    PATH_SUFFIXES include
)

# Find library
find_library(LUAJIT_LIBRARY
    NAMES luajit-5.1 luajit
    HINTS
        ${PC_LUAJIT_LIBDIR}
        ${PC_LUAJIT_LIBRARY_DIRS}
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        ENV LUAJIT_DIR
    PATH_SUFFIXES lib
)

# Extract version
if(LUAJIT_INCLUDE_DIR AND EXISTS "${LUAJIT_INCLUDE_DIR}/luajit.h")
    file(STRINGS "${LUAJIT_INCLUDE_DIR}/luajit.h" LUAJIT_VERSION_LINE
         REGEX "^#define[ \t]+LUAJIT_VERSION[ \t]+")
    string(REGEX REPLACE "^#define[ \t]+LUAJIT_VERSION[ \t]+\"LuaJIT ([0-9.]+)\".*" "\\1"
           LUAJIT_VERSION_STRING "${LUAJIT_VERSION_LINE}")
endif()

# Standard package handling
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LuaJIT
    REQUIRED_VARS LUAJIT_LIBRARY LUAJIT_INCLUDE_DIR
    VERSION_VAR LUAJIT_VERSION_STRING
)

if(LUAJIT_FOUND)
    set(LUAJIT_LIBRARIES ${LUAJIT_LIBRARY})
    set(LUAJIT_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})
endif()

mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARY)
