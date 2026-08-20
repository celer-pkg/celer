// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    id: lightProbe
    width: 800
    height: 480
    color: Qt.rgba(1, 1, 1, 1)

    Node {
        id: sharedScene
        PerspectiveCamera {
            id: camera
            position.z: 200
        }

        PrincipledMaterial {
            id: default_002
            metalness: 1.0
            roughness: 0.0
            baseColor: "white"
        }

        Model {
            id: sphere
            source: "#Sphere"
            materials: [default_002]
        }
        Texture {
            id: layer_lightprobe
            source: "../shared/maps/TestEnvironment-512.hdr"
            mappingMode: Texture.LightProbe
            tilingModeHorizontal: Texture.Repeat
        }
    }

    View3D {
        id: layer1
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sharedScene
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: layer_lightprobe
        }
    }
    View3D {
        id: layer2
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sharedScene
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: layer_lightprobe
            probeOrientation.x: 90
        }
    }
    View3D {
        id: layer3
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sharedScene
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: layer_lightprobe
            probeOrientation.y: 90
        }
    }
    View3D {
        id: layer4
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: parent.width * 0.5
        height: parent.height * 0.5
        importScene: sharedScene
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: layer_lightprobe
            probeOrientation.z: 90
        }
    }
}
