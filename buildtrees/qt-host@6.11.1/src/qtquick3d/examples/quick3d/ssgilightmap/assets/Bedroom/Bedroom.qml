// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import ".."

SceneBase {
    id: node

    // SceneBase properties
    cameraIndex : 0
    cameraPositions: [
        Qt.vector3d(-127.619, 111.187, 7.55733),
        Qt.vector3d(321.195, 193.044, -161.709),
        Qt.vector3d(656.723, 84.3417, 75.358)
    ]
    cameraRotations: [
        Qt.vector3d(-0.431445, -91.3556, 0),
        Qt.vector3d(-25.4781, -132.766, 0),
        Qt.vector3d(-0.228459, -45.7822, 0)
    ]
    lightmapper: Lightmapper {
        source: "qrc:/assets/lightmaps/" + node.lightmapSource
        samples: node.samples
        texelsPerUnit: node.texelsPerUnit
        bounces: node.bounces
        denoiseSigma: node.denoiseSigma
        indirectLightFactor: node.indirectLightFactor
        opacityThreshold: 1
    }
    backgroundMode: SceneEnvironment.SkyBox
    lightProbe: Texture {
        source: "qrc:/assets/Bedroom/bambanani_sunset_4k.ktx"
    }
    enableLightmaps: true
    ssgiIndirectLightBoost: 10
    probeExposure: 1.0

    eulerRotation: Qt.vector3d(0, -90, 0)

    DirectionalLight {
        id: dirLight
        eulerRotation.x: node.dirLightX
        eulerRotation.y: node.dirLightY
        brightness: node.dirLightBrightness
        bakeMode: node.lightBakeMode
        shadowMapQuality: node.shadowMapQuality
        softShadowQuality: node.softShadowQuality
        shadowFactor: node.shadowFactor
        pcfFactor: node.pcfFactor
        castsShadow: true
        lockShadowmapTexels: true
        color: "sandybrown"
    }

    // Common
    property string lightmapSource: "bedroom/lm_bedroom.bin"
    property real indirectLightFactor: 5.0
    property int samples: 16384
    property real texelsPerUnit: 0.1
    property int bounces: 3
    property real denoiseSigma: 8
    property int lightBakeMode: node.enableLightmaps ? Light.BakeModeAll : Light.BakeModeDisabled

    property real shadowFactor: 90
    property int shadowMapQuality: Light.ShadowMapQualityUltra
    property int softShadowQuality: Light.PCF16
    property real pcfFactor: 5

    // Directional light
    property real dirLightX: -45.0
    property real dirLightY: 112
    property real dirLightBrightness: 2.0

    // Resources
    property url textureData: "qrc:/assets/Bedroom/maps/textureData.png"
    property url textureData67: "qrc:/assets/Bedroom/maps/textureData67.png"
    property url textureData75: "qrc:/assets/Bedroom/maps/textureData75.jpg"
    property url textureData8: "qrc:/assets/Bedroom/maps/textureData8.jpg"
    property url textureData50: "qrc:/assets/Bedroom/maps/textureData50.jpg"
    property url textureData10: "qrc:/assets/Bedroom/maps/textureData10.png"
    property url textureData77: "qrc:/assets/Bedroom/maps/textureData77.png"
    property url textureData12: "qrc:/assets/Bedroom/maps/textureData12.png"
    property url textureData237: "qrc:/assets/Bedroom/maps/textureData237.png"
    property url textureData69: "qrc:/assets/Bedroom/maps/textureData69.jpg"
    property url textureData79: "qrc:/assets/Bedroom/maps/textureData79.png"
    property url textureData105: "qrc:/assets/Bedroom/maps/textureData105.png"
    property url textureData40: "qrc:/assets/Bedroom/maps/textureData40.jpg"
    property url textureData24: "qrc:/assets/Bedroom/maps/textureData24.png"
    property url textureData52: "qrc:/assets/Bedroom/maps/textureData52.png"
    property url textureData26: "qrc:/assets/Bedroom/maps/textureData26.png"
    property url textureData42: "qrc:/assets/Bedroom/maps/textureData42.png"
    property url textureData65: "qrc:/assets/Bedroom/maps/textureData65.png"
    property url textureData44: "qrc:/assets/Bedroom/maps/textureData44.png"
    property url textureData54: "qrc:/assets/Bedroom/maps/textureData54.png"
    property url textureData107: "qrc:/assets/Bedroom/maps/textureData107.png"
    property url textureData103: "qrc:/assets/Bedroom/maps/textureData103.jpg"
    property url textureData63: "qrc:/assets/Bedroom/maps/textureData63.jpg"
    property url textureData173: "qrc:/assets/Bedroom/maps/textureData173.jpg"
    property url textureData175: "qrc:/assets/Bedroom/maps/textureData175.png"
    property url textureData177: "qrc:/assets/Bedroom/maps/textureData177.png"
    property url textureData192: "qrc:/assets/Bedroom/maps/textureData192.jpg"
    property url textureData194: "qrc:/assets/Bedroom/maps/textureData194.png"
    property url textureData196: "qrc:/assets/Bedroom/maps/textureData196.png"
    property url textureData235: "qrc:/assets/Bedroom/maps/textureData235.jpg"
    property url textureData239: "qrc:/assets/Bedroom/maps/textureData239.png"
    property url textureData251: "qrc:/assets/Bedroom/maps/textureData251.jpg"
    property url textureData253: "qrc:/assets/Bedroom/maps/textureData253.png"
    property url textureData255: "qrc:/assets/Bedroom/maps/textureData255.png"
    Texture {
        id: _15_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData69
    }
    Texture {
        id: _0_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData8
    }
    Texture {
        id: _1_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData10
    }
    Texture {
        id: _19_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData103
    }
    Texture {
        id: _2_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData12
    }
    Texture {
        id: _12_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData63
    }
    Texture {
        id: _20_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData105
    }
    Texture {
        id: _13_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData65
    }
    Texture {
        id: _21_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData107
    }
    Texture {
        id: _14_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData67
    }
    Texture {
        id: _22_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData173
    }
    Texture {
        id: _3_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData24
    }
    Texture {
        id: _23_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData175
    }
    Texture {
        id: _4_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData26
    }
    Texture {
        id: _24_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData177
    }
    Texture {
        id: _5_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData
    }
    Texture {
        id: _25_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData192
    }
    Texture {
        id: _11_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData54
    }
    Texture {
        id: _26_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData194
    }
    Texture {
        id: _16_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData75
    }
    Texture {
        id: _27_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData196
    }
    Texture {
        id: _17_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData77
    }
    Texture {
        id: _28_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData235
    }
    Texture {
        id: _6_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData40
    }
    Texture {
        id: _29_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData237
    }
    Texture {
        id: _8_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData44
    }
    Texture {
        id: _30_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData239
    }
    Texture {
        id: _7_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData42
    }
    Texture {
        id: _31_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData251
    }
    Texture {
        id: _18_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData79
    }
    Texture {
        id: _32_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData253
    }
    Texture {
        id: _9_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData50
    }
    Texture {
        id: _33_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData255
    }
    Texture {
        id: _10_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData52
    }
    PrincipledMaterial {
        id: koltuk_material
        objectName: "Koltuk"
        baseColorMap: _19_texture
        metalnessMap: _20_texture
        roughnessMap: _20_texture
        metalness: 0
        roughness: 1
        normalMap: _21_texture
        occlusionMap: _20_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: roof_material
        objectName: "Roof"
        baseColorMap: _22_texture
        metalnessMap: _23_texture
        roughnessMap: _23_texture
        metalness: 0
        roughness: 1
        normalMap: _24_texture
        occlusionMap: _23_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: kenarlik_material
        objectName: "Kenarlik"
        baseColorMap: _0_texture
        metalnessMap: _1_texture
        roughnessMap: _1_texture
        metalness: 0
        roughness: 1
        normalMap: _2_texture
        occlusionMap: _1_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: glass_material
        objectName: "Glass"
        baseColorMap: _3_texture
        metalnessMap: _4_texture
        roughnessMap: _4_texture
        metalness: 0
        roughness: 1
        normalMap: _5_texture
        occlusionMap: _4_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Blend
    }
    PrincipledMaterial {
        id: baza_material
        objectName: "Baza"
        baseColorMap: _25_texture
        metalnessMap: _26_texture
        roughnessMap: _26_texture
        metalness: 0
        roughness: 1
        normalMap: _27_texture
        occlusionMap: _26_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: material_material
        objectName: "material"
        baseColorMap: _6_texture
        metalnessMap: _7_texture
        roughnessMap: _7_texture
        metalness: 0
        roughness: 1
        normalMap: _8_texture
        occlusionMap: _7_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: book_material
        objectName: "Book"
        baseColorMap: _9_texture
        metalnessMap: _10_texture
        roughnessMap: _10_texture
        metalness: 0
        roughness: 1
        normalMap: _11_texture
        occlusionMap: _10_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: puff_material
        objectName: "Puff"
        baseColorMap: _28_texture
        metalnessMap: _29_texture
        roughnessMap: _29_texture
        metalness: 0
        roughness: 1
        normalMap: _30_texture
        occlusionMap: _29_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: object_material
        objectName: "Object"
        baseColorMap: _12_texture
        metalnessMap: _13_texture
        roughnessMap: _13_texture
        metalness: 0
        roughness: 1
        normalMap: _14_texture
        occlusionMap: _13_texture
        emissiveMap: _15_texture
        emissiveFactor: Qt.vector3d(20, 20, 20)
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: carpet_material
        objectName: "Carpet"
        baseColorMap: _16_texture
        metalnessMap: _17_texture
        roughnessMap: _17_texture
        metalness: 0
        roughness: 1
        normalMap: _18_texture
        occlusionMap: _17_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: kitaplik_material
        objectName: "Kitaplik"
        baseColorMap: _31_texture
        metalnessMap: _32_texture
        roughnessMap: _32_texture
        metalness: 0
        roughness: 1
        normalMap: _33_texture
        occlusionMap: _32_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
    }

    // Nodes:
    Node {
        id: sketchfab_model
        objectName: "Sketchfab_model"
        rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
        Node {
            id: collada_visual_scene_group
            objectName: "Collada visual scene group"
            rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
            Node {
                id: cube_003
                objectName: "Cube.003"
                position: Qt.vector3d(-83.0017, 242.643, -561.589)
                rotation: Qt.quaternion(-1.37886e-07, 1.37886e-07, 0.707107, 0.707107)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_003"
                    }
                    materials: [
                        kenarlik_material
                    ]
                }
            }
            Node {
                id: cube_002
                objectName: "Cube.002"
                position: Qt.vector3d(82.1048, 242.643, -868.553)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial14
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh15.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_002"
                    }
                    materials: [
                        kenarlik_material
                    ]
                }
            }
            Node {
                id: cube_001
                objectName: "Cube.001"
                position: Qt.vector3d(82.1048, 242.643, -271.349)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial17
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh18.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_001"
                    }
                    materials: [
                        kenarlik_material
                    ]
                }
            }
            Node {
                id: cube_006
                objectName: "Cube.006"
                position: Qt.vector3d(82.1048, 242.643, -868.553)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial20
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh21.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_006"
                    }
                    materials: [
                        glass_material
                    ]
                }
            }
            Node {
                id: cube_005
                objectName: "Cube.005"
                position: Qt.vector3d(-83.0017, 242.643, -561.589)
                rotation: Qt.quaternion(-1.37886e-07, 1.37886e-07, 0.707107, 0.707107)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial30
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh31.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_005"
                    }
                    materials: [
                        glass_material
                    ]
                }
            }
            Node {
                id: cube_004
                objectName: "Cube.004"
                position: Qt.vector3d(82.1048, 242.643, -271.349)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial33
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh34.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_004"
                    }
                    materials: [
                        glass_material
                    ]
                }
            }
            Node {
                id: plane
                objectName: "Plane"
                position: Qt.vector3d(34.2536, 97.1551, -573.346)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial36
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh37.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: cube_026
                objectName: "Cube.026"
                position: Qt.vector3d(-164.576, 5.17816, -763.947)
                rotation: Qt.quaternion(-0.315844, 0.633741, -0.632648, 0.313644)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial46
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh47.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_026"
                    }
                    materials: [
                        book_material
                    ]
                }
            }
            Node {
                id: cube_034
                objectName: "Cube.034"
                position: Qt.vector3d(35.682, 105.502, -1094.02)
                rotation: Qt.quaternion(0.500868, -0.500868, 0.499131, -0.499131)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial56
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh57.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_034"
                    }
                    materials: [
                        book_material
                    ]
                }
            }
            Node {
                id: cube_025
                objectName: "Cube.025"
                position: Qt.vector3d(135.077, 112.5, -1049.02)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial59
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh60.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_025"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cube_024
                objectName: "Cube.024"
                position: Qt.vector3d(-47.077, 0.792417, -231.629)
                rotation: Qt.quaternion(0.707107, -0.707107, 0.000202874, 0.000202874)
                scale: Qt.vector3d(71.9744, 124.329, 100)
                Model {
                    id: defaultMaterial71
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh72.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_024"
                    }
                    materials: [
                        carpet_material
                    ]
                }
            }
            Node {
                id: honeyComb_002
                objectName: "HoneyComb.002"
                position: Qt.vector3d(-199.786, 147.201, -976.832)
                rotation: Qt.quaternion(0.707107, 1.41421e-08, 0.707107, -1.41421e-08)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial81
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh82.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "honeyComb_002"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: honeyComb_001
                objectName: "HoneyComb.001"
                position: Qt.vector3d(-199.786, 92.5245, -882.092)
                rotation: Qt.quaternion(0.707107, 1.41421e-08, 0.707107, -1.41421e-08)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial84
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh85.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "honeyComb_001"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: honeyComb
                objectName: "HoneyComb"
                position: Qt.vector3d(-199.786, 132.451, -767.341)
                rotation: Qt.quaternion(0.707107, 1.41421e-08, 0.707107, -1.41421e-08)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial87
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh88.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "honeyComb"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: pyramid
                objectName: "Pyramid"
                position: Qt.vector3d(184.711, 29.7086, -50.9186)
                rotation: Qt.quaternion(0.694114, -0.694114, 0.134927, 0.134927)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial90
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh91.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "pyramid"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cube_023
                objectName: "Cube.023"
                position: Qt.vector3d(199.424, 82.4496, -73.5072)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial93
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh94.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_023"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cube_022
                objectName: "Cube.022"
                position: Qt.vector3d(-51.8169, 34.758, -215.811)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial96
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh97.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_022"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cylinder_011
                objectName: "Cylinder.011"
                position: Qt.vector3d(-81.7969, 3.92289, -38.8475)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial99
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh100.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_011"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_010
                objectName: "Cylinder.010"
                position: Qt.vector3d(-10.4537, 3.92289, -39.4294)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial109
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh110.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_010"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_009
                objectName: "Cylinder.009"
                position: Qt.vector3d(-81.7969, 3.92289, -83.6463)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial112
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh113.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_009"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_008
                objectName: "Cylinder.008"
                position: Qt.vector3d(-10.4537, 3.92289, -84.2282)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial115
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh116.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_008"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_007
                objectName: "Cylinder.007"
                position: Qt.vector3d(-187.655, 3.92289, -114.479)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial118
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh119.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_007"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_006
                objectName: "Cylinder.006"
                position: Qt.vector3d(-137.769, 3.92289, -115.061)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial121
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh122.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_006"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_005
                objectName: "Cylinder.005"
                position: Qt.vector3d(-187.655, 3.92289, -212.213)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial124
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh125.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_005"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_004
                objectName: "Cylinder.004"
                position: Qt.vector3d(-187.655, 3.92289, -311.919)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial127
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh128.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_004"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_003
                objectName: "Cylinder.003"
                position: Qt.vector3d(-137.769, 3.92289, -311.919)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial130
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh131.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_003"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cylinder_002
                objectName: "Cylinder.002"
                position: Qt.vector3d(-137.769, 3.92289, -212.795)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial133
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh134.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_002"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_013
                objectName: "Plane.013"
                position: Qt.vector3d(-169.54, 50.8475, -135.124)
                rotation: Qt.quaternion(0.54004, -0.546204, -0.1322, -0.626531)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial136
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh137.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_013"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_012
                objectName: "Plane.012"
                position: Qt.vector3d(-169.54, 50.8475, -159.579)
                rotation: Qt.quaternion(0.612844, -0.463038, -0.0409059, -0.639019)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial139
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh140.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_012"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_011
                objectName: "Plane.011"
                position: Qt.vector3d(-173.259, 50.8475, -185.456)
                rotation: Qt.quaternion(0.631627, -0.488404, 0.317879, -0.511333)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial142
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh143.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_011"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_010
                objectName: "Plane.010"
                position: Qt.vector3d(-169.54, 50.8475, -213.906)
                rotation: Qt.quaternion(0.612844, -0.463038, -0.0409059, -0.639019)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial145
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh146.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_010"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_009
                objectName: "Plane.009"
                position: Qt.vector3d(-173.259, 50.8475, -239.782)
                rotation: Qt.quaternion(0.631627, -0.488404, 0.317879, -0.511333)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial148
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh149.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_009"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_008
                objectName: "Plane.008"
                position: Qt.vector3d(-169.54, 50.8475, -267.072)
                rotation: Qt.quaternion(0.612844, -0.463038, -0.0409059, -0.639019)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial151
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh152.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_008"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: plane_007
                objectName: "Plane.007"
                position: Qt.vector3d(-173.259, 50.8475, -292.948)
                rotation: Qt.quaternion(0.631627, -0.488404, 0.317879, -0.511333)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial154
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh155.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_007"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cube_021
                objectName: "Cube.021"
                position: Qt.vector3d(100.811, 51.8636, -64.6431)
                rotation: Qt.quaternion(0.5, -0.5, 0.5, 0.5)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial157
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh158.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_021"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cube_020
                objectName: "Cube.020"
                position: Qt.vector3d(-45.1774, 26.907, -65.5084)
                rotation: Qt.quaternion(0.5, -0.5, 0.5, 0.5)
                scale: Qt.vector3d(45.1093, 43.261, 45.1093)
                Model {
                    id: defaultMaterial160
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh161.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_020"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cube_019
                objectName: "Cube.019"
                position: Qt.vector3d(-154.818, 51.8636, -138.434)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial163
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh164.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_019"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cube_018
                objectName: "Cube.018"
                position: Qt.vector3d(-154.818, 51.8636, -138.434)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial166
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh167.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_018"
                    }
                    materials: [
                        koltuk_material
                    ]
                }
            }
            Node {
                id: cube_017
                objectName: "Cube.017"
                position: Qt.vector3d(188.191, 72.9949, -723.55)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 41.5488, 64.9722)
                Model {
                    id: defaultMaterial169
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh170.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_017"
                    }
                    materials: [
                        roof_material
                    ]
                }
            }
            Node {
                id: cube_016
                objectName: "Cube.016"
                position: Qt.vector3d(188.191, 72.9949, -426.635)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 41.5488, 64.9722)
                Model {
                    id: defaultMaterial179
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh180.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_016"
                    }
                    materials: [
                        roof_material
                    ]
                }
            }
            Node {
                id: cylinder_001
                objectName: "Cylinder.001"
                position: Qt.vector3d(-152.632, 136.586, -1036.54)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial182
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh183.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder_001"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cylinder
                objectName: "Cylinder"
                position: Qt.vector3d(-152.994, 148.508, -1037.05)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial185
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh186.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cylinder"
                    }
                    materials: [
                        object_material
                    ]
                }
            }
            Node {
                id: cube_015
                objectName: "Cube.015"
                position: Qt.vector3d(196.111, 26.7591, -723.661)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial188
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh189.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_015"
                    }
                    materials: [
                        baza_material
                    ]
                }
            }
            Node {
                id: cube_014
                objectName: "Cube.014"
                position: Qt.vector3d(196.111, 26.7591, -426.635)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial198
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh199.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_014"
                    }
                    materials: [
                        baza_material
                    ]
                }
            }
            Node {
                id: cube_009
                objectName: "Cube.009"
                position: Qt.vector3d(199.424, 100, -575.785)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial201
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh202.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_009"
                    }
                    materials: [
                        baza_material
                    ]
                }
            }
            Node {
                id: plane_006
                objectName: "Plane.006"
                position: Qt.vector3d(139.207, 70.6361, -568.34)
                rotation: Qt.quaternion(0.717375, -0.417748, -0.541459, -0.132971)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial204
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh205.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_006"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: plane_005
                objectName: "Plane.005"
                position: Qt.vector3d(152.957, 65.472, -523.732)
                rotation: Qt.quaternion(0.631824, -0.631824, -0.317488, 0.317488)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial207
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh208.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_005"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: plane_004
                objectName: "Plane.004"
                position: Qt.vector3d(152.957, 65.472, -614.283)
                rotation: Qt.quaternion(0.631824, -0.631824, -0.317488, 0.317488)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial210
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh211.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_004"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: plane_003
                objectName: "Plane.003"
                position: Qt.vector3d(176.318, 65.472, -614.283)
                rotation: Qt.quaternion(0.631824, -0.631824, -0.317488, 0.317488)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial213
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh214.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_003"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: plane_002
                objectName: "Plane.002"
                position: Qt.vector3d(176.318, 65.472, -522.363)
                rotation: Qt.quaternion(0.631824, -0.631824, -0.317488, 0.317488)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial216
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh217.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "plane_002"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: cube_007
                objectName: "Cube.007"
                position: Qt.vector3d(-21.9534, 0.792417, -891.456)
                rotation: Qt.quaternion(0.657304, -0.657304, -0.260676, -0.260676)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial219
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh220.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_007"
                    }
                    materials: [
                        carpet_material
                    ]
                }
            }
            Node {
                id: cube_013
                objectName: "Cube.013"
                position: Qt.vector3d(96.1208, 35.5825, -574.683)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial222
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh223.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_013"
                    }
                    materials: [
                        material_material
                    ]
                }
            }
            Node {
                id: cube_012
                objectName: "Cube.012"
                position: Qt.vector3d(99.0772, 21.3614, -574.683)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial225
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh226.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_012"
                    }
                    materials: [
                        baza_material
                    ]
                }
            }
            Node {
                id: cube_011
                objectName: "Cube.011"
                position: Qt.vector3d(98.5665, 6.89833, -574.779)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial228
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh229.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_011"
                    }
                    materials: [
                        baza_material
                    ]
                }
            }
            Node {
                id: sphere_002
                objectName: "Sphere.002"
                position: Qt.vector3d(-41.1477, 19.2197, -793.526)
                rotation: Qt.quaternion(0.0719768, -0.0719768, 0.703434, 0.703434)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial231
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh232.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "sphere_002"
                    }
                    materials: [
                        puff_material
                    ]
                }
            }
            Node {
                id: sphere_001
                objectName: "Sphere.001"
                position: Qt.vector3d(-46.8404, 20.2893, -971.393)
                rotation: Qt.quaternion(0.706723, -0.706723, -0.0232963, -0.0232963)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial241
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh242.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "sphere_001"
                    }
                    materials: [
                        puff_material
                    ]
                }
            }
            Node {
                id: sphere
                objectName: "Sphere"
                position: Qt.vector3d(-131.412, 23.2983, -875.62)
                rotation: Qt.quaternion(0.605882, -0.605881, 0.364565, 0.364565)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial244
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh245.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "sphere"
                    }
                    materials: [
                        puff_material
                    ]
                }
            }
            Node {
                id: cube_010
                objectName: "Cube.010"
                position: Qt.vector3d(0.01955, 120.5, -1090.66)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial247
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh248.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube_010"
                    }
                    materials: [
                        kitaplik_material
                    ]
                }
            }
            Node {
                id: cube
                objectName: "Cube"
                position: Qt.vector3d(0, 100, 0)
                rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
                scale: Qt.vector3d(100, 100, 100)
                Model {
                    id: defaultMaterial257
                    objectName: "defaultMaterial"
                    source: "qrc:/assets/Bedroom/meshes/defaultMaterial_mesh258.mesh"
                    usedInBakedLighting: node.enableLightmaps
                    bakedLightmap: BakedLightmap {
                        enabled: node.enableLightmaps
                        key: "cube"
                    }
                    materials: [
                        roof_material
                    ]
                }
            }
        }
    }

    // Animations:
}
