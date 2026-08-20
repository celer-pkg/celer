// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics

DynamicRigidBody {
    id: torch
    objectName: "Torch"

    required property Fire houseFire
    property bool windEnabled: false
    property vector3d globalWindDir: Qt.vector3d(0.5, 0, -0.5)
    property Node attachedTo: null
    property alias isOn: fire.isOn
    function startFire() {
        fire.life = 1
    }
    function stopFire() {
        fire.life = 0
    }

    physicsMaterial: PhysicsMaterial {
        restitution: 0.1
        dynamicFriction: 10.0
        staticFriction: 10.0
    }

    FrameAnimation {
        running: torch.attachedTo !== null
        onRunningChanged: {
            if (running) {
                torch.collisionShapes = null
                torch.simulationEnabled = false
                torch.pivot = Qt.vector3d(0, 10, 0)
            }else {
                torch.collisionShapes = torch.shapes
                torch.pivot = Qt.vector3d(0, 0, 0)
                torch.simulationEnabled = true
            }
        }

        onTriggered: {
            if (torch.attachedTo) {
                torch.position = torch.attachedTo.scenePosition
                torch.rotation = torch.attachedTo.sceneRotation
                torch.reset(torch.position, torch.rotation.toEulerAngles())
            }
        }
    }
    // Resources
    Texture {
        id: torch_bc
        source: "media/textures/torch/torch_bc.jpg"
    }
    Texture {
        id: torch_n
        source: "media/textures/torch/torch_n.jpg"
    }
    Texture {
        id: torch_r
        source: "media/textures/torch/torch_r.jpg"
    }
    PrincipledMaterial {
        id: torch_material
        baseColor: "#ffcccccc"
        baseColorMap: torch_bc
        normalMap: torch_n
        roughnessMap: torch_r
        roughness: 1
    }

    property list<CollisionShape> shapes: [CapsuleShape {
            eulerRotation: Qt.vector3d(0, 0, -90)
            position: Qt.vector3d(0, 25, 0)
            diameter: 5
            height: 40
        },
        SphereShape {
            diameter: 20
            y: 60
        }
    ]

    collisionShapes: shapes

    sendTriggerReports: true

    Node {
        position: Qt.vector3d(0, 50, 0)

        TriggerBody {
            scale: Qt.vector3d(0.05, 0.05, 0.05)
            collisionShapes: [SphereShape{}]
            onBodyEntered: (body)=>{
                               if (body.objectName === "Fire") {
                                   if (torch.houseFire.isOn)
                                   fire.start()
                                   else if (fire.isOn)
                                   torch.houseFire.start()


                               }
                           }
        }
        Fire {
            id: fire
            Timer {
                running: fire.isOn
                interval: 30000 + Math.random() * 30000
                onTriggered: {
                    fire.stop()
                }
            }
            FrameAnimation{
                running: true
                property vector3d lastPosition: Qt.vector3d(0,0,0)
                onTriggered: {

                    let mappedDirVelocity = fire.mapDirectionFromScene(lastPosition.minus(fire.scenePosition))
                    let mappedDirVelocityLen = mappedDirVelocity.length() * 0.4
                    mappedDirVelocity = mappedDirVelocity.normalized()
                    let dir2DVelocity = Qt.vector2d(-mappedDirVelocity.z, mappedDirVelocity.x).normalized()
                    dir2DVelocity = dir2DVelocity.times((1.0 - Math.max(0, mappedDirVelocity.y)))

                    lastPosition = fire.scenePosition

                    fire.windstrength = 1.5

                    fire.windDir = dir2DVelocity.times(Math.min(mappedDirVelocityLen, 1.0))

                    if (torch.windEnabled) {
                        let mappedDirWind = fire.mapDirectionFromScene(torch.globalWindDir)
                        let dir2DWind = Qt.vector2d(-mappedDirWind.z, mappedDirWind.x).normalized()
                        dir2DWind = dir2DWind.times((1.0 - Math.max(0, mappedDirWind.y)))

                        fire.windDir = fire.windDir.plus(dir2DWind)
                    }
                    let mappedDirUp = fire.mapDirectionFromScene(Qt.vector3d(0, 1.0, 0))
                    let dir2DUp = Qt.vector2d(-mappedDirUp.z, mappedDirUp.x).normalized()
                    dir2DUp = dir2DUp.times((1.0 - Math.max(0, mappedDirUp.y)))

                    fire.windDir = fire.windDir.plus(dir2DUp)

                    fire.windDir = Qt.vector2d( Math.min(fire.windDir.x, 1.0), Math.min(fire.windDir.y, 1.0))

                }
            }

            scale: Qt.vector3d(0.4, 1.0, 0.4)
        }
        Node {
            onSceneRotationChanged: {
                smoke.rotation = sceneRotation.inverted()
            }

            Smoke {
                id: smoke
                visible: fire.isOn
                size: 0.5
                position: Qt.vector3d(0, 65, 0)
            }
        }

    }
    Node {
        id: sketchfab_model
        rotation: Qt.quaternion(1.94707e-07, 1, 0, 0)
        scale: Qt.vector3d(1, 1, 1)
        position: Qt.vector3d(0, 25, 0)
        Node {
            Model {
                source: "media/meshes/torch/object_1_mesh.mesh"
                materials: [
                    torch_material
                ]
            }
        }
    }
}
