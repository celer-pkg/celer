// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Tests that removing and adding objects with active contact callbacks
// does not crash. QTBUG-121033

import QtCore
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import QtQuick

Item {
    width: 800
    height: 600
    visible: true

    PhysicsWorld {
        scene: viewport.scene
    }

    View3D {
        id: viewport
        width: parent.width
        height: parent.height
        focus: true

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            backgroundMode: SceneEnvironment.Color
            clearColor: "#f0f0f0"
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-400, 500, 1000)
            eulerRotation: Qt.vector3d(-20, -20, 0)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-45, 45, 0)
        }

        Node {
            id: shapeSpawner
            property var instancesBoxes: []
            property var boxComponent: Qt.createComponent("Box.qml")
            property int numSpawns: 0

            function createStack() {
                let size = 10

                for (var i = 0; i < 3; i++) {
                    for (var j = 0; j < 3; j++) {
                        let center = Qt.vector3d(j*100, 100*i, 0)
                        let box = boxComponent.incubateObject(shapeSpawner, {
                                                                  "position": center,
                                                              })
                        instancesBoxes.push(box)
                    }
                }

                numSpawns = numSpawns + 1;
            }

            function reset() {
                // Only run method if previous stack has been created fully
                for (var i = 0; i < instancesBoxes.length; i++)
                    if (!instancesBoxes[i].object)
                        return

                instancesBoxes.forEach(box => {
                                           box.object.collisionShapes = []
                                           box.object.destroy()
                                       })
                instancesBoxes = []

                shapeSpawner.createStack()
            }
        }
    }

    FrameAnimation {
        property int frame: 0
        running: true
        onTriggered: {
            frame = frame + 1;
            if (frame % 2 == 0) {
                shapeSpawner.reset()
            }
        }
    }

    TestCase {
        name: "100 cycles"
        when: shapeSpawner.numSpawns > 100
        function triggered() {}
    }

}

