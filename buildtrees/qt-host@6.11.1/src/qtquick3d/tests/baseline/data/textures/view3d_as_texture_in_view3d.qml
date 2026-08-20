// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Item {
    id: root
    property int api: GraphicsInfo.api
    width: 800
    height: 400

    View3D {
        id: src
        renderMode: View3D.Offscreen
        width: parent.width / 2
        height: parent.height
        environment: SceneEnvironment {
            clearColor: "lightGray"
            backgroundMode: SceneEnvironment.Color
        }
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600);
        }
        DirectionalLight {
        }
        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(4, 4, 4)
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "../shared/maps/oulu_2.jpeg"
                }
            }
        }
    }

    View3D {
        width: parent.width / 2
        height: parent.height
        x: parent.width / 2
        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600);
        }
        Model {
            source: "../shared/models/suzanne.mesh"
            scale:  Qt.vector3d(120, 120, 120)
            materials: DefaultMaterial {
                lighting: DefaultMaterial.NoLighting
                diffuseMap: Texture {
                    sourceItem: src
                    // To get identical results on-screen with all graphics APIs.
                    // The other View3D renders into a texture as-is, and texture have Y up in OpenGL.
                    // There is nothing that would correct for this, so apply a V coordinate flip when using the texture.
                    flipV: root.api === GraphicsInfo.OpenGL
                }
            }
        }
    }
}
