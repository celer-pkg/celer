// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            aoStrength: 80
            aoBias: 0.5
            aoSampleRate: 4
            aoDistance: 90
            aoSoftness: 40
            aoDither: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation: Qt.vector3d(-30, 0, 0)
        }
        Model {
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            materials: [ CustomMaterial {
                    cullMode: Material.NoCulling
                    vertexShader: "custom_ssao.vert"
                    fragmentShader: "custom_ssao.frag"
                    property real uTime: 16.0
                    property real uAmplitude: 0.5
                } ]
            eulerRotation: Qt.vector3d(0, 120, 340)
        }
        DirectionalLight {
            id: dirLight
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0);
        }
    }
}
