# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.19)
project(tst_aotstats_integration)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Test)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(tst_aotstats_integration main.cpp)
target_link_libraries(tst_aotstats_integration PRIVATE Qt6::Test)

add_subdirectory(empty)
add_subdirectory(no_bindings)
add_subdirectory(normal)
add_subdirectory(only_bytecode)

target_compile_definitions(tst_aotstats_integration
    PRIVATE
        "SOURCE_DIRECTORY=u\"${CMAKE_CURRENT_SOURCE_DIR}\"_s"
        "BUILD_DIRECTORY=u\"${CMAKE_CURRENT_BINARY_DIR}\"_s"
)

# The all_aotstats target doesn't exist yet. Defer setting its properties.
cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY \"${CMAKE_BINARY_DIR}\" "
    "CALL add_aotstats_target_to_all)")

function(add_aotstats_target_to_all)
    # Ensure aotstats generation happens when building the default "all" target
    set_target_properties(all_aotstats PROPERTIES EXCLUDE_FROM_ALL FALSE)
endfunction()
