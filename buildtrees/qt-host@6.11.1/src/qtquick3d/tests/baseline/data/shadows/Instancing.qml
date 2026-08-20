// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    visible: true
    width: 800
    height: 800

    View3D {
        id: viewport
        width: parent.width/2
        height: parent.height
        x:0
        y:0

        environment: SceneEnvironment {
            clearColor: "gray"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            z: 1000
            position: Qt.vector3d(0, -527.889, 2790.73)
        }

        DirectionalLight {
            position: Qt.vector3d(0, 800, 0)
            eulerRotation.x: -90
            color: Qt.rgba(1.0, 1.0, 0.1, 1.0)
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            ambientColor: "#777"
            shadowFactor: 90
            shadowMapFar: camera.clipFar
        }

        RandomInstancing {
            id: randomWithData
            instanceCount: 10

            position: InstanceRange {
                from: Qt.vector3d(-500, -400, -500)
                to: Qt.vector3d(1500, 400, 1500)
            }
            scale: InstanceRange {
                from: Qt.vector3d(1, 1, 1)
                to: Qt.vector3d(10, 1, 1)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            color: InstanceRange {
                from: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                to: Qt.rgba(1, 1, 1, 1.0)
            }
            // instancedMaterial custom data:  METALNESS, ROUGHNESS, FRESNEL_POWER, SPECULAR_AMOUNT
            customData: InstanceRange {
                from: Qt.vector4d(0, 0, 0, 0)
                to: Qt.vector4d(1, 1, 5, 1)
            }

            randomSeed: 89*780
        }

        Model {
            id: cubeModel
            source: "#Cube"
            instancing: randomWithData
            scale: Qt.vector3d(10, 1, 1)

            materials: DefaultMaterial {
                diffuseColor: "white"
            }

            PropertyAnimation on scale.y {
                from: 0.1
                to: 3
                duration: 200
                loops: 0
            }

            PropertyAnimation on scale.x {
                from: 1
                to: 3
                duration: 200
                loops: 0
            }
            Model {
                id: sphereModel
                source: "#Sphere"
                instancing: parent.instancing
                instanceRoot: parent

                property real scaleFactor: 0.5
                scale: Qt.vector3d(scaleFactor, scaleFactor, scaleFactor);
                position: Qt.vector3d(-50, -50, -50)

                SequentialAnimation on scaleFactor {
                    PauseAnimation {duration: 3000}
                    NumberAnimation {from: 0.5; to: 1.5; duration: 200}
                    NumberAnimation {from: 1.5; to: 0.5; duration: 200}
                    loops: -1
                }
            } // sphereModel
        } // cubeModel

        Model {
            id: floor
            source: "#Rectangle"
            y: -1500
            scale: Qt.vector3d(15, 15, 1)
            eulerRotation.x: -90
            materials: [
                DefaultMaterial {
                    diffuseColor: "pink"
                }
            ]
            castsShadows: false
        }
    } // View3D

    View3D {
        id: viewport1
        width: parent.width/2
        height: parent.height
        x:parent.width/2
        y:0

        environment: SceneEnvironment {
            clearColor: "gray"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            z: 1000
            position: Qt.vector3d(0, -527.889, 2790.73)
        }

        DirectionalLight {
            position: Qt.vector3d(0, 800, 0)
            eulerRotation.x: -90
            color: Qt.rgba(1.0, 1.0, 0.1, 1.0)
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
            ambientColor: "#777"
            shadowFactor: 90
            shadowMapFar: camera.clipFar
        }

        RandomInstancing {
            id: randomWithData1
            instanceCount: 10

            position: InstanceRange {
                from: Qt.vector3d(-500, -400, -500)
                to: Qt.vector3d(1500, 400, 1500)
            }
            scale: InstanceRange {
                from: Qt.vector3d(1, 1, 1)
                to: Qt.vector3d(10, 1, 1)
                proportional: true
            }
            rotation: InstanceRange {
                from: Qt.vector3d(0, 0, 0)
                to: Qt.vector3d(360, 360, 360)
            }
            color: InstanceRange {
                from: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                to: Qt.rgba(1, 1, 1, 1.0)
            }
            // instancedMaterial custom data:  METALNESS, ROUGHNESS, FRESNEL_POWER, SPECULAR_AMOUNT
            customData: InstanceRange {
                from: Qt.vector4d(0, 0, 0, 0)
                to: Qt.vector4d(1, 1, 5, 1)
            }

            randomSeed: 89*780

            shadowBoundsMinimum: Qt.vector3d(-1000, -1000, -1000)
            shadowBoundsMaximum: Qt.vector3d(0, 0, 0)
        }

        Model {
            source: "#Cube"
            instancing: randomWithData1
            scale: Qt.vector3d(10, 1, 1)

            materials: DefaultMaterial {
                diffuseColor: "white"
            }

            PropertyAnimation on scale.y {
                from: 0.1
                to: 3
                duration: 200
                loops: 0
            }

            PropertyAnimation on scale.x {
                from: 1
                to: 3
                duration: 200
                loops: 0
            }
            Model {
                source: "#Sphere"
                instancing: parent.instancing
                instanceRoot: parent

                property real scaleFactor: 0.5
                scale: Qt.vector3d(scaleFactor, scaleFactor, scaleFactor);
                position: Qt.vector3d(-50, -50, -50)

                SequentialAnimation on scaleFactor {
                    PauseAnimation {duration: 3000}
                    NumberAnimation {from: 0.5; to: 1.5; duration: 200}
                    NumberAnimation {from: 1.5; to: 0.5; duration: 200}
                    loops: -1
                }
            } // sphereModel
        } // cubeModel

        Model {
            source: "#Rectangle"
            y: -1500
            scale: Qt.vector3d(15, 15, 1)
            eulerRotation.x: -90
            materials: [
                DefaultMaterial {
                    diffuseColor: "pink"
                }
            ]
            castsShadows: false
        }
    } // View3D
}
