// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

XrView {
    id: xrView

    property bool showControllers: false
    property bool showRuntimeDialog: false

    XrErrorDialog { id: err }
    onInitializeFailed: (errorString) => err.run("XRView", errorString)
    referenceSpace: XrView.ReferenceSpaceLocal

    environment: SceneEnvironment {
        clearColor: "black"
        backgroundMode: SceneEnvironment.Color
    }

    xrOrigin: origin
    XrOrigin {
        id: origin
        objectName: "xrorigin"
        position: Qt.vector3d(0, 0, 100)
        XrController {
            id: leftController
            controller: XrController.ControllerLeft
            poseSpace: XrController.AimPose
            Lazer {
                visible: xrView.showControllers
                enableBeam: true
            }
            Node {
                y: rect.height
                x: -rect.width / 2
                visible: xrView.showRuntimeDialog
                Rectangle {
                    id: rect
                    opacity: 0.7
                    width: uiLayout.implicitWidth + 4
                    height: uiLayout.implicitHeight + 4
                    color: "white"
                    radius: 2
                    ColumnLayout {
                        id: uiLayout
                        spacing: 0
                        anchors.fill: parent
                        anchors.margins: 2
                        Text {
                            text: xrView.runtimeInfo.runtimeName + " " + xrView.runtimeInfo.runtimeVersion + "\n" + xrView.runtimeInfo.graphicsApiName
                            font.pixelSize: 2
                            color: "black"
                        }
                        Text {
                            visible: xrView.multiViewRenderingEnabled
                            text: "Multiview rendering enabled"
                            font.pixelSize: 2
                            color: "green"
                        }
                    }
                }
            }
        }
        XrController {
            id: rightController
            visible: xrView.showControllers
            controller: XrController.ControllerRight
            poseSpace: XrController.AimPose
            Lazer {
                visible: xrView.showControllers
                enableBeam: true
            }
        }
    }

    XrVirtualMouse {
        view: xrView
        source: rightController
        enabled: true
        // should be fixed to follow the api changes
        // leftMouseButton: rightController.actionMapper.triggerPressed
        // rightMouseButton: rightController.actionMapper.button1Pressed
        // middleMouseButton: rightController.actionMapper.button2Pressed
    }
}
