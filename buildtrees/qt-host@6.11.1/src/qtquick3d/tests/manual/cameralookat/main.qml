// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick.Controls

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Camera lookAt")

    View3D {
        anchors.fill: parent

        PerspectiveCamera {
            id: sceneCamera
            z: 300
            x: -200
            y: 100
            eulerRotation: Qt.vector3d(-15, -30, 0)
        }

        Model {
            id: cube
            x: 200
            y: 100
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseColor: "pink"
            }
        }

        Model {
            id: cone
            source: "#Cone"
            materials: DefaultMaterial {
                diffuseColor: "orange"
            }
        }

        Model {
            id: sphere
            x: -200
            y: -100
            source: "#Sphere"
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
        }

        DirectionalLight {

        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        Button {
            text: "Cube"
            onClicked: {
                sceneCamera.rotation = Quaternion.lookAt(sceneCamera.scenePosition, cube.scenePosition)
            }
        }
        Button {
            text: "Cone"
            onClicked: {
                sceneCamera.lookAt(cone);
            }
        }
        Button {
            text: "Sphere"
            onClicked: {
                sceneCamera.lookAt(sphere.scenePosition);
            }
        }

    }

}
