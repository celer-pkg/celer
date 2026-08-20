// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var matLib

    position: Qt.vector3d(-0.09632175415754318, -0.024348465725779533, -0.007334607653319836)
    rotation: Qt.quaternion(0.726334, 0.0632633, -0.68237, 0.0529958)

    CustomModel {
        id: angelWings
        position: Qt.vector3d(0.013663120567798615, -0.17130792140960693, -0.01612933911383152)
        rotation: Qt.quaternion(0.993938, 0.105207, -0.0291857, 0.0129278)
        scale: Qt.vector3d(0.133827, 0.133827, 0.133827)
        visible: AccessoryState.angelWingsVisible
        source: "meshes/plane_018_mesh.mesh"
        materials: root.matLib.angelWings
    }
    CustomModel {
        id: backpack
        position: Qt.vector3d(0, -0.21760517358779907, -0.05233892798423767)
        rotation: Qt.quaternion(0.741539, -0.670845, 0.00898391, 0.00246629)
        scale: Qt.vector3d(0.176869, 0.176869, 0.0940099)
        visible: AccessoryState.backpackVisible
        source: "meshes/cube_001_mesh.mesh"
        materials: root.matLib.backpack
    }
    CustomModel {
        id: bowTie
        position: Qt.vector3d(0.025913089513778687, 0.09789018332958221, -0.18778866529464722)
        rotation: Qt.quaternion(0.728906, -0.684532, -0.00655214, -0.00834157)
        scale: Qt.vector3d(0.0515167, 0.0515167, 0.0515167)
        visible: AccessoryState.bowtieVisible
        source: "meshes/mesh_mesh.mesh"
        materials: root.matLib.bowtie
    }
    CustomModel {
        id: butterflyWings
        position: Qt.vector3d(0.005243896972388029, -0.19553649425506592, -0.07103119045495987)
        rotation: Qt.quaternion(0.999687, 0.0103407, -0.0220312, 0.00574802)
        scale: Qt.vector3d(0.629903, 0.629903, 0.629903)
        visible: AccessoryState.butterflyWingsVisible
        source: "meshes/mesh_012_mesh.mesh"
        materials: root.matLib.butterflyWings
    }
    CustomModel {
        id: necktie
        position: Qt.vector3d(0.01815732754766941, 0.005497889593243599, -0.21858450770378113)
        rotation: Qt.quaternion(0.709103, -0.0177479, -0.0227379, 0.704515)
        scale: Qt.vector3d(0.195457, 0.23, 0.195457)
        visible: AccessoryState.necktieVisible
        source: "meshes/torus_001_mesh.mesh"
        materials: root.matLib.necktie
    }
}
