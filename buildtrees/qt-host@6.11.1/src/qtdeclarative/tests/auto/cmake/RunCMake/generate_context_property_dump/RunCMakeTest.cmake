# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtRunCMake)

# Stub function to make `_qt_internal_get_cmake_test_configure_options` work
function(_qt_internal_get_build_vars_for_external_projects)
endfunction()

set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/GenerateContextProperty")
set(RunCMake_TEST_NO_CLEAN 1)
set(RunCMake_TEST_OUTPUT_MERGE 1)
run_cmake_with_options(
    configuration
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
)
run_cmake_command(build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --target dump_qml_context_properties)

if(NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/.qt/contextPropertyDump.ini)
    message(FATAL_ERROR "${RunCMake_TEST_BINARY_DIR}/.contextPropertyDump.ini was not created.")
endif()

run_cmake_command(build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR} --target clean_qml_context_properties)

if(EXISTS ${RunCMake_TEST_BINARY_DIR}/.qt/contextPropertyDump.ini)
    message(FATAL_ERROR "${RunCMake_TEST_BINARY_DIR}/.contextPropertyDump.ini was not cleaned.")
endif()
