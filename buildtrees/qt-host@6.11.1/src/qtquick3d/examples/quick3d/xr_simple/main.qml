// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
//! [XrView]
import QtQuick3D
import QtQuick3D.Xr

XrView {
    id: xrView
    XrErrorDialog { id: err }
    onInitializeFailed: (errorString) => err.run("XRView", errorString)
    referenceSpace: XrView.ReferenceSpaceLocalFloor
//! [XrView]

    environment: SceneEnvironment {
        clearColor: "black"
        backgroundMode: SceneEnvironment.Color
    }

//! [XrOrigin]
    xrOrigin: theOrigin
    XrOrigin {
        id: theOrigin

        XrController {
            controller: XrController.LeftController
            poseSpace: XrController.AimPose
            CubeModel { color: "blue" }
        }

        XrController {
            controller: XrController.RightController
            poseSpace: XrController.AimPose
            CubeModel { color: "red" }
        }
    }
//! [XrOrigin]

    component CubeModel : Model {
        source: "#Cube"
        scale: Qt.vector3d(0.1, 0.1, 0.1)
        property alias color: boxMaterial.baseColor
        materials: PrincipledMaterial {
            id: boxMaterial
            lighting: PrincipledMaterial.NoLighting
        }
    }

    DirectionalLight {
    }

    Node {
        position: Qt.vector3d(0, 150, -100)

        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0.4, 0.7, 0.0, 1.0)
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
            }
        }
    }

}
