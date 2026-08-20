// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Tests the callback methods.
//
// 00) dynamic vs static
// 01) dynamic vs kinematic
// 02) dynamic vs character
// 03) dynamic vs dynamic
//
// 10) character vs dynamic (no callback)
// 11) character vs static (no callback)
// 12) character vs kinematic (no callback)
// 13) character vs character (no callback)
//
// 20) character vs dynamic (onShapeHit)
// 21) character vs static (onShapeHit)
// 22) character vs kinematic (onShapeHit)
// 23) character vs character (onShapeHit no callback)
//
// 30) kinematic vs static (no callback)
// 31) kinematic vs kinematic (no callback)
// 32) kinematic vs character (no callback)
// 33) kinematic vs dynamic

import QtCore
import QtTest
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics
import QtQuick

Item {
    id: testItem
    property real sceneWidth: 200
    property real sceneHeight: 200

    function xy(i, j) {
        return [j*testItem.sceneWidth, i*testItem.sceneHeight]
    }
    width: sceneWidth*4
    height: sceneHeight*4
    visible: true

    //////////////////////////////////////////////////
    // 00 dynamic vs static

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 0)[0]
        y: xy(0, 0)[1]
        id: scene00
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene00.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "dynamic vs static"
        when: scene00.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 01 dynamic vs kinematic

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 1)[0]
        y: xy(0, 1)[1]
        id: scene01
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene01.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        DynamicRigidBody {
            isKinematic: true
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "dynamic vs kinematic"
        when: scene01.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 02 dynamic vs character

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 2)[0]
        y: xy(0, 2)[1]
        id: scene02
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene02.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 50, 0)
            sendContactReports: true
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "dynamic vs character"
        when: scene02.contact
        function triggered() {}
    }


    //////////////////////////////////////////////////
    // 03 dynamic vs dynamic

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 3)[0]
        y: xy(0, 3)[1]
        id: scene03
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene03.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        DynamicRigidBody {
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "dynamic vs dynamic"
        when: scene03.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 10 character vs dynamic (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 0)[0]
        y: xy(1, 0)[1]
        id: scene10
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene10.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            sendContactReports: true
            gravity: Qt.vector3d(0, -982, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs dynamic (no callback)"
        when: scene10.elapsedTime > 2000 && !scene10.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 11 character vs static (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 1)[0]
        y: xy(1, 1)[1]
        id: scene11
        property bool contact: false

        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene11.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            sendContactReports: true
            gravity: Qt.vector3d(0, -982, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character controller vs static (no callback)"
        when: scene11.elapsedTime > 2000 && !scene11.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 12 character vs kinematic (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 2)[0]
        y: xy(1, 2)[1]
        id: scene12
        property bool contact: false

        DynamicRigidBody {
            isKinematic: true
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene12.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            sendContactReports: true
            gravity: Qt.vector3d(0, -982, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs kinematic (no callback)"
        when: scene12.elapsedTime > 2000 && !scene12.contact
        function triggered() {}
    }


    //////////////////////////////////////////////////
    // 13 character vs character (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 3)[0]
        y: xy(1, 3)[1]
        id: scene13
        property bool contact: false

        CharacterController {
            position: Qt.vector3d(0, 0, 0)
            receiveContactReports: true
            onBodyContact: () => {
                scene13.contact = true
            }
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            collisionShapes: CapsuleShape {}
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            sendContactReports: true
            gravity: Qt.vector3d(0, -982, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs character (no callback)"
        when: scene13.elapsedTime > 2000 && !scene13.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 20 character vs dynamic (onShapeHit)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 0)[0]
        y: xy(2, 0)[1]
        id: scene20
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            gravity: Qt.vector3d(0, -982, 0)
            onShapeHit: () => {
                scene20.contact = true
            }
            enableShapeHitCallback: true

            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs dynamic (onShapeHit)"
        when: scene20.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 21 character vs static (onShapeHit)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 1)[0]
        y: xy(2, 1)[1]
        id: scene21
        property bool contact: false

        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            sendContactReports: true
            gravity: Qt.vector3d(0, -982, 0)
            onShapeHit: () => {
                scene21.contact = true
            }
            enableShapeHitCallback: true
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character controller vs static (onShapeHit)"
        when: scene21.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 22 character vs kinematic (onShapeHit)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 2)[0]
        y: xy(2, 2)[1]
        id: scene22
        property bool contact: false

        DynamicRigidBody {
            isKinematic: true
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            gravity: Qt.vector3d(0, -982, 0)
            onShapeHit: () => {
                scene22.contact = true
            }
            enableShapeHitCallback: true
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs kinematic (onShapeHit)"
        when: scene22.contact
        function triggered() {}
    }


    //////////////////////////////////////////////////
    // 23 character vs character (onShapeHit no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 3)[0]
        y: xy(2, 3)[1]
        id: scene23
        property bool contact: false

        CharacterController {
            position: Qt.vector3d(0, 0, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            collisionShapes: CapsuleShape {}
        }

        CharacterController {
            position: Qt.vector3d(0, 1000, 0)
            gravity: Qt.vector3d(0, -982, 0)
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            onShapeHit: () => {
                scene23.contact = true
            }
            enableShapeHitCallback: true
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "character vs character (onShapeHit no callback)"
        when: scene23.elapsedTime > 2000 && !scene23.contact
        function triggered() {}
    }


    //////////////////////////////////////////////////
    // 30 kinematic vs static (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(3, 0)[0]
        y: xy(3, 0)[1]
        id: scene30
        property bool contact: false

        DynamicRigidBody {
            id: scene30Sphere
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene30.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }

            isKinematic: true
            kinematicPosition: Qt.vector3d(0, Math.max(600 - scene30.elapsedTime, 0), 0)
        }

        StaticRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "kinematic vs static (no callback)"
        when: !scene30.contact && scene30.elapsedTime > 2000
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 31 kinematic vs kinematic (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(3, 1)[0]
        y: xy(3, 1)[1]
        id: scene31
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene31.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }

            isKinematic: true
            kinematicPosition: Qt.vector3d(0, Math.max(600 - scene30.elapsedTime, 0), 0)
        }

        DynamicRigidBody {
            isKinematic: true
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "kinematic vs kinematic (no callback)"
        when: !scene31.contact && scene31.elapsedTime > 2000
        function triggered() {}
    }

    //////////////////////////////////////////////////
    // 32 kinematic vs character (no callback)

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(3, 2)[0]
        y: xy(3, 2)[1]
        id: scene32
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene32.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }

            isKinematic: true
            kinematicPosition: Qt.vector3d(0, Math.max(600 - scene30.elapsedTime, 0), 0)
        }

        CharacterController {
            position: Qt.vector3d(0, 50, 0)
            sendContactReports: true
            Model {
                eulerRotation.z: 90
                geometry: CapsuleGeometry {}
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
            collisionShapes: CapsuleShape {}
        }
    }

    TestCase {
        name: "kinematic vs character (no callback)"
        when: !scene32.contact && scene32.elapsedTime > 2000
        function triggered() {}
    }


    //////////////////////////////////////////////////
    // 33 kinematic vs dynamic

    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(3, 3)[0]
        y: xy(3, 3)[1]
        id: scene33
        property bool contact: false

        DynamicRigidBody {
            position: Qt.vector3d(0, 600, 0)
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene33.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
        }

        DynamicRigidBody {
            position: Qt.vector3d(0, 100, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "kinematic vs dynamic"
        when: scene33.contact
        function triggered() {}
    }

    //////////////////////////////////////////////////
}

