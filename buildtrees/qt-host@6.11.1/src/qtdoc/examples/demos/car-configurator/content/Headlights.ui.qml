// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D

Node {
    id: root

    scale.z: 0.01
    scale.y: 0.01
    scale.x: 0.01
    required property url downloadBase

    Node {
        id: leftLight

        Model {
            id: plane

            x: 697.808
            y: 946.448
            visible: true
            source: "#Rectangle"
            eulerRotation.z: 4.73765
            eulerRotation.y: 33.65914
            eulerRotation.x: -38.47049
            scale.z: 15
            scale.y: 15
            scale.x: 15
            z: 2334.27441
            materials: customMaterial
        }

        Model {
            id: plane1

            x: 769.557
            y: 956.631
            visible: true
            source: "#Rectangle"
            eulerRotation.z: -4.47964
            eulerRotation.y: 51.77982
            eulerRotation.x: -35.8432
            scale.x: 10
            scale.y: 10
            scale.z: 7
            z: 2309.35498
            materials: customMaterial
        }

        Model {
            id: plane4

            x: 681
            y: 978.777
            visible: true
            source: "#Rectangle"
            eulerRotation.z: -30.80816
            eulerRotation.y: 28.77665
            eulerRotation.x: -60.35681
            scale.x: 15
            scale.z: 15
            z: 2276.83838
            materials: customMaterial
            scale.y: 15
        }

        Model {
            id: plane5

            x: 681
            y: 902.211
            visible: true
            source: "#Rectangle"
            scale.x: 15
            scale.z: 15
            eulerRotation.z: 140.20901
            z: 2276.83838
            materials: customMaterial
            scale.y: 10
            eulerRotation.y: -137.54535
            eulerRotation.x: -46.82078
        }

        SpotLight {
            id: lightSpot

            x: 693.537
            y: 949.598
            color: "#defaff"
            brightness: 10
            ambientColor: "#000000"
            quadraticFade: 1
            constantFade: 0
            eulerRotation.z: 180
            eulerRotation.y: 175.09207
            eulerRotation.x: -25
            z: 2425.22314
            coneAngle: 60
            innerConeAngle: 40
        }

        PointLight {
            id: lightPoint

            x: 747.11
            y: 949.598
            visible: true
            color: "#defaff"
            scope: plane
            z: 2395.00732
            eulerRotation.x: -19.93977
            eulerRotation.z: 180
            ambientColor: "#000000"
            constantFade: 0
            quadraticFade: 1
            eulerRotation.y: 175.09207
            brightness: 1
        }
    }
    NumberAnimation {
        id: numberAnimation

        target: customMaterial
        property: "uTime"
        running: true
        to: 1
        from: 0
        loops: -1
        duration: 10000
    }

    Node {
        id: leftLight1

        scale.x: -1

        Model {
            id: plane2

            x: 697.808
            y: 946.448
            visible: true
            source: "#Rectangle"
            castsShadows: false
            receivesShadows: false
            z: 2334.27441
            eulerRotation.x: -38.47049
            eulerRotation.z: 4.73765
            scale.z: 15
            scale.y: 15
            eulerRotation.y: 33.65914
            materials: customMaterial
            scale.x: 15
        }

        Model {
            id: plane3

            x: 769.557
            y: 956.631
            visible: true
            source: "#Rectangle"
            castsShadows: false
            receivesShadows: false
            z: 2309.35498
            eulerRotation.x: -35.8432
            eulerRotation.z: -4.47964
            scale.z: 7
            scale.y: 10
            eulerRotation.y: 51.77982
            materials: customMaterial
            scale.x: 10
        }

        Model {
            id: plane6

            x: 681
            y: 978.777
            visible: true
            source: "#Rectangle"
            castsShadows: false
            receivesShadows: false
            z: 2276.83838
            eulerRotation.x: -60.35681
            eulerRotation.z: -30.80816
            scale.z: 15
            scale.y: 15
            eulerRotation.y: 28.77665
            materials: customMaterial
            scale.x: 15
        }

        Model {
            id: plane7

            x: 681
            y: 902.211
            visible: true
            source: "#Rectangle"
            castsShadows: false
            receivesShadows: false
            z: 2276.83838
            eulerRotation.x: -46.82078
            eulerRotation.z: 140.20901
            scale.z: 15
            scale.y: 10
            eulerRotation.y: -137.54535
            materials: customMaterial
            scale.x: 15
        }

        SpotLight {
            id: lightSpot1

            x: 693.537
            y: 949.598
            color: "#defaff"
            z: 2425.22314
            eulerRotation.x: -25
            eulerRotation.z: 180
            ambientColor: "#000000"
            constantFade: 0
            quadraticFade: 1
            eulerRotation.y: 175.09207
            brightness: 10
            coneAngle: 60
            innerConeAngle: 40
        }

        PointLight {
            id: lightPoint1

            x: 747.11
            y: 949.598
            visible: true
            color: "#defaff"
            scope: plane2
            z: 2395.00732
            eulerRotation.x: -19.93977
            eulerRotation.z: 180
            ambientColor: "#000000"
            constantFade: 0
            quadraticFade: 1
            eulerRotation.y: 175.09207
            brightness: 1
        }
    }

    Node {
        id: __materialLibrary__

        CustomMaterial {
            id: customMaterial

            objectName: "customMaterial"
            sourceBlend: CustomMaterial.SrcAlpha
            destinationBlend: CustomMaterial.OneMinusSrcAlpha
            cullMode: Material.NoCulling
            vertexShader: root.downloadBase + "/content/shaders/flaremat.vert"
            fragmentShader: root.downloadBase + "/content/shaders/flaremat.frag"

            property real uTime: 0

            property TextureInput tex: TextureInput {
                texture: flareTexture
            }

            property TextureInput smoke: TextureInput {
                texture: smokeTexture
            }
            alwaysDirty: false
            depthDrawMode: Material.OpaqueOnlyDepthDraw

            Texture {
                id: flareTexture

                source: root.downloadBase + "/content/images/headlightflare.png"
            }
            Texture {
                id: smokeTexture

                source: root.downloadBase + "/content/images/smoke.png"
            }
        }
    }
}
