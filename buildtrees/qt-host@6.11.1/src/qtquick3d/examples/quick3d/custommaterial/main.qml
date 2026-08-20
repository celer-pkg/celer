// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "Custom Materials Example"

    View3D {
        id: v3d
        anchors.fill: parent

        camera: camera

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            probeExposure: 2
            lightProbe: Texture {
                source: "maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
            skyboxBlurAmount: 0.1
        }

        PerspectiveCamera {
            id: camera
            fieldOfView: 45
            position: Qt.vector3d(0, 100, 900)
        }

        SpotLight {
            position: Qt.vector3d(0, 500, 0)
            eulerRotation.x: -60
            color: Qt.rgba(1.0, 1.0, 0.1, 1.0)
            brightness: 50
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        property real globalRotation: 0

        NumberAnimation {
            target: v3d
            property: "globalRotation"
            from: 0
            to: 360
            duration: 27000
            running: true
            loops: Animation.Infinite
        }

        property real radius: 400
        property real separation: 360/5

        Model {
            id: floor
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(15, 15, 1)
            eulerRotation.x: -90
            materials: [
                PrincipledMaterial  {
                    baseColor: "white"
                }
            ]
        }

        // Start with a simple material, using the built-in implementation for everything.
        Node {
            eulerRotation.y: v3d.globalRotation
            //! [simple]
            Model {
                source: "weirdShape.mesh"
                scale: Qt.vector3d(100, 100, 100)
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                x: v3d.radius

                materials: [
                    CustomMaterial {
                        shadingMode: CustomMaterial.Shaded
                        fragmentShader: "material_simple.frag"
                        property color uDiffuse: "fuchsia"
                        property real uSpecular: 1.0
                    }
                ]
            }
            //! [simple]
        }

        // A metallic material using defaults for everything, except ambient light, and no uniforms.
        Node {
            eulerRotation.y: v3d.globalRotation + v3d.separation * 1
            Model {
                source: "weirdShape.mesh"
                scale: Qt.vector3d(100, 100, 100)
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                x: v3d.radius

                materials: [
                    CustomMaterial {
                        shadingMode: CustomMaterial.Shaded
                        fragmentShader: "material_metallic.frag"
                    }
                ]
            }
        }

        // A material with custom handling of all the lights, but still using
        // the built-in specular function.
        Node {
            eulerRotation.y: v3d.globalRotation + v3d.separation * 2
            Model {
                source: "weirdShape.mesh"
                scale: Qt.vector3d(100, 100, 100)
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                x: v3d.radius
                //! [custom lights]
                materials: [
                    CustomMaterial {
                        shadingMode: CustomMaterial.Shaded
                        fragmentShader: "material_customlights.frag"
                        property color uDiffuse: "orange"
                        property real uShininess: 150
                    }
                ]
                //! [custom lights]
            }
        }

        // Custom handling of everything, including specular.
        Node {
            eulerRotation.y: v3d.globalRotation  + v3d.separation * 3
            Model {
                source: "weirdShape.mesh"
                scale: Qt.vector3d(100, 100, 100)
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                x: v3d.radius
                materials: [
                    CustomMaterial {
                        shadingMode: CustomMaterial.Shaded
                        fragmentShader: "material_customspecular.frag"
                        property color uDiffuse: "green"
                        property real uShininess: 150
                    }
                ]
            }
        }

        // Custom lights, plus custom vertex shader
        Node {
            eulerRotation.y: v3d.globalRotation + v3d.separation * 4
            Model {
                source: "weirdShape.mesh"
                scale: Qt.vector3d(100, 100, 100)
                rotation: Quaternion.fromEulerAngles(-90, 0, 0)
                x: v3d.radius
                //! [custom vertex]
                materials: [
                    CustomMaterial {
                        id: material
                        shadingMode: CustomMaterial.Shaded
                        vertexShader: "material_distortion.vert"
                        fragmentShader: "material_customlights.frag"
                        property real uTime: 0.0
                        property real uAmplitude: 0.3
                        property color uDiffuse: "yellow"
                        property real uShininess: 50
                        NumberAnimation {
                            target: material
                            property: "uTime"
                            from: 0.0
                            to: 31.4
                            duration: 10000
                            loops: Animation.Infinite
                            running: true
                        }
                    }
                ]
                //! [custom vertex]
            }
        }

        // Transparent material, with a refractive effect
        //! [transparent]
        Model {
            id: screenSphere
            source: "#Sphere"
            scale: Qt.vector3d(0.75, 0.75, 0.75)
            y: 60
            z: 750;
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Shaded
                    fragmentShader: "material_transparent.frag"
                }
            ]
        //! [transparent]
            SequentialAnimation on x {
                NumberAnimation { from: 50; to: -50; duration: 20000 }
                NumberAnimation { from: -50; to: 50; duration: 20000 }
                loops: Animation.Infinite
            }
        }
    }
}
