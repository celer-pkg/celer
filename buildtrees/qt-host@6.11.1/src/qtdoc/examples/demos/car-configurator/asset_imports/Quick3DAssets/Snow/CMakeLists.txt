# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick3DParticlesPrivate)

qt_add_library(Ast_Snow STATIC)

qt_add_qml_module(Ast_Snow
    URI "Quick3DAssets.Snow"
    CLASS_NAME "SnowPlugin"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    SOURCES
        particleAffector.h
        particleAffector.cpp
    QML_FILES
        ParticleMask.qml
        PrincipledExSnowMaterial.qml
        SnowParticlesSystem.qml
    RESOURCES
        snow_normal_displacement.png
        sparkles.png
        snow.png
        sphere.png
        snow_mask_effect.frag
)

 target_link_libraries(Ast_Snow
    PRIVATE
    Qt6::Quick3DParticlesPrivate
)
