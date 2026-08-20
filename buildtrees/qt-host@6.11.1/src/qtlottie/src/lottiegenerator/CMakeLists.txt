# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(LottieVectorImageGeneratorPrivate
    INTERNAL_MODULE
    SOURCES
        qlottievisitor_p.h qlottievisitor.cpp
    LIBRARIES
        Qt::Core
        Qt::QuickPrivate
        Qt::QuickShapesPrivate
        Qt::QuickVectorImageGeneratorPrivate
        Qt::LottiePrivate
        Qt::LottieVectorImageHelpers
)
