# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from pluginWithQmlFile.pro.

#####################################################################
## pluginWithQmlFile Generic Library:
#####################################################################

qt_internal_add_cmake_library(pluginWithQmlFile
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestPluginWithQmlFile"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.AutoTestPluginWithQmlFile"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type pluginWithQmlFile TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(pluginWithQmlFile PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:pluginWithQmlFile.pro:<TRUE>:
# IMPORT_FILES = "qmldir" "MyQmlFile.qml"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(pluginWithQmlFile)
# special case begin
file(COPY qmldir MyQmlFile.qml
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestPluginWithQmlFile"
)
# special case end
