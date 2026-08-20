# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_plugin(imageviewer
    CLASS_NAME ImageViewer
    imageviewer.cpp imageviewer.h
)

set_target_properties(imageviewer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

target_link_libraries(imageviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    abstractviewer
)

qt_add_translations(imageviewer
    SOURCE_TARGETS imageviewer
    TS_FILE_BASE imgviewer
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(imageviewer PRIVATE Qt6::PrintSupport)
endif()
