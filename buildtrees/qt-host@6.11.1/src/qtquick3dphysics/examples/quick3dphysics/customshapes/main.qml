// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

//! [window]
Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Qt Quick 3D Physics - Custom Shapes")

    //! [world]
    PhysicsWorld {
        id: physicsWorld
        running: true
        typicalLength: 2
        enableCCD: true
        maximumTimestep: 20
        scene: viewport.scene
    }
    //! [world]

    View3D {
        id: viewport
        anchors.fill: parent

        //! [environment]
        environment: SceneEnvironment {
            clearColor: "white"
            backgroundMode: SceneEnvironment.SkyBox
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            lightProbe: proceduralSky
        }
        //! [environment]

        //! [textures]
        Texture {
            id: proceduralSky
            textureData: ProceduralSkyTextureData {
                sunLongitude: -115
            }
        }

        Texture {
            id: weaveNormal
            source: "maps/weave.png"
            scaleU: 200
            scaleV: 200
            generateMipmaps: true
            mipFilter: Texture.Linear
        }

        Texture {
            id: numberNormal
            source: "maps/numbers-normal.png"
        }

        Texture {
            id: numberFill
            source: "maps/numbers.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }
        //! [textures]

        Node {
            //! [scene]
            id: scene
            scale: Qt.vector3d(2, 2, 2)
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(-45, 25, 60)
                eulerRotation: Qt.vector3d(-6, -33, 0)
                clipFar: 1000
                clipNear: 0.1
            }

            DirectionalLight {
                eulerRotation: Qt.vector3d(-45, 25, 0)
                castsShadow: true
                brightness: 1
                shadowMapQuality: Light.ShadowMapQualityHigh
                pcfFactor: 0.1
                shadowBias: 1
            }
            //! [scene]

            //! [cloth]
            StaticRigidBody {
                position: Qt.vector3d(-15, -8, 0)
                id: tablecloth

                Model {
                    geometry: HeightFieldGeometry {
                        id: tableclothGeometry
                        extents: Qt.vector3d(150, 20, 150)
                        source: "maps/cloth-heightmap.png"
                        smoothShading: false
                    }
                    materials: PrincipledMaterial {
                        baseColor: "#447722"
                        roughness: 0.8
                        normalMap: weaveNormal
                        normalStrength: 0.7
                    }
                }

                collisionShapes: HeightFieldShape {
                    id: hfShape
                    extents: tableclothGeometry.extents
                    source: "maps/cloth-heightmap.png"
                }
            }
            //! [cloth]

            //! [cup]
            DynamicRigidBody {
                id: diceCup
                isKinematic: true
                mass: 0
                property vector3d bottomPos: Qt.vector3d(11, 6, 0)
                property vector3d topPos: Qt.vector3d(11, 45, 0)
                property vector3d unloadPos: Qt.vector3d(0, 45, 0)
                position: bottomPos
                kinematicPivot: Qt.vector3d(0, 6, 0)
                kinematicPosition: bottomPos
                collisionShapes: TriangleMeshShape {
                    id: cupShape
                    source: "meshes/simpleCup.mesh"
                }
                Model {
                    source: "meshes/cup.mesh"
                    materials: PrincipledMaterial {
                        baseColor: "#cc9988"
                        roughness: 0.3
                        metalness: 1
                    }
                }
            }
            //! [cup]

            //! [tower]
            StaticRigidBody {
                id: diceTower
                x: -4
                Model {
                    id: testModel
                    source: "meshes/tower.mesh"
                    materials: [
                        PrincipledMaterial {
                            baseColor: "#ccccce"
                            roughness: 0.3
                        },
                        PrincipledMaterial {
                            id: glassMaterial
                            baseColor: "#aaaacc"
                            transmissionFactor: 0.95
                            thicknessFactor: 1
                            roughness: 0.05
                        }
                    ]
                }
                collisionShapes: TriangleMeshShape {
                    id: triShape
                    source: "meshes/tower.mesh"
                }
            }
            //! [tower]

            //! [dice]
            Component {
                id: diceComponent

                DynamicRigidBody {
                    id: thisBody
                    function randomInRange(min, max) {
                        return Math.random() * (max - min) + min
                    }

                    function restore() {
                        reset(initialPosition, eulerRotation)
                    }

                    scale: Qt.vector3d(scaleFactor, scaleFactor, scaleFactor)
                    eulerRotation: Qt.vector3d(randomInRange(0, 360),
                                               randomInRange(0, 360),
                                               randomInRange(0, 360))

                    property vector3d initialPosition: Qt.vector3d(11 + 1.5 * Math.cos(index/(Math.PI/4)),
                                                                   diceCup.bottomPos.y + index * 1.5,
                                                                   0)
                    position: initialPosition

                    property real scaleFactor: randomInRange(0.8, 1.4)
                    property color baseCol: Qt.hsla(randomInRange(0, 1),
                                                    randomInRange(0.6, 1.0),
                                                    randomInRange(0.4, 0.7),
                                                    1.0)

                    collisionShapes: ConvexMeshShape {
                        id: diceShape
                        source: Math.random() < 0.25 ? "meshes/icosahedron.mesh"
                              : Math.random() < 0.5 ? "meshes/dodecahedron.mesh"
                              : Math.random() < 0.75 ? "meshes/octahedron.mesh"
                                                     : "meshes/tetrahedron.mesh"
                    }

                    Model {
                        id: thisModel
                        source: diceShape.source
                        receivesShadows: false
                        materials: PrincipledMaterial {
                            metalness: 1.0
                            roughness: randomInRange(0.2, 0.6)
                            baseColor: baseCol
                            emissiveMap: numberFill
                            emissiveFactor: Qt.vector3d(1, 1, 1)
                            normalMap: numberNormal
                            normalStrength: 0.75
                        }
                    }
                }
            }

            Repeater3D {
                id: dicePool
                model: 25
                delegate: diceComponent
                function restore() {
                    for (var i = 0; i < count; i++) {
                        objectAt(i).restore()
                    }
                }
            }
            //! [dice]

            //! [animation]
            Connections {
                target: physicsWorld
                property real totalAnimationTime: 7500
                function onFrameDone(timeStep) {
                    let progressStep = timeStep / totalAnimationTime
                    animationController.progress += progressStep
                    if (animationController.progress >= 1) {
                        animationController.completeToEnd()
                        animationController.reload()
                        animationController.progress = 0
                    }
                }
            }

            AnimationController {
                id: animationController
                animation: SequentialAnimation {
                    PauseAnimation { duration: 2500 }
                    PropertyAnimation {
                        target: diceCup
                        property: "kinematicPosition"
                        to: diceCup.topPos
                        duration: 2500
                    }
                    ParallelAnimation {
                        PropertyAnimation {
                            target: diceCup
                            property: "kinematicEulerRotation.z"
                            to: 130
                            duration: 1500
                        }
                        PropertyAnimation {
                            target: diceCup
                            property: "kinematicPosition"
                            to: diceCup.unloadPos
                            duration: 1500
                        }
                    }
                    PauseAnimation { duration: 1000 }
                    ParallelAnimation {
                        PropertyAnimation {
                            target: diceCup
                            property: "kinematicEulerRotation.z"
                            to: 0
                            duration: 1500
                        }
                        PropertyAnimation {
                            target: diceCup
                            property: "kinematicPosition"
                            to: diceCup.topPos
                            duration: 1500
                        }
                    }
                    PropertyAnimation { target: diceCup; property: "kinematicPosition"; to: diceCup.bottomPos; duration: 1500 }
                    PauseAnimation { duration: 2000 }
                    ScriptAction { script: dicePool.restore() }
                }
            }
            //! [animation]
        } // scene
    } // View3D

    //! [controller]
    WasdController {
        keysEnabled: true
        controlledObject: camera
        speed: 0.2
    }
    //! [controller]
}
//! [window]
