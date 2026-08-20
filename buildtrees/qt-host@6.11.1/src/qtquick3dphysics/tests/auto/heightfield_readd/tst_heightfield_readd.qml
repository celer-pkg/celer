// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

// Test loading a heightfield from both 'source' and 'image' property

Item {
    width: 640
    height: 480
    visible: true

    PhysicsWorld {
        scene: viewport.scene
        forceDebugDraw: true
        maximumTimestep: 15
        minimumTimestep: 15
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            position: Qt.vector3d(-200, 100, 500)
            eulerRotation: Qt.vector3d(-20, -20, 0)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: false
            brightness: 1
            shadowFactor: 100
        }

        DynamicRigidBody {
            id: dynamicBoxA
            property bool hit : false
            onBodyContact: () => {
                hit = true
            }
            receiveContactReports: true

            position: Qt.vector3d(-300, 300, 0)
            collisionShapes: BoxShape {}
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }

            onPositionChanged: frameAnimation.stepFrame()
        }

        StaticRigidBody {
            collisionShapes: HeightFieldShape {
                id: hf0
                image: Image {
                    source: "qrc:/data/hf.png"
                }
                position: Qt.vector3d(-300, -300, 0)
                extents: "400, 200, 400"
            }
            sendContactReports: true
        }

        DynamicRigidBody {
            id: dynamicBoxB
            property bool hit : false
            onBodyContact: () => {
                hit = true
            }
            receiveContactReports: true

            position: Qt.vector3d(300, 300, 0)
            collisionShapes: BoxShape {}
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "yellow"
                }
            }
        }

        StaticRigidBody {
            collisionShapes: HeightFieldShape {
                id: hf1
                source: "qrc:/data/hf.png"
                position: Qt.vector3d(300, -300, 0)
                extents: "400, 200, 400"
            }
            sendContactReports: true
        }
    }

    QtObject {
        id: frameAnimation
        property int frame: -1
        property var image: null

        function stepFrame() {
            frame += 1;
            let step = frame % 4;
            if (step == 0) {
                image = Qt.createQmlObject(`
                    import QtQuick
                    Image {
                        source: "qrc:/data/hf.png"
                    }
                    `,
                    frameAnimation,
                    "myImage"
                );
                hf0.image = image
                hf1.image = image
            } else if (step == 1) {
                hf0.image = null
                hf1.image = null
                image.destroy()
                image = null
            } else if (step == 2) {
                image = Qt.createQmlObject(`
                    import QtQuick
                    Image {
                        source: "qrc:/data/hf.png"
                    }
                    `,
                    frameAnimation,
                    "myImage"
                );
                hf0.image = image
                hf1.image = image
            } else if (step == 3) {
                hf0.image.destroy()
                hf1.image.destroy()
            }
        }
    }

    TestCase {
        name: "Box A"
        when: dynamicBoxA.hit
        function triggered() {  }
    }

    TestCase {
        name: "Box B"
        when: dynamicBoxB.hit
        function triggered() {  }
    }
}
