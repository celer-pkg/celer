# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(CMAKE_AUTOMOC ON)

qt_add_executable(sensorsdocsnippet
    creating.cpp
    main.cpp
    mybackend.h
    plugin.cpp
    start.cpp
    myreading.cpp
)

# MyObject is defined in multiple files
set_target_properties(sensorsdocsnippet PROPERTIES
    UNITY_BUILD FALSE
)

target_link_libraries(sensorsdocsnippet PUBLIC
        Qt::Sensors
)

if (GCC AND UNIX)
    target_link_options(sensorsdocsnippet PUBLIC
        -rdynamic
    )
endif()

qt_autogen_tools(sensorsdocsnippet ENABLE_AUTOGEN_TOOLS moc rcc uic)
