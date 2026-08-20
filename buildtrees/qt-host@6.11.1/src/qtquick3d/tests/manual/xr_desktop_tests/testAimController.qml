// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Xr
import QtQuick3D.Helpers

import "../../../examples/quick3d/xr_shared/"

pragma ComponentBehavior: Bound

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#002266"
            backgroundMode: SceneEnvironment.Color
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {
                }
            }
        }

        Node {
            id: originNode
            PerspectiveCamera {
                id: cameraNode
                z: 150
            }
        }
        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        RandomInstancing {
            id: randomTable
            instanceCount: 150
            position: InstanceRange {
                from: Qt.vector3d(-200, -50, -100)
                to: Qt.vector3d(200, 50, 20)
            }
            scale: InstanceRange {
                from: Qt.vector3d(0.08, 0.08, 0.08)
                to: Qt.vector3d(0.15, 0.15, 0.15)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            color: InstanceRange {
                from: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                to: Qt.rgba(1, 1, 1, 1.0)
            }
        }

        InstanceRepeater {
            instancingTable: randomTable

            Model {
                id: delegate
                required property int index
                source: "#Cube"
                materials: PrincipledMaterial { baseColor: randomTable.instanceColor(delegate.index) }
                pickable: true
            }

        }

        property string lorem: "On the other hand, we denounce with righteous indignation and dislike men who are so beguiled and demoralized by the charms of pleasure of the moment, so blinded by desire, that they cannot foresee the pain and trouble that are bound to ensue; and equal blame belongs to those who fail in their duty through weakness of will, which is the same as saying through shrinking from toil and pain. These cases are perfectly simple and easy to distinguish. In a free hour, when our power of choice is untrammelled and when nothing prevents our being able to do what we like best, every pleasure is to be welcomed and every pain avoided. But in certain circumstances and owing to the claims of duty or the obligations of business it will frequently occur that pleasures have to be repudiated and annoyances accepted. The wise man therefore always holds in these matters to this principle of selection: he rejects pleasures to secure other greater pleasures, or else he endures pains to avoid worse pains. "

        XrItem {
            eulerRotation.y: 60
            pivot.x: 100
            x: -150
            y: 50
            width: 100
            height: 100
            contentItem: Rectangle {
                color: "white"
                width: 500
                height: 500
                Text {
                    anchors.fill: parent
                    anchors.margins: 20
                    text: view.lorem + view.lorem
                    wrapMode: Text.WordWrap
                }

            }
        }

        XrItem {
            x: -100
            y: 50
            width: 100
            height: 100
            contentItem: Rectangle {
                color: "white"
                width: 500
                height: 500
                Text {
                    anchors.fill: parent
                    anchors.margins: 20
                    text: view.lorem + view.lorem
                    wrapMode: Text.WordWrap
                }

            }
        }

        XrItem {
            x: 50
            y: 50
            z: -100
            width: 100
            height: 100
            contentItem: Rectangle {
                color: "white"
                width: 500
                height: 500
                Text {
                    anchors.fill: parent
                    anchors.margins: 20
                    text: view.lorem + view.lorem
                    wrapMode: Text.WordWrap
                }

            }
        }

        AimController {
            id: controller
            z: 100
            x: -30
            eulerRotation.x: 0

            view: view
            xrCursor: cursor

            Model {
                source: "#Cylinder"
                scale: "0.05, 0.1, 0.05"
                eulerRotation.x: 90
                materials: PrincipledMaterial {
                    baseColor: "black"
                }
            }

             SequentialAnimation on eulerRotation {
                 id: controllerAnimation
                 PropertyAnimation {
                     from: Qt.vector3d(-10, -60, 0)
                     to: Qt.vector3d(10, 60, 0)
                     duration: 8000
                 }
                 PropertyAnimation {
                     from: Qt.vector3d(10, 60, 0)
                     to: Qt.vector3d(-10, -60, 0)
                     duration: 8000
                 }
                 loops: -1
             }
        }

        XrCursor {
            id: cursor
            cameraNode: cameraNode
            size: 3
            opacity: 0.7
            cursorStyle: controller.pickStatus === PickResult.Model ? XrGadget.CursorStyle.Sphere : XrGadget.CursorStyle.Flat
        }
    } // View3D

    OrbitCameraController {
        id: occ
        origin: originNode
        camera: cameraNode

        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Space) {
                                controllerAnimation.running = !controllerAnimation.running
                            }
                        }
    }
} // Window
