# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Physics/designer")
qt_copy_or_install(
    FILES
        BoxShapeSection.qml
        BoxShapeSpecifics.qml
        CapsuleShapeSection.qml
        CapsuleShapeSpecifics.qml
        CharacterControllerSection.qml
        CharacterControllerSpecifics.qml
        CollisionShapeSection.qml
        ConvexMeshShapeSection.qml
        ConvexMeshShapeSpecifics.qml
        DynamicRigidBodySpecifics.qml
        DynamicRigidBodySection.qml
        HeightFieldShapeSection.qml
        HeightFieldShapeSpecifics.qml
        NodeSection.qml
        physics.metainfo
        PhysicsBodySection.qml
        PhysicsMaterialSection.qml
        PhysicsMaterialSpecifics.qml
        PhysicsNodeSection.qml
        PhysicsWorldSection.qml
        PhysicsWorldSpecifics.qml
        PlaneShapeSpecifics.qml
        SphereShapeSection.qml
        SphereShapeSpecifics.qml
        StaticRigidBodySpecifics.qml
        TriangleMeshShapeSection.qml
        TriangleMeshShapeSpecifics.qml
        TriggerBodySpecifics.qml
    DESTINATION "${destination}"
)

#add_subdirectory(source)
add_subdirectory(images)
