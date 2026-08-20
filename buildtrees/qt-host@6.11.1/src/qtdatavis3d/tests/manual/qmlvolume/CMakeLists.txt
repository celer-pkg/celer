# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(qmlvolume
    GUI
    SOURCES
        datasource.cpp datasource.h
        main.cpp
    )
target_link_libraries(qmlvolume PUBLIC
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::DataVisualization
    )

set(qmlvolume_resource_files
    "qml/qmlvolume/main.qml"
    )

qt_internal_add_resource(qmlvolume "qmlvolume"
    PREFIX
    "/"
    FILES
    ${qmlvolume_resource_files}
    )
