// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick.Controls

import Qt.labs.platform
import QtCore

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.AssetUtils

Window {
    visible: true
    width: 1024
    height: 768

    property bool nightMode: false

    View3D {
        id: view3D
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: nightMode ? "#443366" : "#ccddff"
            lightProbe: Texture {
            }
            InfiniteGrid {
                id: theGrid
                gridInterval: 10
            }
        }

        DirectionalLight {
            eulerRotation: "-30, -20, -40"
            ambientColor: "#777"
        }

        camera: cameraNode.ortho ? orthoCamera : wasdCamera

        Node {
            id: cameraNode
            property real clipFar: 10000
            property real clipNear: 1
            property bool ortho: false
            property bool orthoNegativeNear: true
            position: "-100, 100, -100"
            eulerRotation.x: -20
            eulerRotation.y: -150

            PerspectiveCamera {
                id: wasdCamera
                clipNear: cameraNode.clipNear
                clipFar: cameraNode.clipFar
            }
            OrthographicCamera {
                id: orthoCamera
                clipNear: cameraNode.orthoNegativeNear ? -cameraNode.clipFar : cameraNode.clipNear
                clipFar: cameraNode.clipFar
                property real mag: 2.5
                horizontalMagnification: mag
                verticalMagnification: mag
            }
        }

        // Red cubes along the positive x-axis
        Repeater3D {
            model: 11
            Model {
                source: "#Cube"
                x: 10 * index
                y: 0

                scale: "0.02, 0.02, 0.02"
                materials: PrincipledMaterial { baseColor: index === 0 ? "gray" : "red" }
            }
        }

        // Blue cubes along the positive z-axis
        Repeater3D {
            model: 10
            Model {
                source: "#Cube"
                x: 0
                z: 10 * (index + 1)

                scale: "0.02, 0.02, 0.02"
                materials: PrincipledMaterial { baseColor: "blue" }
            }
        }

        // White cubes along the positive y-axis
        Repeater3D {
            model: 10
            Model {
                source: "#Cube"
                x: 0
                y: 10 * (index + 1)

                scale: "0.02, 0.02, 0.02"
                materials: PrincipledMaterial { baseColor: "white" }
            }
        }

        Repeater3D {
            model: 21
            Model {
                source: "#Cube"
                x: 100 * (index - 10)
                z: 100

                scale: "0.1, 0.1, 0.1"
                materials: PrincipledMaterial{ baseColor: "green" }
            }
        }

        Repeater3D {
            model: 21
            Model {
                source: "#Cube"
                x: 1000 * (index - 10)
                z: -100

                scale: "0.2, 0.2, 0.2"
                materials: PrincipledMaterial{ baseColor: "blue" }
            }
        }
    }

    Text {
        text: "Grid interval: " + theGrid.gridInterval
    }

    WasdController {
        id: wasdController
        controlledObject: cameraNode
        speed: 0.1
        shiftSpeed: 1

        Keys.onPressed: (event)=> {
            if (keysEnabled) handleKeyPress(event);
            if (event.key === Qt.Key_Space) {
                console.log("Zoom grid out")
                theGrid.gridInterval *= 10
            } else if (event.key === Qt.Key_Z) {
                console.log("Zoom grid in")
                theGrid.gridInterval /= 10
            } else if (event.key === Qt.Key_G) {
                theGrid.gridAxes = !theGrid.gridAxes
                console.log("Axes shown", theGrid.gridAxes)
            } else if (event.key === Qt.Key_O) {
                cameraNode.ortho = !cameraNode.ortho
                console.log("Orthographic camera", cameraNode.ortho)
            } else if (event.key === Qt.Key_N) {
                cameraNode.orthoNegativeNear = !cameraNode.orthoNegativeNear
                console.log("Ortho camera near plane is negative far plane:", cameraNode.orthoNegativeNear)
            } else if (event.key === Qt.Key_B) {
                nightMode = !nightMode
                console.log("Dark mode:", nightMode)
            } else if (event.key === Qt.Key_I) {
                console.log("Isometric projection")
                cameraNode.ortho = true;
                cameraNode.eulerRotation = "-30, -135, 0"
            } else if (event.key === Qt.Key_1) {
                cameraNode.eulerRotation.x = 0
                cameraNode.eulerRotation.y = 90
                cameraNode.eulerRotation.z = 0
                console.log("Camera aligned to x-axis")
            } else if (event.key === Qt.Key_2) {
                cameraNode.eulerRotation.x = -90
                cameraNode.eulerRotation.z = 0
                console.log("Camera aligned to y-axis")
            } else if (event.key === Qt.Key_3) {
                cameraNode.eulerRotation.x = 0
                cameraNode.eulerRotation.y = 0
                cameraNode.eulerRotation.z = 0
                console.log("Camera aligned to z-axis")
            } else if (event.key == Qt.Key_H) {
                console.log("Key bindings:")
                console.log("Zoom grid out:", "[Space]")
                console.log("Zoom grid in:", "Z")
                console.log("Show/hide axes:", "G")
                console.log("Perspective/orthographic camera:", "O")
                console.log("Change near plane for orthographic camera:", "N")
                console.log("Dark mode/light mode:", "B")
                console.log("Isometric projection:", "I")
                console.log("Align camera to x-axis:", "1")
                console.log("Align camera to y-axis:", "2")
                console.log("Align camera to z-axis:", "3")
                console.log("Far clip:", "[scroll wheel]")
                console.log("Near clip:", "[Shift] + [scroll wheel]")
                console.log("Orthographic magnification:", "[Ctrl] + [scroll wheel]")
            }
        }
        Keys.onReleased: (event)=> { if (keysEnabled) handleKeyRelease(event) }
    }

    WheelHandler{
        onWheel: (event)=> {

            if (event.modifiers === Qt.ControlModifier) {
                if (cameraNode.ortho) {
                    if (event.angleDelta.y > 0)
                        orthoCamera.mag *= 1.1
                    else
                        orthoCamera.mag /= 1.1
                    console.log("Ortho magnification:", orthoCamera.mag)
                }
            } else if (event.modifiers === Qt.ShiftModifier && !(cameraNode.ortho && cameraNode.orthoNegativeNear)) {
                if (event.angleDelta.y > 0)
                    cameraNode.clipNear *= 1.1
                else
                    cameraNode.clipNear /= 1.1
                console.log("near clip:", cameraNode.clipNear)
            } else {
                if (event.angleDelta.y > 0)
                    cameraNode.clipFar *= 1.1
                else
                    cameraNode.clipFar /= 1.1
                console.log("far clip:", cameraNode.clipFar)
            }
        }
    }
}
