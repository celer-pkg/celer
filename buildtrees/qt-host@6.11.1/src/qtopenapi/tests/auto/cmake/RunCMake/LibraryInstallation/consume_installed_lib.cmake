# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This project builds an executable that links against the previously installed client
# library and runs the executable test.

find_package(Qt6 REQUIRED COMPONENTS BuildInternals Core Network Test OpenApiCommon)

# Avoid erorrs in CI about unsupported SDK and Xcode versions on older CI macOS versions.
if(APPLE)
    set(QT_NO_APPLE_SDK_AND_XCODE_CHECK ON)
endif()

# Needed for qt_internal_add_test to work
qt_build_internals_set_up_private_api()

# This is passed to the project from outside, to include the previously installed libraries.
include("${CMAKE_TARGETS_INCLUDE_PATH}")

qt_standard_project_setup(REQUIRES 6.10)

qt_internal_add_test(cpp-qt-compression
    SOURCES
        src/tst_compression.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

target_link_libraries(cpp-qt-compression PRIVATE
    CompressionClient::CompressionClient
)

if(WIN32)
    add_custom_command(TARGET cpp-qt-compression POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CompressionClient::CompressionClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()

