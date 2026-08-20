// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

View3D {
    id: viewport
    property int numBouncesTop: 0
    property int numBouncesMiddle: 0
    property int numBouncesBottom: 0
    property int numBouncesFloor: 0
    property list<int> filterIgnoreGroups: []

    function setbits(numbers) {
        var result = 0;
        for (let i = 0; i < numbers.length; i++) {
            result = result | (1 << numbers[i])
        }
        return result
    }

    //! [environment]
    environment: SceneEnvironment {
        clearColor: "#d6dbdf"
        backgroundMode: SceneEnvironment.Color
    }
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 1000)
        clipFar: 2000
        clipNear: 1
    }
    DirectionalLight {
        eulerRotation.x: -45
        eulerRotation.y: 45
        castsShadow: false
        brightness: 1
        shadowFactor: 100
    }
    //! [environment]

    //! [sphere]
    DynamicRigidBody {
        id: sphere
        position: Qt.vector3d(0, 600, 0)
        sendContactReports: true
        collisionShapes: SphereShape {}
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "red"
            }
        }
        filterIgnoreGroups: setbits(viewport.filterIgnoreGroups)
    }
    //! [sphere]
    //! [box]
    StaticRigidBody {
        position: Qt.vector3d(0, 350, 0)
        scale: Qt.vector3d(3, 1, 3)
        collisionShapes: BoxShape {}
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(1, 0, 0, 0.2)
                alphaMode: PrincipledMaterial.Blend
            }
        }
        receiveContactReports: true
        onBodyContact: (body, positions, impulses, normals) => {
            viewport.numBouncesTop += 1;
        }
        filterGroup: 1
    }
    //! [box]
    //! [box]
    StaticRigidBody {
        position: Qt.vector3d(0, 200, 0)
        scale: Qt.vector3d(3, 1, 3)
        collisionShapes: BoxShape {}
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0, 1, 0, 0.2)
                alphaMode: PrincipledMaterial.Blend
            }
        }
        receiveContactReports: true
        onBodyContact: (body, positions, impulses, normals) => {
            viewport.numBouncesMiddle += 1;
        }
        filterGroup: 2
    }
    //! [box]
    //! [box]
    StaticRigidBody {
        position: Qt.vector3d(0, 50, 0)
        scale: Qt.vector3d(3, 1, 3)
        collisionShapes: BoxShape {}
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(0, 0, 1, 0.2)
                alphaMode: PrincipledMaterial.Blend
            }
        }
        receiveContactReports: true
        onBodyContact: (body, positions, impulses, normals) => {
            viewport.numBouncesBottom += 1;
        }
        filterGroup: 3
    }
    //! [box]
    //! [plane]
    StaticRigidBody {
        position: Qt.vector3d(0, -100, 0)
        eulerRotation: Qt.vector3d(-90, 0, 0)
        collisionShapes: PlaneShape {}
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(500, 500, 1)
            materials: PrincipledMaterial {
                baseColor: "green"
            }
            castsShadows: false
            receivesShadows: true
        }
        receiveContactReports: true
        onBodyContact: (body, positions, impulses, normals) => {
            viewport.numBouncesFloor += 1;
        }
        filterGroup: 4
    }
    //! [plane]
}
