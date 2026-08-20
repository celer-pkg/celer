// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: sketchfab_model
    rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)

    Node {
        id: round_carpet_obj_cleaner_materialmerger_gles

        Model {
            id: object_2
            source: "meshes/object_2.mesh"

            PrincipledMaterial {
                id: carpet_texture_material
                baseColorMap: Texture {
                    source: "maps/carpet_texture_baseColor.jpeg"
                    tilingModeHorizontal: Texture.Repeat
                    tilingModeVertical: Texture.Repeat
                }
                opacityChannel: Material.A
                metalness: 0
                roughness: 1
                cullMode: Material.NoCulling
            }
            materials: [carpet_texture_material]
        }

        Model {
            id: object_3
            source: "meshes/object_3.mesh"

            PrincipledMaterial {
                id: contour_material
                baseColor: "#ff1b0a05"
                metalness: 0
                roughness: 0.983808
                cullMode: Material.NoCulling
            }
            materials: [contour_material]
        }
    }
}
