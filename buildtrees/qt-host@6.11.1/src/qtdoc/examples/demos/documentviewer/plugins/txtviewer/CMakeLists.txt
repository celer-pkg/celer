# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_add_plugin(txtviewer
    CLASS_NAME TxtViewer
    txtviewer.cpp txtviewer.h
)

set_target_properties(txtviewer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

target_link_libraries(txtviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    abstractviewer
)

qt_add_translations(txtviewer
    SOURCE_TARGETS txtviewer
    TS_FILE_BASE txtviewer
)

if(TARGET Qt6::PrintSupport)
    target_link_libraries(txtviewer PRIVATE Qt6::PrintSupport)
endif()
