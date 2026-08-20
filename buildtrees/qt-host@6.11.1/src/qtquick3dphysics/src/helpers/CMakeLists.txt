# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtquick3dhelpersplugin Plugin:
#####################################################################

qt_internal_add_qml_module(Quick3DPhysicsHelpers
    URI "QtQuick3D.Physics.Helpers"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtQuick3DPhysicsHelpersPlugin
    PLUGIN_TARGET qtquick3dphysicshelpersplugin
    SOURCES
        qcapsulegeometry.cpp qcapsulegeometry_p.h
    DEPENDENCIES
        QtQuick3D.Physics/auto
        QtQuick3D/auto
        QtQuick/auto
    PAST_MAJOR_VERSIONS 1
    DEFINES
        QT_BUILD_QUICK3DPHYSICSHELPERS_LIB
    DEPENDENCIES
        QtQuick3D
    LIBRARIES
        Qt::Quick3DPhysics
    PUBLIC_LIBRARIES
        Qt::Qml
        Qt::Quick
        Qt::Quick3D
    NO_GENERATE_CPP_EXPORTS
)

