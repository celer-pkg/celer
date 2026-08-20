// Copyright (C) 2024 The Qt Company Ltd.
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
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
            probeExposure: 1
            lightProbe: Texture {
                source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(5, 5, 5)
            eulerRotation.x: -90
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    vertexShader: "customdiffusespecular.vert"
                    fragmentShader: "customclearcoat.frag"
                    property real uTime: 0.0
                    property real uAmplitude: 0.0
                }
            ]
        }

        Model {
            position: Qt.vector3d(-50, 0, -50)
            eulerRotation.x: 30.0
            eulerRotation.y: 100.0
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    vertexShader: "customdiffusespecular.vert"
                    fragmentShader: "customclearcoat.frag"
                    property real uTime: 1.0
                    property real uAmplitude: 50.0
                }
            ]
        }
    }
}
