// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    required property AccessoryModel accessoryModel

    readonly property alias items_generic: items_generic
    readonly property alias annoyedEyes: annoyedEyes
    readonly property alias eyeBrows_Black: eyeBrows_Black
    readonly property alias mouth: mouth
    readonly property alias crazyMouthMat: crazyMouthMat
    readonly property alias quteEyes: quteEyes
    readonly property alias eyeBrows_Black_004: eyeBrows_Black_004
    readonly property alias powerpuffEyes_001: powerpuffEyes_001
    readonly property alias eyeBrows_Black_001: eyeBrows_Black_001
    readonly property alias happyMouthMat: happyMouthMat
    readonly property alias defaultEyes: defaultEyes
    readonly property alias eyeBrows_Black_005: eyeBrows_Black_005
    readonly property alias smallMouthMat: smallMouthMat
    readonly property alias eyeBrows_Black_002: eyeBrows_Black_002
    readonly property alias eyeBrows_Black_003: eyeBrows_Black_003
    readonly property alias material_001: material_001
    readonly property alias eyewear_generic: eyewear_generic
    readonly property alias nose: nose
    readonly property alias nvgoggles_lens: nvgoggles_lens
    readonly property alias nvgoggles_body: nvgoggles_body
    readonly property alias darkTextile: darkTextile
    readonly property alias nvgoggleDetail: nvgoggleDetail
    readonly property alias bandana: bandana
    readonly property alias beanie: beanie
    readonly property alias cap: cap
    readonly property alias capFront: capFront
    readonly property alias bdayHat: bdayHat
    readonly property alias headphones: headphones
    readonly property alias wizardHat: wizardHat
    readonly property alias whiskers: whiskers
    readonly property alias angelWings: angelWings
    readonly property alias backpack: backpack
    readonly property alias bowtie: bowtie
    readonly property alias butterflyWings: butterflyWings
    readonly property alias necktie: necktie

    component AccessoryMaterial: PrincipledMaterial {
        readonly property AccessoryModel accessoryModel: root.accessoryModel
        property color defaultColor: "white"
        property string accessoryName: ""
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Opaque
        baseColor: {
            if (accessoryName === "")
                return defaultColor
            if (!accessoryModel)
                return defaultColor
            const color = accessoryModel.colorOf(accessoryName)
            return color.valid ? color : defaultColor
        }
    }

    component AccessoryGenericMaterial: AccessoryMaterial {
        metalness: 0.80838
        roughness: 0.92842
        alphaMode: PrincipledMaterial.Default

        baseColorMap: baseColor === defaultColor ? generic_colorAtlas2_baseColor : null
        Texture {
            id: generic_colorAtlas2_baseColor
            source: "images/Generic_ColorAtlas2_BaseColor.png"
        }
        metalnessMap: Texture {
            source: "images/Generic_ColorAtlas2_Metallic.png"
        }
        roughnessMap: Texture {
            source: "images/Generic_ColorAtlas2_Roughness.png"
        }
    }

    // headwear materials
    AccessoryMaterial {
        id: beanie
        roughness: 1
        defaultColor: "#ff890412"
        accessoryName: "Beanie"
    }

    AccessoryMaterial {
        id: cap
        roughness: 1
        defaultColor: "#ff890412"
        accessoryName: "Cap"
    }

    AccessoryMaterial {
        id: capFront
        roughness: 0.5
        defaultColor: "#ffcccccc"
    }

    AccessoryMaterial {
        id: bdayHat
        metalness: 1
        roughness: 1
        accessoryName: "Party Hat"
        baseColorMap: baseColor === defaultColor ? bday_hat_baseColor : null
        Texture {
            id: bday_hat_baseColor
            source: "images/BdayHat_BaseColor.png"
        }
    }

    AccessoryGenericMaterial {
        id: headphones
        accessoryName: "Headphones"
    }

    AccessoryMaterial {
        id: wizardHat
        roughness: 1
        accessoryName: "Wizard Hat"
        baseColorMap: baseColor === defaultColor ? wizardhat_BaseColor : null
        Texture {
            id: wizardhat_BaseColor
            source: "images/Wizardhat_BaseColor.png"
        }
    }

    AccessoryMaterial {
        id: whiskers
        roughness: 1
        defaultColor: "#050505"
        accessoryName: "Whiskers"
    }

    AccessoryMaterial {
        id: bandana
        roughness: 0.95533
        defaultColor: "#ff50031e"
        accessoryName: "Bandana Hat"
    }

    // eyewear materials
    AccessoryGenericMaterial {
        id: eyewear_generic
        accessoryName: {
            if (AccessoryState.monacleVisible)
                return "Monacle"
            if (AccessoryState.eyePatchVisible)
                return "EyePatch"
            if (AccessoryState.sunglassesVisible)
                return "Sunglasses"
            if (AccessoryState.roundGlassesVisible)
                return "Round Glasses"
            return ""
        }
        roughnessMap: null
        metalnessMap: null
    }

    AccessoryMaterial {
        id: nose
        roughness: 0.85185
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Default
        accessoryName: "Incognito"
        baseColorMap: Texture { source: "images/incognito_color.png" }
    }

    AccessoryMaterial {
        id: nvgoggles_lens
        emissiveFactor.x: 0
        emissiveFactor.y: 2.27113
        emissiveFactor.z: 0.0106965
        defaultColor: "#0eff00"
        accessoryName: "Night Vision Goggles"
    }

    AccessoryMaterial {
        id: nvgoggles_body
        metalness: 1
        roughness: 0.5589519739151001
        defaultColor: "#373737"
        accessoryName: "Night Vision Goggles"
    }

    AccessoryMaterial {
        id: darkTextile
        roughness: 1
        defaultColor: "#171714"
        accessoryName: "Night Vision Goggles"
    }

    AccessoryMaterial {
        id: nvgoggleDetail
        roughness: 0.8253275156021118
        defaultColor: "#404040"
        accessoryName: "Night Vision Goggles"
    }

    // eyes materials
    AccessoryMaterial {
        id: eyeBrows_Black_005
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: "Small Eyes"
    }

    AccessoryMaterial {
        id: defaultEyes
        roughness: 0.3590908944606781
        defaultColor: "#ff040404"
        accessoryName: {
            if (AccessoryState.smallEyesVisible)
                return "Small Eyes"
            if (AccessoryState.surprisedEyesVisible)
                return "Surprised Eyes"
            return ""
        }
    }

    AccessoryMaterial {
        id: smallMouthMat
        roughness: 1
        alphaMode: PrincipledMaterial.Mask
        defaultColor: "#ff040404"
        baseColorMap: Texture { source: "images/smallmouth.png" }
        accessoryName: {
            if (AccessoryState.smallEyesVisible)
                return "Small Eyes"
            if (AccessoryState.wideEyesVisible)
                return "Wide Eyes"
            return ""

        }
    }

    AccessoryMaterial {
        id: quteEyes
        roughness: 0.35423195362091064
        defaultColor: "#ff040404"
        accessoryName: "Cute Eyes"
        baseColorMap: Texture { source: "images/quteEyes.png" }
    }

    AccessoryMaterial {
        id: eyeBrows_Black_004
        roughness: 1
        accessoryName: {
            if (AccessoryState.cuteEyesVisible)
                return "Cute Eyes"
            if (AccessoryState.wideEyesVisible)
                return "Wide Eyes"
            return ""
        }
        defaultColor: "#ff050505"
    }

    AccessoryMaterial {
        id: mouth
        objectName: "Mouth"
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: {
            if (AccessoryState.cuteEyesVisible)
                return "Cute Eyes"
            if (AccessoryState.annoyedEyesVisible)
                return "Annoyed Eyes"
            return ""
        }
    }

    AccessoryMaterial {
        id: annoyedEyes
        clearcoatAmount: 0.83622
        metalness: 0
        roughness: 0.03846
        accessoryName: "Annoyed Eyes"
        baseColorMap: Texture { source: "images/EyesImages/AnnoyedEyesImage1.png" }
    }

    AccessoryMaterial {
        id: eyeBrows_Black
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: {
            if (AccessoryState.smallEyesVisible)
                return "Annoyed Eyes"
            if (AccessoryState.surprisedEyesVisible)
                return "Surprised Eyes"
            if (AccessoryState.confusedEyesVisible)
                return "Confused Eyes"
            if (AccessoryState.wideEyesVisible)
                return "Wide Eyes"
            return ""
        }
    }

    AccessoryMaterial {
        id: happyMouthMat
        roughness: 1
        alphaMode: PrincipledMaterial.Blend
        baseColorMap: Texture {
            source: "images/wowmouth.png"
            autoOrientation: true
        }
    }

    AccessoryMaterial {
        id: crazyMouthMat
        roughness: 0.5
        alphaMode: PrincipledMaterial.Blend
        baseColorMap: Texture { source: "images/crazyMouth1.png" }
    }

    AccessoryMaterial {
        id: powerpuffEyes_001
        roughness: 0.5636363625526428
        accessoryName: "Power Puff"
        baseColorMap: Texture { source: "images/PowerpuffEye.png" }
    }

    AccessoryMaterial {
        id: eyeBrows_Black_001
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: "Power Puff"
    }

    AccessoryMaterial {
        id: eyeBrows_Black_002
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: "Wide Eyes"
    }

    AccessoryMaterial {
        id: eyeBrows_Black_003
        roughness: 1
        defaultColor: "#ff050505"
        accessoryName: "Wide Eyes"
    }

    AccessoryMaterial {
        id: material_001
        defaultColor: "#ffcccccc"
        accessoryName: "Wide Eyes"
    }

    // items materials
    AccessoryMaterial {
        id: butterflyWings
        roughness: 0.5
        accessoryName: "Butterfly Wings"
        baseColorMap: baseColor === defaultColor ? butterflywings_BaseColor : null
        Texture {
            id: butterflywings_BaseColor
            source: "images/Butterflywings_BaseColor.png"
        }
    }

    AccessoryMaterial {
        id: angelWings
        roughness: 0.5
        accessoryName: "Angel Wings"
        baseColorMap: baseColor === defaultColor ? angelWings_BaseColor : null
        Texture {
            id: angelWings_BaseColor
            source: "images/AngelWings_BaseColor.png"
        }
    }

    AccessoryMaterial {
        id: bowtie
        roughness: 1
        defaultColor: "#ffcc1436"
        accessoryName: "Bowtie"
    }

    AccessoryMaterial {
        id: backpack
        roughness: 1
        accessoryName: "Backpack"
        baseColorMap: baseColor === defaultColor ? backpack_BaseColor : null
        Texture {
            id: backpack_BaseColor
            source: "images/Backpack_BaseColor.png"
        }
    }

    AccessoryMaterial {
        id: necktie
        roughness: 1
        defaultColor: "#ff2b0100"
        accessoryName: "Necktie"
    }

    AccessoryGenericMaterial {
        id: items_generic
        accessoryName: "Braclets"
    }
}
