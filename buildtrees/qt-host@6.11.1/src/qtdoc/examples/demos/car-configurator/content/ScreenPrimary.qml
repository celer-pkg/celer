// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Timeline
import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Particles3D
import QtQuick3D.Helpers
import CarRendering
import Quick3DAssets.Venodhb_LOD0
import "doorIcon"
import "WallEffect1"
import "WallEffect2"
import "WallEffect3"
import Quick3DAssets.EV_SportsCar_low
import Quick3DAssets.Pebbles
import Quick3DAssets.InteriorShadow
import Quick3DAssets.ShadowPlane
import Quick3DAssets.Snow

Rectangle {
    id: root

    width: Constants.width
    height: Constants.height
    color: Constants.screenColor
    state: "State1"
    focus: true
    transformOrigin: Item.Top

    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_1) {
            gui.visible = !gui.visible
        }
    }

    Component.onCompleted: root.forceActiveFocus()

    // this property tells a parent when this object is completely loaded (including preloaded assets)
    property bool assetPreLoadComplete: false

    // this property enables performance enhancements for android targets
    readonly property bool perfMode: Qt.platform.os === 'android'

    property real demoCameraRotation: 0
    required property url downloadBase

    Timer {
        id: preloadTimer

        running: true
        repeat: true
        interval: 1

        property int count: 0
    }

    Connections {
        target: preloadTimer

        function onTriggered() {
            if (preloadTimer.count == 0)
                btnAnimated.toggleCheck()
            else if (preloadTimer.count == 1)
                btnStudio.toggleCheck()
            else if (preloadTimer.count == 2)
                btnGarage.toggleCheck()
            else if (preloadTimer.count == 3)
                btnDesert.toggleCheck()
            else if (preloadTimer.count == 4)
                btnLight.toggleCheck()
            else if (preloadTimer.count == 5)
                btnLight.toggleCheck()
            else {
                root.assetPreLoadComplete = true
                preloadTimer.running = false
            }
            ++preloadTimer.count
        }
    }

    ParticleMask {
        id: particleMaskGround
        clipNear: 345
        clipFar: 355
        particleSize: 7
        width: 1500
        height: 1500
        nodePosition: originalSnowParticleSystem.scenePosition
        snowParticleSystemVisible: btnSnow.checked
    }

    ParticleMask {
        id: particleMaskCar
        clipNear: 260
        clipFar: 270
        particleSize: 6
        width: 512
        height: 512
        nodePosition: originalSnowParticleSystem.scenePosition
        snowParticleSystemVisible: btnSnow.checked
    }

    View3D {
        id: view3D
        visible: true

        anchors.fill: parent
        camera: sceneCamera2
        environment: showhall

        Texture {
            id: particleMaskGroundTexture
            sourceItem: particleMaskGround.maskSource
            scaleU: -1
            scaleV: -1
        }

        Texture {
            id: particleMaskCarTexture
            sourceItem: particleMaskCar.maskSource
            scaleU: -1
            scaleV: -1
        }

        Node {
            id: scene2

            ReflectionProbe {
                id: reflectionProbe1

                boxSize.x: 3000
                boxSize.y: 2000
                boxSize.z: 3000
                timeSlicing: ReflectionProbe.IndividualFaces
                clearColor: "#000000"
            }

            Node {
                id: sceneObjects1

                y: -500
                z: -1000

                Node {
                    id: camOverride

                    x: 10
                    z: -10

                    Node {
                        id: cameraRoot
                        y: 0
                        eulerRotation.x: 0

                        eulerRotation.y: 0
                        eulerRotation.z: 0
                        onEulerRotationChanged: {
                            if (cameraResetAnimation.running)
                                return
                            cameraRoot.eulerRotation.y -=
                                    Math.floor(cameraRoot.eulerRotation.y / 360) * 360
                        }

                        PerspectiveCamera {
                            id: sceneCamera2

                            x: -31.83
                            y: 223.61
                            z: 665.37
                            eulerRotation.x: -12.17219
                            fieldOfView: 64
                            fieldOfViewOrientation: PerspectiveCamera.Horizontal

                            Behavior on x {
                                enabled: !demo.running
                                NumberAnimation{duration: 2000}
                            }
                            Behavior on y {
                                enabled: !demo.running
                                NumberAnimation{duration: 2000}
                            }
                            Behavior on z {
                                enabled: !demo.running
                                NumberAnimation{duration: 2000}
                            }
                            Behavior on fieldOfView {
                                enabled: !demo.running
                                NumberAnimation{}
                            }
                            Behavior on eulerRotation {
                                enabled: !demo.running
                                NumberAnimation{duration: 2000}
                            }
                        }
                    }
                }

                Model {
                    id: plane3

                    x: -13.056
                    y: 4.1
                    visible: true
                    source: "#Rectangle"
                    castsShadows: false
                    receivesShadows: false
                    receivesReflections: false
                    z: -2.65002
                    eulerRotation.x: -90
                    eulerRotation.y: -45
                    eulerRotation.z: -90
                    scale.x: 12.00987
                    scale.y: 14.11107
                    scale.z: 13
                    materials: groundMat1
                    depthBias: -500
                }

                Model {
                    id: plane2

                    y: 250
                    visible: false
                    source: "#Sphere"
                    receivesReflections: false
                    z: -500
                    scale.x: 50
                    scale.y: 10
                    scale.z: 10
                    materials: principledMaterial2
                    castsShadows: false
                    receivesShadows: true
                }

                Headlights {
                    id: headlights
                    x: -18.519
                    y: -22.316
                    opacity: 1
                    z: 6.0437
                    visible: btnLight.checked
                    eulerRotation.x: 0
                    eulerRotation.y: -45
                    eulerRotation.z: 0
                    scale.x: 0.1
                    scale.y: 0.1
                    scale.z: 0.1
                    downloadBase: root.downloadBase

                    Model {
                        id: backlight_red

                        x: -0.001
                        y: 65.679
                        source: "#Rectangle"
                        depthBias: -24
                        materials: backlightNew
                        z: -2540.17969
                        eulerRotation.x: -90
                        eulerRotation.y: 180
                        scale.x: 42
                        scale.y: 25
                        scale.z: 25
                    }
                }

                Venodhb_LOD0 {
                    id: desertGround

                    y: -21.468
                    visible: false
                    scale.x: 10
                    scale.y: 10
                    scale.z: 10
                    downloadBase: root.downloadBase

                    Pebbles {
                        id: pebbles
                        downloadBase: root.downloadBase
                    }
                }

                Model {
                    id: garageFloor

                    y: 0.5
                    visible: true
                    source: "#Rectangle"
                    castsShadows: false
                    receivesReflections: true
                    scale.x: 70
                    scale.y: 70
                    scale.z: 70
                    eulerRotation.y: 0
                    eulerRotation.x: -90
                    materials: rectMaterial
                }

                Model {
                    id: studioFloor

                    y: -1
                    visible: false
                    source: "#Rectangle"
                    eulerRotation.z: 45
                    eulerRotation.x: -90
                    materials: rectMaterial2
                    eulerRotation.y: 0
                    scale.y: 50
                    scale.z: 35

                    Model {
                        id: areaLight

                        source: "#Rectangle"
                        materials: rectMaterial3
                        x: 0
                        y: 0
                        eulerRotation.z: 0
                        eulerRotation.y: 0
                        scale.y: 0.02
                        scale.x: 0.1
                        eulerRotation.x: 180
                        z: 11
                    }

                    Node {
                        id: group

                        Model {
                            id: areaLight1

                            x: 0
                            y: 35
                            source: "#Rectangle"
                            eulerRotation.z: 0
                            materials: rectMaterial3
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
                            materials: rectMaterial3
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
                            materials: rectMaterial3
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
                            materials: rectMaterial3
                            eulerRotation.y: 90
                            z: 20
                            scale.x: 0.7
                            eulerRotation.x: 0
                            scale.y: 0.01
                        }
                    }
                    scale.x: 50
                    receivesReflections: true
                }

                Model {
                    id: animatedstateFloor
                    visible: false
                    geometry: PlaneGeometry {
                        width: 100.0
                        height: 100.0
                        plane: PlaneGeometry.XZ
                        meshResolution: Qt.size(200, 200)
                    }
                    materials: rectMaterial4
                    scale.z: 15
                    scale.y: 15
                    scale.x: 15
                    receivesReflections: true
                    castsReflections: true
                }
                Model {
                    visible: animatedstateFloor.visible
                    source: "#Rectangle"
                    pickable: true
                    materials: rectMaterialBase
                    y: -0.1
                    eulerRotation.x: -90
                    scale.x: 45
                    scale.y: 45
                    scale.z: 45
                    receivesReflections: true
                    castsReflections: true
                }

                SnowParticlesSystem {
                    id: originalSnowParticleSystem
                    visible: btnSnow.checked
                }

                Ev_SportsCar_low {
                    id: ev_SportsCar_low
                    x: -19
                    y: 1.965
                    desert: false
                    rain: btnRain.checked
                    snowStrength: __materialLibrary__.snowStrength
                    headlightsVisible: !btnLight.checked
                    trunkIsOpen: trunkbutton.isChecked
                    hoodIsOpen: hoodButton.isChecked
                    z: 0
                    stateController: 0
                    eulerRotation.y: -45
                    scale.z: 120
                    scale.y: 120
                    scale.x: 120
                    lightsOn: btnLight.checked
                    doorLeftIsOpen: doorButtonLeft.isChecked
                    doorRightIsOpen: doorButtonRight.isChecked
                    downloadBase: root.downloadBase
                    particleMaskCar: particleMaskCar
                    particleMaskCarTexture: particleMaskCarTexture

                    PointLight {
                        id: lightPoint

                        x: 0.774
                        y: 1.143
                        visible: false
                        color: "#ff2f00"
                        linearFade: 10
                        constantFade: 1
                        quadraticFade: 10
                        z: -2.60213
                        brightness: 2
                    }

                    PointLight {
                        id: lightPoint1
                        x: -0.832
                        y: 1.143
                        visible: false
                        color: "#ff2f00"
                        linearFade: 10
                        constantFade: 1
                        quadraticFade: 10
                        brightness: 2
                        z: -2.49196
                    }

                    InteriorPointLight {
                        id: pointLight
                        x: 0
                        y: 0.931
                        quadraticFade: 6.24865
                        scope: ev_SportsCar_low
                        z: 0.01401
                        shadowBias: 0.04099
                        shadowMapQuality: Light.ShadowMapQualityVeryHigh
                        shadowFilter: 93
                        shadowFactor: 98
                        castsShadow: false

                        isOn: ev_SportsCar_low.doorLeftIsOpen || ev_SportsCar_low.doorRightIsOpen? true : false
                    }

                    InteriorShadow {
                        id: interiorShadow
                        visible: true
                        downloadBase: root.downloadBase
                    }

                    ShadowPlane {
                        id: shadowPlane
                        visible: false
                        hoodOpen: ev_SportsCar_low.hoodIsOpen
                        doorOpen: ev_SportsCar_low.doorLeftIsOpen
                        downloadBase: root.downloadBase
                    }
                }
            }

            DirectionalLight {
                id: lightDirectional

                castsShadow: false
                brightness: btnRain.checked ? 0.2 : 0.5
                eulerRotation.z: -180
                eulerRotation.y: 180
                eulerRotation.x: -54.99136
            }

            DirectionalLight {
                id: lightDirectional1

                shadowFactor: 100
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                castsShadow: false
                brightness: btnRain.checked ? 0.2 : 0.5
                eulerRotation.z: -0.00002
                eulerRotation.y: 0.00002
                eulerRotation.x: -57.72036
            }
        }
        SceneRain {
            id: sceneRain
            enabled: btnRain.checked
            scene: scene2
            center: ev_SportsCar_low.scenePosition
            extents: Qt.vector3d(1000, 500, 1000)
            excluded: [animatedstateFloor, shadowPlane, interiorShadow, headlights, ...ev_SportsCar_low.shapeExluded]
        }

        Node {
            id: environments1

            //! [Showhall Scene Environment]
            ExtendedSceneEnvironment {
                id: showhall

                glowBloom: 0
                glowStrength: 0
                glowEnabled: false
                depthOfFieldFocusDistance: 600
                depthOfFieldFocusRange: 627
                depthOfFieldBlurAmount: 19.97
                depthOfFieldEnabled: false
                exposure: 1.2

                lightProbe: _Hall
                backgroundMode: SceneEnvironment.SkyBox
                tonemapMode: SceneEnvironment.TonemapModeLinear
                probeExposure: 1
                depthPrePassEnabled: false
                depthTestEnabled: true
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: root.perfMode ? SceneEnvironment.Medium : SceneEnvironment.VeryHigh
                clearColor: "#000000"
                probeHorizon: 0.5

                Texture {
                    id: _Hall
                    source: root.downloadBase + "/content/images/HDR/_Hall.ktx"
                    mipFilter: Texture.Linear
                    scaleV: 2
                }
                temporalAAEnabled: false
            }
            //! [Showhall Scene Environment]

            //! [Desert Scene Environment]
            ExtendedSceneEnvironment {
                id: desert

                lightProbe: _Desert
                glowStrength: 2
                glowBloom: 0.11191
                glowEnabled: true

                depthPrePassEnabled: false
                probeExposure: btnRain.checked ? 1.0 : 1.5
                tonemapMode: SceneEnvironment.TonemapModeLinear
                backgroundMode: SceneEnvironment.SkyBox
                depthTestEnabled: true
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: root.perfMode ? SceneEnvironment.Medium : SceneEnvironment.VeryHigh
                clearColor: "#000000"
                probeHorizon: 0.5
                temporalAAEnabled: false
                fog: Fog {}
            }
            //! [Desert Scene Environment]

            //! [Videoroom Scene Environment]
            ExtendedSceneEnvironment {
                id: videoRoom

                aoSoftness: 0
                aoDistance: 0
                probeExposure: 1
                tonemapMode: SceneEnvironment.TonemapModeLinear
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: root.perfMode ? SceneEnvironment.Medium : SceneEnvironment.VeryHigh
                clearColor: "#000000"
                depthPrePassEnabled: false
                backgroundMode: SceneEnvironment.Color
                depthTestEnabled: true
                probeHorizon: 0
                Vignette {
                    id: vignette
                    vignetteStrength: 15
                    vignetteColor: Qt.vector3d(0, 0, 0)
                }
                temporalAAEnabled: false
            }
            //! [Videoroom Scene Environment]

            //! [Studio Scene Environment]
            ExtendedSceneEnvironment {
                id: studio
                aoEnabled: false
                antialiasingQuality: SceneEnvironment.VeryHigh
                antialiasingMode: SceneEnvironment.MSAA

                tonemapMode: SceneEnvironment.TonemapModeLinear
                probeExposure: 1
                probeHorizon: 1
                clearColor: "#000000"
                temporalAAEnabled: false
                depthTestEnabled: true
                backgroundMode: SceneEnvironment.Color
                depthPrePassEnabled: false
            }
            //! [Studio Scene Environment]
        }

        Node {
            id: __materialLibrary__

            PrincipledMaterial {
                id: groundMat1
                opacity: 1
                normalStrength: 0.5
                specularAmount: 0
                metalness: 0
                opacityChannel: Material.A
                blendMode: PrincipledMaterial.SourceOver
                lighting: PrincipledMaterial.NoLighting
                specularTint: 1
                baseColor: "#000000"
                clearcoatRoughnessAmount: 0
                emissiveFactor.y: 0
                objectName: "groundMat1"
                alphaMode: PrincipledMaterial.Default
                depthDrawMode: Material.AlwaysDepthDraw
                occlusionAmount: 1
                roughness: 1
                opacityMap: ground1
                occlusionMap: ground1
                transmissionChannel: Material.R
                clearcoatAmount: 0
                transmissionFactor: 0
                alphaCutoff: 1
                cullMode: Material.BackFaceCulling
                emissiveFactor.x: 0
            }

            PrincipledMaterial {
                id: principledMaterial2
                specularAmount: 0
                clearcoatAmount: 0
                lighting: PrincipledMaterial.NoLighting
                specularTint: 0
                baseColor: "#ffffff"
                objectName: "principledMaterial2"
                baseColorMap: videoBG

                Texture {
                    id: videoBG
                    // Adjust these to suit the target HW quality/performance
                    property real videoWallWidth: 1024
                    property real videoWallHeight: 512
                    mappingMode: Texture.Environment
                    sourceItem: wallEffect1

                    WallEffect1 {
                        id: wallEffect1

                        width: videoBG.videoWallWidth
                        height: videoBG.videoWallHeight
                        timeRunning: visible
                        visible: videoBG.sourceItem == wallEffect1
                        downloadBase: root.downloadBase
                    }

                    WallEffect2 {
                        id: wallEffect2

                        width: videoBG.videoWallWidth
                        height: videoBG.videoWallHeight
                        timeRunning: visible
                        visible: videoBG.sourceItem == wallEffect2
                        downloadBase: root.downloadBase
                    }

                    WallEffect3 {
                        id: wallEffect3

                        width: videoBG.videoWallWidth
                        height: videoBG.videoWallHeight
                        timeRunning: visible
                        visible: videoBG.sourceItem == wallEffect3
                        downloadBase: root.downloadBase
                    }
                }
                roughness: 0
            }

            PrincipledMaterial {
                id: rectMaterial
                fresnelScaleBiasEnabled: false
                opacityMap: dot
                baseColor: "#b1a5b7"
                objectName: "rectMaterial"
                baseColorMap: tg1kfdzq_2K_Albedo
                depthDrawMode: Material.OpaqueOnlyDepthDraw
                normalMap: tg1kfdzq_2K_Normal
                roughnessMap: tg1kfdzq_2K_Roughness
                roughness: 0.07365
                occlusionMap: tg1kfdzq_2K_AO
            }

            Texture {
                id: _Desert
                source: root.downloadBase + "/content/images/HDR/low/_Desert.ktx"
                mipFilter: Texture.Linear
                scaleV: 3
            }

            Texture {
                id: concrete1_Height1
                source: root.downloadBase + "/content/images/concrete1_Height.png"
                mipFilter: Texture.Linear
                scaleV: 10
                mappingMode: Texture.UV
                generateMipmaps: true
                scaleU: 5
            }

            Texture {
                id: concrete1_Normalogl1
                source: root.downloadBase + "/content/images/concrete1_Normal-ogl.png"
                mipFilter: Texture.Linear
                scaleV: 10
                generateMipmaps: true
                scaleU: 5
            }

            Texture {
                id: ground1
                source: root.downloadBase + "/content/images/Ground.png"
                autoOrientation: true
            }

            Texture {
                id: tg1kfdzq_2K_Albedo
                source: root.downloadBase + "/content/images/tg1kfdzq_2K_Albedo.jpg"
                mipFilter: Texture.Linear
                scaleV: 40
                generateMipmaps: true
                scaleU: 40
            }

            Texture {
                id: tg1kfdzq_2K_AO
                source: root.downloadBase + "/content/images/tg1kfdzq_2K_AO.jpg"
                mipFilter: Texture.Linear
                scaleV: 40
                generateMipmaps: true
                scaleU: 40
            }

            Texture {
                id: tg1kfdzq_2K_Normal
                source: root.downloadBase + "/content/images/tg1kfdzq_2K_Normal.jpg"
                mipFilter: Texture.Linear
                scaleV: 40
                generateMipmaps: true
                scaleU: 40
            }

            Texture {
                id: tg1kfdzq_2K_Roughness
                source: root.downloadBase + "/content/images/tg1kfdzq_2K_Roughness.jpg"
                mipFilter: Texture.Linear
                scaleV: 40
                generateMipmaps: true
                scaleU: 40
            }

            PrincipledMaterial {
                id: rectMaterial3
                emissiveFactor.z: 3
                emissiveFactor.x: 3
                emissiveFactor.y: 3
                objectName: "rectMaterial3"
            }

            PrincipledMaterial {
                id: rectMaterial2
                fresnelScaleBiasEnabled: false
                specularAmount: 0.3
                clearcoatAmount: 0
                specularTint: 0.95
                baseColor: "#222222"
                objectName: "rectMaterial2"
                baseColorMap: vlkhcah_2K_Albedo
                depthDrawMode: Material.OpaqueOnlyDepthDraw
                normalMap: vlkhcah_2K_Normal
                roughnessMap: vlkhcah_2K_Roughness
                roughness: 0.5
                occlusionMap: vlkhcah_2K_AO
            }

            Texture {
                id: vlkhcah_2K_Albedo
                source: root.downloadBase + "/content/images/vlkhcah_2K_Albedo.jpg"
                mipFilter: Texture.Linear
                scaleV: 20
                generateMipmaps: true
                scaleU: 20
            }

            Texture {
                id: vlkhcah_2K_AO
                source: root.downloadBase + "/content/images/vlkhcah_2K_AO.jpg"
                mipFilter: Texture.Linear
                scaleV: 20
                generateMipmaps: true
                scaleU: 20
            }

            Texture {
                id: vlkhcah_2K_Normal
                source: root.downloadBase + "/content/images/vlkhcah_2K_Normal.jpg"
                mipFilter: Texture.Linear
                scaleV: 20
                generateMipmaps: true
                scaleU: 20
            }

            Texture {
                id: vlkhcah_2K_Roughness
                source: root.downloadBase + "/content/images/vlkhcah_2K_Roughness.jpg"
                mipFilter: Texture.Linear
                scaleV: 20
                generateMipmaps: true
                scaleU: 20
            }

            property real snowStrength: 1.0
            NumberAnimation {
                running: !btnSnow.checked
                target: __materialLibrary__
                property: "snowStrength"
                to: 0
                duration: 10000
            }

            NumberAnimation {
                running: btnSnow.checked
                target: __materialLibrary__
                property: "snowStrength"
                to: 1.
                duration: 5000
            }

            PrincipledExSnowMaterial {
                id: rectMaterial4
                specularAmount: 0.5
                clearcoatAmount: 0
                snowStrength: __materialLibrary__.snowStrength
                snowViewProjection: particleMaskGround.viewProjection
                snowDisplacementMap: particleMaskGroundTexture
                snowMaxDisplacement: 5
                // specularTint: 0
                baseColor: "#222222"
                objectName: "rectMaterial4"
                baseColorMap: vlkhcah_2K_Albedo1
                normalMap: vlkhcah_2K_Normal1
                roughnessChannel: Material.G
                roughnessMap: vlkhcah_2K_Roughness1
                roughness: 0.35
                occlusionMap: vlkhcah_2K_AO1
                clearcoatRoughnessAmount: 0
            }


            PrincipledMaterial {
                id: rectMaterialBase
                specularAmount: 0.5
                clearcoatAmount: 0
                // specularTint: 0
                baseColor: "#222222"
                objectName: "rectMaterialBase"
                baseColorMap: vlkhcah_2K_Albedo1Tile
                normalMap: vlkhcah_2K_Normal1Tile
                roughnessChannel: Material.G
                roughnessMap: vlkhcah_2K_Roughness1Tile
                roughness: 0.35
                occlusionMap: vlkhcah_2K_AO1Tile
                clearcoatRoughnessAmount: 0
                opacityMap: sphereMask
            }

            DefaultMaterial {
                id: backlight
                opacity: 0.3
                lighting: DefaultMaterial.NoLighting
                opacityChannel: Material.A
                diffuseColor: "#ff0000"
                objectName: "backlight"

            }

            PrincipledMaterial {
                id: backlightNew
                opacity: 0.551
                lighting: PrincipledMaterial.NoLighting
                blendMode: PrincipledMaterial.Screen
                emissiveFactor.x: 1
                opacityMap: backlight1
                baseColor: "#fe0303"
                objectName: "New Material"
            }

            Texture {
                id: vlkhcah_2K_Albedo1
                source: root.downloadBase + "/content/images/vlkhcah_2K_Albedo.jpg"
                mipFilter: Texture.Linear
                scaleV: 3
                generateMipmaps: true
                scaleU: 3
            }

            Texture {
                id: vlkhcah_2K_AO1
                source: root.downloadBase + "/content/images/vlkhcah_2K_AO.jpg"
                mipFilter: Texture.Linear
                scaleV: 3
                generateMipmaps: true
                scaleU: 3
            }

            Texture {
                id: vlkhcah_2K_Normal1
                source: root.downloadBase + "/content/images/vlkhcah_2K_Normal.jpg"
                mipFilter: Texture.Linear
                scaleV: 3
                generateMipmaps: true
                scaleU: 3
            }

            Texture {
                id: vlkhcah_2K_Roughness1
                source: root.downloadBase + "/content/images/vlkhcah_2K_Roughness.jpg"
                mipFilter: Texture.Linear
                scaleV: 3
                generateMipmaps: true
                scaleU: 3
            }

            Texture {
                id: vlkhcah_2K_Albedo1Tile
                source: root.downloadBase + "/content/images/vlkhcah_2K_Albedo.jpg"
                mipFilter: Texture.Linear
                scaleV: 9
                generateMipmaps: true
                scaleU: 9
            }

            Texture {
                id: vlkhcah_2K_AO1Tile
                source: root.downloadBase + "/content/images/vlkhcah_2K_AO.jpg"
                mipFilter: Texture.Linear
                scaleV: 9
                generateMipmaps: true
                scaleU: 9
            }

            Texture {
                id: vlkhcah_2K_Normal1Tile
                source: root.downloadBase + "/content/images/vlkhcah_2K_Normal.jpg"
                mipFilter: Texture.Linear
                scaleV: 9
                generateMipmaps: true
                scaleU: 9
            }

            Texture {
                id: vlkhcah_2K_Roughness1Tile
                source: root.downloadBase + "/content/images/vlkhcah_2K_Roughness.jpg"
                mipFilter: Texture.Linear
                scaleV: 9
                generateMipmaps: true
                scaleU: 9
            }

            Texture {
                id: backlight1
                source: root.downloadBase + "/content/images/backlight.png"
            }

            Texture {
                id: dot
                source: root.downloadBase + "/content/images/dot.png"
            }

            Texture {
                id: sphereMask
                source: "qrc:/qt/qml/Quick3DAssets/Snow/sphere.png"
            }
        }


    }

    MouseRotatorAndWASD {
        xInvert: false
        yInvert: true
        controlledObject: btnDemo.checked? null : cameraRoot
    }

    Timer {
        id: releaseTimer
        running: false
        interval: 1000
        onTriggered: {
            particleMaskGround.brushPositionInUV = Qt.vector2d(-10., -10.)
            particleMaskCar.brushPositionInUV = Qt.vector2d(-10., -10.)
        }
    }

    DragHandler {
        id: dragHandler

        target: null
        enabled: true
        acceptedButtons: Qt.RightButton

        onCentroidChanged: {
            let result = view3D.pick(centroid.position.x, centroid.position.y)

            if (result) {
                if (result.scenePosition.y > animatedstateFloor.scenePosition.y) {
                    const viewWidth = particleMaskCar.width
                    particleMaskCar.brushPositionInUV = Qt.vector2d(((result.scenePosition.x - originalSnowParticleSystem.scenePosition.x) / viewWidth + 0.5),
                                                                   ((result.scenePosition.z - originalSnowParticleSystem.scenePosition.z) / viewWidth + 0.5))
                    particleMaskCar.brushSize = sceneCamera2.scenePosition.minus(ev_SportsCar_low.scenePosition).length() / viewWidth * 0.03
                }else {
                    const viewWidth = particleMaskGround.width
                    particleMaskGround.brushPositionInUV = Qt.vector2d(((result.scenePosition.x - originalSnowParticleSystem.scenePosition.x) / viewWidth + 0.5),
                                                                   ((result.scenePosition.z - originalSnowParticleSystem.scenePosition.z) / viewWidth + 0.5))
                    particleMaskGround.brushSize = sceneCamera2.scenePosition.minus(ev_SportsCar_low.scenePosition).length() / viewWidth * 0.03
                }
                releaseTimer.restart()
            }
        }
    }

    PinchWidget {
        anchors.fill: parent
        camera: sceneCamera2
    }

    DoorButton {
        id: doorButtonLeft
        x: 0
        y: 0
        // > than 326
        isRendered: cameraRoot.eulerRotation.y < 123 && !demo.running || cameraRoot.eulerRotation.y > 326 && !demo.running || ev_SportsCar_low.doorLeftIsOpen? 1.0 : 0.0 && !demo.running
        visible: true
        trackedWorldPosition: ev_SportsCar_low.leftDoorPositionerPos
        designEffectBackgroundLayer: view3D
        rootItem: root
        downloadBase: root.downloadBase
        camera: sceneCamera2
    }
    DoorButton {
        id: doorButtonRight
        x: 0
        y: 0
        isRendered: cameraRoot.eulerRotation.y < 305 && cameraRoot.eulerRotation.y > 150 && !demo.running || ev_SportsCar_low.doorRightIsOpen? 1.0 : 0.0 && !demo.running
        visible: true
        trackedWorldPosition: ev_SportsCar_low.rightDoorPositionerPos
        designEffectBackgroundLayer: view3D
        rootItem: root
        downloadBase: root.downloadBase
        camera: sceneCamera2
    }
    DoorButton {
        id: hoodButton
        x: 0
        y: 0
        isRendered: cameraRoot.eulerRotation.y < 50 && !demo.running || cameraRoot.eulerRotation.y > 226 && !demo.running || ev_SportsCar_low.hoodIsOpen? 1.0 : 0.0&& !demo.running
        visible: true
        trackedWorldPosition: ev_SportsCar_low.hoodPositionerPos
        designEffectBackgroundLayer: view3D
        rootItem: root
        downloadBase: root.downloadBase
        camera: sceneCamera2
    }
    DoorButton {
        id: trunkbutton
        x: 0
        y: 0
        isRendered: cameraRoot.eulerRotation.y > 50 && cameraRoot.eulerRotation.y < 226 && !demo.running || ev_SportsCar_low.trunkIsOpen? 1.0 : 0.0&& !demo.running
        visible: true
        trackedWorldPosition: ev_SportsCar_low.trunkPositionerPos
        designEffectBackgroundLayer: view3D
        rootItem: root
        downloadBase: root.downloadBase
        camera: sceneCamera2
    }
    Item {
        id: gui

        anchors.fill: parent

        Logo {
            id: logo

            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 28
            anchors.topMargin: 28

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    debugBox.visible = !debugBox.visible
                }
            }
        }

        DebugBox {
            id: debugBox

            visible: false
            anchors.right: parent.right
            anchors.top: logo.bottom
            anchors.rightMargin: 15
            anchors.topMargin: 15

            sView: view3D
        }


        Item {
            id: weatherControls

            visible: btnWeather.checked
            width: weatherRow.width
            height: weatherRow.height
            anchors.right: mainControls.right
            anchors.rightMargin: 95
            anchors.bottomMargin: 5
            anchors.bottom: mainControls.top

            scale: mainControls.scale

            Column {
                id: weatherRow

                spacing: 5

                KissButton {
                    id: btnRain
                    visible: btnWeather.checked
                    downloadBase: root.downloadBase
                    iconId: btnRain.checked ? 27 : 26
                    buttonText: "Rain"
                    group: "toggle"
                    checked: false
                    onClicked: {
                        ev_SportsCar_low.stateController = btnRain.checked ? ev_SportsCar_low.stateController + 4 : ev_SportsCar_low.stateController
                        if (checked)
                            btnSnow.checked = false
                    }
                }

                KissButton {
                    id: btnSnow
                    visible: btnWeather.checked
                    downloadBase: root.downloadBase
                    iconId: btnSnow.checked ? 29 : 28
                    buttonText: "Snow"
                    group: "toggle"
                    checked: false
                    property bool btnAnimatedCheckState: btnAnimated.checked
                    onBtnAnimatedCheckStateChanged: {
                        if (!btnAnimatedCheckState && checked)
                            btnSnow.toggleCheck()
                    }
                    onClicked: {
                        if (checked) {
                            btnRain.checked = false
                            if (!btnAnimated.checked)
                                btnAnimated.toggleCheck()
                        }
                    }
                }
            }
        }

        Rectangle {
            id: videoControls

            visible: false
            width: videoRow.width
            height: videoRow.height
            radius: 8
            color: "#5c000000"
            anchors.horizontalCenter: mainControls.horizontalCenter
            anchors.bottom: mainControls.top
            anchors.bottomMargin: 16 - (1 - videoControls.scale) * 110

            scale: mainControls.scale

            Row {
                id: videoRow

                padding: 10
                spacing: 30

                QtObject {
                    id: groupVideo
                    property var buttons: [btnVideo1, btnVideo2, btnVideo3]
                    property Item checkedButton: btnVideo1
                }

                KissButton {
                    id: btnVideo1

                    width: 53
                    height: 43
                    radius: 8
                    iconId: 5
                    group: groupVideo
                    checked: true
                    downloadBase: root.downloadBase

                    Image {
                        source: btnVideo1.checked ? root.downloadBase + "/content/images/1_active.png"
                                                  : root.downloadBase + "/content/images/1_idle.png"
                        anchors.centerIn: parent
                    }

                    onClicked: videoBG.sourceItem = wallEffect1
                }

                KissButton {
                    id: btnVideo2

                    width: 53
                    height: 43
                    radius: 8
                    iconId: 5
                    group: groupVideo
                    downloadBase: root.downloadBase
                    onClicked: videoBG.sourceItem = wallEffect2

                    Image {
                        source: btnVideo2.checked ? root.downloadBase + "/content/images/2_active.png"
                                                  : root.downloadBase + "/content/images/2_idle.png"
                        anchors.centerIn: parent
                    }
                }

                KissButton {
                    id: btnVideo3

                    width: 53
                    height: 43
                    radius: 8
                    iconId: 5
                    group: groupVideo
                    downloadBase: root.downloadBase
                    onClicked: videoBG.sourceItem = wallEffect3

                    Image {
                        source: btnVideo3.checked ? root.downloadBase + "/content/images/3_active.png"
                                                  : root.downloadBase + "/content/images/3_idle.png"
                        anchors.centerIn: parent
                    }
                }
            }
        }

        Row {
            id: mainControls
            property bool isOn: false
            spacing: 8
            width: parent.width

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 28 - (1 - mainControls.scale) * 60
            layer.enabled: true
            rightPadding: 0
            scale: parent.width / Constants.width
            transformOrigin: Item.Left


            NumberAnimation {
                id: menuOffAnimation
                target: mainControls
                property: "x"
                duration: 1000
                easing.type: Easing.InOutQuad
                running: !mainControls.isOn
                from: 0
                to: root.width
            }
            NumberAnimation {
                id: menuOnAnimation
                target: mainControls
                property: "x"
                duration: 1000
                easing.type: Easing.InOutQuad
                running: mainControls.isOn
                from: root.width
                to: 0

            }
            NumberAnimation {
                id: menuOnAnimationOpacity
                target: mainControls
                property: "opacity"
                duration: 500
                easing.type: Easing.InOutQuad
                running: mainControls.isOn
                from: 0
                to: 1
            }
            NumberAnimation {
                id: menuOffAnimationOpacity
                target: mainControls
                property: "opacity"
                duration: 500
                easing.type: Easing.InOutQuad
                running: !mainControls.isOn
                from: 1
                to: 0
            }

            Item {
                width: 20
                height: 1
            }

            KissButton {
                id: btnDemo

                iconId: btnDemo.checked ? 8 : 7
                buttonText: "Demo Mode"
                group: "toggle"
                downloadBase: root.downloadBase
                onClicked: {
                    cameraRoot.eulerRotation.y > 180? cameraResetAnimation.yVal = 365 : cameraResetAnimation.yVal = 5
                    btnDemo.checked? cameraResetAnimation.running = true : demo.running = false
                }

            }

            KissButton {
                id: btnReset

                iconId: btnReset.checked ? 2 : 3
                buttonText: "Reset camera"
                downloadBase: root.downloadBase
                onClicked: {
                    cameraRoot.eulerRotation.y > 180? cameraResetAnimation.yVal = 360 : cameraResetAnimation.yVal = 0
                    cameraResetAnimation.running = true

                    if (btnDemo.checked)
                        btnDemo.toggleCheck()
                }
            }

            KissButtonSeparator {
            }

            QtObject {
                id: groupScene

                property var buttons: [btnDesert, btnGarage, btnStudio, btnAnimated]
                property Item checkedButton: btnDesert
            }

            KissButton {
                id: btnDesert

                buttonText: "Desert"
                iconId: btnDesert.checked ? 22 : 23
                group: groupScene
                downloadBase: root.downloadBase
            }

            KissButton {
                id: btnGarage

                buttonText: "Garage"
                iconId: btnGarage.checked ? 20 : 19
                group: groupScene
                downloadBase: root.downloadBase
            }

            KissButton {
                id: btnStudio

                buttonText: "Studio"
                iconId: btnStudio.checked ? 4 : 21
                group: groupScene
                downloadBase: root.downloadBase

            }

            KissButton {
                id: btnAnimated

                buttonText: "Animated"
                iconId: btnAnimated.checked ? 13 : 1
                group: groupScene
                downloadBase: root.downloadBase
            }

            KissButtonSeparator {
            }

            QtObject {
                id: groupPaint

                property var buttons: [whitePaint, blackPaint, yellowPaint, redPaint]
                property Item checkedButton: blackPaint
            }

            KissButton {
                id: whitePaint

                iconId: 17
                buttonText: "White"
                group: groupPaint

                onClicked: ev_SportsCar_low.stateController = 1
                downloadBase: root.downloadBase
            }

            KissButton {
                id: blackPaint
                checked: true

                iconId: 11
                buttonText: "Black"
                group: groupPaint
                downloadBase: root.downloadBase

                onClicked: ev_SportsCar_low.stateController = 0
            }

            KissButton {
                id: yellowPaint

                iconId: 16
                buttonText: "Yellow"
                group: groupPaint
                downloadBase: root.downloadBase

                onClicked: ev_SportsCar_low.stateController = 2
            }

            KissButton {
                id: redPaint

                iconId: 18
                buttonText: "Red"
                group: groupPaint
                downloadBase: root.downloadBase

                onClicked: ev_SportsCar_low.stateController = 3
            }

            KissButtonSeparator {
            }

            KissButton {
                id: btnLight

                iconId: btnLight.checked ? 15 : 14
                buttonText: "Lights"
                group: "toggle"
                checked: true
                downloadBase: root.downloadBase
            }

            KissButton {
                id: btnWeather
                downloadBase: root.downloadBase
                iconId: btnWeather.checked ? 31 : 30
                buttonText: "Weather"
                group: "toggle"
                checked: false
                onClicked: {
                    if (!checked) {
                        btnRain.checked = false
                        btnSnow.checked = false
                    }
                }
            }

            KissButtonSeparator {
            }

            KissButton {
                id: rightseparator
                iconId: btnLight.checked ? 15 : 14
                group: "toggle"
                checked: true
                buttonText: ""
                width: 70
                visible: false
                downloadBase: root.downloadBase
            }
        }

        KissButton {
            id: menutoggle
            menubutton: true
            anchors.right: parent.right
            anchors.rightMargin: 30 - (1 - mainControls.scale) * 60
            iconId: mainControls.isOn ? 25 : 24
            group: "toggle"
            checked: false
            buttonText: ""
            width: 70
            scale: parent.width / Constants.width
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 38 - (1 - mainControls.scale) * 60
            onCheckedChanged: !checked? mainControls.isOn = false : mainControls.isOn = true
            downloadBase: root.downloadBase
            onClicked: {
                buttonLoopAnimation.running = false
                menutoggle.scale = 1
            }

            SequentialAnimation{
                id: buttonLoopAnimation
                running: true
                loops: -1

                NumberAnimation {
                    target: menutoggle
                    property: "scale"
                    duration: 1000
                    easing.type: Easing.InOutQuad
                    from: 1
                    to: 1.1
                }

                NumberAnimation {
                    target: menutoggle
                    property: "scale"
                    duration: 1000
                    easing.type: Easing.InOutQuad
                    from: 1.1
                    to: 1
                }
            }
        }
    }

    Timeline {
        id: demoAnimation

        animations: [
            TimelineAnimation {
                id: demo
                running: false
                pingPong: false
                loops: -1
                duration: 61114
                to: 61114
                from: 0
            }
        ]
        enabled: true
        startFrame: 0
        endFrame: 61114

        KeyframeGroup {
            target: sceneCamera2
            property: "fieldOfView"
            Keyframe {
                value: 64
                frame: 5
            }

            Keyframe {
                value: 84
                frame: 12005
            }

            Keyframe {
                value: 64
                frame: 12004
            }

            Keyframe {
                value: 64
                frame: 16491
            }

            Keyframe {
                value: 56
                frame: 16492
            }

            Keyframe {
                value: 79.85046
                frame: 21003
            }

            Keyframe {
                value: 23.85
                frame: 21004
            }

            Keyframe {
                value: 15.85
                frame: 34501
            }

            Keyframe {
                value: 51.85
                frame: 46746
            }

            Keyframe {
                value: 15.87412
                frame: 40300
            }

            Keyframe {
                frame: 34500
                value: 15.85881
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "x"
            Keyframe {
                value: -31.82979
                frame: 5
            }

            Keyframe {
                value: 37.88391
                frame: 5979
            }

            Keyframe {
                value: -229.261
                frame: 5979
            }

            Keyframe {
                value: -174.30124
                frame: 12004
            }

            Keyframe {
                value: -31.82972
                frame: 5
            }

            Keyframe {
                value: -337.83273
                frame: 12005
            }

            Keyframe {
                value: -307.71436
                frame: 16491
            }

            Keyframe {
                value: 938.85046
                frame: 16492
            }

            Keyframe {
                value: 550.57489
                frame: 21003
            }

            Keyframe {
                value: -255.06708
                frame: 21004
            }

            Keyframe {
                value: -102.92751
                frame: 28504
            }

            Keyframe {
                value: -634.68378
                frame: 28505
            }

            Keyframe {
                value: -779.27954
                frame: 34500
            }

            Keyframe {
                value: -574.65594
                frame: 34501
            }

            Keyframe {
                value: -566.81525
                frame: 40301
            }

            Keyframe {
                value: 378.97412
                frame: 40302
            }

            Keyframe {
                value: 595.88428
                frame: 46745
            }

            Keyframe {
                value: 649.59082
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: -371.06754
                frame: 61114
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "y"
            Keyframe {
                value: 128.48135
                frame: 5
            }

            Keyframe {
                value: 140.70691
                frame: 5979
            }

            Keyframe {
                value: 161.34668
                frame: 5979
            }

            Keyframe {
                value: 197.42435
                frame: 12004
            }

            Keyframe {
                value: 223.60617
                frame: 5
            }

            Keyframe {
                value: 95.44727
                frame: 12005
            }

            Keyframe {
                value: 152.79813
                frame: 16491
            }

            Keyframe {
                value: 152.798
                frame: 16492
            }

            Keyframe {
                value: 185.36548
                frame: 21003
            }

            Keyframe {
                value: 251.7169
                frame: 21004
            }

            Keyframe {
                value: 359.80371
                frame: 28504
            }

            Keyframe {
                value: 359.80347
                frame: 28505
            }

            Keyframe {
                value: 359.80347
                frame: 34500
            }

            Keyframe {
                value: 222.64905
                frame: 34501
            }

            Keyframe {
                value: 222.64905
                frame: 40301
            }

            Keyframe {
                value: 222.6488
                frame: 40302
            }

            Keyframe {
                value: 222.64883
                frame: 46745
            }

            Keyframe {
                value: 222.64902
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: 147.90091
                frame: 61114
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "z"
            Keyframe {
                value: 224.36035
                frame: 5
            }

            Keyframe {
                value: 218.07159
                frame: 5979
            }

            Keyframe {
                value: -157.46875
                frame: 5979
            }

            Keyframe {
                value: 6.80872
                frame: 12004
            }

            Keyframe {
                value: 665.36609
                frame: 5
            }

            Keyframe {
                value: 149.68378
                frame: 12005
            }

            Keyframe {
                value: 180.37372
                frame: 16491
            }

            Keyframe {
                value: -407.83264
                frame: 16492
            }

            Keyframe {
                value: -282.50183
                frame: 21003
            }

            Keyframe {
                value: 770.57483
                frame: 21004
            }

            Keyframe {
                value: 770.57465
                frame: 28504
            }

            Keyframe {
                value: -622.59058
                frame: 28505
            }

            Keyframe {
                value: -510.37622
                frame: 34500
            }

            Keyframe {
                value: 358.71863
                frame: 34501
            }

            Keyframe {
                value: 438.66461
                frame: 40301
            }

            Keyframe {
                value: -915.28137
                frame: 40302
            }

            Keyframe {
                value: -673.06482
                frame: 46745
            }

            Keyframe {
                value: 223.57843
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: 506.23959
                frame: 61114
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "eulerRotation.x"
            Keyframe {
                value: -12.17219
                frame: 5
            }

            Keyframe {
                value: -11.9523
                frame: 5979
            }

            Keyframe {
                value: -13.40152
                frame: 5979
            }

            Keyframe {
                value: -32.41507
                frame: 12004
            }

            Keyframe {
                value: -13.89176
                frame: 12005
            }

            Keyframe {
                value: -13.89176
                frame: 16491
            }

            Keyframe {
                value: -6.35897
                frame: 16492
            }

            Keyframe {
                value: -10.07228
                frame: 21003
            }

            Keyframe {
                value: -16.93475
                frame: 21004
            }

            Keyframe {
                value: -17.91578
                frame: 28504
            }

            Keyframe {
                value: -17.91577
                frame: 28505
            }

            Keyframe {
                value: -17.95313
                frame: 34500
            }

            Keyframe {
                value: -17.95315
                frame: 34501
            }

            Keyframe {
                value: -17.95315
                frame: 40301
            }

            Keyframe {
                value: -6.10891
                frame: 40302
            }

            Keyframe {
                value: -6.10891
                frame: 46745
            }

            Keyframe {
                value: -6.10892
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: -6.10892
                frame: 61114
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "eulerRotation.y"
            Keyframe {
                value: 0
                frame: 5
            }

            Keyframe {
                value: 11.06957
                frame: 5979
            }

            Keyframe {
                value: -117.02943
                frame: 5979
            }

            Keyframe {
                value: -74.9571
                frame: 12004
            }

            Keyframe {
                value: -77.58958
                frame: 12005
            }

            Keyframe {
                value: -68.8636
                frame: 16491
            }

            Keyframe {
                value: 110.72486
                frame: 16492
            }

            Keyframe {
                value: 114.77937
                frame: 21003
            }

            Keyframe {
                value: -10.47536
                frame: 21004
            }

            Keyframe {
                value: -10.28948
                frame: 28504
            }

            Keyframe {
                value: -129.832
                frame: 28505
            }

            Keyframe {
                value: -128.30945
                frame: 34500
            }

            Keyframe {
                value: -60.4214
                frame: 34501
            }

            Keyframe {
                value: -59.90898
                frame: 40301
            }

            Keyframe {
                value: 161.23984
                frame: 40302
            }

            Keyframe {
                value: 141.07248
                frame: 46745
            }

            Keyframe {
                value: 82.73631
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: -36.03967
                frame: 61114
            }
        }

        KeyframeGroup {
            target: sceneCamera2
            property: "eulerRotation.z"
            Keyframe {
                value: -0
                frame: 5
            }

            Keyframe {
                value: -2.32017
                frame: 5979
            }

            Keyframe {
                value: 0
                frame: 5979
            }

            Keyframe {
                value: 1.92757
                frame: 12004
            }

            Keyframe {
                value: 0
                frame: 12005
            }

            Keyframe {
                value: 0
                frame: 16491
            }

            Keyframe {
                value: -0.00002
                frame: 16492
            }

            Keyframe {
                value: -0.15695
                frame: 21003
            }

            Keyframe {
                value: -0.55012
                frame: 21004
            }

            Keyframe {
                value: -1.1708
                frame: 28504
            }

            Keyframe {
                value: -1.1708
                frame: 28505
            }

            Keyframe {
                value: -3.75297
                frame: 34500
            }

            Keyframe {
                value: -3.75297
                frame: 34501
            }

            Keyframe {
                value: -3.75297
                frame: 40301
            }

            Keyframe {
                value: 0
                frame: 40302
            }

            Keyframe {
                value: -0
                frame: 46745
            }

            Keyframe {
                value: -0.00001
                frame: 46746
            }

            Keyframe {
                easing.bezierCurve: [0.19, 1, 0.22, 1, 1, 1]
                value: -0
                frame: 61114
            }
        }

        KeyframeGroup {
            target: cameraRoot
            property: "eulerRotation.y"

            Keyframe {
                value: 5
                frame: 0
            }

            Keyframe {
                value: 0
                frame: 5979
            }
        }

        KeyframeGroup {
            target: cameraRoot
            property: "eulerRotation.x"

            Keyframe {
                value: 9
                frame: 0
            }

            Keyframe {
                value: 0
                frame: 5979
            }
        }

        KeyframeGroup {
            target: cameraRoot
            property: "eulerRotation.z"

            Keyframe {
                value: 0
                frame: 5
            }
        }
        KeyframeGroup {
            target: cameraRoot
            property: "y"

            Keyframe {
                value: -5
                frame: 5
            }

            Keyframe {
                value: 0
                frame: 5979
            }
        }
        KeyframeGroup {
            target: cameraRoot
            property: "x"

            Keyframe {
                value: 0
                frame: 5
            }
        }
        KeyframeGroup {
            target: cameraRoot
            property: "z"

            Keyframe {
                value: 0
                frame: 5
            }
        }
    }

    ParallelAnimation {
        id:cameraResetAnimation
        running: false
        property real yVal: 0
        onFinished: btnDemo.checked? demo.running = true : demo.running = false
        NumberAnimation {
            target: cameraRoot
            property: "eulerRotation.y"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.eulerRotation.y
            to: cameraResetAnimation.yVal
        }

        NumberAnimation {
            target: cameraRoot
            property: "eulerRotation.x"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.eulerRotation.x
            to: 9
        }

        NumberAnimation {
            target: cameraRoot
            property: "eulerRotation.z"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.eulerRotation.z
            to: 0
        }

        NumberAnimation {
            target: cameraRoot
            property: "x"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.x
            to: 0
        }

        NumberAnimation {
            target: cameraRoot
            property: "y"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.y
            to: -5
        }

        NumberAnimation {
            target: cameraRoot
            property: "z"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: cameraRoot.z
            to: 0
        }

        NumberAnimation {
            target: sceneCamera2
            property: "fieldOfView"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.fieldOfView
            to:64
        }

        NumberAnimation {
            target: sceneCamera2
            property: "x"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.x
            to:-31.83
        }

        NumberAnimation {
            target: sceneCamera2
            property: "y"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.y
            to: 223.61
        }

        NumberAnimation {
            target: sceneCamera2
            property: "z"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.z
            to:665.36
        }

        NumberAnimation {
            target: sceneCamera2
            property: "eulerRotation.x"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.eulerRotation.x
            to:-12.17
        }

        NumberAnimation {
            target: sceneCamera2
            property: "eulerRotation.y"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.eulerRotation.y
            to:0
        }

        NumberAnimation {
            target: sceneCamera2
            property: "eulerRotation.z"
            duration: 1000
            easing.type: Easing.InOutQuad
            from: sceneCamera2.eulerRotation.z
            to:0
        }
    }

    states: [
        State {
            name: "State1"

            when: btnDesert.checked

            PropertyChanges {
                target: view3D
                environment: desert
            }

            PropertyChanges {
                target: lightDirectional
                color: "#e1e1e1"
                shadowMapQuality: Light.ShadowMapQualityVeryHigh
                shadowMapFar: 0
                castsShadow: false
                brightness: 0
                ambientColor: "#385667"
                eulerRotation.z: -179.99934
                eulerRotation.y: 179.99934
                eulerRotation.x: -88.85005
            }

            PropertyChanges {
                target: lightDirectional1
                color: "#e6f4fe"
                brightness: 2.58
                ambientColor: "#223843"
                eulerRotation.z: -0.00001
                eulerRotation.y: 0.00001
                eulerRotation.x: -42.00026
            }

            PropertyChanges {
                target: desertGround
                x: 0
                y: 1
                visible: true
                scale.z: 60
                scale.x: 60

            }

            PropertyChanges {
                target: garageFloor
                visible: false
            }

            PropertyChanges {
                target: desert
                probeOrientation.y: -70
                lightProbe: _Desert
                antialiasingQuality: SceneEnvironment.VeryHigh
                fxaaEnabled: false
                specularAAEnabled: false
                aoStrength: 72
                aoSoftness: 39.8591
                aoBias: 1
                lutEnabled: false
                aoDither: true
                aoSampleRate: 4
                temporalAAStrength: 2
                temporalAAEnabled: true
                aoDistance: 25
                depthOfFieldBlurAmount: 3
                depthOfFieldFocusRange: 200
                depthOfFieldEnabled: true
                lensFlareDistortion: 0.20282
                lensFlareBlurAmount: 2.45517
                lensFlareApplyDirtTexture: false
                lensFlareLensColorTexture: _Desert
                sharpnessAmount: 0
                whitePoint: 0.83817
                skyboxBlurAmount: 0.18745
                glowBlendMode: ExtendedSceneEnvironment.GlowBlendMode.Additive
                glowUseBicubicUpscale: false
                exposure: 0.8
                lensFlareApplyStarburstTexture: false
                lensFlareGhostCount: 0
                lensFlareBloomBias: 0.38856
                lensFlareEnabled: false
                vignetteEnabled: false
                glowHDRScale: 0
                glowQualityHigh: true
                glowHDRMaximumValue: 27.76875
                glowStrength: 0.02647
                glowIntensity: 2
                glowHDRMinimumValue: 3.77218
                glowLevel: ExtendedSceneEnvironment.GlowLevel.One | ExtendedSceneEnvironment.GlowLevel.Two | ExtendedSceneEnvironment.GlowLevel.Three | ExtendedSceneEnvironment.GlowLevel.Four | ExtendedSceneEnvironment.GlowLevel.Five | ExtendedSceneEnvironment.GlowLevel.Six | ExtendedSceneEnvironment.GlowLevel.Seven
                glowBloom: 0.15158
                glowEnabled: false
                aoEnabled: false
                probeExposure:  btnRain.checked ? 1.0 : 2
            }

            PropertyChanges {
                target: plane3
                x: -11.348
                y: 4.23
                opacity: 1
                visible: true
                z: -5.52203
                receivesShadows: true
                castsReflections: false
                levelOfDetailBias: 0
                depthBias: 500
            }

            PropertyChanges {
                target: groundMat1
                opacity: 0.93
                lightProbe: _Desert
                cullMode: Material.NoCulling
                vertexColorsEnabled: false
                depthDrawMode: Material.AlwaysDepthDraw
            }

            PropertyChanges {
                target: headlights
                x: -18.519
                y: -22.316
                z: 6.0437
            }

            PropertyChanges {
                target: lightPoint1
                visible: btnLight.checked
            }

            PropertyChanges {
                target: backlight_red
                x: -0.001
                y: 252.129
            }

            PropertyChanges {
                target: pebbles
                x: 0.241
                y: -0.013
                z: -2.06169
                eulerRotation.z: -0.00002
                eulerRotation.y: -47.88068
                eulerRotation.x: -0.00001
                scale.z: 7
                scale.y: 27
                scale.x: 7
            }

            PropertyChanges {
                target: _Desert
                source: root.downloadBase + "/content/images/HDR/low/_Desert.ktx"
            }

            PropertyChanges {
                target: desert.fog
                color: "#aea89f"
                depthCurve: 1
                transmitCurve: 15
                heightCurve: 0
                depthFar: 5000
                transmitEnabled: true
                density: 0.70495
                leastIntenseY: 2
                depthNear: 200
                heightEnabled: false
                depthEnabled: true
                enabled: true
            }

            PropertyChanges {
                target: ev_SportsCar_low
                x: -19
                y: 2.343
                desert: true
                z: 0
            }

            PropertyChanges {
                target: shadowPlane
                x: 0
                y: 0.008
                opacity: 0.705
                visible: true
                z: -0
            }

            PropertyChanges {
                target: plane2
                visible: false
            }
        },
        State {
            name: "State2"
            when: btnStudio.checked

            PropertyChanges {
                target: reflectionProbe1
                quality: ReflectionProbe.Low
                timeSlicing: ReflectionProbe.AllFacesAtOnce
                boxSize.z: 7500
                boxSize.y: 400
                boxSize.x: 7500
                parallaxCorrection: true
            }
            PropertyChanges {
                target: lightDirectional
                visible: false
                ambientColor: "#3c3c3c"
                brightness: 0.5
                eulerRotation.z: 180
                eulerRotation.y: -180
                eulerRotation.x: -6.93953
            }

            PropertyChanges {
                target: lightDirectional1
                visible: false
                castsShadow: false
                ambientColor: "#282828"
                brightness: 2
                eulerRotation.z: -0.00001
                eulerRotation.y: 0
                eulerRotation.x: -6.62763
            }

            PropertyChanges {
                target: garageFloor
                visible: false
            }

            PropertyChanges {
                target: studioFloor
                visible: true
                eulerRotation.z: 0
                castsShadows: false
            }

            PropertyChanges {
                target: view3D
                environment: studio
            }

            PropertyChanges {
                target: studio
                lightProbe: _Hall
                backgroundMode: SceneEnvironment.Color
                depthOfFieldEnabled: false
                glowBloom: 0.40265
                glowLevel: ExtendedSceneEnvironment.GlowLevel.One | ExtendedSceneEnvironment.GlowLevel.Two | ExtendedSceneEnvironment.GlowLevel.Three | ExtendedSceneEnvironment.GlowLevel.Four | ExtendedSceneEnvironment.GlowLevel.Five | ExtendedSceneEnvironment.GlowLevel.Six | ExtendedSceneEnvironment.GlowLevel.Seven
                glowEnabled: false
                lensFlareEnabled: false
                aoDither: true
                aoDistance: 1
                aoStrength: 100
                aoEnabled: false
                clearColor: "#121212"
                probeExposure: 1
            }

            PropertyChanges {
                target: sceneObjects1
                y: -199
                z: 0
            }

            PropertyChanges {
                target: rectMaterial2
                specularAmount: 0.51741
                fresnelPower: 3.8
                metalness: 0.24579
                baseColor: "#828282"
                occlusionAmount: 1
                opacityMap: dot
                roughness: 0.26648
                normalStrength: 1
            }

            PropertyChanges {
                target: groundMat1
                opacity: 1
            }

            PropertyChanges {
                target: plane3
                x: -15.512
                y: 4.739
                z: 10.0102
            }

            PropertyChanges {
                target: desert
                antialiasingQuality: SceneEnvironment.VeryHigh
                temporalAAEnabled: true
                glowLevel: ExtendedSceneEnvironment.GlowLevel.One
                glowHDRMinimumValue: 0.33772
                glowIntensity: 0.54665
                glowQualityHigh: true
            }

            PropertyChanges {
                target: backlight_red
                x: -0.002
                y: 222.524
                z: -2765.60449
            }

            PropertyChanges {
                target: backlightNew
                opacity: 0.264
            }

            PropertyChanges {
                target: videoRoom
                temporalAAStrength: 2
                temporalAAEnabled: true
                antialiasingQuality: SceneEnvironment.VeryHigh
            }
        },
        State {
            name: "State3"
            when: btnAnimated.checked

            PropertyChanges {
                target: plane2
                x: 0
                visible: true
                scale.z: 50
                scale.y: 50
                eulerRotation.z: 0
                eulerRotation.y: 139.22893
                eulerRotation.x: -90
                y: 222.14
                z: -13.11121
            }

            PropertyChanges {
                target: view3D
                environment: videoRoom
            }

            PropertyChanges {
                target: lightDirectional1
                castsShadow: false
                ambientColor: "#2b2b2b"
                brightness: 1
                eulerRotation.z: -0.00001
                eulerRotation.y: 0.00001
                eulerRotation.x: -45.75426
            }

            PropertyChanges {
                target: lightDirectional
                ambientColor: "#262626"
                brightness: 0.3
                eulerRotation.z: 179.99997
                eulerRotation.y: -179.99997
                eulerRotation.x: -84.03297
            }

            PropertyChanges {
                target: showhall
                aoBias: 22
            }

            PropertyChanges {
                target: videoControls
                visible: true
            }

            PropertyChanges {
                target: garageFloor
                visible: false
            }

            PropertyChanges {
                target: animatedstateFloor
                visible: true
            }

            PropertyChanges {
                target: studioFloor
                visible: false
                materials: rectMaterial2
            }

            PropertyChanges {
                target: reflectionProbe1
                x: 0
                y: 0
                z: -598.11938
                parallaxCorrection: true
                timeSlicing: ReflectionProbe.AllFacesAtOnce
                refreshMode: ReflectionProbe.EveryFrame
                quality: ReflectionProbe.Low
                boxSize.z: 6000
                boxSize.x: 6000
                boxSize.y: 6000
            }

            PropertyChanges {
                target: principledMaterial2
                cullMode: Material.NoCulling
                baseColor: "#ffffff"
                roughness: 1
            }

            PropertyChanges {
                target: sceneObjects1
                z: -750
            }

            PropertyChanges {
                target: backlight_red
                x: -0.003
                y: 116.157
                z: -2740.36426
            }

            PropertyChanges {
                target: backlightNew
                opacity: 0.275
                depthDrawMode: Material.AlwaysDepthDraw
                cullMode: Material.BackFaceCulling
            }

            PropertyChanges {
                target: videoRoom
                lightProbe: _Desert
            }
        }
    ]
}
