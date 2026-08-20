// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: pointlight
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000
        }

        DirectionalLight {
            id: light
            shadowFactor: 10
        }

        PointLight {

        }

        Node {
            id: originNodeBase
            Model {
                id: originCube
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
            }
        }

        Node {
            id: originNodeModelRotated
            Model {
                source: "#Cube"
                rotation: Quaternion.fromEulerAngles(0, 0, 45)
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        Node {
            id: originNodeModelPivot
            Model {
                source: "#Cube"
                pivot: Qt.vector3d(-50, 0, 0)
                rotation: Quaternion.fromEulerAngles(0, 0, 45)
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }

        Node {
            id: originNodeModelPivotSacled
            Model {
                source: "#Cube"
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                pivot: Qt.vector3d(-50, 0, 0)
                rotation: Quaternion.fromEulerAngles(0, 0, 135)
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        Node {
            id: originNodeModelParentPivotSacled
            pivot: Qt.vector3d(-150, 0, 0)
            rotation: Quaternion.fromEulerAngles(0, 0, 225)
            Model {
                source: "#Cube"
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                materials: PrincipledMaterial {
                    baseColor: "pink"
                }
            }
        }
    }
}
