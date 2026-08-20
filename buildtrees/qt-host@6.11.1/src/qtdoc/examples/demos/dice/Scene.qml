// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers

Item {
    id: item
    property real settingGravity: 980.7
    property alias settingsStaticFriction: physicsMaterial.staticFriction
    property alias settingsDynamicFriction: physicsMaterial.dynamicFriction
    property alias settingsRestitution: physicsMaterial.restitution
    property alias settingsDiceWidth: diceSpawner.diceWidth
    property alias settingsDiceCount: diceSpawner.count
    property alias rollForce: diceSpawner.rollForce
    property alias cameraControllerEnabled : orbitCameraController.enabled

    function spawnDice() {
        diceSpawner.respawn()
    }

    Screen.onPrimaryOrientationChanged: {
        var orientation = Screen.primaryOrientation
        var isPortrait = orientation === Qt.PortraitOrientation
                || orientation === Qt.InvertedPortraitOrientation
        viewport.camera.position = Qt.vector3d(0, -20, isPortrait ? 100 : 60)
    }

    PhysicsWorld {
        id: physicsWorld
        running: true
        enableCCD: true
        scene: viewport.scene
        gravity: Qt.vector3d(0, -item.settingGravity, 0)
        typicalLength: 1
        typicalSpeed: 1000
        minimumTimestep: 15
        maximumTimestep: 20
    }

    PhysicsMaterial {
        id: physicsMaterial
        staticFriction: 0.5
        dynamicFriction: 0.5
        restitution: 0.5
    }

    OrbitCameraController {
        id: orbitCameraController
        anchors.fill: parent
        origin: originNode
        camera: camera
        panEnabled: false
    }

    View3D {
        id: viewport
        anchors.fill: parent
        camera: camera

        Node {
            id: originNode
            eulerRotation: Qt.vector3d(-14.2885, 410.287, 0)
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, -20, 100)
                clipFar: 1000
                clipNear: 0.1
            }
        }

        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: proceduralSky
        }

        Texture {
            id: proceduralSky
            textureData: ProceduralSkyTextureData {
                sunLongitude: -115
            }
        }

        Node {
            id: scene

            PointLight {
                y: 80
                x: 80
                z: 80
                castsShadow: true
                brightness: 1
                shadowFactor: 100
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                softShadowQuality: Light.PCF8
                shadowBias: 0.6
                pcfFactor: 0.2
                quadraticFade: 0.001
                shadowMapFar: 500
            }

            PhysicalTable {
                id: table
            }

            DiceSpawner {
                id: diceSpawner
                physicsMaterial: physicsMaterial
                diceWidth: 3.5
                rollForce: Qt.vector3d(0, 0, 0)
            }

            Carpet {
                position: Qt.vector3d(0, -50, 0)
            }

            // floor
            StaticRigidBody {
                position: Qt.vector3d(0, -50, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                collisionShapes: PlaneShape {}
            }
        }
    }
}
