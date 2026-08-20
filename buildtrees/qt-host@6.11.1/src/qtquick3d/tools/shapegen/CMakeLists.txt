# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name shapegen)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D # special case
    SOURCES
        shapemanager.cpp shapemanager.h
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
        Qt::Quick3DPrivate
        Qt::Quick3DParticlesPrivate
)
