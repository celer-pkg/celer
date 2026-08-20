# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_plugin(jsonviewer
    CLASS_NAME JsonViewer
    jsonviewer.cpp jsonviewer.h
)

set_target_properties(jsonviewer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

target_link_libraries(jsonviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    abstractviewer
)

qt_add_translations(jsonviewer
    SOURCE_TARGETS jsonviewer
    TS_FILE_BASE jsonviewer
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(jsonviewer PRIVATE Qt6::PrintSupport)
endif()

