# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from plugin.pro.

#####################################################################
## plugin Generic Library:
#####################################################################

qt_internal_add_cmake_library(plugin
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlPluginType"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.AutoTestQmlPluginType"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type plugin TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(plugin PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:plugin.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(plugin)
# special case begin
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlPluginType"
)
# special case end
