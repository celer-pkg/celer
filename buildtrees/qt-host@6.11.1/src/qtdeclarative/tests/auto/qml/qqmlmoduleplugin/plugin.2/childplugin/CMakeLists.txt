# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from childplugin.pro.

#####################################################################
## childplugin Generic Library:
#####################################################################

qt_internal_add_cmake_library(childplugin.2 # special case
    MODULE # special case
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../imports/org/qtproject/AutoTestQmlPluginType.2/ChildPlugin"
    SOURCES
        childplugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.AutoTestQmlPluginType.ChildPlugin"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type childplugin.2 TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(childplugin.2 PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:childplugin.pro:<TRUE>:
# IMPORT_DIR = "DESTDIR"
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

# special case begin
qt_autogen_tools_initial_setup(childplugin.2)
set_target_properties(childplugin.2 PROPERTIES OUTPUT_NAME "childplugin")
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../../imports/org/qtproject/AutoTestQmlPluginType.2/ChildPlugin"
)
# special case end
