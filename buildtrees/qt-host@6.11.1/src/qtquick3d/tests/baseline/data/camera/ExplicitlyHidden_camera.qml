// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: orthographic_camera
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: left
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2
        camera: OrthographicCamera {
            z: 500
        }

        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "green"
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }

    View3D {
        id: right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: left.right

        camera: OrthographicCamera {
            z: 500
            visible: false
        }

        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "red"
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }

}
