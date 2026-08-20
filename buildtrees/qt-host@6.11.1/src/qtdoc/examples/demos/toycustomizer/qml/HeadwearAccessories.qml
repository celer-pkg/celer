// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var matLib

    position: Qt.vector3d(0.4897215664386749, -0.1153830736875534, 0.0317189060151577)
    rotation: Qt.quaternion(0.676818, -0.0683734, -0.726353, -0.0982528)

    CustomModel {
        id: bandana
        position: Qt.vector3d(-0.02493593841791153, 0.054058559238910675, 0.20184841752052307)
        rotation: Qt.quaternion(0.712502, -0.698243, 0.0671324, 0.0170576)
        scale: Qt.vector3d(0.4, 0.405009, 0.349713)
        visible: AccessoryState.bandanaVisible
        source: "meshes/cylinder_004_mesh.mesh"
        materials: root.matLib.bandana
    }
    CustomModel {
        id: beanie
        position: Qt.vector3d(-0.01518290676176548, 0.08178307861089706, 0.1975589543581009)
        rotation: Qt.quaternion(0.372753, 0.114828, -0.0645995, 0.91853)
        scale: Qt.vector3d(0.161593, 0.161593, 0.161593)
        visible: AccessoryState.beanieVisible
        source: "meshes/cylinder_002_mesh.mesh"
        materials: root.matLib.beanie
    }
    CustomModel {
        id: cap
        position: Qt.vector3d(-0.029357653111219406, 0.14411550760269165, 0.04530174657702446)
        rotation: Qt.quaternion(0.771718, -0.61766, -0.108644, 0.105566)
        scale: Qt.vector3d(0.127482, 0.127482, 0.127482)
        visible: AccessoryState.capVisible
        source: "meshes/cap_mesh.mesh"
        materials: [root.matLib.cap, root.matLib.capFront]
    }
    CustomModel {
        id: coneHat
        position: Qt.vector3d(-0.08551103621721268, 0.1563655585050583, -0.17683182656764984)
        rotation: Qt.quaternion(0.770705, -0.62816, 0.0945508, 0.0498881)
        scale: Qt.vector3d(0.135254, 0.227668, 0.135254)
        visible: AccessoryState.partyHatVisible
        source: "meshes/cone_mesh.mesh"
        materials: root.matLib.bdayHat
    }
    CustomModel {
        id: headphones
        position: Qt.vector3d(-0.014, 0.109, 0.329)
        eulerRotation: Qt.vector3d(61, 0, 87)
        visible: AccessoryState.headphonesVisible
        source: "meshes/cylinder_001_mesh.mesh"
        materials: root.matLib.headphones
    }
    CustomModel {
        id: wizardhat
        position: Qt.vector3d(-0.013, 0.211, -0.136)
        rotation: Qt.quaternion(0.817492, -0.573833, -0.0348424, 0.0347799)
        visible: AccessoryState.wizardHatVisible
        source: "meshes/cylinder_005_mesh.mesh"
        materials: root.matLib.wizardHat
    }

    CustomModel {
        id: whiskers
        position: Qt.vector3d(0.008, 0.037, 1.123)
        eulerRotation: Qt.vector3d(-91, 2, 0)
        visible: AccessoryState.whiskersVisible
        source: "meshes/mesh_019_mesh.mesh"
        materials: root.matLib.whiskers
    }
}
