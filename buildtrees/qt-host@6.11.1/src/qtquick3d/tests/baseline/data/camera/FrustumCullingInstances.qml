// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    width: 1280
    height: 720
    visible: true

    property int numColumns: 3
    property int numRows: 2

    function createView(col, row, posX, posY, posZ, rotX, rotY, rotZ, frustumEnabled, castShadows) {
        return Qt.createQmlObject(`
            import QtQuick
            import QtQuick3D
            import QtQuick3D.Helpers

            Item {
                // bind width/height/x/y to parent/window dynamically
                width: parent.width / ${numColumns}
                height: parent.height / ${numRows}
                x: parent.width / ${numColumns} * ${col}
                y: parent.height / ${numRows} * ${row}

                // --- 3D view ---
                View3D {
                    anchors.fill: parent
                    focus: true

                    environment: SceneEnvironment { clearColor: "black" }

                    PerspectiveCamera {
                        id: cam
                        position: Qt.vector3d(${posX}, ${posY}, ${posZ})
                        eulerRotation: Qt.vector3d(${rotX}, ${rotY}, ${rotZ})
                        frustumCullingEnabled: ${frustumEnabled}
                        clipFar: 5000
                    }

                    DirectionalLight {
                        eulerRotation.x: -30
                        eulerRotation.y: -70
                        castsShadow: ${castShadows}
                        ambientColor: Qt.rgba(0.5,0.5,0.5,1)
                    }

                    Node {
                        id: originGizmo
                        // X axis (red)
                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(1, 0.01, 0.01)
                            position: Qt.vector3d(2.5, 0, 0) // half-length offset
                            materials: PrincipledMaterial { baseColor: "red" }
                        }
                        // Y axis (green)
                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(0.01, 1, 0.01)
                            position: Qt.vector3d(0, 2.5, 0)
                            materials: PrincipledMaterial { baseColor: "green" }
                        }
                        // Z axis (blue)
                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(0.01, 0.01, 1)
                            position: Qt.vector3d(0, 0, 2.5)
                            materials: PrincipledMaterial { baseColor: "blue" }
                        }
                    }

                    Node {
                        RandomInstancing {
                            id: randomInstancing
                            instanceCount: 200
                            position: InstanceRange { from: Qt.vector3d(-300,-200,-500); to: Qt.vector3d(300,200,200) }
                            scale: InstanceRange { from: Qt.vector3d(5,5,5); to: Qt.vector3d(10,10,10); proportional: true }
                            rotation: InstanceRange { from: Qt.vector3d(0,0,0); to: Qt.vector3d(360,360,360) }
                            randomSeed: 42
                        }

                        Model {
                            source: "#Cube"
                            instancing: randomInstancing
                            scale: Qt.vector3d(0.01,0.01,0.01)
                            materials: PrincipledMaterial { baseColor: "green" }
                        }
                    }

                    WasdController { controlledObject: cam; speed: 1 }
                }

                // --- Overlay label ---
                Rectangle {
                    color: "white"
                    radius: 4
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.margins: 5
                    z: 1

                    Text {
                        anchors.fill: parent
                        anchors.margins: 4
                        color: "black"
                        font.pixelSize: 16
                        text: "Culling: " + (${frustumEnabled} ? "ON" : "OFF") +
                              ", Shadows: " + (${castShadows} ? "ON" : "OFF")
                    }
                }
            }
        `, window)
    }

    Component.onCompleted: {
        // Row 0: forward
        createView(0, 0, -144, 126, 192, -25, -36, 0, true, false)
        createView(1, 0, -144, 126, 192, -25, -36, 0, true, true)
        createView(2, 0, -144, 126, 192, -25, -36, 0, false, false)

        // Row 1: angled left
        createView(0, 1, -144, 126, 192, -26.7, 3, 0, true, false)
        createView(1, 1, -144, 126, 192, -26.7, 3, 0, true, true)
        createView(2, 1, -144, 126, 192, -26.7, 3, 0, false, false)
    }
}
