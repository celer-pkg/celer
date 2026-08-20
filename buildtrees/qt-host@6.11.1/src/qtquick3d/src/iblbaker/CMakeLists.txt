# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(Quick3DIblBaker
    SOURCES
        qssgiblbaker.cpp qssgiblbaker_p.h
        qtquick3diblbaker_p.h
    DEFINES
        QT_BUILD_QUICK3DIBLBAKER_LIB
    LIBRARIES
        Qt::Quick3DRuntimeRenderPrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::Quick3DRuntimeRender
    PRIVATE_MODULE_INTERFACE
        Qt::Quick3DRuntimeRenderPrivate
        Qt::GuiPrivate
)
