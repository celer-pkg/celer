// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    id: window
    width: 600
    height: 800
    visible: true

    View3D {
        width: parent.width/2
        height: parent.height/2

        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 200)
            eulerRotation.x: -30
            scale: Qt.vector3d(1, 1, 1)
            fieldOfView: 40
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Model {
            source: "#Cylinder"
            materials: [ DefaultMaterial {
                    diffuseColor: "red"
                }
            ]
        }
    }

    View3D {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2

        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 200)
            eulerRotation.x: -30
            scale: Qt.vector3d(1, 2, 3)
            fieldOfView: 40
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Model {
            source: "#Cylinder"
            materials: [ DefaultMaterial {
                    diffuseColor: "red"
                }
            ]
        }
    }

    // Use a camera as a node without scale
    View3D {
        width: parent.width/2
        height: parent.height/2
        y: parent.height/2
        camera: camera1

        PerspectiveCamera {
            id: camera1
            position: Qt.vector3d(0, 100, 200)
            eulerRotation.x: -30
            fieldOfView: 40
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        PerspectiveCamera {
            eulerRotation: Qt.vector3d(50, 20, 0)
            //eulerRotation.x: 50
            Model {
                source: "#Cylinder"
                materials: [ DefaultMaterial {
                        diffuseColor: "red"
                    }
                ]
            }
        }
    }

    // Use a camera as a node with scale
    View3D {
        width: parent.width/2
        height: parent.height/2
        x: parent.width/2
        y: parent.height/2
        camera: camera2

        PerspectiveCamera {
            id: camera2
            position: Qt.vector3d(0, 100, 200)
            eulerRotation.x: -30
            scale: Qt.vector3d(1, 2, 3)
            fieldOfView: 40
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        PerspectiveCamera {
            eulerRotation: Qt.vector3d(50, 20, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            Model {
                source: "#Cylinder"
                materials: [ DefaultMaterial {
                        diffuseColor: "red"
                    }
                ]
            }
        }
    }
}
