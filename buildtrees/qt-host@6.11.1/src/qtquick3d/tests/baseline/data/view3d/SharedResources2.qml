// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

// NOTE: SharedResources.qml has an Item as its root item (This affects how items are initialized)
Window {
    width: 460
    height: 460
    visible: true

    // Hidden view
    View3D {
        id: leftView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width / 3
        visible: false

        Node {
            DirectionalLight {
            }

            PerspectiveCamera {
                z: 550
            }

            Model {
                source: "#Sphere"
                materials: [ theMaterial ]
            }
            PrincipledMaterial {
                id: theMaterial
                baseColor: "blue"
            }
        }
    }

    // Center view using material from hidden view and view that's initialized after this view (right view)
    View3D {
        id: centerView
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        x: parent.width / 3
        width: parent.width / 3

        Node {
            DirectionalLight {
                id: directionalLight
            }

            PerspectiveCamera {
                z: 550
            }

            // Center view should show two cubes, blue and red.
            Model {
                source: "#Cube"
                // Using material from hidden view.
                materials: [ theMaterial ]
                x: -50
            }
            Model {
                source: "#Cube"
                // Using material from the view that is initialized after this view.
                materials: [ theMaterial2 ]
                x: 50
            }
        }
    }

    View3D {
        id: rightView
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: parent.width / 3

        Node {
            DirectionalLight {
            }

            PerspectiveCamera {
                z: 550
            }

            Model {
                source: "#Cone"
                materials: [ theMaterial2 ]
            }
            PrincipledMaterial {
                id: theMaterial2
                baseColor: "red"
            }
        }
    }
}
