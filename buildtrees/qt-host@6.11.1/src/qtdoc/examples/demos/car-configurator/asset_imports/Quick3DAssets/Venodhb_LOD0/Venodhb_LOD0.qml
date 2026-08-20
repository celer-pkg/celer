// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root
    required property url downloadBase

    Model {
        id: aset_nature_rock_M_venodhb_LOD0

        source: "#Rectangle"
        eulerRotation.z: 0
        eulerRotation.y: 0
        eulerRotation.x: -90
        scale.z: 6
        scale.y: 6
        scale.x: 6
        materials: [
            _mat_defaultMat_material
        ]
    }

    Node {
        id: __materialLibrary__

        PrincipledMaterial {
            id: _mat_defaultMat_material
            roughnessMap: venodhb_2K_Roughness
            normalMap: venodhb_2K_Normal_LOD0
            opacityMap: dot1
            alphaMode: PrincipledMaterial.Opaque
            normalStrength: 0.14815

            objectName: "_mat_defaultMat_material"
            heightAmount: 1
            roughness: 0.91394
            baseColorMap: venodhb_2K_Albedo

            Texture {
                id: venodhb_2K_Albedo

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Albedo.jpg"
                generateMipmaps: true
                mipFilter: Texture.Nearest
                scaleV: 60
                scaleU: 60
            }

            Texture {
                id: venodhb_2K_Normal_LOD0

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Normal_LOD0.jpg"
                scaleV: 60
                scaleU: 60
            }

            Texture {
                id: venodhb_2K_Roughness

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Roughness.jpg"
                scaleV: 60
                scaleU: 60
            }
        }

        Texture {
            id: dot1
            source: root.downloadBase + "/content/images/dot.png"
        }

        PrincipledMaterial {
            id: _mat_defaultMat_materialCopy
            normalMap: venodhb_2K_Normal_LOD02
            roughnessMap: venodhb_2K_Roughness2
            roughness: 0.91394
            objectName: "_mat_defaultMat_material copy"
            normalStrength: 0.14815
            heightAmount: 1
            baseColorMap: venodhb_2K_Albedo2
            alphaMode: PrincipledMaterial.Opaque
            Texture {
                id: venodhb_2K_Albedo2

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Albedo.jpg"
                generateMipmaps: true
                mipFilter: Texture.Nearest
                scaleV: 10
                scaleU: 10
            }

            Texture {
                id: venodhb_2K_Normal_LOD02

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Normal_LOD0.jpg"
                scaleV: 10
                scaleU: 10
            }

            Texture {
                id: venodhb_2K_Roughness2

                source: root.downloadBase + "/asset_imports/Quick3DAssets/Venodhb_LOD0/images/venodhb_2K_Roughness.jpg"
                scaleV: 10
                scaleU: 10
            }
        }
    }

    Model {
        id: aset_nature_rock_M_venodhb_LOD1
        x: 0
        y: -0.071
        source: "#Rectangle"
        z: 0
        scale.z: 1
        scale.y: 1
        scale.x: 1
        materials: _mat_defaultMat_materialCopy
        eulerRotation.z: 0
        eulerRotation.y: 0
        eulerRotation.x: -90
    }
}
