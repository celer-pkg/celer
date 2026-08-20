// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: root
    anchors.fill: parent

    Timer {
        id: lightsUpdateTimer
        interval: 0
        triggeredOnStart: true
        onTriggered: root.updateLightsArray();
    }

    // Update the lights array of the particles
    function updateLightsArray() {
        var newLights = [];
        if (checkBoxDirectionalLightUse.checked)
            newLights.push(lightDirectional);
        if (checkBoxPointLightUse.checked)
            newLights.push(lightPoint);
        if (checkBoxSpotLightUse.checked)
            newLights.push(lightSpot);
        // Particles to use the enabled lights
        spriteParticle.lights = newLights; // qmllint disable read-only-property
    }

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 100, 700)
            eulerRotation.x: -10
        }

        DirectionalLight {
            id: lightDirectional
            color: Qt.rgba(1.0, 0.1, 0.1, 1.0)
            position: Qt.vector3d(0, 200, 0)
            rotation: Quaternion.fromEulerAngles(-70, 0, 0)
            visible: checkBoxDirectionalLight.checked
            brightness: sliderDirectionalLight.sliderValue
        }

        PointLight {
            id: lightPoint
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            position: Qt.vector3d(0, 300, 0)
            visible: checkBoxPointLight.checked
            brightness: sliderPointLight.sliderValue
            constantFade: sliderPointLightConstantFade.sliderValue
            linearFade: sliderPointLightLinearFade.sliderValue
            quadraticFade: sliderPointLightQuadraticFade.sliderValue
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 400
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    to: -400
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        SpotLight {
            id: lightSpot
            color: Qt.rgba(0.0, 0.0, 1.0, 1.0)
            position: Qt.vector3d(0, 250, 0)
            eulerRotation.x: -45
            visible: checkBoxSpotLight.checked
            brightness: sliderSpotLight.sliderValue
            coneAngle: 50
            innerConeAngle: 20
            constantFade: sliderSpotLightConstantFade.sliderValue
            linearFade: sliderSpotLightLinearFade.sliderValue
            quadraticFade: sliderSpotLightQuadraticFade.sliderValue
            PropertyAnimation on eulerRotation.y {
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 10000
            }
        }

        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(15, 15, 15)
            eulerRotation.x: -90
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                }
            ]
        }
        Model {
            source: "#Rectangle"
            z: -400
            scale: Qt.vector3d(15, 15, 15)
            materials: [
                PrincipledMaterial {
                    baseColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
                }
            ]
        }

        ParticleSystem3D {
            id: psystem

            startTime: 5000

            SpriteParticle3D {
                id: spriteParticle
                sprite: Texture {
                    source: "images/sphere.png"
                }
                maxAmount: 10000
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.5);
                fadeInDuration: 1000
                fadeOutDuration: 1000
                billboard: true
                // Disable this to see the unlit particles
                blendMode: SpriteParticle3D.Screen
            }

            ParticleEmitter3D {
                id: emitter
                particle: spriteParticle
                position: Qt.vector3d(0, 350, 0)
                depthBias: -100
                scale: Qt.vector3d(8.0, 0.0, 8.0)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                }
                particleScale: 2.0
                particleScaleVariation: 1.0;
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, -100, 0)
                    directionVariation: Qt.vector3d(20, 50, 20)
                }
                emitRate: 2000
                lifeSpan: 5000
            }

            PointRotator3D {
                pivotPoint: Qt.vector3d(0, 0, 0)
                direction: Qt.vector3d(0, 1, 0)
                magnitude: 20
            }
        }
    }

    SettingsView {
        CustomCheckBox {
            id: checkBoxDirectionalLight
            text: qsTr("Directional Light")
            checked: true
        }
        CustomSlider {
            id: sliderDirectionalLight
            sliderValue: 0.4
            fromValue: 0.0
            toValue: 1
        }
        CustomCheckBox {
            id: checkBoxDirectionalLightUse
            text: qsTr("Use also for particles")
            checked: true
            onCheckedChanged: lightsUpdateTimer.restart();
        }
        Item { width: 1; height: 40 }
        CustomCheckBox {
            id: checkBoxPointLight
            text: qsTr("Point Light")
            checked: true
        }
        CustomSlider {
            id: sliderPointLight
            sliderValue: 6
            fromValue: 0.0
            toValue: 10
        }
        CustomCheckBox {
            id: checkBoxPointLightUse
            text: qsTr("Use also for particles")
            checked: true
            onCheckedChanged: lightsUpdateTimer.restart();
        }
        CustomLabel {
            text: qsTr("constantFade")
        }
        CustomSlider {
            id: sliderPointLightConstantFade
            sliderValue: 1.0
            fromValue: 0.1
            toValue: 20.0
        }
        CustomLabel {
            text: qsTr("linearFade")
        }
        CustomSlider {
            id: sliderPointLightLinearFade
            sliderValue: 0.0
            fromValue: 0.0
            toValue: 20.0
        }
        CustomLabel {
            text: qsTr("quadraticFade")
        }
        CustomSlider {
            id: sliderPointLightQuadraticFade
            sliderValue: 1.0
            fromValue: 0.1
            toValue: 20.0
        }

        Item { width: 1; height: 40 }
        CustomCheckBox {
            id: checkBoxSpotLight
            text: qsTr("Spot Light")
            checked: true
        }
        CustomSlider {
            id: sliderSpotLight
            sliderValue: 40
            fromValue: 0.0
            toValue: 100
        }
        CustomCheckBox {
            id: checkBoxSpotLightUse
            text: qsTr("Use also for particles")
            checked: true
            onCheckedChanged: lightsUpdateTimer.restart();
        }
        CustomLabel {
            text: qsTr("constantFade")
        }
        CustomSlider {
            id: sliderSpotLightConstantFade
            sliderValue: 1.0
            fromValue: 0.1
            toValue: 20.0
        }
        CustomLabel {
            text: qsTr("linearFade")
        }
        CustomSlider {
            id: sliderSpotLightLinearFade
            sliderValue: 0.0
            fromValue: 0.0
            toValue: 20.0
        }
        CustomLabel {
            text: qsTr("quadraticFade")
        }
        CustomSlider {
            id: sliderSpotLightQuadraticFade
            sliderValue: 1.0
            fromValue: 0.1
            toValue: 20.0
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }

}
