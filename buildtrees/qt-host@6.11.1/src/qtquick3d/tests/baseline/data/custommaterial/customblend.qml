// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "white"
        }
        PerspectiveCamera {
            id: camera
            z: 600
        }
        DirectionalLight { }
        Model {
            source: "#Cube"
	    x: -150
            eulerRotation.x: 60
            eulerRotation.y: 20
            materials: CustomMaterial {
                fragmentShader: "customblend.frag"
            }
        }
        Model {
            source: "#Cube"
            eulerRotation.x: 60
            eulerRotation.y: 20
            materials: CustomMaterial {
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
                fragmentShader: "customblend.frag"
            }
        }
        Model {
            source: "#Cube"
	    x: 150
            eulerRotation.x: 60
            eulerRotation.y: 20
            materials: CustomMaterial {
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
                fragmentShader: "customblend.frag"
            }
	    opacity: 0.5
        }
    }
}
