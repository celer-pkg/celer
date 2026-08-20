// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

Node {
    id: house
    property alias fire: fire

    // Resources
    Texture {
        id: wood_bc
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/wood_bc.jpg"
        scaleU: 2
        scaleV: 2
    }

    Texture {
        id: wood_n
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/wood_n.jpg"
        scaleU: 2
        scaleV: 2
    }

    Texture {
        id: wood_r
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/wood_r.jpg"
        scaleU: 2
        scaleV: 2
    }

    Texture {
        id: rock_bc
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/rock_bc.jpg"
        scaleU: 3
        scaleV: 3
    }

    Texture {
        id: rock_n
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/rock_n.jpg"
        scaleU: 3
        scaleV: 3
    }

    Texture {
        id: rock_r
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "media/textures/house/rock_r.jpg"
        scaleU: 3
        scaleV: 3
    }

    PrincipledMaterial {
        id: darkwood_material
        roughness: 1
        baseColorMap : wood_bc
        baseColor: "gray"
        normalMap : wood_n
        normalStrength: 3.
        roughnessMap : wood_r
    }

    PrincipledMaterial {
        id: lightwood_material
        roughness: 1
        baseColorMap : wood_bc
        baseColor: "white"
        normalMap : wood_n
        normalStrength: 3.
        roughnessMap : wood_r
    }

    PrincipledMaterial {
        id: rock_material
        roughness: 1
        objectName: "rock"
        baseColorMap : rock_bc
        baseColor: "white"
        normalMap : rock_n
        normalStrength: 3.
        roughnessMap : rock_r
    }

    Node {
        id: rootNode1
        objectName: "RootNode"

        StaticRigidBody {
            objectName: "HouseFloor"
            scale: Qt.vector3d(11.0, 0.1, 10.6)
            position: Qt.vector3d(0, 195, 0)
            collisionShapes: BoxShape {
            }

            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]

            }
        }

        StaticRigidBody {
            objectName: "HouseBackWall"
            scale: Qt.vector3d(10.8, 0.8, 20)
            position: Qt.vector3d(-500, 500, 0)
            eulerRotation: Qt.vector3d(-90, -90, 0)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            objectName: "HouseLeftWall"
            scale: Qt.vector3d(20, 1.3, 10.4)
            position: Qt.vector3d(0, 500, -500)
            eulerRotation: Qt.vector3d(0, -90, -90)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            objectName: "HouseRightWall"
            scale: Qt.vector3d(20, 1.6, 10.4)
            position: Qt.vector3d(0, 500, 520)
            eulerRotation: Qt.vector3d(0, -90, -90)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            scale: Qt.vector3d(10.8, 0.8, 20)
            position: Qt.vector3d(500, 1520, 0)
            eulerRotation: Qt.vector3d(-90, -90, 0)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            scale: Qt.vector3d(4.32, 0.8, 20)
            position: Qt.vector3d(500, 500, 310)
            eulerRotation: Qt.vector3d(-90, -90, 0)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            scale: Qt.vector3d(4.32, 0.8, 20)
            position: Qt.vector3d(500, 500, -310)
            eulerRotation: Qt.vector3d(-90, -90, 0)
            collisionShapes: BoxShape {

            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        StaticRigidBody {
            position: Qt.vector3d(470, 50 * scale.y, 0)
            scale: Qt.vector3d(1.5, 2.0, 3.7)
            collisionShapes: BoxShape {
            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }
        StaticRigidBody {
            position: Qt.vector3d(537, 50 * scale.y, 0)
            scale: Qt.vector3d(2, 1.44, 2.5)
            collisionShapes: BoxShape {
            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }
        StaticRigidBody {
            position: Qt.vector3d(605, 50 * scale.y, 0)
            scale: Qt.vector3d(2, 0.95, 2.5)
            collisionShapes: BoxShape {
            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }
        StaticRigidBody {
            position: Qt.vector3d(678, 50 * scale.y, 0)
            scale: Qt.vector3d(2, 0.40, 2.5)
            collisionShapes: BoxShape {
            }
            Model {
                pickable: true
                source: "#Cube"
                materials: [InvisibleMaterial]
            }
        }

        TriggerBody {
            position: Qt.vector3d(600, 270, 0)
            scale: Qt.vector3d(5, 5, 2.5)
            collisionShapes: BoxShape {}

            Model {
                source: "#Cube"
                materials: [InvisibleMaterial]
            }

            onBodyEntered: {
                door.angle = -45
            }

            onBodyExited: {
                door.angle = -180
            }
        }

        Model {
            id: construction_006
            objectName: "ceilling"
            position: Qt.vector3d(492.157, 378.451, -95.0073)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(43.9016, 43.9016, 43.9016)
            source: "media/meshes/house/cube_020_mesh.mesh"
            materials: [
                lightwood_material
            ]
        }

        Model {
            id: construction_005
            objectName: "floor"
            position: Qt.vector3d(492.157, 378.451, -95.0073)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(43.9016, 43.9016, 43.9016)
            source: "media/meshes/house/cube_019_mesh.mesh"
            materials: [
                lightwood_material
            ]
        }
        Model {
            id: construction_003
            objectName: "walls"
            position: Qt.vector3d(0, -122.331, 0)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(51.7539, 51.7539, 51.7539)
            source: "media/meshes/house/cube_016_mesh.mesh"
            materials: [
                lightwood_material
            ]
        }

        Node {
            id: door
            property real angle: -180
            Behavior on angle {
                NumberAnimation {
                    duration: 1000
                    easing.type: Easing.OutBack
                }
            }

            eulerRotation: Qt.vector3d(0, angle, 0)
            position: Qt.vector3d(492.157, 370, -95.0073)

            Node {
                scale: Qt.vector3d(0.2, 3.2, 2.2)
                position: Qt.vector3d(0, 0, -50 * scale.z)
                Model {
                    pickable: true
                    source: "#Cube"
                    materials: [InvisibleMaterial]
                }
            }
        }

        Model {
            objectName: "door"
            position: Qt.vector3d(492.157, 374.603, -95.0073)
            eulerRotation: Qt.vector3d(90, door.angle, 0)
            scale: Qt.vector3d(100, 100, 100)
            source: "media/meshes/house/cube_014_mesh.mesh"
            materials: [
                lightwood_material
            ]
        }
        Model {
            id: cube_007
            position: Qt.vector3d(0, -122.331, 0)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(100, 100, 100)
            objectName: "base"
            source: "media/meshes/house/cube_012_mesh.mesh"
            materials: [
                rock_material
            ]
        }
        Model {
            position: Qt.vector3d(0, -122.331, 0)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(100, 100, 100)
            id: cube_006
            objectName: "stairs"
            pickable: true
            source: "media/meshes/house/cube_007_mesh.mesh"
            materials: [
                darkwood_material
            ]
        }
        Model {
            id: cube_005
            objectName: "supports"
            position: Qt.vector3d(0, 77.6693, 2.84217e-13)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(100, 100, 100)
            source: "media/meshes/house/cube_006_mesh.mesh"
            materials: [
                darkwood_material
            ]
        }
        Model {
            id: cube_003
            objectName: "roof2"
            position: Qt.vector3d(-9.53674e-05, 77.6691, 2.55795e-12)
            rotation: Qt.quaternion(0.5, -0.5, 0.5, 0.5)
            scale: Qt.vector3d(40, 25, 50)
            source: "media/meshes/house/cube_010_mesh.mesh"
            materials: [
                darkwood_material
            ]
        }
        Model {
            id: cube_004
            objectName: "supports_roof"
            position: Qt.vector3d(0, 77.6693, 2.84217e-13)
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale: Qt.vector3d(100, 80, 100)
            source: "media/meshes/house/cube_004_mesh.mesh"
            materials: [
                darkwood_material
            ]
        }
        Fire {
            id: fire
            Component.onCompleted: {
                start()
            }
            StaticRigidBody {
                objectName: "Fire"
                scale: Qt.vector3d(0.5, 0.1, 0.5)
                y: 25
                sendTriggerReports: true
                collisionShapes: [
                    CapsuleShape {eulerRotation: Qt.vector3d(0, 0, -90)}
                ]
            }
            windstrength: 0
            baseSize: 1.
            scale: Qt.vector3d(1.3, 2.0, 1.3)
            position: Qt.vector3d(0, 220, 520)
        }
        Smoke {
            size: 1.3
            position: Qt.vector3d(fire.x, fire.y + 720, fire.z)
        }
    }

    // Animations:
}
