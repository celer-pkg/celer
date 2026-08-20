# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_plugin(pdfviewer
    CLASS_NAME PdfViewer
    pdfviewer.cpp pdfviewer.h
    zoomselector.cpp zoomselector.h
    hoverwatcher.cpp hoverwatcher.h
)

set_target_properties(pdfviewer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

target_link_libraries(pdfviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PdfWidgets
    abstractviewer
)

qt_add_translations(pdfviewer
    SOURCE_TARGETS pdfviewer
    TS_FILE_BASE pdfviewer
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(pdfviewer PRIVATE Qt6::PrintSupport)
endif()

