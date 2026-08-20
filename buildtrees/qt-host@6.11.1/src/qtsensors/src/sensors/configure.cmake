# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries
# special case begin
if (LINUX)
    qt_find_package(Sensorfw PROVIDED_TARGETS Sensorfw::Sensorfw MODULE_NAME sensors QMAKE_LIB sensorfw)
endif()
# special case end

#### Tests

if (WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt/CMakeLists.txt")
    qt_config_compile_test("winrt_sensors"
                           LABEL "WinRT sensors"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt")
endif()

#### Features

# special case begin
qt_feature("sensorfw" PRIVATE
    LABEL "sensorfw"
    CONDITION Sensorfw_FOUND
)
# special case end

qt_feature("winrt_sensors" PRIVATE
    LABEL "WinRT sensors backend"
    CONDITION WIN32 AND TEST_winrt_sensors
)

qt_configure_add_summary_section(NAME "Qt Sensors")
if (LINUX)
    # At the moment there is no Qt6 version of sensorfw, and while the
    # FindSensorfw.cmake in this repo would find the package using pkg-config,
    # it would not work. Once the Qt6 version exists, remove the
    # "sensorfw_enabled_with_cmake" entry, enable build in
    # src/plugins/sensors/CMakeLists.txt and fix any potential errors.
    qt_configure_add_summary_entry(ARGS "sensorfw")
#    qt_configure_add_summary_entry(ARGS "sensorfw_enabled_with_cmake")
endif()

if (WIN32)
    qt_configure_add_summary_entry(ARGS "winrt_sensors")
endif()

qt_configure_end_summary_section() # end of "Qt Sensors" section
