# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Quick3DPhysics Module:
#####################################################################

qt_internal_add_qml_module(Quick3DPhysics
    URI "QtQuick3D.Physics"
        VERSION "${PROJECT_VERSION}"
        CLASS_NAME QtQuick3DPhysicsPlugin
        PLUGIN_TARGET qquick3dphysicsplugin
        DESIGNER_SUPPORTED
        NO_PLUGIN_OPTIONAL
        NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        physxnode/qabstractphysxnode.cpp physxnode/qabstractphysxnode_p.h
        physxnode/qphysxactorbody.cpp physxnode/qphysxactorbody_p.h
        physxnode/qphysxcharactercontroller.cpp physxnode/qphysxcharactercontroller_p.h
        physxnode/qphysxdynamicbody.cpp physxnode/qphysxdynamicbody_p.h
        physxnode/qphysxrigidbody.cpp physxnode/qphysxrigidbody_p.h
        physxnode/qphysxstaticbody.cpp physxnode/qphysxstaticbody_p.h
        physxnode/qphysxtriggerbody.cpp physxnode/qphysxtriggerbody_p.h
        physxnode/qphysxworld.cpp physxnode/qphysxworld_p.h
        qabstractcollisionshape.cpp qabstractcollisionshape_p.h
        qabstractphysicsbody.cpp qabstractphysicsbody_p.h
        qabstractphysicsnode.cpp qabstractphysicsnode_p.h
        qboxshape.cpp qboxshape_p.h
        qcacheutils.cpp qcacheutils_p.h
        qcapsuleshape.cpp qcapsuleshape_p.h
        qcharactercontroller.cpp qcharactercontroller_p.h
        qcollisiondebugmeshbuilder.cpp qcollisiondebugmeshbuilder_p.h
        qconvexmeshshape.cpp qconvexmeshshape_p.h
        qdebugdrawhelper.cpp qdebugdrawhelper_p.h
        qdynamicrigidbody.cpp qdynamicrigidbody_p.h
        qheightfieldshape.cpp qheightfieldshape_p.h
        qmeshshape.cpp qmeshshape_p.h
        qphysicscommands.cpp qphysicscommands_p.h
        qphysicsmaterial.cpp qphysicsmaterial_p.h
        qphysicsmeshutils_p_p.h
        qphysicsutils_p.h
        qphysicsworld.cpp qphysicsworld_p.h
        qplaneshape.cpp qplaneshape_p.h
        qsphereshape.cpp qsphereshape_p.h
        qtquick3dphysicsglobal_p.h
        qtquick3dphysicsglobal.h
        qtrianglemeshshape.cpp qtrianglemeshshape_p.h
        qtriggerbody.cpp qtriggerbody_p.h
        qstaticphysxobjects.cpp qstaticphysxobjects_p.h
        qstaticrigidbody.cpp qstaticrigidbody_p.h
    DEFINES
        QT_BUILD_QUICK3DPHYSICS_LIB
        PX_PHYSX_STATIC_LIB
    SYSTEM_INCLUDE_DIRECTORIES
        ../3rdparty/PhysX/include
        ../3rdparty/PhysX/pxshared/include
    DEPENDENCIES
        QtQuick3D
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DPrivate
        Qt::QuickPrivate
        Qt::BundledPhysX
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3D
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3D
        Qt::QuickPrivate
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(qquick3dphysicsplugin
    SOURCES
        plugin.cpp
    LIBRARIES
        Qt::Quick3DPhysicsPrivate
)

if (UNIX OR MINGW)
    # Needed for PxPreprocessor.h error
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        qt_internal_extend_target(Quick3DPhysics DEFINES _DEBUG)
    else()
        qt_internal_extend_target(Quick3DPhysics DEFINES NDEBUG)
    endif()
endif()

qt_internal_add_docs(Quick3DPhysics
    doc/qtquick3dphysics.qdocconf
)

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS)
    add_subdirectory(designer)
endif()
