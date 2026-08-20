// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: childTransformCamera
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
            clipFar: 5000

            Model {
                id: sphere
                position: Qt.vector3d(0, 0, -203)
                source: "#Sphere"
                
                

                DefaultMaterial {
                    id: default_008
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }

                Model {
                    id: moon
                    position: Qt.vector3d(72.9504, 0, 0)
                    scale: Qt.vector3d(0.1, 0.1, 0.1)
                    source: "#Sphere"
                    
                    

                    DefaultMaterial {
                        id: default_009
                        lighting: DefaultMaterial.FragmentLighting
                        indexOfRefraction: 1.5
                        specularAmount: 0
                        specularRoughness: 0
                        bumpAmount: 0.5
                        translucentFalloff: 1
                    }
                    materials: [default_009]
                }
                materials: [default_008]
            }
        }

        DirectionalLight {
            id: light
            color: Qt.rgba(1, 1, 0.964706, 1)
            ambientColor: Qt.rgba(0.168627, 0.164706, 0.160784, 1)
            shadowFactor: 10
        }

        Node {
            id: arrowUp
            position: Qt.vector3d(-393.171, 121.883, 0)

            Model {
                id: cone
                source: "#Cone"
                
                

                DefaultMaterial {
                    id: default_
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(0, 1, 0, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_]
            }

            Model {
                id: cylinder
                position: Qt.vector3d(0, -97.5005, 0)
                scale: Qt.vector3d(0.5, 2, 0.5)
                source: "#Cylinder"
                
                

                DefaultMaterial {
                    id: default_001
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(0, 1, 0, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_001]
            }

            PerspectiveCamera {
                id: camera_001
                position: Qt.vector3d(0, 98.234, 0)
                rotation: Quaternion.fromEulerAngles(-26, -90, 0)
                clipFar: 5000
            }
        }

        Node {
            id: arrowForward
            position: Qt.vector3d(-138.558, 0, 0)
            rotation: Quaternion.fromEulerAngles(-90, 0, 0)

            Model {
                id: cone_001
                source: "#Cone"
                
                

                DefaultMaterial {
                    id: default_002
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(1, 0, 0, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_002]
            }

            Model {
                id: cylinder_001
                position: Qt.vector3d(0, -97.5005, 0)
                scale: Qt.vector3d(0.5, 2, 0.5)
                source: "#Cylinder"
                
                

                DefaultMaterial {
                    id: default_003
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(1, 0, 0, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_003]
            }
        }

        Node {
            id: arrowDown
            position: Qt.vector3d(91.1513, 0, 0)
            rotation: Quaternion.fromEulerAngles(-180, 0, 0)

            Model {
                id: cone_002
                source: "#Cone"
                
                

                DefaultMaterial {
                    id: default_004
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(0.333333, 1, 1, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_004]
            }

            Model {
                id: cylinder_002
                position: Qt.vector3d(0, -97.5005, 0)
                scale: Qt.vector3d(0.5, 2, 0.5)
                source: "#Cylinder"
                
                

                DefaultMaterial {
                    id: default_005
                    lighting: DefaultMaterial.FragmentLighting
                    diffuseColor: Qt.rgba(0.333333, 1, 1, 1)
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_005]
            }
        }

        Node {
            id: arrowBackwards
            position: Qt.vector3d(312.117, 0, 0)
            rotation: Quaternion.fromEulerAngles(90, 0, 0)

            Model {
                id: cone_003
                source: "#Cone"
                
                

                DefaultMaterial {
                    id: default_006
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_006]
            }

            Model {
                id: cylinder_003
                position: Qt.vector3d(0, -97.5005, 0)
                scale: Qt.vector3d(0.5, 2, 0.5)
                source: "#Cylinder"
                
                

                DefaultMaterial {
                    id: default_007
                    lighting: DefaultMaterial.FragmentLighting
                    indexOfRefraction: 1.5
                    specularAmount: 0
                    specularRoughness: 0
                    bumpAmount: 0.5
                    translucentFalloff: 1
                }
                materials: [default_007]
            }
        }

        DirectionalLight {
            id: light2
            rotation: Quaternion.fromEulerAngles(-180, -90, 0)
            color: Qt.rgba(1, 0.988235, 0.882353, 1)
            shadowFactor: 10
        }
    }
}
