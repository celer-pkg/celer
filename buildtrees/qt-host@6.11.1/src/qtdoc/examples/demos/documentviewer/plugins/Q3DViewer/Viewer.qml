// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick3D.AssetUtils

Rectangle {
    id: window
    anchors.fill: parent
    visible: true
    color: "#848895"
    border.color: "black"
    property url fileName
    property bool isMesh
    property var myLoader

    onFileNameChanged: {
        if (myLoader) {
            myLoader.destroy()
            myLoader = undefined
        }

        if (isMesh) {
            displayModel.source = fileName
        } else {
            displayModel.source = ""
            myLoader = dynamicLoader.createObject(displayModel)
        }
    }

    View3D {
        anchors.fill: parent
        camera: camera

        DirectionalLight {
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
            brightness: 1.0
            eulerRotation.x: -25
        }

        Model {
            id: displayModel
            scale: Qt.vector3d(1, 1, 1)
            materials: [
                PrincipledMaterial {
                    baseColor: "#41cd52"
                    metalness: 0.0
                    roughness: 0.1
                    opacity: 1.0
                }
            ]

            function handleBoundsChanged(bounds) {
                let scaleF = 1/bounds.maximum.plus(bounds.minimum).length()
                scale = Qt.vector3d(scaleF, scaleF, scaleF)
                position = bounds.maximum.plus(bounds.minimum).times(-0.5 * scaleF)
                camera.z = bounds.maximum.minus(bounds.minimum).length() * scaleF / Math.tan(camera.fieldOfView / 180 * Math.PI)
            }

            Component {
                id: dynamicLoader
                RuntimeLoader {
                    id: loader
                    source: window.fileName
                    onBoundsChanged: { displayModel.handleBoundsChanged(loader.bounds) }
                }
            }

            onBoundsChanged: {
                if (window.isMesh) {
                    displayModel.handleBoundsChanged(displayModel.bounds)
                }
            }
        }

        Node {
            id: cameraNode
            property real dragScale: - (camera.z - 0.5) * Math.tan(camera.fieldOfView / 180 * Math.PI)
            PerspectiveCamera {
                id: camera
                clipNear: 0.0001
                clipFar: 1000
                position: Qt.vector3d(0.0, 0.0, 10)
                fieldOfView: 20
            }
        }
    }

    WheelHandler {
        target: null
        onWheel: event => camera.z -= camera.z * event.angleDelta.y * 0.0005;
    }

    DragHandler {
        id: orbitDrag
        target: null
        xAxis.onActiveValueChanged: delta => cameraNode.eulerRotation.y += delta * cameraNode.dragScale
        yAxis.onActiveValueChanged: delta => cameraNode.eulerRotation.x += delta * cameraNode.dragScale
    }
}
