// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick3D

Model {
    id: side_Table_Pine_LOD0_M_Side_Table_Natural_Wenge_Wood_4K_0
    source: "meshes/side_Table_Pine_LOD0_M_Side_Table_Natural_Wenge_Wood_4K_0.mesh"

    PrincipledMaterial {
        id: m_Side_Table_Natural_Wenge_Wood_4K_material
        baseColorMap: Texture {
            source: "maps/M_Side_Table_Natural_Wenge_Wood_4K_baseColor_small.jpg"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        opacityChannel: Material.A
        metalnessMap: Texture {
            source: "maps/M_Side_Table_Natural_Wenge_Wood_4K_metallicRoughness_small.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        metalnessChannel: Material.B
        roughnessMap: Texture {
            source: "maps/M_Side_Table_Natural_Wenge_Wood_4K_metallicRoughness_small.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        roughnessChannel: Material.G
        metalness: 0
        roughness: 1
        normalMap: Texture {
            source: "maps/M_Side_Table_Natural_Wenge_Wood_4K_normal_small.png"
            tilingModeHorizontal: Texture.Repeat
            tilingModeVertical: Texture.Repeat
        }
        cullMode: Material.NoCulling
    }
    materials: [m_Side_Table_Natural_Wenge_Wood_4K_material]
}
