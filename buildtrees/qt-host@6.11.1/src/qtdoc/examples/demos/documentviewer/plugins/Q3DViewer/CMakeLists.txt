# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_plugin(Q3Dviewer
    CLASS_NAME Q3DViewer
    q3dviewer.cpp q3dviewer.h
)

set_target_properties(Q3Dviewer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

qt_add_qml_module(Q3DViewerModule
    URI Q3DViewer
    QML_FILES
        Viewer.qml
        QueryMimeTypes.qml
)

target_link_libraries(Q3Dviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick3D
    abstractviewer
    Q3DViewerModule
)

qt_add_translations(Q3Dviewer
    SOURCE_TARGETS Q3Dviewer
    TS_FILE_BASE q3dviewer
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(Q3Dviewer PRIVATE Qt6::PrintSupport)
endif()
