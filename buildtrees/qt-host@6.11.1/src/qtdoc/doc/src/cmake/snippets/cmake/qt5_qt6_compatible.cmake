# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [versionless_targets]
find_package(Qt6 COMPONENTS Core)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core)
endif()

add_executable(helloworld
    ...
)

target_link_libraries(helloworld PRIVATE Qt::Core)
#! [versionless_targets]

#! [older_qt_versions]
find_package(Qt6 COMPONENTS Core)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core)
    set(QT_VERSION_MAJOR 5)
endif()

add_executable(helloworld
    ...
)

target_link_libraries(helloworld PRIVATE Qt${QT_VERSION_MAJOR}::Core)
#! [older_qt_versions]

#! [disable_unicode_defines]

find_package(Qt6 COMPONENTS Core)

add_executable(helloworld
    ...
)

qt_disable_unicode_defines(helloworld)

#! [disable_unicode_defines]
