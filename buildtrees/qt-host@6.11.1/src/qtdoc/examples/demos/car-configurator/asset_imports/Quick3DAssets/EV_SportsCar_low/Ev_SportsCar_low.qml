// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import Quick3DAssets.LightDecal
import Quick3DAssets.Snow
Node {
    id: node

    required property url downloadBase
    required property ParticleMask particleMaskCar
    required property Texture particleMaskCarTexture
    property int stateController: 0
    property bool desert: true
    property real snowStrength: 0.6
    property bool rain: false

    property real rainStrength : 0.5
    property real normalFactor : 0.08
    property real rainSize : 0.8
    property real rainFrequency : 1.0
    property real rainPower : 1.0
    property real rainScale : 0.01
    property real dripSize : 0.14
    property real dripSpeed : 0.005
    property real dripLength : 0.01
    property real dripSharpness : 10.0

    property list<Node> shapeExluded : [hoodPositioner, trunkPositioner, leftDoorPositioner, rightDoorPositioner]

    // Resources
    property url textureData: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData.jpg"
    property url textureData53: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData53.jpg"
    property url textureData108: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData108.jpg"
    property url textureData6: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData6.png"
    property url textureData48: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData48.jpg"
    property url textureData8: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData8.jpg"
    property url textureData50: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData50.jpg"
    property url textureData42: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData42.jpg"
    property url textureData56: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData56.jpg"
    property url textureData30: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData30.jpg"
    property url textureData63: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData63.jpg"
    property url textureData45: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData45.jpg"
    property url textureData67: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData67.jpg"
    property url textureData74: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData74.jpg"
    property url textureData17: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData17.png"
    property url textureData90: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData90.jpg"
    property url textureData105: downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/maps/textureData105.png"
    property bool lightsOn: true
    property alias doorLeftIsOpen: doorLeft.isOpen
    property alias doorRightIsOpen: doorRight.isOpen
    property alias hoodIsOpen: hood.isOpen
    property alias trunkIsOpen: trunkLid.isOpen
    property alias leftDoorPositionerPos: leftDoorPositioner.scenePosition
    property alias rightDoorPositionerPos: rightDoorPositioner.scenePosition
    property alias hoodPositionerPos: hoodPositioner.scenePosition
    property alias trunkPositionerPos: trunkPositioner.scenePosition
    property bool headlightsVisible: true
    Texture {
        id: _0_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData6
    }
    Texture {
        id: _1_texture
        pivotV: 1
        positionV: 4
        scaleU: 5
        scaleV: 5
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData8
    }
    Texture {
        id: _5_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData42
    }
    Texture {
        id: _10_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData56
    }
    Texture {
        id: _6_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData45
        scaleU: 0.5
        scaleV: 0.5
    }
    Texture {
        id: _9_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData53
    }
    Texture {
        id: _2_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData17
    }
    Texture {
        id: _7_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData48
    }
    Texture {
        id: _3_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData
    }
    Texture {
        id: _13_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData74
        scaleU: 0.6
        scaleV: 0.6
    }
    Texture {
        id: _8_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData50
    }
    Texture {
        id: _14_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData90
    }
    Texture {
        id: _12_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData67
    }
    Texture {
        id: _15_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData105
    }
    Texture {
        id: _11_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData63
    }
    Texture {
        id: _16_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData108
    }
    Texture {
        id: _4_texture
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: node.textureData30
    }

    // Nodes:
    Node {
        id: root_object
        scale.z: 1
        scale.y: 1
        scale.x: 1
        objectName: "ROOT"
        Model {
            id: body
            objectName: "Body"
            pickable: true
            y: 0.6449694037437439
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/body_mesh.mesh"
            receivesReflections: true
            materials: [
                node.rain ? carPaint_material_rain : carPaint_material,
                node.rain ? carPaintBlackBump_material_rain : carPaintBlackBump_material,
                node.rain ? metalDark_material9_rain : metalDark_material9,
                node.rain ? plasticBlack_material_rain : plasticBlack_material,
                node.rain ? chrome_material11_rain : chrome_material11,
                node.rain ? glassLights_material12_rain : glassLights_material12,
                node.rain ? glassRedLights_material13_rain : glassRedLights_material13,
                glassLightsIllum_material14,
                node.rain ? glassWindsSide_material_rain : glassWindsSide_material,
                intCarpet_material18
            ]
        }
        Model {
            id: chargingCap
            objectName: "ChargingCap"
            y: 1.004989743232727
            z: -1.5241049528121948
            rotation: Qt.quaternion(0.845673, -0.533702, 0, 0)
            scale.x: 1
            scale.y: 1
            scale.z: 1
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/chargingCap_mesh.mesh"
            materials: [
                carPaint_material23,
                plasticBlack_material24
            ]
        }
        Model {
            id: headlights
            objectName: "Headlights"
            y: 0.5664713978767395
            z: 1.7861577272415161
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/headlights_mesh.mesh"
            materials: [
                chrome_material,
                chrome_material,
                chromeLightsBMP_material,
                glassLightsLens_material,
                glassLights_material,
                metalDark_material
            ]
        }
        Hood {
            id: hood
            rain: node.rain
            y: 0.7891814112663269
            isOpen: false
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/hood_mesh.mesh"
            materials: [
                rain ? carPaint_material_rain : carPaint_material,
                rain ? plasticBlack_material24_rain : plasticBlack_material24,
                rain ? chrome_material_rain : chrome_material
            ]
            Model {
                id: hoodPositioner
                x: 0
                y: 0
                source: "#Cube"
                z: 1.3
                scale.z: 0.01
                scale.y: 0.01
                scale.x: 0.01
                materials: invisibleMat
            }

            Model {
                id: leftHoodHydraulics
                x: 0.64
                y: -0.05
                source: "#Cylinder"
                eulerRotation.z: 0
                eulerRotation.y: -0
                eulerRotation.x: -50-(hood.eulerRotation.x*1.5)
                z: 0.58162
                scale.z: 0.0003
                scale.y: 0.0025
                scale.x: 0.0003
                pivot.y: 46
                materials: plasticBlack_material
            }

            Model {
                id: rightHoodHydraulics
                x: -0.643
                y: -0.03
                source: "#Cylinder"
                eulerRotation.x: -50-(hood.eulerRotation.x*1.5)
                eulerRotation.z: 0
                eulerRotation.y: -0
                z: 0.59075
                pivot.y: 46
                scale.z: 0.0003
                scale.y: 0.0025
                scale.x: 0.0003
                materials: plasticBlack_material
            }
            Model {
                id: leftHoodHydraulicsChrome
                x: 0.64
                y: -0.05
                source: "#Cylinder"
                eulerRotation.z: 0
                eulerRotation.y: -0
                eulerRotation.x: -50-(hood.eulerRotation.x*1.5)
                z: 0.58162
                scale.z: 0.00015
                scale.y: 0.005
                scale.x: 0.00015
                pivot.y: 46
                materials: chrome_material
            }

            Model {
                id: rightHoodHydraulicsChrome
                x: -0.643
                y: -0.03
                source: "#Cylinder"
                eulerRotation.x: -50-(hood.eulerRotation.x*1.5)
                eulerRotation.z: 0
                eulerRotation.y: -0
                z: 0.59075
                pivot.y: 46
                scale.z: 0.00015
                scale.y: 0.005
                scale.x: 0.00015
                materials: chrome_material
            }
        }
        Model {
            id: interior
            objectName: "Interior"
            y: 0.7498878240585327
            z: 0.1537650227546692
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/interior_mesh.mesh"
            materials: [
                plasticBlack_material24,
                chrome_material,
                metalMirror_material,
                intCarpet_material,
                intAlcanataraGrey_material,
                intLeatherBlack_material,
                intSeatBelt_material,
                intButtons_material,
                intGrillBump_material
            ]
        }
        Model {
            id: dash
            objectName: "Dash"
            y: 0.6341137886047363
            z: 0.24422581493854523
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/dash_mesh.mesh"
            materials: [
                metalDark_material,
                plasticBlack_material24,
                chrome_material,
                glassLights_material,
                glassRedLights_material,
                glassLightsIllum_material,
                intAlcanataraGrey_material,
                intLeatherBlack_material,
                intAluminiumBrushed_material,
                carPaint_material,
                intLeatherSeatsPattern_material,
                intButtons_material,
                intGrillBump_material
            ]
        }
        Model {
            id: seats
            objectName: "Seats"
            y: 0.6852515935897827
            z: -0.17120154201984406
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/seats_mesh.mesh"
            materials: [
                metalDark_material,
                plasticBlack_material24,
                aluminium_material,
                carPaint_material,
                intLeatherPerforatedBlack_material,
                intLeatherBlack_material
            ]
        }
        Model {
            id: steeringWheel
            objectName: "SteeringWheel"
            x: 0.35999995470046997
            y: 0.7381047606468201
            z: 0.3709505796432495
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/steeringWheel_mesh.mesh"
            materials: [
                plasticBlack_material24,
                chrome_material,
                intLeatherBlack_material,
                intAluminiumBrushed_material,
                intButtons_material
            ]
        }
        Model {
            id: taillights
            objectName: "Taillights"
            y: 0.7833704948425293
            z: -1.7988189458847046
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/taillights_mesh.mesh"
            materials: [
                plasticBlack_material24,
                chrome_material,
                glassRedLights_material,
                glassLights_material,
                tailLightsIllum_material
            ]
        }
        TrunkLid {
            id: trunkLid
            y: 1.1552858352661133
            rain: node.rain
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/trunkLid_mesh.mesh"
            Model {
                id: trunkPositioner
                x: 0
                y: -0.13
                source: "#Cube"
                z: -1.3
                scale.z: 0.01
                scale.y: 0.01
                scale.x: 0.01
                materials: invisibleMat
            }
            materials: [
                rain ? carPaint_material_rain : carPaint_material,
                rain ? carPaintBlackBump_material82_rain : carPaintBlackBump_material82,
                rain ? glassWindsSide_material_rain : glassWindsSide_material,
                rain ? plasticBlack_material24_rain : plasticBlack_material24
            ]
        }
        Model {
            id: wingFlaps
            objectName: "WingFlaps"
            y: 0.41297996044158936
            z: 0.16267994046211243
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/mesh_014_mesh.mesh"
            materials: [
                metalDark_material
            ]
        }
        Model {
            id: brakeDiskRearLeft
            objectName: "BrakeDiskRearLeft"
            x: 0.00032216310501098633
            y: 0.3521454930305481
            z: -1.3741973638534546
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/mesh_016_mesh.mesh"
            materials: [
                wheelBrakeDisk_material
            ]
        }
        Model {
            id: hoodEngineCover
            objectName: "HoodEngineCover"
            y: 0.5375130772590637
            z: 1.4772337675094604
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/hoodEngineCover_mesh.mesh"
            materials: [
                plasticBlack_material24,
                intCarpet_material
            ]
        }
        Model {
            id: trunkEngineCover
            objectName: "TrunkEngineCover"
            y: 0.7631296515464783
            z: -0.810766875743866
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/trunkEngineCover_mesh.mesh"
            materials: [
                plasticBlack_material24,
                intCarpet_material
            ]
        }
        Model {
            id: brakeDiskFrLeft
            objectName: "BrakeDiskFrLeft"
            y: 0.3521455228328705
            z: 1.2830324172973633
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/mesh_021_mesh.mesh"
            materials: [
                wheelBrakeDisk_material
            ]
        }
        Model {
            id: brakeCaliperFrLeft
            objectName: "BrakeCaliperFrLeft"
            y: 0.352143794298172
            z: 1.283031940460205
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/brakeCaliperFrLeft_mesh.mesh"
            materials: [
                plasticBlack_material24,
                wheelCaliper_material
            ]
        }
        Model {
            id: brakeCaliperBkLeft
            objectName: "BrakeCaliperBkLeft"
            y: 0.352143794298172
            z: -1.374194860458374
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/brakeCaliperBkLeft_mesh.mesh"
            materials: [
                plasticBlack_material24,
                wheelCaliper_material
            ]
        }
        Model {
            id: wheelFrLeft
            objectName: "WheelFrLeft"
            x: 0.8290001153945923
            y: 0.3521455228328705
            z: 1.2830325365066528
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/wheelFrLeft_mesh.mesh"
            materials: [
                wheelTireBump_material,
                wheelRimBlack_material,
                chrome_material,
                carPaint_material
            ]
        }
        Model {
            id: wheelBkLeft
            objectName: "WheelBkLeft"
            x: 0.8666445016860962
            y: 0.3521454632282257
            z: -1.3748871088027954
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/wheelBkLeft_mesh.mesh"
            materials: [
                wheelTireBump_material,
                wheelRimBlack_material,
                chrome_material,
                carPaint_material
            ]
        }
        MyDoorLeft {
            id: doorLeft
            x: 0.8845329880714417
            y: 0.6892746090888977
            receivesReflections: true
            rain: node.rain
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/doorLeft_mesh.mesh"

            Model {
                id: leftDoorPositioner
                x: 0.08
                y: 0
                source: "#Cube"
                z: -1.3
                scale.z: 0.01
                scale.y: 0.01
                scale.x: 0.01
                materials: invisibleMat
            }
            materials: [
                rain ? carPaint_material_rain : carPaint_material,
                rain ? metalDark_material_rain : metalDark_material,
                rain ? plasticBlack_material24_rain : plasticBlack_material24,
                rain ? chrome_material_rain : chrome_material,
                rain ? glassLights_material_rain : glassLights_material,
                rain ? glassRedLights_material_rain : glassRedLights_material,
                rain ? chromeLightsBMP_material_rain : chromeLightsBMP_material,
                glassLightsIllum_material,
                metalMirror_material,
                rain ? aluminium_material_rain : aluminium_material,
                rain ? glassWindsSide_material_rain : glassWindsSide_material,
                intAlcanataraGrey_material,
                intLeatherBlack_material,
                rain ? carPaint_material_rain : carPaint_material,
                intLeatherSeatsPattern_material,
                intButtons_material,
                intGrillBump_material
            ]

        }
        MyDoorRight {
            id: doorRight
            x: -0.8845332264900208
            y: 0.6892746090888977
            rain: node.rain
            scale.x: 1
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/doorRight_mesh.mesh"

            Model {
                id: rightDoorPositioner
                x: -0.12
                y: 0
                source: "#Cube"
                z: -1.3
                scale.z: 0.01
                scale.y: 0.01
                scale.x: 0.01
                materials: invisibleMat
            }

            materials: [
                rain ? carPaint_material_rain : carPaint_material,
                rain ? metalDark_material_rain : metalDark_material,
                rain ? plasticBlack_material24_rain : plasticBlack_material24,
                rain ? chrome_material_rain : chrome_material,
                rain ? glassLights_material_rain : glassLights_material,
                rain ? glassRedLights_material_rain : glassRedLights_material,
                rain ? chromeLightsBMP_material_rain : chromeLightsBMP_material,
                glassLightsIllum_material,
                metalMirror_material,
                rain ? aluminium_material_rain : aluminium_material,
                rain ? glassWindsSide_material_rain : glassWindsSide_material,
                intAlcanataraGrey_material,
                intLeatherBlack_material,
                rain ? carPaint_material_rain : carPaint_material,
                intLeatherSeatsPattern_material,
                intButtons_material,
                intGrillBump_material
            ]
        }
        Model {
            id: wheelFrRight
            objectName: "WheelFrRight"
            x: -0.8290001153945923
            y: 0.3521454930305481
            z: 1.2830324172973633
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/wheelFrRight_mesh.mesh"
            materials: [
                wheelTireBump_material,
                wheelRimBlack_material,
                chrome_material,
                carPaint_material
            ]
        }
        Model {
            id: wheelBkRight
            objectName: "WheelBkRight"
            x: -0.8660000562667847
            y: 0.3521454632282257
            z: -1.3748869895935059
            source: node.downloadBase + "/asset_imports/Quick3DAssets/EV_SportsCar_low/meshes/wheelBkRight_mesh.mesh"
            materials: [
                wheelTireBump_material,
                wheelRimBlack_material,
                chrome_material,
                carPaint_material
            ]
        }

        LightDecal {
            id: lightDecal
            downloadBase: node.downloadBase
        }
    }

    Node {
        id: __materialLibrary__

        PrincipledMaterial {
            id: chrome_material
            fresnelPower: 5.8
            specularAmount: 1
            clearcoatAmount: 0.75619
            objectName: "Chrome"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.50512
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: chrome_material_rain
            fresnelPower: 5.8
            specularAmount: 1
            clearcoatAmount: 0.75619
            objectName: "Chrome"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.2
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: chromeLightsBMP_material
            objectName: "ChromeLightsBMP"
            baseColor: "#ffff4c4c"
            metalness: 1
            roughness: 0.2136363983154297
            normalMap: _4_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: chromeLightsBMP_material_rain
            objectName: "ChromeLightsBMP"
            baseColor: "#ffff4c4c"
            metalness: 1
            specularAmount: 1
            roughness: 0.2
            normalTexture: _4_texture.source
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: glassTextured_material
            objectName: "GlassTextured"
            baseColor: "#ff0f0f0f"
            baseColorMap: _2_texture
            metalness: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: glassLightsLens_material
            opacity: node.lightsOn? 1 : 0.5
            clearcoatRoughnessAmount: 0.18232
            clearcoatAmount: node.lightsOn? 0 : 2
            objectName: "GlassLightsLens"
            baseColor: node.lightsOn? "#ffffff" : "#b3141313"
            metalness: 0.92015
            roughness: 0.0369
            emissiveFactor.z: node.lightsOn? 1 : 0
            emissiveFactor.y: node.lightsOn? 1 : 0
            emissiveFactor.x: node.lightsOn? 1 : 0
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: glassLights_material
            opacity: node.headlightsVisible? 0.6 : 0
            clearcoatRoughnessAmount: 0.1
            clearcoatAmount: 0
            objectName: "GlassLights"
            baseColor: "#1c1f23"
            metalness: 1
            roughness: 0.41143
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }
        RainMaterial {
            id: glassLights_material_rain
            opacity: node.headlightsVisible? 0.6 : 0
            clearcoatRoughnessAmount: 0.1
            clearcoatAmount: 0
            objectName: "GlassLights"
            baseColor: "#1c1f23"
            metalness: 1
            roughness: 0.2
            specularAmount: 1
            blending: true
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: metalDark_material
            clearcoatAmount: 0
            objectName: "MetalDark"
            baseColor: "#1a1a1a"
            metalness: 0.79861
            roughness: 0.8
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: metalDark_material_rain
            clearcoatAmount: 0
            objectName: "MetalDark"
            baseColor: "#1a1a1a"
            metalness: 0.79861
            roughness: 0.2
            specularAmount: 1
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: metalMirror_material
            objectName: "MetalMirror"
            baseColor: "#ff5a5a5a"
            metalness: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intCarpet_material
            objectName: "IntCarpet"
            baseColorMap: _3_texture
            metalness: 1
            roughness: 0.6722149848937988
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intAlcanataraGrey_material
            objectName: "intAlcanataraGrey"
            baseColorMap: _5_texture
            metalness: 1
            roughness: 0.5
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intSeatBelt_material
            objectName: "IntSeatBelt"
            baseColorMap: _7_texture
            metalness: 1
            roughness: 0.25
            normalMap: _8_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intButtons_material
            objectName: "intButtons"
            baseColorMap: _9_texture
            roughness: 0.05000000074505806
            emissiveMap: _9_texture
            emissiveFactor.x: 3
            emissiveFactor.y: 3
            emissiveFactor.z: 3
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intGrillBump_material
            objectName: "intGrillBump"
            baseColor: "#ff202020"
            metalness: 1
            roughness: 0.4000000059604645
            normalMap: _10_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intLeatherBlack_material
            objectName: "intLeatherBlack"
            baseColor: "#ff121212"
            metalness: 1
            roughness: 0.44999998807907104
            normalMap: _6_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: carPaintBlackBump_material
            objectName: "CarPaintBlackBump"
            baseColor: "#ff050505"
            baseColorMap: _0_texture
            roughness: 0.4954545497894287
            normalMap: _1_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }
        RainMaterial {
            id: carPaintBlackBump_material_rain
            objectName: "CarPaintBlackBumpRain"
            baseTexture: _0_texture.source
            baseColor: "#ff050505"
            roughness: 0.2
            normalTexture: _1_texture.source
            cullMode: PrincipledMaterial.NoCulling
            blending: true
            indexOfRefraction: 1.4500000476837158
            specularAmount: 1
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: glassRedLights_material
            objectName: "GlassRedLights"
            baseColor: "#80250000"
            metalness: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: glassRedLights_material_rain
            objectName: "GlassRedLights"
            baseColor: "#80250000"
            metalness: 1
            blending: true
            specularAmount: 1
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: glassLightsIllum_material
            baseColor: "#ffffff"
            emissiveFactor.z: 1
            emissiveFactor.y: 1
            emissiveFactor.x: 1
            objectName: "GlassLightsIllum"
            metalness: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intAluminiumBrushed_material
            objectName: "intAluminiumBrushed"
            baseColorMap: _11_texture
            metalness: 1
            roughness: 0.20000000298023224
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: metalDark_material9
            objectName: "MetalDark"
            baseColor: "#090909"
            metalness: 1
            roughness: 0.33000001311302185
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }
        RainMaterial {
            id: metalDark_material9_rain
            objectName: "MetalDark"
            baseColor: "#090909"
            metalness: 1
            specularAmount: 1
            roughness: 0.2
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: plasticBlack_material
            clearcoatAmount: 0
            objectName: "PlasticBlack"
            baseColor: "#0e0e0e"
            metalness: 0.29461
            roughness: 0.92173
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }
        RainMaterial {
            id: plasticBlack_material_rain
            clearcoatAmount: 0
            objectName: "PlasticBlack"
            baseColor: "#0e0e0e"
            metalness: 0.29461
            roughness: 0.2
            specularAmount: 1
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: intStitchesRed_material
            objectName: "intStitchesRed"
            baseColor: "#ff7a0000"
            metalness: 1
            roughness: 0.6000000238418579
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intLeatherSeatsPattern_material
            objectName: "intLeatherSeatsPattern"
            baseColor: "#ff121212"
            metalness: 1
            roughness: 0.44999998807907104
            normalMap: _12_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: chrome_material11
            clearcoatAmount: 0.7357
            objectName: "Chrome"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.10000000149011612
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: chrome_material11_rain
            clearcoatAmount: 0.7357
            objectName: "Chrome"
            baseColor: "#ffffff"
            metalness: 1
            specularAmount: 1
            roughness: 0.10000000149011612
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: glassLights_material12
            clearcoatRoughnessAmount: 0.02425
            clearcoatAmount: 0.73028
            objectName: "GlassLights"
            baseColor: "#ff0000"
            metalness: 1
            roughness: 0.64424
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: glassLights_material12_rain
            clearcoatRoughnessAmount: 0.02425
            clearcoatAmount: 0.73028
            objectName: "GlassLights"
            baseColor: "#ff0000"
            metalness: 1
            specularAmount: 1
            roughness: 0.2
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: aluminium_material
            objectName: "Aluminium"
            baseColor: "#ff808080"
            metalness: 1
            roughness: 0.20000000298023224
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: aluminium_material_rain
            objectName: "Aluminium"
            baseColor: "#ff808080"
            metalness: 1
            specularAmount: 1
            roughness: 0.20000000298023224
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: plasticRed_material
            objectName: "PlasticRed"
            baseColor: "#ff630000"
            roughness: 0.40297383069992065
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: intLeatherPerforatedBlack_material
            objectName: "intLeatherPerforatedBlack"
            baseColor: "#ff121212"
            metalness: 1
            roughness: 0.3499999940395355
            normalMap: _13_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: glassRedLights_material13
            objectName: "GlassRedLights"
            baseColor: "#80250000"
            metalness: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }
        RainMaterial {
            id: glassRedLights_material13_rain
            objectName: "GlassRedLights"
            baseColor: "#80250000"
            metalness: 1
            blending: true
            specularAmount: 1
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: glassLightsIllum_material14
            objectName: "GlassLightsIllum"
            metalness: 1
            emissiveFactor.x: 1
            emissiveFactor.y: 1
            emissiveFactor.z: 1
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: tailLightsIllum_material
            baseColor: "#ffccb6"
            objectName: "TailLightsIllum"
            metalness: 1
            roughness: 0.858578622341156
            emissiveFactor.x: node.lightsOn? 3 : 0
            emissiveFactor.y: 0
            emissiveFactor.z: 0
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158

        }

        PrincipledMaterial {
            id: carPaintBlackBump_material82
            objectName: "CarPaintBlackBump"
            baseColor: "#ff050505"
            baseColorMap: _0_texture
            roughness: 0.4954545497894287
            normalMap: _1_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: carPaintBlackBump_material82_rain
            objectName: "CarPaintBlackBump"
            baseColor: "#ff050505"
            baseTexture: _0_texture.source
            roughness: 0.2
            normalTexture: _1_texture.source
            blending: true
            specularAmount: 1
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: wheelBrakeDisk_material
            objectName: "WheelBrakeDisk"
            baseColorMap: _14_texture
            metalness: 1
            roughness: 0.3499999940395355
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: carPaint_material_Custom
            objectName: "CarPaint"
            baseColor: "#131313"
            roughness: 0.10000000149011612
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.029999999329447746
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledExSnowMaterial {
            id: carPaint_material
            snowStrength: node.snowStrength
            snowViewProjection: node.particleMaskCar.viewProjection
            snowDisplacementMap: node.particleMaskCarTexture
            specularAmount: 0.1
            fresnelScale: 3
            fresnelBias: -0.1
            clearcoatFresnelBias: -0.1
            clearcoatRoughnessAmount: 0.01
            metalness: 1
            fresnelScaleBiasEnabled: true
            fresnelPower: 8
            clearcoatFresnelScale: 3
            clearcoatFresnelPower: 8
            clearcoatFresnelScaleBiasEnabled: true
            clearcoatAmount: 0.51265
            roughness: 0.3
            baseColor: "#000000"
            objectName: "Car Paint"
        }
        RainMaterial {
            id: carPaint_material_rain
            baseColor: "#000000"
            specularAmount: 0.8
            fresnelPower: 8
            fresnelScale: 3
            fresnelBias: -0.1
            clearcoatRoughnessAmount: 0.01
            clearcoatFresnelBias: -0.1
            clearcoatFresnelScale: 3
            clearcoatFresnelPower: 8
            clearcoatAmount: 0.51265
            roughness: 0.3
            metalness: 1

            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: intCarpet_material18
            objectName: "IntCarpet"
            baseColorMap: _3_texture
            metalness: 1
            roughness: 0.6722149848937988
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: wheelCaliper_material
            objectName: "WheelCaliper"
            baseColor: "#ff800000"
            metalness: 1
            roughness: 0.44999998807907104
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: wheelTireBump_material
            baseColorMap: node.desert? textureDirtTire : textureData901
            normalStrength: 1
            specularAmount: 0.24748
            objectName: "WheelTireBump"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.89478
            normalMap: _15_texture
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }

        PrincipledMaterial {
            id: carPaint_material23
            objectName: "CarPaint"
            baseColor: "#131313"
            roughness: 0.10000000149011612
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            clearcoatAmount: 1
            clearcoatRoughnessAmount: 0.029999999329447746
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: wheelRimBlack_material
            baseColor: "#acacac"
            objectName: "WheelRimBlack"
            baseColorMap: _16_texture
            metalness: 1
            roughness: 0.85566
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            clearcoatAmount: 0.31042
            clearcoatRoughnessAmount: 0.029999999329447746
            indexOfRefraction: 1.4500000476837158
        }

        PrincipledMaterial {
            id: plasticBlack_material24
            objectName: "PlasticBlack"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.91248
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Opaque
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: plasticBlack_material24_rain
            objectName: "PlasticBlack"
            baseColor: "#ffffff"
            metalness: 1
            roughness: 0.91248
            specularAmount: 1
            cullMode: PrincipledMaterial.NoCulling
            indexOfRefraction: 1.4500000476837158
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledExSnowMaterial {
            id: glassWindsSide_material
            snowStrength: node.snowStrength
            snowViewProjection: node.particleMaskCar.viewProjection
            snowDisplacementMap: node.particleMaskCarTexture
            opacity: 0.379
            roughness: 0.63159
            clearcoatRoughnessAmount: 0.01919
            clearcoatAmount: 0.3178
            objectName: "GlassWindsSide"
            baseColor: "#383838"
            metalness: 0.20139
            cullMode: PrincipledMaterial.NoCulling
            alphaMode: PrincipledMaterial.Blend
            indexOfRefraction: 1.4500000476837158
        }

        RainMaterial {
            id: glassWindsSide_material_rain
            opacity: 0.379
            roughness: 0.2
            clearcoatRoughnessAmount: 0.01919
            clearcoatAmount: 0.3178
            objectName: "GlassWindsSide"
            baseColor: "#383838"
            metalness: 0.80139
            indexOfRefraction: 1.4500000476837158
            specularAmount: 1
            rainStrength : node.rainStrength
            normalFactor : node.normalFactor
            rainSize : node.rainSize
            rainFrequency : node.rainFrequency
            rainPower : node.rainPower
            rainScale : node.rainScale
            dripSize : node.dripSize
            dripSpeed : node.dripSpeed
            dripLength : node.dripLength
            dripSharpness : node.dripSharpness
        }

        PrincipledMaterial {
            id: invisibleMat
            opacity: 0
            alphaMode: PrincipledMaterial.Opaque
            lighting: PrincipledMaterial.NoLighting
            objectName: "InvisibleMat"
        }

        Texture {
            id: textureData901
            source: node.downloadBase + "/content/images/textureData90.jpg"
        }

        Texture {
            id: textureDirtTire
            source: node.downloadBase + "/content/images/textureDirtTire.jpg"
        }
    }
    states: [
        State {
            name: "black"
            when: node.stateController == 0

            PropertyChanges {
                target: body
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: chargingCap
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: headlights
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: hood
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: taillights
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: trunkLid
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wingFlaps
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeDiskRearLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: hoodEngineCover
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: trunkEngineCover
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeDiskFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeCaliperFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeCaliperBkLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelBkLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: doorRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: doorLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelFrRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelBkRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: metalDark_material9
                clearcoatAmount: 0.23325
                roughness: 0.82009
            }

            PropertyChanges {
                target: carPaint_material
                // secondaryColor: "#000000" // Doesn't have this property???
            }
        },
        State {
            name: "white"
            when: node.stateController == 1

            PropertyChanges {
                target: carPaint_material
                metalness: 0.1
                specularAmount: 0.1
                baseColor: "#a6a6a6"
            }
        },
        State {
            name: "yellow"
            when: node.stateController == 2

            PropertyChanges {
                target: carPaint_material
                metalness: 0.1
                specularAmount: 0.1
                baseColor: "#de8517"
            }
        },
        State {
            name: "red"
            when: node.stateController == 3

            PropertyChanges {
                target: carPaint_material
                metalness: 0.5
                specularAmount: 0.1
                baseColor: "#a21010"
            }
        },
        State {
            name: "rainblack"
            when: node.stateController == 4

            PropertyChanges {
                target: body
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: chargingCap
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: headlights
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: hood
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: taillights
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: trunkLid
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wingFlaps
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeDiskRearLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: hoodEngineCover
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: trunkEngineCover
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeDiskFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeCaliperFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: brakeCaliperBkLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelFrLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelBkLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: doorRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: doorLeft
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelFrRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: wheelBkRight
                receivesShadows: false
                castsShadows: false
            }

            PropertyChanges {
                target: metalDark_material9_rain
                clearcoatAmount: 0.23325
                roughness: 0.82009
            }

            PropertyChanges {
                target: carPaint_material_rain
                specularAmount: 0.8
            }
        },
        State {
            name: "rainwhite"
            when: node.stateController == 5

            PropertyChanges {
                target: carPaint_material_rain
                metalness: 0.1
                specularAmount: 0.8
                baseColor: "#a6a6a6"
            }
        },
        State {
            name: "rainyellow"
            when: node.stateController == 6

            PropertyChanges {
                target: carPaint_material_rain
                metalness: 0.1
                specularAmount: 0.8
                baseColor: "#de8517"
            }
        },
        State {
            name: "rainred"
            when: node.stateController == 7

            PropertyChanges {
                target: carPaint_material_rain
                metalness: 0.5
                specularAmount: 0.8
                baseColor: "#a21010"
            }
        }
    ]
    transitions: [
        Transition {
            id: transition
            ParallelAnimation {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: metalDark_material9
                        property: "clearcoatAmount"
                        duration: 1359
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: metalDark_material9
                        property: "roughness"
                        duration: 1359
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: metalDark_material9_rain
                        property: "roughness"
                        duration: 1359
                    }
                }
            }

            ParallelAnimation {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: carPaint_material
                        property: "baseColor"
                        duration: 734
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: carPaint_material_rain
                        property: "baseColor"
                        duration: 734
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: carPaint_material
                        property: "metalness"
                        duration: 734
                    }
                }

                SequentialAnimation {
                    PauseAnimation {
                        duration: 50
                    }

                    PropertyAnimation {
                        target: carPaint_material_rain
                        property: "metalness"
                        duration: 734
                    }
                }
            }
            to: "*"
            from: "*"
        }
    ]
}
