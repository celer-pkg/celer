# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtWaylandSetup)
qt_internal_get_wayland_protocols_dir(protocols_dir)
qt_internal_get_wayland_extensions_dir(extensions_dir)

#####################################################################
##Client test shared components:
#####################################################################

qt_manual_moc(moc_files
    mockcompositor.h
    coreprotocol.h
    corecompositor.h
    datadevice.h
    fullscreenshellv1.h
    fractionalscalev1.h
    iviapplication.h
    textinput.h
    qttextinput.h
    viewport.h
    xdgdialog.h
    xdgoutputv1.h
    xdgshell.h
)

add_library(SharedWaylandClientTest
    OBJECT
        corecompositor.cpp corecompositor.h
        coreprotocol.cpp coreprotocol.h
        datadevice.cpp datadevice.h
        fullscreenshellv1.cpp fullscreenshellv1.h
        fractionalscalev1.cpp fractionalscalev1.h
        iviapplication.cpp iviapplication.h
        mockcompositor.cpp mockcompositor.h
        textinput.cpp textinput.h
        qttextinput.cpp qttextinput.h
        xdgoutputv1.cpp xdgoutputv1.h
        xdgshell.cpp xdgshell.h
        xdgdialog.cpp xdgdialog.h
        viewport.cpp viewport.h
        ${moc_files}
)

qt6_generate_wayland_protocol_server_sources(SharedWaylandClientTest
    FILES
        ${protocols_dir}/cursor-shape/cursor-shape-v1.xml
        ${protocols_dir}/fullscreen-shell/fullscreen-shell-unstable-v1.xml
        ${PROJECT_SOURCE_DIR}/src/3rdparty/protocol/ivi/ivi-application.xml
        ${protocols_dir}/wp-primary-selection/wp-primary-selection-unstable-v1.xml
        ${protocols_dir}/tablet/tablet-unstable-v2.xml
        ${protocols_dir}/text-input/v2/text-input-unstable-v2.xml
        ${extensions_dir}/qt-text-input-method-unstable-v1.xml
        ${protocols_dir}/fractional-scale/fractional-scale-v1.xml
        ${protocols_dir}/viewporter/viewporter.xml
        ${protocols_dir}/wayland/wayland.xml
        ${protocols_dir}/xdg-decoration/xdg-decoration-unstable-v1.xml
        ${protocols_dir}/xdg-dialog/xdg-dialog-v1.xml
        ${protocols_dir}/xdg-output/xdg-output-unstable-v1.xml
        ${protocols_dir}/xdg-shell/xdg-shell.xml
)

if(QT_FEATURE_opengl)
    set(optional_libraries Qt::OpenGL)
endif()

target_link_libraries(SharedWaylandClientTest
    PUBLIC
        Qt::Gui
        ${optional_libraries}
        Qt::WaylandClientPrivate
        Wayland::Server
        Threads::Threads # special case
)

target_include_directories(SharedWaylandClientTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
