// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: node
    property alias isOn: fire.isOn
    property alias fireColor: fire.fireColor
    property alias lightColor: fire.lightColor
    function startFire() {
        fire.life = 1
    }
    function stopFire() {
        fire.life = 0
    }

    // Resources
    StaticRigidBody {
        y: 50
        collisionShapes: BoxShape {
        }
        Model {
            y: -35
            scale: Qt.vector3d(1.1, 1., 1.1)
            source: "#Cone"
            pickable: true
            materials: [InvisibleMaterial]
        }
    }

    Fire {
        id: fire
        Timer {
            running: fire.isOn
            interval: 60000 + Math.random() * 60000
            onTriggered: {
                fire.stop()
            }
        }
        scale: Qt.vector3d(1.6, 4., 1.6)
        StaticRigidBody {
            objectName: "Fire"
            scale: Qt.vector3d(0.5, 0.1, 0.5)
            y: 20
            sendTriggerReports: true
            collisionShapes: [
                CapsuleShape {eulerRotation: Qt.vector3d(0, 0, -90)}
            ]
        }
    }

    Smoke {
        visible: fire.isOn
        position: Qt.vector3d(0, 210, 0)
    }

    // Nodes:
    Node {
        eulerRotation: Qt.vector3d(-90, 0, 0)
        Node {
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            Node {
                id: rootNode
                objectName: "RootNode"
                y: 66
                Node {
                    position: Qt.vector3d(0, 135.393, 0)
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cylinder_low_campfire_0
                        objectName: "Cylinder_low_campfire_0"
                        source: "media/meshes/campfire/cylinder_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(-0.0740571, 19.4206, 69.8315)
                    rotation: Qt.quaternion(0.707098, -0.707098, -0.00352214, -0.00352214)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_low_campfire_0
                        objectName: "Cube_low_campfire_0"
                        source: "media/meshes/campfire/cube_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(0, -3.0181, 0)
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cylinder_002_low_campfire_0
                        objectName: "Cylinder.002_low_campfire_0"
                        source: "media/meshes/campfire/cylinder_002_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(0, -5.36013, 78.4066)
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_004_low_campfire_0
                        objectName: "Cube.004_low_campfire_0"
                        source: "media/meshes/campfire/cube_004_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(78.4066, -5.36013, -5.96046e-06)
                    rotation: Qt.quaternion(-0.5, 0.5, -0.5, -0.5)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_005_low_campfire_0
                        objectName: "Cube.005_low_campfire_0"
                        source: "media/meshes/campfire/cube_005_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(-1.19209e-05, -5.36013, -78.4066)
                    rotation: Qt.quaternion(-1.37679e-07, 1.37679e-07, 0.707107, 0.707107)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_006_low_campfire_0
                        objectName: "Cube.006_low_campfire_0"
                        source: "media/meshes/campfire/cube_006_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(-78.4066, -5.36013, 1.78814e-05)
                    rotation: Qt.quaternion(0.5, -0.5, -0.5, -0.5)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_007_low_campfire_0
                        objectName: "Cube.007_low_campfire_0"
                        source: "media/meshes/campfire/cube_007_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(55.845, 19.4206, 41.9258)
                    rotation: Qt.quaternion(0.633932, -0.633932, 0.313257, 0.313257)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_003_low_campfire_0
                        objectName: "Cube.003_low_campfire_0"
                        source: "media/meshes/campfire/cube_003_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(0, 135.393, 91.0465)
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube21_low_campfire_0
                        objectName: "Cube21_low_campfire_0"
                        source: "media/meshes/campfire/cube21_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(0, -5.54692, 84.0394)
                    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_0012_low_campfire_0
                        objectName: "Cube.0012_low_campfire_0"
                        source: "media/meshes/campfire/cube_0012_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(84.0394, -5.54692, -5.96046e-06)
                    rotation: Qt.quaternion(-0.5, 0.5, -0.5, -0.5)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_0022_low_campfire_0
                        objectName: "Cube.0022_low_campfire_0"
                        source: "media/meshes/campfire/cube_0022_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
                Node {
                    position: Qt.vector3d(91.0465, 135.393, -5.96046e-06)
                    rotation: Qt.quaternion(-0.5, 0.5, -0.5, -0.5)
                    scale: Qt.vector3d(100, 100, 100)
                    Model {
                        id: cube_0032_low_campfire_0
                        objectName: "Cube.0032_low_campfire_0"
                        source: "media/meshes/campfire/cube_0032_low_campfire_0_mesh.mesh"
                        materials: [
                            CampfireMaterial
                        ]
                    }
                }
            }
        }
    }

    // Animations:
}
