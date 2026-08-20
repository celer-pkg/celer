// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick.Controls

Item {
    id: mainWindow

    readonly property int burstCount: 300
    readonly property int maxEmitterCount: 10
    readonly property int trailEmitRate: 10

    property var modelEmitters: []
    property int modelEmittersAmount: 0
    property var spriteEmitters: []
    property int spriteEmittersAmount: 0

    function createModelEmitter() {
        var newObject = modelEmitterComponent.createObject(psystem);
        modelEmitters.push(newObject);
        modelEmittersAmount = modelEmitters.length;
    }

    function createSpriteEmitter() {
        var newObject = spriteEmitterComponent.createObject(psystem);
        spriteEmitters.push(newObject);
        spriteEmittersAmount = spriteEmitters.length;
    }

    anchors.fill: parent
    Component.onCompleted: {
        createModelEmitter();
        createSpriteEmitter();
    }

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 100)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Model shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: PrincipledMaterial {
                }
            }
        }

        Component {
            id: modelEmitterComponent
            Model {
                source: "#Cylinder"
                materials: PrincipledMaterial {}
                position: Qt.vector3d(Math.random() * 250 - 300, -150, 0)
                scale: Qt.vector3d(0.5, 0.1, 0.5)
                ParticleEmitter3D {
                    system: psystem
                    particle: particleWhite
                    particleScaleVariation: 0.4
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 400, 0)
                        directionVariation: Qt.vector3d(40, 40, 0)
                    }
                    emitRate: 2
                    lifeSpan: 4000
                }
            }
        }

        Component {
            id: spriteEmitterComponent
            Model {
                source: "#Cylinder"
                materials: PrincipledMaterial { baseColor: "#ffff00" }
                position: Qt.vector3d(Math.random() * 250 + 50, -150, 0)
                scale: Qt.vector3d(0.5, 0.1, 0.5)
                ParticleEmitter3D {
                    system: psystem
                    particle: particleSprite
                    particleScaleVariation: 0.4
                    particleRotationVariation: Qt.vector3d(180, 180, 180)
                    particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                    velocity: VectorDirection3D {
                        direction: Qt.vector3d(0, 400, 0)
                        directionVariation: Qt.vector3d(40, 40, 0)
                    }
                    emitRate: 2
                    lifeSpan: 4000
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            useRandomSeed: checkBoxRandomize.checked

            onTimeChanged: {
                if (time > 9500)
                    psystem.paused = true;
            }

            // Particles
            ModelParticle3D {
                id: particleTrailModel
                delegate: particleComponent
                maxAmount: mainWindow.maxEmitterCount * 8 * mainWindow.trailEmitRate
                fadeInDuration: 200
                fadeOutDuration: 500
                color: "#808080"
                colorVariation: Qt.vector4d(0.2, 0.2, 0.2, 0.5)
                unifiedColorVariation: true
            }
            ModelParticle3D {
                id: particleWhite
                delegate: particleComponent
                maxAmount: mainWindow.maxEmitterCount * 8
                color: "#ffffff"
            }
            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: mainWindow.burstCount * 3
                color: "#ff0000"
            }
            SpriteParticle3D {
                id: particleSprite
                sprite: Texture {
                    source: "images/star2.png"
                }
                maxAmount: mainWindow.maxEmitterCount * 8
                color: "#ffff00"
                particleScale: 30.0
            }
            SpriteParticle3D {
                id: particleTrailSprite
                sprite: Texture {
                    source: "images/star2.png"
                }
                maxAmount: mainWindow.maxEmitterCount * 8 * mainWindow.trailEmitRate
                fadeInDuration: 200
                fadeOutDuration: 500
                color: "#999900"
                particleScale: 15.0
            }

            // Emitters, one per particle
            TrailEmitter3D {
                id: modelTrailEmitter
                particle: particleTrailModel
                follow: particleWhite
                particleScale: 0.5
                particleScaleVariation: 0.2
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(100, 100, 100);
                velocity: VectorDirection3D {
                    directionVariation: Qt.vector3d(20, 20, 20)
                }
                emitRate: mainWindow.trailEmitRate
                lifeSpan: 1000
            }
            TrailEmitter3D {
                id: spriteTrailEmitter
                particle: particleTrailSprite
                follow: particleSprite
                particleScaleVariation: 0.2
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(100, 100, 100);
                velocity: VectorDirection3D {
                    directionVariation: Qt.vector3d(20, 20, 20)
                }
                emitRate: mainWindow.trailEmitRate
                lifeSpan: 1000
            }

            ParticleEmitter3D {
                id: burstEmitter
                particle: particleRed
                scale: Qt.vector3d(0.5, 0.5, 0.5)
                particleScale: 0.2
                particleEndScale: 0.4
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: false
                }
                velocity: TargetDirection3D {
                    position: burstEmitter.position
                    magnitude: -4.0
                }
                lifeSpan: 1000
                lifeSpanVariation: 500
            }

            Gravity3D {
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -200
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            var pos = view3D.mapTo3DScene(Qt.vector3d(mouseX, mouseY, camera.z));
            burstEmitter.setPosition(pos);
            burstEmitter.burst(mainWindow.burstCount);
        }
    }

    SettingsView {
        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: psystem.running ? qsTr("Stop") : qsTr("Start")
                font.pointSize: AppSettings.fontSizeSmall
                onClicked: {
                    psystem.running = !psystem.running;
                }
            }
            Button {
                text: psystem.paused ? qsTr("Continue") : qsTr("Pause")
                font.pointSize: AppSettings.fontSizeSmall
                enabled: psystem.running
                onClicked: {
                    psystem.paused = !psystem.paused;
                }
            }
        }
        Item {
            width: 1
            height: 10
        }
        CustomLabel {
            text: "ParticleSystem time"
            opacity: timeSlider.sliderEnabled ? 1.0 : 0.4
        }
        CustomSlider {
            id: timeSlider
            sliderValue: psystem.time
            sliderEnabled: psystem.paused
            fromValue: 0
            toValue: 10000
            onSliderValueChanged: psystem.setTime(sliderValue);
        }
        Item {
            width: 1
            height: 10
        }
        CustomLabel {
            text: "ParticleSystem seed: " + psystem.seed
        }
        CustomCheckBox {
            id: checkBoxRandomize
            text: "Use random seed"
            checked: true
        }
        CustomLabel {
            text: "Custom seed"
            opacity: psystem.useRandomSeed ? 0.4 : 1.0
        }
        CustomSlider {
            sliderValue: 0
            sliderEnabled: !psystem.useRandomSeed
            fromValue: 0
            toValue: 99
            sliderStepSize: 1
            onSliderValueChanged: psystem.setSeed(sliderValue);
        }
        Item { width: 1; height: 20 }
        CustomLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Model Emitters: " + mainWindow.modelEmittersAmount
        }
        Item { width: 1; height: 5 }
        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: qsTr("Add")
                font.pointSize: AppSettings.fontSizeSmall
                enabled: mainWindow.modelEmittersAmount < 10
                onClicked: mainWindow.createModelEmitter();
            }
            Button {
                text: qsTr("Remove")
                font.pointSize: AppSettings.fontSizeSmall
                enabled: mainWindow.modelEmittersAmount > 0
                onClicked: {
                    let instance = mainWindow.modelEmitters.pop();
                    instance.destroy();
                    mainWindow.modelEmittersAmount = mainWindow.modelEmitters.length;
                }
            }
        }
        Item { width: 1; height: 20 }
        CustomLabel {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Sprite Emitters: " + mainWindow.spriteEmittersAmount
        }
        Item { width: 1; height: 5 }
        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: qsTr("Add")
                font.pointSize: AppSettings.fontSizeSmall
                enabled: mainWindow.spriteEmittersAmount < 10
                onClicked: mainWindow.createSpriteEmitter();
            }
            Button {
                text: qsTr("Remove")
                font.pointSize: AppSettings.fontSizeSmall
                enabled: mainWindow.spriteEmittersAmount > 0
                onClicked: {
                    let instance = mainWindow.spriteEmitters.pop();
                    instance.destroy();
                    mainWindow.spriteEmittersAmount = mainWindow.spriteEmitters.length;
                }
            }
        }

    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
