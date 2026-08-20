# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from pluginMixed.pro.

#####################################################################
## pluginMixed Generic Library:
#####################################################################

qt_internal_add_cmake_library(pluginMixed
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlMixedPluginType"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.AutoTestQmlMixedPluginType"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type pluginMixed TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(pluginMixed PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:pluginMixed.pro:<TRUE>:
# IMPORT_FILES = "Foo.qml" "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(pluginMixed)
# special case begin
file(COPY qmldir Foo.qml
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlMixedPluginType"
)
# special case end
