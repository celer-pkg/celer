# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtRunCMake)

set(build_case "build_and_install_tools_package")
set(consume_case "consume_tools_package")
set(consume_case_via_module "consume_tools_package_via_module")

function(run_cmake_and_build case)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)

    # Set an install prefix that is common to both projects.
    set(cmake_install_prefix ${RunCMake_BINARY_DIR}/installed)

    set(options
        "-DQt6_DIR=${Qt6_DIR}"
        "-DQT_REPO_MODULE_VERSION=${QT_REPO_MODULE_VERSION}"
    )

    # For prefix builds, install into a separate dir rather than the Qt one.
    # For non-prefix, files will end up being copied to the Qt dir.
    if(QT_WILL_INSTALL)
        list(APPEND options
            "-DCMAKE_INSTALL_PREFIX=${cmake_install_prefix}"
            "-DQT_ADDITIONAL_PACKAGES_PREFIX_PATH=${cmake_install_prefix}"
            "-DQT_ADDITIONAL_HOST_PACKAGES_PREFIX_PATH=${cmake_install_prefix}"
        )
    endif()

    # Merge output, because some of configure also outputs some stuff to stderr even when
    # everything is fine and CMake treats it as an error.
    set(RunCMake_TEST_OUTPUT_MERGE 1)

    # Configure.
    run_cmake_with_options(${case} ${options})

    # Do not remove the current RunCMake_TEST_BINARY_DIR for the next operations.
    set(RunCMake_TEST_NO_CLEAN 1)

    # Build and install
    run_cmake_command(${case}-build "${CMAKE_COMMAND}" --build .)

    if(QT_WILL_INSTALL)
        run_cmake_command(${case}-install "${CMAKE_COMMAND}" --install .)
    endif()
endfunction()

# Build and install the tools package.
run_cmake_and_build("${build_case}")

# Find the tools package.
run_cmake_and_build("${consume_case}")

# Find the tools package via module.
run_cmake_and_build("${consume_case_via_module}")
