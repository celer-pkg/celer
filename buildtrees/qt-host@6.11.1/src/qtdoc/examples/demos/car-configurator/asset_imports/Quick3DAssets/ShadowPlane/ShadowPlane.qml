// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: node
    required property url downloadBase
    property bool doorOpen: false
    onDoorOpenChanged: doorOpen? openDoor.running = true : closeDoor.running = true
    property bool hoodOpen: false
    onHoodOpenChanged: hoodOpen? openHood.running = true : closeHood.running = true

    // Resources
    MorphTarget {
        id: morphTarget
        weight: 0
        attributes: MorphTarget.Position | MorphTarget.Normal
    }

    MorphTarget {
        id: morphTarget5
        weight: 0
        attributes: MorphTarget.Position | MorphTarget.Normal
    }

    NumberAnimation {
        id: openDoor
        target: morphTarget
        property: "weight"
        duration: 1000
        easing.type: Easing.InOutQuad
        from: 0
        to: 1
    }

    NumberAnimation {
        id: closeDoor
        target: morphTarget
        property: "weight"
        duration: 1000
        easing.type: Easing.InOutQuad
        from: 1
        to: 0
    }

    NumberAnimation {
        id: openHood
        target: morphTarget5
        property: "weight"
        duration: 1000
        easing.type: Easing.InOutQuad
        from: 0
        to: 1
    }
    NumberAnimation {
        id: closeHood
        target: morphTarget5
        property: "weight"
        duration: 1000
        easing.type: Easing.InOutQuad
        from: 1
        to: 0
    }

    Model {
        id: plane
        objectName: "Plane"
        scale.x: 4.24742
        scale.y: 4.24742
        scale.z: 4.24742
        source: node.downloadBase + "/asset_imports/Quick3DAssets/ShadowPlane/meshes/plane_008_mesh.mesh"
        materials: shadowMat_material
        morphTargets: [
            morphTarget,
            morphTarget5
        ]
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            property real hoodWeight: morphTarget5.weight
            property real doorWeight: morphTarget.weight
            id: shadowMat_material
            vertexShader: node.downloadBase + "/content/shaders/shadowmat.vert"
            shadingMode: CustomMaterial.Shaded
            alwaysDirty: true
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            sourceBlend: CustomMaterial.SrcAlpha
            fragmentShader: node.downloadBase + "/content/shaders/shadowmat.frag"
            depthDrawMode: Material.AlwaysDepthDraw
            objectName: "shadowMat"
            cullMode: Material.BackFaceCulling
        }
    }
}
