# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from strictModule.pro.

#####################################################################
## strictModule Generic Library:
#####################################################################

qt_internal_add_cmake_library(strictModule
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/StrictModule"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.StrictModule"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type strictModule TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(strictModule PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:strictModule.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_internal_extend_target(strictModule CONDITION NOT BUILD_SHARED_LIBS
    DEFINES
        QT_STATICPLUGIN
)

qt_autogen_tools_initial_setup(strictModule)
# special case begin
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/StrictModule"
)
# special case end
