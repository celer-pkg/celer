// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
Node {
    scale: Qt.vector3d(200, 200, 200)
    //! [resource loader]
    ResourceLoader {
        meshSources: [
            frame.source,
            curtain.source
        ]
        textures: [
            tilePatternTexture,
            curtainNormalTexture
        ]
    }

    Model {
        id: frame
        z: -1.95
        source: "meshes/frame.mesh"

        PrincipledMaterial {
            id: frame_material
            baseColor: "#ffcccccc"
            metalness: 1
            roughness: 0.259091
            alphaMode: PrincipledMaterial.Opaque
        }
        materials: [
            frame_material
        ]
    }
    Model {
        id: curtain
        y: 3.02413
        z: 2.04922
        source: "meshes/curtain.mesh"

        PrincipledMaterial {
            id: curtain_material
            baseColorMap: Texture {
                id: tilePatternTexture
                source: "maps/tilepattern.png"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            opacityChannel: Material.A
            roughness: 0.5
            normalMap: Texture {
                id: curtainNormalTexture
                source: "maps/curtain_normal.jpg"
                generateMipmaps: true
                mipFilter: Texture.Linear
            }
            cullMode: Material.NoCulling
            alphaMode: PrincipledMaterial.Opaque
        }
        materials: [
            curtain_material
        ]
    }
    //! [resource loader]
}
