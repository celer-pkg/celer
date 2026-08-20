// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import ".."

SceneBase {
    id: node
    visible: false

    // SceneBase properties
    cameraIndex : 0
    cameraPositions: [
        Qt.vector3d(-501.325, 326.578, -66.7134),
        Qt.vector3d(912.591, 211.582, -33.0186),
        Qt.vector3d(1062.89, 256.128, -472.006)
    ]
    cameraRotations: [
        Qt.vector3d(-4.76112, -91.447, 0),
        Qt.vector3d(-10.0246, -90.3894, 0),
        Qt.vector3d(-18.0934, 88.4485, 0)
    ]
    lightmapper: Lightmapper {
        source: "qrc:/assets/lightmaps/" + node.lightmapSource
        samples: node.samples
        texelsPerUnit: node.texelsPerUnit
        bounces: node.bounces
        denoiseSigma: node.denoiseSigma
        indirectLightFactor: node.indirectLightFactor
    }
    backgroundMode: SceneEnvironment.SkyBox
    lightProbe: Texture {
        source: "qrc:/assets/Sponza/moonless_golf_4k.ktx"
    }
    enableLightmaps: true
    ssgiIndirectLightBoost: 5
    probeExposure: 0.2

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
        visible: node.visible
    }

    Model {
        source: "#Sphere"
        scale: Qt.vector3d(1.5, 1.5, 1.5)
        x: 500
        y: 300
        z: -50
        usedInBakedLighting: node.enableLightmaps
        bakedLightmap: BakedLightmap {
            enabled: node.enableLightmaps
            key: "sphere"
        }

        materials: PrincipledMaterial {
            emissiveFactor: Qt.vector3d(10, 0, 10)
        }
    }


    property string lightmapSource: "sponza/lm_sponza.bin"
    property real indirectLightFactor: 2.5
    property int samples: 1024
    property real texelsPerUnit: 0.1
    property int bounces: 3
    property real denoiseSigma: 8
    property int lightBakeMode: node.enableLightmaps ? Light.BakeModeIndirect : Light.BakeModeDisabled

    property real shadowFactor: 90
    property int shadowMapQuality: Light.ShadowMapQualityUltra
    property int softShadowQuality: Light.PCF16
    property real pcfFactor: 5

    // Directional light
    property real dirLightX: -60.0
    property real dirLightY: -90
    property real dirLightBrightness: 0.1

    // Resources
    Texture {
        id: node4477655471536070370_jpg_texture
        objectName: "4477655471536070370.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4477655471536070370.jpg"
    }
    Texture {
        id: node5061699253647017043_png_texture
        objectName: "5061699253647017043.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/5061699253647017043.png"
    }
    Texture {
        id: node11872827283454512094_jpg_texture
        objectName: "11872827283454512094.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/11872827283454512094.jpg"
    }
    Texture {
        id: node8773302468495022225_jpg_texture
        objectName: "8773302468495022225.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8773302468495022225.jpg"
    }
    Texture {
        id: node755318871556304029_jpg_texture
        objectName: "755318871556304029.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/755318871556304029.jpg"
    }
    Texture {
        id: node8006627369776289000_png_texture
        objectName: "8006627369776289000.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8006627369776289000.png"
    }
    Texture {
        id: node715093869573992647_jpg_texture
        objectName: "715093869573992647.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/715093869573992647.jpg"
    }
    Texture {
        id: node12501374198249454378_jpg_texture
        objectName: "12501374198249454378.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/12501374198249454378.jpg"
    }
    Texture {
        id: node2411100444841994089_jpg_texture
        objectName: "2411100444841994089.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2411100444841994089.jpg"
    }
    Texture {
        id: node7268504077753552595_jpg_texture
        objectName: "7268504077753552595.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/7268504077753552595.jpg"
    }
    Texture {
        id: node8503262930880235456_jpg_texture
        objectName: "8503262930880235456.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8503262930880235456.jpg"
    }
    Texture {
        id: node4871783166746854860_jpg_texture
        objectName: "4871783166746854860.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4871783166746854860.jpg"
    }
    Texture {
        id: node3827035219084910048_jpg_texture
        objectName: "3827035219084910048.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/3827035219084910048.jpg"
    }
    Texture {
        id: node8750083169368950601_jpg_texture
        objectName: "8750083169368950601.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8750083169368950601.jpg"
    }
    Texture {
        id: node16885566240357350108_jpg_texture
        objectName: "16885566240357350108.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/16885566240357350108.jpg"
    }
    Texture {
        id: node13982482287905699490_jpg_texture
        objectName: "13982482287905699490.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/13982482287905699490.jpg"
    }
    Texture {
        id: node17556969131407844942_jpg_texture
        objectName: "17556969131407844942.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/17556969131407844942.jpg"
    }
    Texture {
        id: node5792855332885324923_jpg_texture
        objectName: "5792855332885324923.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/5792855332885324923.jpg"
    }
    Texture {
        id: node11968150294050148237_jpg_texture
        objectName: "11968150294050148237.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/11968150294050148237.jpg"
    }
    Texture {
        id: node16299174074766089871_jpg_texture
        objectName: "16299174074766089871.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/16299174074766089871.jpg"
    }
    Texture {
        id: node10381718147657362067_jpg_texture
        objectName: "10381718147657362067.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/10381718147657362067.jpg"
    }
    Texture {
        id: node14650633544276105767_jpg_texture
        objectName: "14650633544276105767.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/14650633544276105767.jpg"
    }
    Texture {
        id: node6772804448157695701_jpg_texture
        objectName: "6772804448157695701.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/6772804448157695701.jpg"
    }
    Texture {
        id: node2051777328469649772_jpg_texture
        objectName: "2051777328469649772.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2051777328469649772.jpg"
    }
    Texture {
        id: node13196865903111448057_jpg_texture
        objectName: "13196865903111448057.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/13196865903111448057.jpg"
    }
    Texture {
        id: node15295713303328085182_jpg_texture
        objectName: "15295713303328085182.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/15295713303328085182.jpg"
    }
    Texture {
        id: node9916269861720640319_jpg_texture
        objectName: "9916269861720640319.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/9916269861720640319.jpg"
    }
    Texture {
        id: node10388182081421875623_jpg_texture
        objectName: "10388182081421875623.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/10388182081421875623.jpg"
    }
    Texture {
        id: node759203620573749278_jpg_texture
        objectName: "759203620573749278.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/759203620573749278.jpg"
    }
    Texture {
        id: node6047387724914829168_jpg_texture
        objectName: "6047387724914829168.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/6047387724914829168.jpg"
    }
    Texture {
        id: node8051790464816141987_jpg_texture
        objectName: "8051790464816141987.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8051790464816141987.jpg"
    }
    Texture {
        id: node15722799267630235092_jpg_texture
        objectName: "15722799267630235092.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/15722799267630235092.jpg"
    }
    Texture {
        id: node2969916736137545357_jpg_texture
        objectName: "2969916736137545357.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2969916736137545357.jpg"
    }
    Texture {
        id: node5823059166183034438_jpg_texture
        objectName: "5823059166183034438.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/5823059166183034438.jpg"
    }
    Texture {
        id: node13824894030729245199_jpg_texture
        objectName: "13824894030729245199.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/13824894030729245199.jpg"
    }
    Texture {
        id: node14267839433702832875_jpg_texture
        objectName: "14267839433702832875.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/14267839433702832875.jpg"
    }
    Texture {
        id: node7645212358685992005_jpg_texture
        objectName: "7645212358685992005.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/7645212358685992005.jpg"
    }
    Texture {
        id: node7441062115984513793_jpg_texture
        objectName: "7441062115984513793.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/7441062115984513793.jpg"
    }
    Texture {
        id: node8114461559286000061_jpg_texture
        objectName: "8114461559286000061.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8114461559286000061.jpg"
    }
    Texture {
        id: node6667038893015345571_jpg_texture
        objectName: "6667038893015345571.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/6667038893015345571.jpg"
    }
    Texture {
        id: node8747919177698443163_jpg_texture
        objectName: "8747919177698443163.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8747919177698443163.jpg"
    }
    Texture {
        id: node11490520546946913238_jpg_texture
        objectName: "11490520546946913238.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/11490520546946913238.jpg"
    }
    Texture {
        id: node3455394979645218238_jpg_texture
        objectName: "3455394979645218238.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/3455394979645218238.jpg"
    }
    Texture {
        id: node3628158980083700836_jpg_texture
        objectName: "3628158980083700836.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/3628158980083700836.jpg"
    }
    Texture {
        id: node14118779221266351425_jpg_texture
        objectName: "14118779221266351425.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/14118779221266351425.jpg"
    }
    Texture {
        id: node6151467286084645207_jpg_texture
        objectName: "6151467286084645207.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/6151467286084645207.jpg"
    }
    Texture {
        id: node8783994986360286082_jpg_texture
        objectName: "8783994986360286082.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8783994986360286082.jpg"
    }
    Texture {
        id: node8481240838833932244_jpg_texture
        objectName: "8481240838833932244.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/8481240838833932244.jpg"
    }
    Texture {
        id: white_png_texture
        objectName: "white.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/white.png"
    }
    Texture {
        id: node4975155472559461469_jpg_texture
        objectName: "4975155472559461469.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4975155472559461469.jpg"
    }
    Texture {
        id: node3371964815757888145_jpg_texture
        objectName: "3371964815757888145.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/3371964815757888145.jpg"
    }
    Texture {
        id: node2299742237651021498_jpg_texture
        objectName: "2299742237651021498.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2299742237651021498.jpg"
    }
    Texture {
        id: node4675343432951571524_jpg_texture
        objectName: "4675343432951571524.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4675343432951571524.jpg"
    }
    Texture {
        id: node7815564343179553343_jpg_texture
        objectName: "7815564343179553343.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/7815564343179553343.jpg"
    }
    Texture {
        id: node7056944414013900257_jpg_texture
        objectName: "7056944414013900257.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/7056944414013900257.jpg"
    }
    Texture {
        id: node2775690330959970771_jpg_texture
        objectName: "2775690330959970771.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2775690330959970771.jpg"
    }
    Texture {
        id: node2374361008830720677_jpg_texture
        objectName: "2374361008830720677.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2374361008830720677.jpg"
    }
    Texture {
        id: node2185409758123873465_jpg_texture
        objectName: "2185409758123873465.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/2185409758123873465.jpg"
    }
    Texture {
        id: node332936164838540657_jpg_texture
        objectName: "332936164838540657.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/332936164838540657.jpg"
    }
    Texture {
        id: node17876391417123941155_jpg_texture
        objectName: "17876391417123941155.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/17876391417123941155.jpg"
    }
    Texture {
        id: node466164707995436622_jpg_texture
        objectName: "466164707995436622.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/466164707995436622.jpg"
    }
    Texture {
        id: node6593109234861095314_jpg_texture
        objectName: "6593109234861095314.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/6593109234861095314.jpg"
    }
    Texture {
        id: node11474523244911310074_jpg_texture
        objectName: "11474523244911310074.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/11474523244911310074.jpg"
    }
    Texture {
        id: node4601176305987539675_jpg_texture
        objectName: "4601176305987539675.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4601176305987539675.jpg"
    }
    Texture {
        id: node9288698199695299068_jpg_texture
        objectName: "9288698199695299068.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/9288698199695299068.jpg"
    }
    Texture {
        id: node4910669866631290573_jpg_texture
        objectName: "4910669866631290573.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/4910669866631290573.jpg"
    }
    Texture {
        id: node16275776544635328252_png_texture
        objectName: "16275776544635328252.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/16275776544635328252.png"
    }
    Texture {
        id: node1219024358953944284_jpg_texture
        objectName: "1219024358953944284.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/1219024358953944284.jpg"
    }
    Texture {
        id: node14170708867020035030_jpg_texture
        objectName: "14170708867020035030.jpg"
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "qrc:/assets/Sponza/maps/14170708867020035030.jpg"
    }
    PrincipledMaterial {
        id: principledMaterial
        baseColor: "#ffc9c9c9"
        baseColorMap: node8006627369776289000_png_texture
        metalnessMap: node715093869573992647_jpg_texture
        roughnessMap: node715093869573992647_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node12501374198249454378_jpg_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Mask
        depthDrawMode: PrincipledMaterial.OpaquePrePassDepthDraw
    }
    PrincipledMaterial {
        id: principledMaterial10
        baseColor: "#ffc9c9c9"
        baseColorMap: node7268504077753552595_jpg_texture
        metalnessMap: node8503262930880235456_jpg_texture
        roughnessMap: node8503262930880235456_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node4477655471536070370_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial14
        baseColor: "#ffc9c9c9"
        baseColorMap: node8750083169368950601_jpg_texture
        metalnessMap: node16885566240357350108_jpg_texture
        roughnessMap: node16885566240357350108_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node13982482287905699490_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial18
        baseColor: "#ffc9c9c9"
        baseColorMap: node5792855332885324923_jpg_texture
        metalnessMap: node11968150294050148237_jpg_texture
        roughnessMap: node11968150294050148237_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node16299174074766089871_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial22
        baseColor: "#ffc9c9c9"
        baseColorMap: node14650633544276105767_jpg_texture
        metalnessMap: node4871783166746854860_jpg_texture
        roughnessMap: node4871783166746854860_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node2051777328469649772_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial2
        baseColor: "#ffc9c9c9"
        baseColorMap: node5061699253647017043_png_texture
        metalnessMap: node11872827283454512094_jpg_texture
        roughnessMap: node11872827283454512094_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node8773302468495022225_jpg_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Mask
        depthDrawMode: PrincipledMaterial.OpaquePrePassDepthDraw
    }
    PrincipledMaterial {
        id: principledMaterial26
        baseColor: "#ffc9c9c9"
        baseColorMap: node15295713303328085182_jpg_texture
        metalnessMap: node9916269861720640319_jpg_texture
        roughnessMap: node9916269861720640319_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node10388182081421875623_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial30
        baseColor: "#ffc9c9c9"
        baseColorMap: node6047387724914829168_jpg_texture
        metalnessMap: node8051790464816141987_jpg_texture
        roughnessMap: node8051790464816141987_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node15722799267630235092_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial34
        baseColor: "#ffc9c9c9"
        baseColorMap: node5823059166183034438_jpg_texture
        metalnessMap: node13824894030729245199_jpg_texture
        roughnessMap: node13824894030729245199_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node14267839433702832875_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial42
        baseColor: "#ffc9c9c9"
        baseColorMap: node11490520546946913238_jpg_texture
        metalnessMap: node3455394979645218238_jpg_texture
        roughnessMap: node3455394979645218238_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node3628158980083700836_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial46
        baseColor: "#ffc9c9c9"
        baseColorMap: node6151467286084645207_jpg_texture
        metalnessMap: node8783994986360286082_jpg_texture
        roughnessMap: node8783994986360286082_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node7645212358685992005_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial60
        baseColor: "#ffc9c9c9"
        baseColorMap: node2775690330959970771_jpg_texture
        metalnessMap: node7815564343179553343_jpg_texture
        roughnessMap: node7815564343179553343_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node2374361008830720677_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial73
        baseColor: "#ffc9c9c9"
        baseColorMap: node9288698199695299068_jpg_texture
        metalnessMap: node466164707995436622_jpg_texture
        roughnessMap: node466164707995436622_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node4910669866631290573_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial84
        baseColor: "#ffc9c9c9"
        baseColorMap: node8481240838833932244_jpg_texture
        metalnessMap: node17556969131407844942_jpg_texture
        roughnessMap: node17556969131407844942_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node10381718147657362067_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial38
        baseColor: "#ffc9c9c9"
        baseColorMap: node7441062115984513793_jpg_texture
        metalnessMap: node8114461559286000061_jpg_texture
        roughnessMap: node8114461559286000061_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node6667038893015345571_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial66
        baseColor: "#ffc9c9c9"
        baseColorMap: node17876391417123941155_jpg_texture
        metalnessMap: node466164707995436622_jpg_texture
        roughnessMap: node466164707995436622_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node6593109234861095314_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial52
        baseColor: "#ffc9c9c9"
        baseColorMap: node4975155472559461469_jpg_texture
        metalnessMap: node3371964815757888145_jpg_texture
        roughnessMap: node3371964815757888145_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node2299742237651021498_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial88
        baseColor: "#ffc9c9c9"
        baseColorMap: node6772804448157695701_jpg_texture
        metalnessMap: node13196865903111448057_jpg_texture
        roughnessMap: node13196865903111448057_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node759203620573749278_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial76
        baseColor: "#ffc9c9c9"
        baseColorMap: node16275776544635328252_png_texture
        metalnessMap: node1219024358953944284_jpg_texture
        roughnessMap: node1219024358953944284_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node14170708867020035030_jpg_texture
        cullMode: PrincipledMaterial.NoCulling
        alphaMode: PrincipledMaterial.Mask
        depthDrawMode: PrincipledMaterial.OpaquePrePassDepthDraw
    }
    PrincipledMaterial {
        id: principledMaterial50
        baseColor: "#ffc9c9c9"
        baseColorMap: white_png_texture
        roughness: 1
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial63
        baseColor: "#ffc9c9c9"
        baseColorMap: node2185409758123873465_jpg_texture
        metalnessMap: node7815564343179553343_jpg_texture
        roughnessMap: node7815564343179553343_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node332936164838540657_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial92
        baseColor: "#ffc9c9c9"
        baseColorMap: node2969916736137545357_jpg_texture
        metalnessMap: node8747919177698443163_jpg_texture
        roughnessMap: node8747919177698443163_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node14118779221266351425_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial70
        baseColor: "#ffc9c9c9"
        baseColorMap: node11474523244911310074_jpg_texture
        metalnessMap: node466164707995436622_jpg_texture
        roughnessMap: node466164707995436622_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node4601176305987539675_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial80
        baseColor: "#ffc9c9c9"
        baseColorMap: node755318871556304029_jpg_texture
        metalnessMap: node2411100444841994089_jpg_texture
        roughnessMap: node2411100444841994089_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node3827035219084910048_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }
    PrincipledMaterial {
        id: principledMaterial56
        baseColor: "#ffc9c9c9"
        baseColorMap: node4675343432951571524_jpg_texture
        metalnessMap: node7815564343179553343_jpg_texture
        roughnessMap: node7815564343179553343_jpg_texture
        metalness: 1
        roughness: 1
        normalMap: node7056944414013900257_jpg_texture
        alphaMode: PrincipledMaterial.Opaque
    }

    // Nodes:
    Model {
        usedInBakedLighting: node.enableLightmaps
        texelsPerUnit: node.texelsPerUnit
        bakedLightmap: BakedLightmap {
            enabled: node.enableLightmaps
            key: "Sponza_nodes_0_"
        }
        id: nodes_0_
        objectName: "nodes[0]"
        source: "qrc:/assets/Sponza/meshes/nodes_0__mesh.mesh"
        materials: [
            principledMaterial2,
            principledMaterial,
            principledMaterial10,
            principledMaterial14,
            principledMaterial18,
            principledMaterial22,
            principledMaterial26,
            principledMaterial30,
            principledMaterial22,
            principledMaterial34,
            principledMaterial26,
            principledMaterial22,
            principledMaterial38,
            principledMaterial18,
            principledMaterial26,
            principledMaterial18,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial26,
            principledMaterial22,
            principledMaterial18,
            principledMaterial22,
            principledMaterial18,
            principledMaterial42,
            principledMaterial18,
            principledMaterial42,
            principledMaterial18,
            principledMaterial42,
            principledMaterial18,
            principledMaterial38,
            principledMaterial18,
            principledMaterial34,
            principledMaterial30,
            principledMaterial22,
            principledMaterial46,
            principledMaterial50,
            principledMaterial18,
            principledMaterial52,
            principledMaterial2,
            principledMaterial56,
            principledMaterial60,
            principledMaterial63,
            principledMaterial56,
            principledMaterial60,
            principledMaterial56,
            principledMaterial63,
            principledMaterial60,
            principledMaterial52,
            principledMaterial66,
            principledMaterial70,
            principledMaterial73,
            principledMaterial70,
            principledMaterial73,
            principledMaterial70,
            principledMaterial66,
            principledMaterial73,
            principledMaterial70,
            principledMaterial66,
            principledMaterial76,
            principledMaterial80,
            principledMaterial76,
            principledMaterial80,
            principledMaterial76,
            principledMaterial80,
            principledMaterial76,
            principledMaterial80,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial,
            principledMaterial10,
            principledMaterial84,
            principledMaterial88,
            principledMaterial14,
            principledMaterial88,
            principledMaterial14,
            principledMaterial18,
            principledMaterial92,
            principledMaterial18
        ]
    }

    // Animations:
}
