// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

XrView {
    id: xrView
    XrErrorDialog { id: err }
    onInitializeFailed: (errorString) => err.run("XRView", errorString)
    referenceSpace: XrView.ReferenceSpaceLocalFloor

    xrOrigin: theOrigin

    environment: SceneEnvironment {
        clearColor: "black"
        backgroundMode: SceneEnvironment.Color
    }

    property string pinchGesture: "none"
    property color cubeColor: "green"

    XrOrigin {
        id: theOrigin

        LeftHand {
            id: left

            XrInputAction {
                hand: XrInputAction.LeftHand
                actionId: [XrInputAction.IndexFingerPinch]
                onTriggered: {
                    pinchGesture = "Left Index finger"
                    cubeColor = "red"
                }
            }
            XrInputAction {
                hand: XrInputAction.LeftHand
                actionId: [XrInputAction.MiddleFingerPinch]
                onTriggered: {
                    pinchGesture = "Left Middle finge"
                    cubeColor = "blue"
                }
            }
            XrInputAction {
                hand: XrInputAction.LeftHand
                actionId: [XrInputAction.RingFingerPinch]
                onTriggered: {
                    pinchGesture = "Left Ring finger"
                    cubeColor = "green"
                }
            }
            XrInputAction {
                hand: XrInputAction.LeftHand
                actionId: [XrInputAction.LittleFingerPinch]
                onTriggered: {
                    pinchGesture = "Left Little finge"
                    cubeColor = "yellow"
                }
            }
        }

        RightHand {
            id: right

            XrInputAction {
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.IndexFingerPinch]
                onTriggered: {
                    pinchGesture = "Right Index finger"
                    cubeColor = "purple"
                }
            }
            XrInputAction {
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.MiddleFingerPinch]
                onTriggered: {
                    pinchGesture = "Right Middle finge"
                    cubeColor = "orange"
                }
            }
            XrInputAction {
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.RingFingerPinch]
                onTriggered: {
                    pinchGesture = "Right Ring finger"
                    cubeColor = "pink"
                }
            }
            XrInputAction {
                hand: XrInputAction.RightHand
                actionId: [XrInputAction.LittleFingerPinch]
                onTriggered: {
                    pinchGesture = "Right Little finge"
                    cubeColor = "brown"
                }
            }
        }

    }

    DirectionalLight {
    }


    Node {
        position: Qt.vector3d(0, 150, -100)

        Model {
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseColor: cubeColor
            }

            y: -10

            scale: Qt.vector3d(0.1, 0.1, 0.1)


            NumberAnimation  on eulerRotation.y {
                duration: 10000
                easing.type: Easing.InOutQuad
                from: 0
                to: 360
                running: true
                loops: -1
            }
        }

        Node {
            x: 10
            y: 20
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 0
                Text {
                    text: "Qt 6 in VR"
                    font.pointSize: 12
                    color: "white"
                }
                Text {
                    text: "On " + xrView.runtimeInfo.runtimeName + " " + xrView.runtimeInfo.runtimeVersion + " with " + xrView.runtimeInfo.graphicsApiName
                    font.pointSize: 4
                    color: "white"
                }
                Text {
                    visible: xrView.multiViewRenderingEnabled
                    text: "Multiview rendering enabled"
                    font.pointSize: 4
                    color: "green"
                }
                Text {
                    text: "Pinch gesture: " + xrView.pinchGesture
                    font.pointSize: 4
                    color: "green"
                }
            }
        }
    }

}
