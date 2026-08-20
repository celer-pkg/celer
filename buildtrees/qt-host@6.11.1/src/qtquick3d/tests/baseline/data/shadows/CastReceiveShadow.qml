// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: directionallight
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: view
        anchors.fill: parent
        camera: camera1

        DirectionalLight {
            castsShadow: true
            shadowFactor: 25
            eulerRotation: Qt.vector3d(-60, -20, 0)
        }

	// The ground must have shadows from the cube, the cylinder, and the big sphere above the cube.
	// The ground must not show shadowing from the smaller sphere in the center.
	// The cube must not show shadowing from from the big sphere.

        Model {
            id: ground
            source: "#Cube"
            scale: Qt.vector3d(10, 0.01, 10)
            materials: DefaultMaterial {
                diffuseColor: Qt.rgba(1.0, 1.0, 0.0, 1.0)
            }
            castsShadows: false
        }

        Model {
            source: "#Sphere"
            y: 50
            materials: DefaultMaterial {
            }
            castsShadows: false // no shadow should be visible on the ground for this
        }

        Model {
            source: "#Cylinder"
            y: 200
            x: -250
            scale: Qt.vector3d(1, 5, 1)
            materials: DefaultMaterial {
            }
        }

        Model {
            source: "#Sphere"
            x: -250
            y: 200 // above the cube
            z: 250
            materials: DefaultMaterial {
            }
        }

        Model {
            source: "#Cube"
            x: -250
            z: 250
            y: 50
            materials: DefaultMaterial {
            }
            receivesShadows: false // the sphere above would shadow it otherwise
        }

        PerspectiveCamera {
            id: camera1
            z: 600
            y: 300
            clipFar: 1000
            eulerRotation: Qt.vector3d(-20, 0, 0)
        }
    }
}
