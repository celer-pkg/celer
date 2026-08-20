// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: viewport
    property int numBounces: 0

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
    }
    //! [plane]
    //! [sphere]
    DynamicRigidBody {
        id: sphere
        massMode: DynamicRigidBody.CustomDensity
        density: 0.00001
        position: Qt.vector3d(0, 600, 0)
        property bool inArea: false
        sendContactReports: true
        receiveTriggerReports: true
        onEnteredTriggerBody: {
            inArea = true;
        }
        onExitedTriggerBody: {
            inArea = false;
        }
        collisionShapes: SphereShape {}
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: sphere.inArea ? "yellow" : "red"
            }
        }
    }
    //! [sphere]
    //! [box]
    TriggerBody {
        position: Qt.vector3d(0, 350, 0)
        scale: Qt.vector3d(1, 2, 1)
        collisionShapes: BoxShape {
            id: boxShape
        }
        Model {
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: Qt.rgba(1, 0, 1, 0.2)
                alphaMode: PrincipledMaterial.Blend
            }
        }
    }
    //! [box]
    //! [impeller]
    StaticRigidBody {
        position: Qt.vector3d(0, 0, 0)
        scale: Qt.vector3d(2, 2, 2)
        receiveContactReports: true
        collisionShapes: SphereShape {}
        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "blue"
            }
        }
        onBodyContact: (body, positions, impulses, normals) => {
            for (var normal of normals) {
                let force = normal.times(-2000);
                body.applyCentralImpulse(force);
                viewport.numBounces += 1;
            }
        }
    }
    //! [impeller]
}
