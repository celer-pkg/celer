# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from invalidNamespaceModule.pro.

#####################################################################
## invalidNamespaceModule Generic Library:
#####################################################################

qt_internal_add_cmake_library(invalidNamespaceModule
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/InvalidNamespaceModule"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.InvalidNamespaceModule"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type invalidNamespaceModule TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(invalidNamespaceModule PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:invalidNamespaceModule.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(invalidNamespaceModule)
# special case begin
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/InvalidNamespaceModule"
)
# special case end
