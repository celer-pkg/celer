# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(qmlheightmap
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(qmlheightmap PUBLIC
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::DataVisualization
    )

set_source_files_properties("gradientGRAY8.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapGRAY8"
    )
set_source_files_properties("gradientGRAY16.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapGRAY16"
    )
set_source_files_properties("gradientRGB8.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapRGB8"
    )
set_source_files_properties("gradientRGB16.png"
    PROPERTIES QT_RESOURCE_ALIAS "mapRGB16"
    )

set(qmlheightmap_resource_files
    "qml/qmlheightmap/main.qml"
    "gradientGRAY8.png"
    "gradientGRAY16.png"
    "gradientRGB8.png"
    "gradientRGB16.png"
    )
qt_internal_add_resource(qmlheightmap "qmlheightmap"
    PREFIX
    "/"
    FILES
    ${qmlheightmap_resource_files}
    )
