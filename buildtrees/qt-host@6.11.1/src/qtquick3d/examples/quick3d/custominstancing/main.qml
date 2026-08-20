// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import InstancingExample

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#8fafff"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: cameraNode
            PerspectiveCamera {
                property vector3d basePos: Qt.vector3d(0, 0, 750)
                property vector3d halfZoomPos: Qt.vector3d(0, -300, 400)
                property vector3d zoomPos: Qt.vector3d(0, -300, 100)
                property vector3d otherZoomPos: Qt.vector3d(-200, -300, 400)
                id: camera
                position: basePos
                eulerRotation.x: -20
            }
        }

        property int zoomDuration: 4000
        property int rotationDuration: 7000

        SequentialAnimation {
            running: true
            PauseAnimation { duration: 1000 }
            NumberAnimation {
                from: 0; to: 90
                target: cameraNode; property: "eulerRotation.y"
                duration: view.rotationDuration
            }

            PauseAnimation { duration: 500 }
            Vector3dAnimation {
                from: camera.basePos; to: camera.halfZoomPos
                target: camera; property: "position"
                duration: view.zoomDuration
            }
            Vector3dAnimation {
                from: camera.halfZoomPos; to: camera.zoomPos
                target: camera; property: "position"
                duration: view.zoomDuration
            }
            PauseAnimation { duration: 1000 }
            Vector3dAnimation {
                from: camera.zoomPos; to: camera.basePos
                target: camera; property: "position"
                duration: view.zoomDuration
            }

            PauseAnimation { duration: 500 }
            NumberAnimation {
                from: 90; to: 180
                target: cameraNode; property: "eulerRotation.y"
                duration: view.rotationDuration
            }

            PauseAnimation { duration: 500 }
            Vector3dAnimation {
                from: camera.basePos; to: camera.halfZoomPos
                target: camera; property: "position"
                duration: view.zoomDuration
            }
            Vector3dAnimation {
                from: camera.halfZoomPos; to: camera.zoomPos
                target: camera; property: "position"
                duration: view.zoomDuration
            }
            PauseAnimation { duration: 1000 }
            Vector3dAnimation {
                from: camera.zoomPos; to: camera.basePos
                target: camera; property: "position"
                duration: view.zoomDuration
            }

            PauseAnimation { duration: 500 }
            NumberAnimation {
                from: 180; to: 360
                target: cameraNode; property: "eulerRotation.y"
                duration: view.rotationDuration
            }

            PauseAnimation { duration: 500 }
            Vector3dAnimation {
                from: camera.basePos; to: camera.otherZoomPos
                target: camera; property: "position"
                duration: view.zoomDuration
            }
            PauseAnimation { duration: 1000 }
            Vector3dAnimation {
                from: camera.otherZoomPos; to: camera.basePos
                target: camera; property: "position"
                duration: view.zoomDuration
            }

            loops: Animation.Infinite
        }

        DirectionalLight {
            eulerRotation.x: 250
            eulerRotation.y: -30
            brightness: 1.0
            ambientColor: "#7f7f7f"
        }

        //! [material]
        CustomMaterial {
            id: cubeMaterial
            property real uTime: frametimer.elapsedTime
            FrameAnimation {
                id: frametimer
                running: true
            }

            vertexShader: "cubeMaterial.vert"
            fragmentShader: "cubeMaterial.frag"
        }
        //! [material]

        //! [model]
        Model {
            id: instancedCube
            property real cubeSize: 15
            scale: Qt.vector3d(cubeSize/100, cubeSize/100, cubeSize/100)
            source: "#Cube"
            instancing: CppInstanceTable {
                gridSize: 65
                gridSpacing: instancedCube.cubeSize
                randomSeed: 1522562186
            }
            materials: [ cubeMaterial ]
        }
        //! [model]
    }
}
