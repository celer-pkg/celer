# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtquick3dhelpersplugin Plugin:
#####################################################################

qt_internal_add_qml_module(Quick3DHelpers
    URI "QtQuick3D.Helpers"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuick3DHelpersPlugin
    PLUGIN_TARGET qtquick3dhelpersplugin
    DEPENDENCIES
        QtQuick3D.Helpers.impl/auto
        QtQuick3D/auto
    PAST_MAJOR_VERSIONS 1
    SOURCES
        gridgeometry.cpp gridgeometry_p.h
        heightfieldgeometry.cpp heightfieldgeometry_p.h
        randominstancing.cpp randominstancing_p.h
        lookatnode.cpp lookatnode_p.h
        instancerepeater.cpp instancerepeater_p.h
        infinitegrid_p.h infinitegrid.cpp
        proceduralskytexturedata.cpp proceduralskytexturedata_p.h
        qquick3dtexturedatafrontend_p.h qquick3dtexturedatafrontend.cpp
        qquick3drenderoutputprovider_p.h qquick3drenderoutputprovider.cpp
        qquick3dsimplequadrenderpass_p.h qquick3dsimplequadrenderpass.cpp
        proceduralmesh_p.h proceduralmesh.cpp
        extrudedtextgeometry_p.h extrudedtextgeometry.cpp
        torusgeometry_p.h torusgeometry.cpp
        conegeometry_p.h conegeometry.cpp
        cylindergeometry_p.h cylindergeometry.cpp
        spheregeometry_p.h spheregeometry.cpp
        planegeometry_p.h planegeometry.cpp
        cuboidgeometry_p.h cuboidgeometry.cpp
        capsulegeometry_p.h capsulegeometry.cpp
        qtquick3dhelpersglobal_p.h
    QML_FILES
        AxisHelper.qml
        DebugView.qml
        WasdController.qml
        OrbitCameraController.qml
        LodManager.qml
        ExtendedSceneEnvironment.qml
    RESOURCES
        meshes/axisGrid.mesh
    DEFINES
        QT_BUILD_QUICK3DHELPERS_LIB
    LIBRARIES
        Qt::Quick3D
        Qt::Quick3DPrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Qml
        Qt::Quick
    PRIVATE_MODULE_INTERFACE
        Qt::Quick3DPrivate
        Qt::GuiPrivate
)

qt_internal_extend_target(Quick3DHelpers CONDITION QT_FEATURE_concurrent
    LIBRARIES
        Qt::Concurrent
)

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()

add_subdirectory(impl)

_qt_internal_add_qml_static_plugin_dependency(Quick3DHelpers Quick3DHelpersImpl)
