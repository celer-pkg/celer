# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# begin special case

#### Libraries

qt_find_package(Gypsy PROVIDED_TARGETS Gypsy::Gypsy)
qt_find_package(Gconf PROVIDED_TARGETS Gconf::Gconf)

#### Tests

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt/CMakeLists.txt")
    qt_config_compile_test("winrt_geolocation"
                           LABEL "WinRT geolocation"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt")
endif()

#### Features

qt_feature("gypsy" PUBLIC
    LABEL "Gypsy - A GPS Multiplexing Daemon"
    CONDITION Gypsy_FOUND AND Gconf_FOUND
)

qt_feature("winrt_geolocation" PRIVATE
    LABEL "WinRT geolocation API"
    CONDITION WIN32 AND TEST_winrt_geolocation
)

# end special case
