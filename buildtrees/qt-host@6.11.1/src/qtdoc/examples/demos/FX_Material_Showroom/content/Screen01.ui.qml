// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Helpers
import PocketDemo
import ComponentBundles.MaterialBundle
import "Figma_Assets"
import "Pocket_Demo_SkylightUI"

Rectangle {
    id: rectangle1
    width: Constants.width
    height: Constants.height
    color: "#000000"
    border.width: 0
    layer.format: ShaderEffectSource.RGB
    property bool settingsopen: true
    property string oldstate: "state_type_Materials"
    property Material matChoice: silver
    property string matselText: material_pop_up.matSelection

    View3D {
        id: view3D
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        camera: sceneCamera
        anchors.leftMargin: 636 * menutransition_open.phase
        layer.format: ShaderEffectSource.RGB

        environment: layout.dotBtns.sceneState
                     === 2 ? colorful : (layout.dotBtns.sceneState === 1 ? studio : bright)

        Node {
            id: scene

            Node {
                id: lightGroups

                Node {
                    id: brightLights
                    visible: layout.dotBtns.sceneState === 0
                    DirectionalLight {
                        visible: true
                        shadowBias: 0.02
                        shadowMapQuality: Light.ShadowMapQualityHigh
                        shadowFilter: 5
                        shadowMapFar: 5000
                        shadowFactor: 100
                        eulerRotation.y: 0.20483
                        brightness: 0.5
                        castsShadow: true
                        eulerRotation.x: -40.00001
                        eulerRotation.z: 89.99592
                    }

                    DirectionalLight {
                        x: -6
                        y: -6
                        z: -6
                        shadowBias: 0.02
                        shadowFilter: 5
                        shadowMapFar: 5000
                        eulerRotation.y: 178.69228
                        brightness: 0.5
                        castsShadow: true
                        shadowFactor: 100
                        eulerRotation.x: -19.04617
                        eulerRotation.z: 89.4164
                        shadowMapQuality: Light.ShadowMapQualityHigh
                    }
                }

                Node {
                    id: colorfulLights
                    visible: layout.dotBtns.sceneState === 2
                    DirectionalLight {
                        shadowMapFar: 5000
                        shadowBias: 0.02
                        shadowMapQuality: Light.ShadowMapQualityHigh
                        eulerRotation.y: 25.32839
                        brightness: 0.5
                        castsShadow: true
                        shadowFactor: 100
                        eulerRotation.x: -40.00002
                        eulerRotation.z: 89.99591
                    }
                }
            }

            FloorComponent {
                id: floor
                receivesReflections: true
                aoState: material_pop_up.modelsL.selection
                         === "None" ? 3 : (material_pop_up.modelsL.selection
                                          === "Dragon" ? 1 : (material_pop_up.modelsL.selection
                                                             === "Rabbit" ? 2 : 0))
                scale.x: 150
                scale.y: 150
                scale.z: 150
                source: "file:content/Meshes/floor.mesh"
                eulerRotation.z: 0
                eulerRotation.y: 0
                eulerRotation.x: 0
                visible: true
                y: 4
            }

            Model {
                id: studioFloor
                y: -1
                visible: layout.dotBtns.sceneState === 1
                opacity: 1
                source: "#Rectangle"
                eulerRotation.z: 0
                eulerRotation.x: -90
                materials: [floorMat]
                eulerRotation.y: 0
                scale.y: 35
                scale.z: 35

                Model {
                    id: areaLight
                    source: "#Rectangle"
                    materials: [lightMaterial]
                    eulerRotation.z: 0
                    eulerRotation.y: 0
                    scale.y: 0.05
                    scale.x: 0.25
                    eulerRotation.x: 180
                    z: 20
                }

                Model {
                    id: pillar
                    source: "#Cube"
                    materials: [asphalt]
                    x: 0
                    y: 38
                    receivesReflections: true
                    scale.z: 0.2
                    z: 10.02753
                    scale.y: 0.05
                    scale.x: 0.05
                }

                Model {
                    id: pillar1
                    source: "#Cube"
                    materials: [asphalt]
                    x: 38
                    y: 0
                    receivesReflections: true
                    z: 9.57775
                    scale.z: 0.2
                    scale.y: 0.05
                    scale.x: 0.05
                }

                Model {
                    id: pillar2
                    source: "#Cube"
                    materials: [asphalt]
                    x: -38
                    y: 0
                    z: 10.03454
                    receivesReflections: true
                    scale.z: 0.2
                    scale.y: 0.05
                    scale.x: 0.05
                }

                Model {
                    id: pillar3
                    source: "#Cube"
                    materials: [asphalt]
                    x: 0
                    y: -38
                    z: 9.60788
                    receivesReflections: true
                    scale.z: 0.2
                    scale.y: 0.05
                    scale.x: 0.05
                }

                Node {
                    id: groupLights

                    Model {
                        id: areaLight1
                        x: 0
                        y: 35
                        source: "#Rectangle"
                        eulerRotation.z: 0
                        materials: [lightMaterial]
                        eulerRotation.y: 0
                        z: 20
                        scale.x: 0.7
                        eulerRotation.x: 90
                        scale.y: 0.01
                    }

                    Model {
                        id: areaLight2
                        x: 35
                        y: 0
                        source: "#Rectangle"
                        eulerRotation.z: 90
                        materials: [lightMaterial]
                        eulerRotation.y: -90
                        z: 20
                        scale.x: 0.7
                        eulerRotation.x: 0
                        scale.y: 0.01
                    }

                    Model {
                        id: areaLight3
                        x: 0
                        y: -35
                        source: "#Rectangle"
                        eulerRotation.z: 0
                        materials: [lightMaterial]
                        eulerRotation.y: 0
                        z: 20
                        scale.x: 0.7
                        eulerRotation.x: -90
                        scale.y: 0.01
                    }

                    Model {
                        id: areaLight4
                        x: -35
                        y: 0
                        source: "#Rectangle"
                        eulerRotation.z: 90
                        materials: [lightMaterial]
                        eulerRotation.y: 90
                        z: 20
                        scale.x: 0.7
                        eulerRotation.x: 0
                        scale.y: 0.01
                    }
                }

                ReflectionProbe {
                    id: reflectionProbe
                    x: 0
                    y: 0
                    boxOffset.y: 990
                    debugView: false
                    z: 0
                    boxSize.z: 3000
                    timeSlicing: ReflectionProbe.None
                    clearColor: "#000000"
                    boxSize.x: 3000
                    refreshMode: ReflectionProbe.EveryFrame
                    parallaxCorrection: true
                    quality: ReflectionProbe.Low
                    boxSize.y: 2000
                }
                scale.x: 35
                receivesReflections: true
            }

            Node {
                id: cameraRoot
                eulerRotation.z: 0
                eulerRotation.y: 35.26906
                eulerRotation.x: -24.99024

                PerspectiveCamera {
                    id: sceneCamera
                    x: -0
                    y: 95.285
                    z: 530.90942
                }
            }

            Node {
                id: effects

                HeatwaveEffect {
                    id: heatwave
                    visible: material_pop_up.effectsL.selection === "Heatwave"
                }

                RainEffect {
                    id: rain
                    sceneCamera: sceneCamera
                    visible: material_pop_up.effectsL.selection === "Rain"
                }

                SmokeEffect {
                    id: smoke
                    visible: material_pop_up.effectsL.selection === "Smoke"
                }

                SteamEffect {
                    id: steam
                    y: 100
                    visible: material_pop_up.effectsL.selection === "Steam"
                    z: 100
                }

                SnowEffect {
                    id: snow
                    visible: material_pop_up.effectsL.selection === "Snow"
                }

                FireEffect {
                    id: fire
                    sceneCamera: sceneCamera
                    visible: material_pop_up.effectsL.selection === "Flames"
                }

                Node {
                    id: group
                    eulerRotation.y: sparkrotation.phase

                    SparksEffect {
                        id: sparks
                        visible: material_pop_up.effectsL.selection === "Sparks"
                        y: 100
                        z: 250
                    }
                }

                ShockwaveEffect {
                    id: shockwave
                    visible: material_pop_up.effectsL.selection === "Shockwave"
                }

                DustEffect {
                    id: dust
                    sceneCamera: sceneCamera
                    visible: material_pop_up.effectsL.selection === "Dust"
                }

                FlashEffect {
                    id: flash
                    visible: material_pop_up.effectsL.selection === "Flash"
                }

                ExplosionEffect {
                    id: explosion
                    visible: material_pop_up.effectsL.selection === "Explosion"
                }

                BubblesEffect {
                    id: bubbles
                    sceneCamera: sceneCamera
                    x: 0
                    y: 125.976
                    visible: material_pop_up.effectsL.selection === "Bubbles"
                    z: 62.12978
                }

                CloudEffect {
                    id: cloud
                    sceneCamera: sceneCamera
                    y: 100
                    visible: material_pop_up.effectsL.selection === "Clouds"
                    time: 10000
                    startTime: 0
                }
            }

            Model {
                id: sphere
                visible: false
                materials: [skyboxMaterial]
                source: "#Sphere"
                receivesShadows: false
                castsShadows: false
                scale.z: 100
                scale.y: 100
                scale.x: 100
            }

            Model {
                id: dragon1
                x: 1
                y: -9
                visible: false
                source: "file:content/Meshes/stanford_Dragon.mesh"
                depthBias: 0
                eulerRotation.x: -90
                receivesReflections: true
                z: 1
                scale.y: 200
                scale.x: 200
                castsReflections: false
                materials: [bubbleMat1]
                scale.z: 200
            }

            Node {
                id: models
                visible: true

                Model {
                    id: materialBall
                    y: 4

                    visible: material_pop_up.modelsL.selection === "Qt material ball"
                    source: "file:content/Meshes/materialBall.mesh"
                    castsReflections: false
                    receivesReflections: true
                    scale.z: 150
                    scale.y: 150
                    scale.x: 150
                    materials: [rectangle1.matChoice]
                }

                Model {
                    id: bunny
                    x: 50
                    y: -75
                    visible: material_pop_up.modelsL.selection === "Rabbit"
                    source: "file:content/Meshes/bunnyUV.mesh"
                    castsReflections: false
                    z: -50
                    eulerRotation.x: -90
                    receivesReflections: true
                    scale.z: 2000
                    scale.y: 2000
                    scale.x: 2000
                    materials: [rectangle1.matChoice]
                }

                Model {
                    id: dragon
                    y: -10
                    visible: material_pop_up.modelsL.selection === "Dragon"
                    source: "file:content/Meshes/stanford_Dragon.mesh"
                    eulerRotation.x: -90
                    castsReflections: false
                    receivesReflections: true
                    depthBias: 0
                    scale.z: 200
                    scale.y: 200
                    materials: [rectangle1.matChoice]
                    scale.x: 200
                }
            }
        }

        Node {
            id: environments

            View3D {
                id: darkLoader
                width: 0
                height: 0
                environment: dark
                SceneEnvironment {
                    id: dark
                    //effects: [hDRBloomTonemap]
                    specularAAEnabled: true
                    tonemapMode: SceneEnvironment.TonemapModeLinear
                    probeExposure: 1
                    probeHorizon: 1
                    clearColor: "#000000"
                    antialiasingQuality: SceneEnvironment.VeryHigh
                    temporalAAEnabled: false
                    depthTestEnabled: true
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                    depthPrePassEnabled: false
                }
            }

            View3D {
                id: brightLoader
                x: -8
                y: -8
                width: 400
                height: 400
                environment: bright
                SceneEnvironment {
                    id: bright
                    fog: fog
                    tonemapMode: SceneEnvironment.TonemapModeLinear
                    skyboxBlurAmount: 0
                    probeHorizon: 0.5
                    clearColor: "#000000"
                    specularAAEnabled: true
                    backgroundMode: SceneEnvironment.SkyBox
                    lightProbe: dreamstime_xl_182890747
                    antialiasingQuality: SceneEnvironment.High
                    antialiasingMode: SceneEnvironment.MSAA

                    Texture {
                        id: dreamstime_xl_182890747
                        source: "file:content/images/HDR/dreamstime_xl_182890747.png"
                    }

                    Fog {
                        id: fog
                        color: "#608080"
                        transmitCurve: 10
                        transmitEnabled: false
                        heightEnabled: false
                        depthFar: 1000
                        depthNear: 0
                        density: 0
                        depthEnabled: true
                        enabled: false
                    }
                }
            }

            View3D {
                id: colorLoader
                x: -14
                y: -14
                width: 400
                height: 400
                environment: colorful
                SceneEnvironment {
                    id: colorful
                    fog: fog
                    tonemapMode: SceneEnvironment.TonemapModeLinear
                    probeHorizon: 0.5
                    clearColor: "#000000"
                    backgroundMode: SceneEnvironment.SkyBox
                    specularAAEnabled: true
                    lightProbe: dreamstime_xl_119184006
                    antialiasingQuality: SceneEnvironment.High
                    antialiasingMode: SceneEnvironment.MSAA

                    Texture {
                        id: dreamstime_xl_119184006
                        source: "file:content/images/HDR/dreamstime_xl_119184006.png"
                    }
                }
            }

            View3D {
                id: studioLoader
                width: 400
                height: 400

                SceneEnvironment {
                    id: studio
                    fog: fog
                    specularAAEnabled: true
                    tonemapMode: SceneEnvironment.TonemapModeLinear
                    probeExposure: 1
                    probeHorizon: 1
                    effects: [hDRBloomTonemap9, colorMaster]
                    clearColor: "#000000"
                    antialiasingQuality: SceneEnvironment.VeryHigh
                    temporalAAEnabled: false
                    depthTestEnabled: true
                    HDRBloomTonemap {
                        id: hDRBloomTonemap9
                        blurFalloff: 1
                        channelThreshold: 1
                        bloomThreshold: 0
                        exposure: 0
                        gamma: 1
                        tonemappingLerp: 0
                    }

                    ColorMaster {
                        id: colorMaster
                        greenStrength: 1
                    }
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                    depthPrePassEnabled: false
                }
            }
        }

        DebugBox {
            id: debugView
            x: 1615
            y: 840
            color: "#99000000"

            //scale: 1
            radius: 8
            width: 250

            anchors.right: parent.right
            anchors.bottom: parent.bottom
            sView: view3D
            fpsSize: 24
            secondaryinfoSize: 18
            visible: layoutFull.debugOn
            anchors.bottomMargin: 121
            anchors.rightMargin: 27
        }
    }

    WasdController {
        id: wasdController
        controlledObject: cameraRoot
        anchors.top: parent.top
        anchors.fill: parent
        anchors.leftMargin: 636 * menutransition_open.phase
        anchors.topMargin: 0
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        preventStealing: false
        cursorShape: Qt.SizeAllCursor
        anchors.leftMargin: 636 * menutransition_open.phase
        drag.axis: Drag.YAxis

        Connections {
            target: mouseArea

            function onWheel(wheel) {
                sceneCamera.fieldOfView += wheel.angleDelta.y * 0.04
                             * (sceneCamera.fieldOfView + wheel.angleDelta.y * 0.04 > 0.0)
                             * (sceneCamera.fieldOfView + wheel.angleDelta.y * 0.04 < 60.0)
            }
        }

        PinchArea {
            id: pinchArea
            anchors.fill: parent

            Connections {
                target: pinchArea
                function onPinchUpdated(pinch) {
                    if (pinch.previousScale - pinch.scale > 0.008
                     || pinch.previousScale - pinch.scale < -1 * 0.008) {
                        let velocity = (pinch.previousScale - pinch.scale) > 0 ? 1.25 : -1.25
                        sceneCamera.fieldOfView += velocity
                                * (sceneCamera.fieldOfView + velocity > 0.0)
                                * (sceneCamera.fieldOfView + velocity < 60.0)
                    }
                }
            }
        }
    }

    Rectangle {
        id: rectangle
        width: 904
        height: 80
        visible: false
        color: "#ffffff"
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.horizontalCenter: parent.horizontalCenter

        RoundButton {
            id: rainBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Rain"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 8
            anchors.topMargin: 8
            checkable: true
            highlighted: true
        }

        RoundButton {
            id: snowBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Snow"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            checkable: true
            anchors.topMargin: 8
            anchors.leftMargin: 136
        }

        RoundButton {
            id: smokeBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Smoke"
            anchors.left: parent.left
            anchors.top: parent.top
            checked: true
            highlighted: true
            checkable: true
            anchors.topMargin: 8
            anchors.leftMargin: 264
        }

        RoundButton {
            id: heatwaveBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Heatwave"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            checkable: true
            anchors.topMargin: 8
            anchors.leftMargin: 392
        }

        RoundButton {
            id: fireBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Fire"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            anchors.topMargin: 8
            anchors.leftMargin: 520
            checkable: true
        }

        RoundButton {
            id: sparkBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Sparks"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            anchors.topMargin: 8
            anchors.leftMargin: 648
            checkable: true
        }

        RoundButton {
            id: dustBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Dust"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            anchors.topMargin: 8
            anchors.leftMargin: 776
            checkable: true
        }
    }

    Item {
        id: __materialLibrary__
        DefaultMaterial {
            id: principledMaterial
            opacity: 0.5
            cullMode: Material.NoCulling
            vertexColorsEnabled: true
            objectName: "Default Material"
            diffuseColor: "#ffffff"
        }

        CustomMaterial {
            id: skyboxMaterial
            property TextureInput skyboxTexture: skyboxTex
            property color fogColor: fog.color
            property real fogAmount: fog.density

            cullMode: Material.FrontFaceCulling
            vertexShader: "shaders/sky.vert"
            fragmentShader: "shaders/sky.frag"
            objectName: "Skybox Material"

            TextureInput {
                id: skyboxTex
                texture: view3D.environment.lightProbe
                enabled: true
            }
        }

        GoldMaterial {
            id: gold
            objectName: "Gold"
        }

        AsphaltMaterial {
            id: asphalt
            objectName: "Asphalt"
        }

        PrincipledMaterial {
            id: silver
            cullMode: Material.NoCulling
            specularAmount: 1
            baseColor: "#fefefd"
            objectName: "Silver"
            metalness: 1
            roughness: 0.05
            clearcoatAmount: 0
        }

        PrincipledMaterial {
            id: steelBrushed
            normalMap: metal009_2K_NormalGL
            cullMode: Material.NoCulling
            baseColor: "#d9d7d2"
            clearcoatAmount: 0
            roughness: 1
            objectName: "Brushed Steel"
            metalness: 1
            specularAmount: 1
            roughnessMap: metal009_2K_Roughness

            Texture {
                id: metal009_2K_NormalGL
                source: "file:content/images/Metal009_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: metal009_2K_Roughness
                source: "file:content/images/Metal009_2K_Roughness.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }
        }

        PrincipledMaterial {
            id: copper
            cullMode: Material.NoCulling
            baseColor: "#ffbd7f"
            objectName: "Copper"
            metalness: 1
            roughness: 0.15
            clearcoatAmount: 0
        }

        PrincipledMaterial {
            id: wood
            normalMap: wood048_2K_NormalGL
            roughnessMap: wood048_2K_Roughness
            baseColorMap: wood048_2K_Color
            cullMode: Material.NoCulling
            transmissionFactor: 0
            roughnessChannel: Material.B
            metalness: 0
            specularAmount: 1
            roughness: 1
            clearcoatAmount: 0
            objectName: "Wood"
            baseColor: "#ffffff"

            Texture {
                id: wood048_2K_Color
                source: "file:content/images/woodtextures/Wood048_2K-PNG/Wood048_2K_Color.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: wood048_2K_Roughness
                source: "file:content/images/woodtextures/Wood048_2K-PNG/Wood048_2K_Roughness.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: wood048_2K_NormalGL
                source: "file:content/images/woodtextures/Wood048_2K-PNG/Wood048_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }
        }

        PrincipledMaterial {
            id: stone
            roughnessMap: rock023_2K_Roughness
            occlusionMap: rock023_2K_AmbientOcclusion
            normalMap: rock023_2K_NormalGL
            baseColorMap: rock023_2K_Color
            normalStrength: 1
            cullMode: Material.NoCulling
            roughness: 1
            objectName: "Stone"
            baseColor: "#ffffff"
            specularAmount: 1
            clearcoatAmount: 0
            metalness: 0

            Texture {
                id: rock023_2K_Color
                source: "file:content/images/stonetextures/Rock023_2K_Color.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: rock023_2K_NormalGL
                source: "file:content/images/stonetextures/Rock023_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: rock023_2K_AmbientOcclusion
                source: "file:content/images/stonetextures/Rock023_2K_AmbientOcclusion.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: rock023_2K_Roughness
                source: "file:content/images/stonetextures/Rock023_2K_Roughness.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }
        }

        PrincipledMaterial {
            id: carbonFiber
            cullMode: Material.NoCulling
            normalMap: fabric004_2K_NormalGL
            objectName: "CarbonFiber"
            metalness: 0
            baseColor: "#050505"
            transmissionFactor: 0
            roughnessChannel: Material.B
            specularAmount: 1
            clearcoatAmount: 1
            roughness: 0

            Texture {
                id: fabric004_2K_NormalGL
                source: "file:content/images/Fabric004_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 30
                scaleU: 30
            }
        }

        PrincipledMaterial {
            id: plasticTextured
            normalStrength: 0.06125
            normalMap: noisenormal
            clearcoatAmount: 0
            objectName: "Textured Plastic"
            roughness: 0.3
            specularAmount: 0.93
            transmissionFactor: 0
            metalness: 0
            cullMode: Material.NoCulling
            baseColor: "#ff0000"
            roughnessChannel: Material.B

            Texture {
                id: noisenormal
                source: "file:content/images/noisenormal.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 10
                scaleU: 10
            }
        }

        PrincipledMaterial {
            id: fabric
            baseColorMap: fabric031_2K_Color
            normalStrength: 0.5
            occlusionMap: fabric031_2K_Detail
            roughnessMap: fabric031_2K_Roughness
            normalMap: fabric031_2K_NormalGL
            clearcoatAmount: 0
            objectName: "Fabric"
            roughness: 1
            specularAmount: 1
            cullMode: Material.NoCulling
            metalness: 0
            transmissionFactor: 0
            alphaMode: PrincipledMaterial.Opaque
            occlusionAmount: 0.75
            baseColor: "#d7c9b5"
            depthDrawMode: Material.AlwaysDepthDraw
            roughnessChannel: Material.R

            Texture {
                id: fabric031_2K_NormalGL
                source: "file:content/images/Fabric031_2K_NormalGL.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: fabric031_2K_Roughness
                source: "file:content/images/Fabric031_2K_Roughness.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: fabric031_2K_Displacement
                source: "file:content/images/Fabric031_2K_Displacement.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: fabric031_2K_Detail
                source: "file:content/images/Fabric031_2K_Detail.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: fabric031_2K_Color
                source: "file:content/images/Fabric031_2K_Color.png"
                scaleV: 3
                scaleU: 3
                mipFilter: Texture.Linear
                generateMipmaps: true
            }
        }

        PrincipledMaterial {
            id: leather
            roughnessMap: leather037_2K_Roughness
            normalMap: leather037_2K_NormalGL
            baseColorMap: leather037_2K_Color
            cullMode: Material.NoCulling
            roughness: 1
            roughnessChannel: Material.R
            baseColor: "#ffa983"
            specularAmount: 1
            metalness: 0
            objectName: "Steel"
            clearcoatAmount: 0
            transmissionFactor: 0

            Texture {
                id: leather037_2K_Color
                source: "file:content/images/leathertextures/Leather037_2K-PNG/Leather037_2K_Color.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: leather037_2K_NormalGL
                source: "file:content/images/leathertextures/Leather037_2K-PNG/Leather037_2K_NormalGL.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }

            Texture {
                id: leather037_2K_Roughness
                source: "file:content/images/leathertextures/Leather037_2K-PNG/Leather037_2K_Roughness.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
                scaleV: 3
                scaleU: 3
            }
        }

        CustomMaterial {
            id: waxMat
            fragmentShader: "shaders/SSS.frag"
            destinationBlend: CustomMaterial.NoBlend
            depthDrawMode: Material.OpaqueOnlyDepthDraw
            vertexShader: "shaders/SSS.vert"
            sourceBlend: CustomMaterial.NoBlend
            cullMode: Material.BackFaceCulling
            property color baseColor: "#ffffff"
            property color subsurfaceColor: "#fcdfc8"
            property real specular: 0.5
            property real density: 0
            property real opacity: 1
            property real roughness: 0.25
            property real clearcoat: 1
            property real metalness: 0.0
            property real sssDistortion: 0.0
            property real sssPower: 10
            property real sssScale: 2
            shadingMode: CustomMaterial.Shaded
            objectName: "New Material"
        }

        CustomMaterial {
            id: floorMat
            fragmentShader: "shaders/floor.frag"
            destinationBlend: CustomMaterial.NoBlend
            depthDrawMode: Material.OpaqueOnlyDepthDraw
            vertexShader: "shaders/floor.vert"
            sourceBlend: CustomMaterial.NoBlend
            cullMode: Material.BackFaceCulling
            property color baseColor: "#ffffff"
            property color subsurfaceColor: "#fcdfc8"
            property real specular: 0
            property real density: 1
            property real opacity: 1
            property real roughness: 0.0
            property real clearcoat: 1
            property real metalness: 0.0
            property real sssDistortion: 0.0
            property real sssPower: 10
            property real sssScale: 2
            property vector2d aoMaskScale: Qt.vector2d(0.9, 0.9)
            property vector2d aoMaskPos: Qt.vector2d(0.01, 0.01)
            shadingMode: CustomMaterial.Shaded
            objectName: "New Material"

            property color aoMask: "#0000ff"
            property TextureInput aoTexture: aoTexInput
            TextureInput {
                id: aoTexInput
                texture: aoTex
                enabled: true
            }

            Texture {
                id: aoTex
                source: "file:content/images/AOBake.png"
                mipFilter: Texture.Linear
                generateMipmaps: true
            }
        }

        CustomMaterial {
            id: bubbleMat1
            objectName: "New Material"
            destinationBlend: CustomMaterial.NoBlend
            property color baseColor: "#ffffff"
            property real stepSize: 0.01
            property real densityFalloff: 0.01
            fragmentShader: "shaders/smokerm.frag"
            shadingMode: CustomMaterial.Shaded
            depthDrawMode: Material.NeverDepthDraw
            vertexShader: "shaders/smokerm.vert"
            sourceBlend: CustomMaterial.NoBlend
            cullMode: Material.BackFaceCulling
        }

        PrincipledMaterial {
            id: lightMaterial
            emissiveFactor.z: 1
            emissiveFactor.y: 1
            emissiveFactor.x: 1
            objectName: "Default Material"
            cullMode: Material.NoCulling
        }

        DefaultMaterial {
            id: newMaterial
            objectName: "New Material"
        }
    }

    LayoutFull {
        id: layoutFull
        openmenuTransition: menutransition_open
        openmenuTransitionPhase: menutransition_open.phase
    }

    Layout {
        id: layout
    }

    Material_pop_up {
        id: material_pop_up
        openmenuTransitionPhase: menutransition_open.phase
        closemenuTransition: menutransition_close
        screen: rectangle1
    }

    Rectangle {
        id: rectangle2
        width: 776
        height: 80
        visible: false
        color: "#ffffff"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        RoundButton {
            id: flashBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Flash"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            checkable: true
            anchors.leftMargin: 8
            highlighted: true
        }

        RoundButton {
            id: shockwaveBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Shockwave"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            checkable: true
            anchors.leftMargin: 136
            highlighted: true
        }

        RoundButton {
            id: explosionBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Explosion"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            checkable: true
            anchors.leftMargin: 264
            highlighted: true
        }

        RoundButton {
            id: bubblesBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Bubbles"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            checkable: true
            anchors.leftMargin: 392
            highlighted: true
        }

        RoundButton {
            id: cloudBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Cloud"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 8
            checkable: true
            anchors.leftMargin: 520
            highlighted: true
        }

        RoundButton {
            id: steamBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Steam"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 648
            checkable: true
            anchors.topMargin: 8
            highlighted: true
        }
        anchors.horizontalCenter: parent.horizontalCenter
    }

    NumberAnimation {
        id: menutransition_open
        target: menutransition_open
        property: "phase"
        easing.bezierCurve: [0.445, 0.05, 0.55, 0.95, 1, 1]
        duration: 1000
        to: 1
        from: 0
        property real phase: 0.0
    }

    NumberAnimation {
        id: menutransition_close
        target: menutransition_open
        property: "phase"
        easing.bezierCurve: [0.445, 0.05, 0.55, 0.95, 1, 1]
        duration: 1000
        to: 0
        from: 1
    }

    Rectangle {
        id: rectangle3
        width: 136
        height: 224
        visible: false
        color: "#ffffff"
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
        RoundButton {
            id: bunnyBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Bunny"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            checkable: true
            anchors.topMargin: 8
            anchors.leftMargin: 8
        }

        RoundButton {
            id: bustBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Bust"
            anchors.left: parent.left
            anchors.top: parent.top
            checked: true
            highlighted: true
            checkable: true
            anchors.topMargin: 80
            anchors.leftMargin: 8
        }

        RoundButton {
            id: dragonBtn
            width: 120
            height: 64
            radius: checked ? 32 : 8
            text: "Dragon"
            anchors.left: parent.left
            anchors.top: parent.top
            highlighted: true
            checkable: true
            anchors.topMargin: 152
            anchors.leftMargin: 8
        }
    }

    Timer {
        id: timer
    }

    NumberAnimation {
        id: sparkrotation
        target: sparkrotation
        property: "phase"
        running: material_pop_up.effectsL.selection === "Sparks"
        loops: -1
        easing.bezierCurve: [0.2, 0.2, 0.8, 0.8, 1, 1]
        duration: 10000
        property real phase: 0
        to: 360
        from: 0
    }

    NumberAnimation {
        id: lineRotation
        target: lineRotation
        property: "phase"
        running: true
        loops: -1
        easing.bezierCurve: [0.2, 0.2, 0.8, 0.8, 1, 1]
        duration: 10000
        property real phase: 0
        to: 360
        from: 0
    }

    states: [
        State {
            name: "silverState"
            when: rectangle1.matselText === "Silver"
            PropertyChanges {
                target: rectangle1
                matChoice: silver
            }
        },
        State {
            name: "steelState"
            when: rectangle1.matselText === "Brushed Steel"
            PropertyChanges {
                target: rectangle1
                matChoice: steelBrushed
            }
        },
        State {
            name: "copperState"
            when: rectangle1.matselText === "Copper"
            PropertyChanges {
                target: rectangle1
                matChoice: copper
            }
        },
        State {
            name: "glassState"
            when: rectangle1.matselText === "Wax"
            PropertyChanges {
                target: rectangle1
                matChoice: waxMat
            }
        },
        State {
            name: "woodState"
            when: rectangle1.matselText === "Wood"
            PropertyChanges {
                target: rectangle1
                matChoice: wood
            }
        },
        State {
            name: "stoneState"
            when: rectangle1.matselText === "Stone"
            PropertyChanges {
                target: rectangle1
                matChoice: stone
            }
        },
        State {
            name: "carbonfiberState"
            when: rectangle1.matselText === "Carbon Fiber"
            PropertyChanges {
                target: rectangle1
                matChoice: carbonFiber
            }
        },
        State {
            name: "plasticState"
            when: rectangle1.matselText === "Plastic"
            PropertyChanges {
                target: rectangle1
                matChoice: plasticTextured
            }
        },
        State {
            name: "fabricState"
            when: rectangle1.matselText === "Fabric"
            PropertyChanges {
                target: rectangle1
                matChoice: fabric
            }
        },
        State {
            name: "leatherState"
            when: rectangle1.matselText === "Leather"
            PropertyChanges {
                target: rectangle1
                matChoice: leather
            }
        }
    ]
}
