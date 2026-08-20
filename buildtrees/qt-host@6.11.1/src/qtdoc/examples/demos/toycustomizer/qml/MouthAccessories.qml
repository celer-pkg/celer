// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var currentElement
    property var matLib

    position: Qt.vector3d(root.currentElement.mouthPosX,
                          root.currentElement.mouthPosY,
                          root.currentElement.mouthPosZ)
    eulerRotation: Qt.vector3d(root.currentElement.mouthRotZ,
                               root.currentElement.mouthRotX,
                               root.currentElement.mouthRotY)
    visible: root.currentElement.name !== qsTr("Pig")

    CustomModel {
        id: smallMouth1
        visible: AccessoryState.smallEyesVisible
        eulerRotation: Qt.vector3d(0, 96, 0)
        scale: Qt.vector3d(0.02, 0.02, 0.02)
        source: "meshes/empty_001_mesh.mesh"
        materials: root.matLib.smallMouthMat
    }
    CustomModel {
        id: cuteMouth
        visible: AccessoryState.cuteEyesVisible
        scale: Qt.vector3d(0.3, 0.1, 0.3)
        position: Qt.vector3d(0.130, 0.051, -0.137)
        eulerRotation: Qt.vector3d(5, -82, -183)
        source: "meshes/mesh_013_mesh.mesh"
        materials: root.matLib.mouth
    }
    CustomModel {
        id: annoyedMouth
        visible: AccessoryState.annoyedEyesVisible
        position: Qt.vector3d(0.127, -0.031, 0.127)
        eulerRotation: Qt.vector3d(0, -96, 0)
        scale: Qt.vector3d(0.3, 0.1, 0.3)
        source: "meshes/mesh_013_mesh.mesh"
        materials: root.matLib.mouth
    }
    CustomModel {
        id: confusedMouth
        visible: AccessoryState.confusedEyesVisible
        scale: Qt.vector3d(0.14, 0.11, 0.11)
        position: Qt.vector3d(0, 0.001, 0)
        eulerRotation: Qt.vector3d(0, -96, 0)
        source: "meshes/empty_mesh.mesh"
        materials: root.matLib.crazyMouthMat
    }
    CustomModel {
        id: powerpuffMouth
        visible: AccessoryState.powerpuffEyesVisible
        scale: Qt.vector3d(0.022, 0.022, 0.022)
        position: Qt.vector3d(0, 0, 0)
        eulerRotation: Qt.vector3d(-1, -86, 0)
        source: "meshes/empty_003_mesh.mesh"
        materials: root.matLib.happyMouthMat
    }
    CustomModel {
        id: surprisedMouth
        visible: AccessoryState.surprisedEyesVisible
        position: Qt.vector3d(0, 0, 0)
        eulerRotation: Qt.vector3d(-1, -86, 0)
        scale: Qt.vector3d(0.022, 0.022, 0.022)
        source: "meshes/empty_003_mesh.mesh"
        materials: root.matLib.happyMouthMat
    }
    CustomModel {
        id: wideMouth
        visible: AccessoryState.wideEyesVisible
        position: Qt.vector3d(0, 0, 0)
        eulerRotation: Qt.vector3d(0, 96, 0)
        scale: Qt.vector3d(0.022, 0.022, 0.022)
        materials: root.matLib.smallMouthMat
        source: "meshes/empty_001_mesh.mesh"
    }
}
