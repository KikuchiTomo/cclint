# SafeFindClang.cmake
# Safely attempt to find Clang libraries without failing on broken installations

function(safe_find_clang)
    set(CLANG_FOUND FALSE PARENT_SCOPE)

    # Check if ClangConfig.cmake exists and is loadable
    find_package(LLVM QUIET CONFIG)

    if(NOT LLVM_FOUND)
        message(STATUS "LLVM not found")
        return()
    endif()

    # Check if clangBasic library actually exists before trying to load package
    find_library(CLANG_BASIC_CHECK
        NAMES clangBasic libclangBasic
        HINTS ${LLVM_LIBRARY_DIRS}
        NO_DEFAULT_PATH
    )

    if(NOT CLANG_BASIC_CHECK)
        message(STATUS "Clang libraries not found in ${LLVM_LIBRARY_DIRS}")
        return()
    endif()

    # Libraries exist, try to find the package
    # Suppress errors from broken ClangTargets.cmake by redirecting stderr
    find_package(Clang QUIET CONFIG)

    if(Clang_FOUND)
        set(CLANG_FOUND TRUE PARENT_SCOPE)
        set(LLVM_FOUND ${LLVM_FOUND} PARENT_SCOPE)
        set(LLVM_PACKAGE_VERSION ${LLVM_PACKAGE_VERSION} PARENT_SCOPE)
        set(LLVM_DEFINITIONS ${LLVM_DEFINITIONS} PARENT_SCOPE)
        set(LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS} PARENT_SCOPE)
        set(LLVM_LIBRARY_DIRS ${LLVM_LIBRARY_DIRS} PARENT_SCOPE)
    endif()
endfunction()
