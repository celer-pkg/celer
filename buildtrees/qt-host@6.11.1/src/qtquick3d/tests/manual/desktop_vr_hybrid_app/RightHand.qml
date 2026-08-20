// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Xr

XrController {
    id: root
    property XrView view
    controller: XrController.ControllerRight
    handInput.poseSpace: XrHandInput.AimPose
    Lazer {
        enableBeam: true
    }
    ActionMapper {}
    Node {
        x: 10
        y: 10
        Rectangle {
            radius: 8
            color: "lightGray"
            width: 50
            height: 30
            Text {
                font.pointSize: 8
                text: root.view.renderStats.fps + " FPS"
                anchors.centerIn: parent
            }
        }
    }
}
