# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from nestedPlugin.pro.

#####################################################################
## nestedPlugin Generic Library:
#####################################################################

qt_internal_add_cmake_library(nestedPlugin
    MODULE # special case
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlNestedPluginType"
    SOURCES
        nestedPlugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.AutoTestQmlNestedPluginType"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type nestedPlugin TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(nestedPlugin PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:nestedPlugin.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

# special case begin
qt_autogen_tools_initial_setup(nestedPlugin)
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/AutoTestQmlNestedPluginType"
)
# special case end TEMPLATE = "lib"
