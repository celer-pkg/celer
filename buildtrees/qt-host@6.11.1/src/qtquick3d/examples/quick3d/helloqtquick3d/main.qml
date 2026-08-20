// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Timeline
import QtQuick3D
import HelloExample

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: cameraNode

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 650)
            }

            property bool isRotating: false
            PropertyAnimation on eulerRotation.x {
                from: 0
                to: 360
                duration: 20000
                loops: Animation.Infinite
                running: cameraNode.isRotating
            }

            Timeline {
                id: timeline0
                startFrame: 0
                endFrame: 1000
                enabled: true

                SequentialAnimation on currentFrame {
                    NumberAnimation {
                        from: 0
                        to: 500
                        duration: 4000
                    }
                    NumberAnimation {
                        from: 500
                        to: 1000
                        duration: 10000
                        loops: Animation.Infinite
                    }
                }

                KeyframeGroup {
                    target: cameraNode
                    property: "isRotating"

                    Keyframe {
                        frame: 0
                        value: false
                    }
                    Keyframe {
                        frame: 500
                        value: true
                    }
                }

                KeyframeGroup {
                    target: oldLogo
                    property: "isRotating"

                    Keyframe {
                        frame: 0
                        value: false
                    }
                    Keyframe {
                        frame: 100
                        value: true
                    }
                }

                KeyframeGroup {
                    target: camera
                    property: "z"

                    Keyframe {
                        frame: 0
                        value: 2000
                    }
                    Keyframe {
                        frame: 100
                        value: 2000
                    }
                    Keyframe {
                        frame: 400
                        value: 100
                    }
                    Keyframe {
                        frame: 500
                        value: 200
                    }
                    Keyframe {
                        frame: 550
                        value: 200
                    }
                    Keyframe {
                        frame: 700
                        value: 1000
                        easing.type: Easing.OutElastic
                    }
                    Keyframe {
                        frame: 750
                        value: 1000
                    }
                    Keyframe {
                        frame: 800
                        value: 1000
                    }
                    Keyframe {
                        frame: 950
                        value: 200
                        easing.type: Easing.OutElastic
                    }
                    Keyframe {
                        frame: 1000
                        value: 200
                    }

                }
            }
        }

        DirectionalLight {
            eulerRotation.x: 250
            eulerRotation.y: -30
            brightness: 1.0
            ambientColor: "#7f7f7f"
        }

        Model {
            id: oldLogo
            property real s: 4.75
            property real r: 0
            property bool isRotating: false
            scale: Qt.vector3d(s, s, s);
            source: "oldqtlogo.mesh"
            instancing: ImageInstanceTable {
                gridSize: 40
                gridSpacing: 20
                image: ":/qt_logo.png"
            }
            materials: PrincipledMaterial { baseColor: "white" }
            NumberAnimation {
                target: oldLogo
                property: "r"
                running: oldLogo.isRotating
                from: 0
                to: 360
                duration: 3000
                loops: Animation.Infinite
            }
            onRChanged: rotate(2.5, Qt.vector3d(1, -1, 0), Node.LocalSpace)
        }
    }
}
