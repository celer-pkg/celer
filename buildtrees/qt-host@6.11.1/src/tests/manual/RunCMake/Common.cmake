# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(top_repo_dir_path "${CMAKE_CURRENT_LIST_DIR}/../../..")
get_filename_component(top_repo_dir_path "${top_repo_dir_path}" ABSOLUTE)
set(qtbase_path "${top_repo_dir_path}/qtbase")
set(run_cmake_path "${qtbase_path}/src/testinternal/3rdparty/cmake")
if(NOT IS_DIRECTORY "${qtbase_path}")
    message(FATAL_ERROR
        "qtbase submodule is not initialized.\n"
        " Cannot run these tests without them."
    )
endif()
if(NOT IS_DIRECTORY "${run_cmake_path}")
    message(FATAL_ERROR
        "qtbase submodule is missing the QtRunCMakeTestHelpers at ${run_cmake_path}.\n"
        " Cannot run these tests without them."
    )
endif()

macro(qt_ir_setup_test_include_paths)
    set(ir_script_path "${top_repo_dir_path}/cmake")
    list(APPEND CMAKE_MODULE_PATH
        "${ir_script_path}"
        "${run_cmake_path}"
    )
    include(QtIRHelpers)
    qt_ir_include_all_helpers()
    # RunCMakeTestHelpers from qtbase/src/testinternal/3rdparty/cmake
    include(QtRunCMakeTestHelpers)
endmacro()
qt_ir_setup_test_include_paths()

# Used by add_RunCMake_test
set(CMAKE_CMAKE_COMMAND "${CMAKE_COMMAND}")
set(_isMultiConfig FALSE)
