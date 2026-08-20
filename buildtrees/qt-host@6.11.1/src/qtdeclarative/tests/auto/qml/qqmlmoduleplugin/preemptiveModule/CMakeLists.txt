# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from preemptiveModule.pro.

#####################################################################
## preemptiveModule Generic Library:
#####################################################################

qt_internal_add_cmake_library(preemptiveModule
    MODULE
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/PreemptiveModule"
    SOURCES
        plugin.cpp
    MOC_OPTIONS "-Muri=org.qtproject.PreemptiveModule"
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
)

get_target_property(target_type preemptiveModule TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(preemptiveModule PRIVATE QT_STATICPLUGIN)
endif()

#### Keys ignored in scope 1:.:.:preemptiveModule.pro:<TRUE>:
# IMPORT_FILES = "qmldir"
# TEMPLATE = "lib"

qt_autogen_tools_initial_setup(preemptiveModule)
# special case begin
file(COPY qmldir
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../imports/org/qtproject/PreemptiveModule"
)
# special case end
