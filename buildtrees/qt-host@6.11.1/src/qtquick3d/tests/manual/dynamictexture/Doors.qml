// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Node {
    id: doorRoot
    property PerspectiveCamera activeCamera: camera
    required property Item targetItem

    PointLight {
        id: lamp
        x: -10
        y: 100
        z: -100
        color: "#ffffffff"
        linearFade: 1
        brightness: 3
    }

    PerspectiveCamera {
        id: camera

        x: 180.067
        y: 225.598
        z: -411.521
        eulerRotation.x: -15.4614

        eulerRotation.y: 171.605

        fieldOfViewOrientation: PerspectiveCamera.Horizontal
    }
    //! [material]
    DefaultMaterial {
        id: doorMaterial
        diffuseMap: Texture {
            sourceItem: doorRoot.targetItem
        }
    }
    //! [material]

    //! [model heading]
    Model {
        id: door1
    //! [model heading]
        objectName: "left door"
        pivot.x: 20
        x: 80
        y: 70
        scale.x: 1.98
        scale.y: 3.5
        scale.z: 0.5

    //! [model content]
        source: "meshes/door1.mesh"
        materials: [ doorMaterial ]
        pickable: true
    //! [model content]

        //! [state]
        states: State {
            name: "opened"
            PropertyChanges {
                door1.eulerRotation.y: 90
            }
        }
        transitions: Transition {
            to: "opened"
            reversible: true
            SequentialAnimation {
                PropertyAnimation { property: "eulerRotation.y"; duration: 2000 }
            }
        }
        //! [state]
     }

    Model {
        id: wall
        y: 100
        scale.x: 400
        scale.y: 100
        scale.z: 10
        source: "meshes/wall.mesh"

        DefaultMaterial {
            id: wallMaterial
            diffuseColor: "lightgreen"
        }
        materials: [
            wallMaterial
        ]
    }

    Model {
        id: door2
        objectName: "right door"
        x: -80
        y: 70
        scale.x: 1.98
        scale.y: 3.5
        scale.z: 0.5
        pivot.x: -20
        source: "meshes/door2.mesh"
        pickable: true
        materials: [ doorMaterial ]

        states: State {
            name: "opened"
            PropertyChanges {
                door2.eulerRotation.y: -90
            }
        }
        transitions: Transition {
            to: "opened"
            reversible: true
            SequentialAnimation {
                PropertyAnimation { property: "eulerRotation.y"; duration: 2000 }
            }
        }
    }
}
