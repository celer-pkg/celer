// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D
import QtQuick3D.Helpers

Item {
    width: 800
    height: 600
    visible: true

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            //position: Qt.vector3d(0, 400, 600)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            clipFar: 5000
            clipNear: 1
            NumberAnimation on position.y { from: 400; to: 500; duration: 50 }
            NumberAnimation on position.z { from: 600; to: 1000; duration: 50 }
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 100
        }

        Node {
            position: Qt.vector3d(0, 0, 0)
            // Nodes:
            Node {
                scale: Qt.vector3d(100, 100, 100)
                id: root_object
                objectName: "ROOT"

                Model {
                    id: hood
                    objectName: "Hood"
                    y: 0.7891814112663269
                    z: 0.6023856401443481
                    source: "../shared/models/box.mesh"
                    materials: [carPaint_material23, plasticBlack_material24, chrome_material]
                }
            }

            Node {
                id: __materialLibrary__

                PrincipledMaterial {
                    id: chrome_material
                    objectName: "Chrome"
                    baseColor: "#ffffffff"
                    metalness: 1
                    roughness: 0.10000000149011612
                    cullMode: PrincipledMaterial.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    indexOfRefraction: 1.4500000476837158
                }

                PrincipledMaterial {
                    id: carPaint_material23
                    objectName: "CarPaint"
                    baseColor: "#ffffffff"
                    roughness: 0.10000000149011612
                    cullMode: PrincipledMaterial.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    clearcoatAmount: 1
                    clearcoatRoughnessAmount: 0.029999999329447746
                    indexOfRefraction: 1.4500000476837158
                }

                PrincipledMaterial {
                    id: plasticBlack_material24
                    objectName: "PlasticBlack"
                    baseColor: "#ffffffff"
                    metalness: 1
                    roughness: 0.30000001192092896
                    cullMode: PrincipledMaterial.NoCulling
                    alphaMode: PrincipledMaterial.Opaque
                    indexOfRefraction: 1.4500000476837158
                }
            }
        }

        Model {
            scale: Qt.vector3d(10, 0.1, 10)
            source: "#Cube"
            castsShadows: false
            materials: PrincipledMaterial {
                baseColor: "yellow"
            }
        }
    }

    WasdController {
        controlledObject: camera
    }

    //DebugView {
    //    source: viewport
    //}
}
