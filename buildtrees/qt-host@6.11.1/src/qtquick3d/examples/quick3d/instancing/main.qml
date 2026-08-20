// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [import]
import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
//! [import]

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

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 300)
            clipNear: 1.0
            NumberAnimation on z {
                from: 300
                to: 0
                duration: 10 * 1000
            }
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
        }

        //! [randomInstancing]
        RandomInstancing {
            id: randomInstancing
            instanceCount: 1500

            position: InstanceRange {
                from: Qt.vector3d(-300, -200, -500)
                to: Qt.vector3d(300, 200, 200)
            }
            scale: InstanceRange {
                from: Qt.vector3d(1, 1, 1)
                to: Qt.vector3d(10, 10, 10)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            color: InstanceRange {
                from: "grey"
                to: "white"
                proportional: true
            }

            randomSeed: 2021
        }
        //! [randomInstancing]

        //! [manualInstancing]
        InstanceListEntry {
            id: redShip
            eulerRotation: Qt.vector3d(0, 180, 0)
            color: "red"
            PropertyAnimation on position {
                from: Qt.vector3d(50, 10, 100)
                to: Qt.vector3d(-70, 10, 100)
                duration: 8000
            }
        }

        InstanceListEntry {
            id: greenShip
            position: Qt.vector3d(0, 0, -60)
            eulerRotation: Qt.vector3d(-10, 0, 30)
            color: "green"
        }

        InstanceListEntry {
            id: blueShip
            position: Qt.vector3d(-100, -100, 0)
            color: "blue"
        }

        InstanceList {
            id: manualInstancing
            instances: [ redShip, greenShip, blueShip ]
        }
        //! [manualInstancing]

        //! [objects]
        Asteroid {
            instancing: randomInstancing
            NumberAnimation on eulerRotation.x {
                from: 0
                to: 360
                duration: 11000
                loops: Animation.Infinite
            }
        }

        SimpleSpaceship {
            instancing: manualInstancing
        }
        //! [objects]
    }
}
