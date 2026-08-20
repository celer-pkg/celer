# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


cmake_minimum_required(VERSION 3.16)

project(qtactivetqt_cmake_tests)

enable_testing()

set(required_packages Core)
set(optional_packages Widgets Gui)

# Setup the test when called as a completely standalone project.
if(TARGET Qt6::Core)
    # Tests are built as part of the qtactiveqt build tree.
    # Setup paths so that the Qt packages are found.
    qt_internal_set_up_build_dir_package_paths()
endif()

find_package(Qt6 REQUIRED COMPONENTS ${required_packages})
find_package(Qt6 OPTIONAL_COMPONENTS ${optional_packages})

# Setup common test variables which were previously set by ctest_testcase_common.prf.
set(CMAKE_MODULES_UNDER_TEST "${required_packages}" "${optional_packages}")

foreach(qt_package ${CMAKE_MODULES_UNDER_TEST})
    set(package_name "${QT_CMAKE_EXPORT_NAMESPACE}${qt_package}")
    if(${package_name}_FOUND)
        set(CMAKE_${qt_package}_MODULE_MAJOR_VERSION "${${package_name}_VERSION_MAJOR}")
        set(CMAKE_${qt_package}_MODULE_MINOR_VERSION "${${package_name}_VERSION_MINOR}")
        set(CMAKE_${qt_package}_MODULE_PATCH_VERSION "${${package_name}_VERSION_PATCH}")
    endif()
endforeach()

include("${_Qt6CTestMacros}")

if(TARGET Qt6::Widgets AND TARGET Qt6::Gui)
    _qt_internal_test_expect_pass(test_target_typelibs)
    _qt_internal_test_expect_pass(test_target_typelibs_absolute)
    _qt_internal_test_expect_fail(test_target_typelibs_empty_libs)
    _qt_internal_test_expect_fail(test_target_typelibs_non_existing_libs)
    _qt_internal_test_expect_pass(test_target_typelibs_out_dir)
    _qt_internal_test_expect_pass(test_modules/container)
    _qt_internal_test_expect_pass(test_modules/server)
endif()
