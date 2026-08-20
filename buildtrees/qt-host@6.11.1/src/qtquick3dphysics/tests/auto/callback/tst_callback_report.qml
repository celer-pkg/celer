// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Tests the callback methods with different reporting properties set.
// 'reportKinematicKinematicCollisions' and 'reportStaticKinematicCollisions'
//
// 00) static vs static (statkin off) (no callback)
// 01) static vs static (statkin on) (no callback)
// 10) static vs kinematic (statkin off) (no callback)
// 11) static vs kinematic (statkin on) (callback)
// 20) kinematic vs kinematic (kinkin off) (no callback)
// 21) kinematic vs kinematic (kinkin on) (callback)

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
        return Qt.vector2d(j*testItem.sceneWidth, i*testItem.sceneHeight)
    }
    width: sceneWidth*3
    height: sceneHeight*2
    visible: true

    //////////////////////////////////////////////////
    // 00) static vs static (statkin off) (no callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 0).x
        y: xy(0, 0).y
        id: scene00
        property bool contact: false
        reportKinematicKinematicCollisions: false
        reportStaticKinematicCollisions: false

        StaticRigidBody {
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
            position: Qt.vector3d(0, Math.max(600 - scene00.elapsedTime, 0), 0)
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

    //////////////////////////////////////////////////
    // 01) static vs static (statkin on) (no callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(0, 1).x
        y: xy(0, 1).y
        id: scene01
        property bool contact: false
        reportKinematicKinematicCollisions: false
        reportStaticKinematicCollisions: true

        StaticRigidBody {
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
            position: Qt.vector3d(0, Math.max(600 - scene01.elapsedTime, 0), 0)
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

    //////////////////////////////////////////////////
    // 10) static vs kinematic (statkin off) (no callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 0).x
        y: xy(1, 0).y
        id: scene10
        property bool contact: false
        reportKinematicKinematicCollisions: false
        reportStaticKinematicCollisions: false

        StaticRigidBody {
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
            position: Qt.vector3d(0, Math.max(600 - scene10.elapsedTime, 0), 0)
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

    //////////////////////////////////////////////////
    // 11) static vs kinematic (statkin on) (callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(1, 1).x
        y: xy(1, 1).y
        id: scene11
        property bool contact: false
        reportKinematicKinematicCollisions: false
        reportStaticKinematicCollisions: true

        StaticRigidBody {
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
            position: Qt.vector3d(0, Math.max(600 - scene11.elapsedTime, 0), 0)
        }

        DynamicRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            isKinematic: true

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    //////////////////////////////////////////////////
    // 20) kinematic vs kinematic (kinkin off) (no callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 0).x
        y: xy(2, 0).y
        id: scene20
        property bool contact: false
        reportKinematicKinematicCollisions: false
        reportStaticKinematicCollisions: false

        DynamicRigidBody {
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene20.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            isKinematic: true
            kinematicPosition: Qt.vector3d(0, Math.max(600 - scene20.elapsedTime, 0), 0)
        }

        DynamicRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            isKinematic: true

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    //////////////////////////////////////////////////
    // 21) kinematic vs kinematic (kinkin on) (callback)
    PlaneScene {
        width: parent.sceneWidth
        height: parent.sceneHeight
        x: xy(2, 1).x
        y: xy(2, 1).y
        id: scene21
        property bool contact: false
        reportKinematicKinematicCollisions: true
        reportStaticKinematicCollisions: false

        DynamicRigidBody {
            collisionShapes: SphereShape {}
            receiveContactReports: true
            onBodyContact: () => {
                scene21.contact = true
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            isKinematic: true
            kinematicPosition: Qt.vector3d(0, Math.max(600 - scene21.elapsedTime, 0), 0)
        }

        DynamicRigidBody {
            position: Qt.vector3d(0, 0, 0)
            collisionShapes: SphereShape {}
            sendContactReports: true
            isKinematic: true

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "blue"
                }
            }
        }
    }

    TestCase {
        name: "00) static vs static (statkin off) (no callback)"
        when: !scene00.contact && scene00.elapsedTime > 2000
        function triggered() {}
    }

    TestCase {
        name: "01) static vs static (statkin on) (no callback)"
        when: !scene01.contact && scene01.elapsedTime > 2000
        function triggered() {}
    }

    TestCase {
        name: "10) static vs kinematic (statkin off) (no callback)"
        when: !scene10.contact && scene10.elapsedTime > 2000
        function triggered() {}
    }

    TestCase {
        name: "11) static vs kinematic (statkin on) (callback)"
        when: scene11.contact
        function triggered() {}
    }

    TestCase {
        name: "20) kinematic vs kinematic (kinkin off) (no callback)"
        when: !scene20.contact && scene20.elapsedTime > 2000
        function triggered() {}
    }

    TestCase {
        name: "21) kinematic vs kinematic (kinkin on) (callback)"
        when: scene21.contact
        function triggered() {}
    }
}

