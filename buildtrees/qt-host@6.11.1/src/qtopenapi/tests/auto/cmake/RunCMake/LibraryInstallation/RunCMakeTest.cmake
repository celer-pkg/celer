# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtRunCMake)

set(check_deps_case "find_openapi_deps")
set(main_build_case "build_and_install_lib")
set(consume_case "consume_installed_lib")

# A file that will say if all openapi deps are found.
set(DEPS_AVAILABLE_INCLUDE_PATH
    "${RunCMake_BINARY_DIR}/${check_deps_case}-build/DepsAvailable.cmake")

function(run_cmake_and_build case)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)

    # Set an install prefix where the first project will be installed.
    set(cmake_install_prefix ${RunCMake_BINARY_DIR}/installed)

    # A file that will include the client library target, created by the first project, consumed
    # by the second one.
    set(cmake_targets_include_path
        "${RunCMake_BINARY_DIR}/${main_build_case}-build/CompressionTargetsInclude.cmake")

    set(options
        "-DQt6_DIR=${Qt6_DIR}"
        "-DCMAKE_INSTALL_PREFIX=${cmake_install_prefix}"
        "-DCMAKE_TARGETS_INCLUDE_PATH=${cmake_targets_include_path}"
        "-DDEPS_AVAILABLE_INCLUDE_PATH=${DEPS_AVAILABLE_INCLUDE_PATH}"
    )

    # Configure.
    if(case STREQUAL "${check_deps_case}")
        # Don't error out due to stderr output.
        set(RunCMake_TEST_OUTPUT_MERGE 1)
    endif()
    run_cmake_with_options(${case} ${options})

    # Do not remove the current RunCMake_TEST_BINARY_DIR for the next operations.
    set(RunCMake_TEST_NO_CLEAN 1)

    # Merge output, because some of the tooling outputs to stderr even when everything is fine,
    # and CMake treats it as an error.
    set(RunCMake_TEST_OUTPUT_MERGE 1)

    # Build and install
    if(NOT case STREQUAL "${check_deps_case}")
        run_cmake_command(${case}-build "${CMAKE_COMMAND}" --build .)
        run_cmake_command(${case}-install "${CMAKE_COMMAND}" --install .)
    endif()

    # Run the test executable at the end
    if(case STREQUAL "${consume_case}")
        if(CMAKE_HOST_WIN32)
            message(STATUS "Skipping RunCMake.LibraryInstallation test, because it hasn't been "
                "fixed for Windows hosts yet")
        else()
            run_cmake_command(${case}-test "${CMAKE_CTEST_COMMAND}" -V)
        endif()
    endif()
endfunction()

# Check if all openapi dependencies are available. If not, skip the rest of the steps.
run_cmake_and_build("${check_deps_case}")
include("${DEPS_AVAILABLE_INCLUDE_PATH}")

if(NOT DEPS_AVAILABLE)
    message(STATUS "Skipping RunCMake.LibraryInstallation test, because not all OpenAPI "
        "dependencies are available.")
    return()
endif()

# Build and install the libraries.
run_cmake_and_build("${main_build_case}")

# Find, and link to the installed libraries, and build an executable.
run_cmake_and_build("${consume_case}")
