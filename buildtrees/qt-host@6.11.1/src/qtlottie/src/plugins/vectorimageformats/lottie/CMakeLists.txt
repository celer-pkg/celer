# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QLottieVectorImagePlugin
    OUTPUT_NAME qlottievectorimage
    PLUGIN_TYPE vectorimageformats
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
        Qt::QuickPrivate
        Qt::QuickVectorImageGeneratorPrivate
        Qt::LottieVectorImageGeneratorPrivate
        Qt::LottiePrivate
)
