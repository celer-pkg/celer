# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from invalidFirstCommandModule.pro.

#####################################################################
## invalidFirstCommandModule Generic Library:
#####################################################################

qt_internal_add_cmake_library(invalidFirstCommandModule
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/InvalidFirstCommandModule"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.InvalidFirstCommandModule"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type invalidFirstCommandModule TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(invalidFirstCommandModule PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:invalidFirstCommandModule.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(invalidFirstCommandModule)
# special case begin
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/InvalidFirstCommandModule"
)
# special case end
