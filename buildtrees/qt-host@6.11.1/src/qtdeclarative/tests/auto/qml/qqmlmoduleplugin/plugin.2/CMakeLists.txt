# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from plugin.2.pro.

#####################################################################
## plugin.2 Generic Library:
#####################################################################

qt_internal_add_cmake_library(plugin.2
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlPluginType.2"
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

get_target_property(target_type plugin.2 TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(plugin.2 PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:plugin.2.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(plugin.2)
# special case begin
set_target_properties(plugin.2 PROPERTIES OUTPUT_NAME "plugin")
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlPluginType.2"
)
# special case end
