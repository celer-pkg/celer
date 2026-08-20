// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property var matLib

    position: Qt.vector3d(0.19979211688041687, 0.3617419898509979, 0.026170935481786728)
    rotation: Qt.quaternion(0.676818, -0.0683734, -0.726353, -0.0982528)

    CustomModel {
        id: eyePatch
        position: Qt.vector3d(-0.13067705929279327, -0.13895738124847412, -0.026907149702310562)
        rotation: Qt.quaternion(0.981983, 0.0887153, 0.126944, 0.108274)
        scale: Qt.vector3d(0.12576, 0.125759, 0.125759)
        visible: AccessoryState.eyePatchVisible
        source: "meshes/mesh_002_mesh.mesh"
        materials: root.matLib.eyewear_generic
    }
    CustomModel {
        id: incognito
        position: Qt.vector3d(-0.031, 0.013, -0.006)
        rotation: Qt.quaternion(0.730921, 0.107435, -0.0349893, 0.673044)
        scale: Qt.vector3d(0.205174, 0.205174, 0.205174)
        visible: AccessoryState.incognitoVisible
        source: "meshes/mesh_006_mesh.mesh"
        materials: root.matLib.nose
    }
    CustomModel {
        id: monocle
        position: Qt.vector3d(-0.21734198927879333, -0.01033180020749569, 0.030987516045570374)
        rotation: Qt.quaternion(0.619253, -0.452281, -0.434292, -0.472607)
        scale: Qt.vector3d(0.0103497, 0.0103497, 0.0103497)
        visible: AccessoryState.monacleVisible
        source: "meshes/mesh_015_mesh.mesh"
        materials: root.matLib.eyewear_generic
    }
    CustomModel {
        id: nvgoggles
        position: Qt.vector3d(-0.02781829796731472, 0.08119115233421326, -0.008172246627509594)
        rotation: Qt.quaternion(0.993711, 0.104748, 0.0372885, 0.0132723)
        scale: Qt.vector3d(0.0613698, 0.0613698, 0.0613698)
        visible: AccessoryState.nvGogglesVisible
        source: "meshes/nvgoggles_mesh.mesh"
        materials: [
            root.matLib.nvgoggles_lens,
            root.matLib.nvgoggles_body,
            root.matLib.darkTextile,
            root.matLib.nvgoggleDetail
        ]
    }
    CustomModel {
        id: roundGlasses
        position: Qt.vector3d(-0.028860123828053474, -0.000948253320530057, -0.022175507619976997)
        rotation: Qt.quaternion(0.992192, 0.117822, 0.0350279, 0.0211196)
        scale: Qt.vector3d(0.236699, 0.236699, 0.236699)
        visible: AccessoryState.roundGlassesVisible
        source: "meshes/torus_002_mesh.mesh"
        materials: root.matLib.eyewear_generic
    }
    CustomModel {
        id: sunglasses
        position: Qt.vector3d(-0.15279579162597656, -0.0003664400428533554, -0.01976967044174671)
        rotation: Qt.quaternion(0.991817, 0.121886, 0.0273882, 0.0263249)
        scale: Qt.vector3d(-0.0557833, -0.0557833, -0.0557833)
        visible: AccessoryState.sunglassesVisible
        source: "meshes/plane_001_mesh.mesh"
        materials: root.matLib.eyewear_generic
    }
}
