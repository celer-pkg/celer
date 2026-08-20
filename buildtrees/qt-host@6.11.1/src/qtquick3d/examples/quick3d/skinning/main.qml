// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import Example

Window {
    id: window
    visible: true
    width: 800
    height: 480
    title: qsTr("Simple Skinning")

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position.z: 3.0
            clipNear: 1.0
            clipFar: 10.0
        }

        SimpleSkinning {
            x: -1
        }
        SimpleSkinningNew {
            x: 1
        }
    }
}
