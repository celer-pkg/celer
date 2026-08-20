// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Item {
    width: 400
    height: 400

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "lightGray"
            backgroundMode: SceneEnvironment.Color
        }

        OrthographicCamera { // no need to confuse the output with depth, orthographic is good here
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        camera: camera

        DirectionalLight {
        }

        // Show the same model with the same material, just switch the UV
        // channel from 0 to 1 in the second one. The mesh contains both UV0 and
        // UV1, with a different mapping, so the difference should be visually
        // obvious. If UV1 is not picked up correctly the second model (bottom
        // right corner) will be identical to the first one, which is wrong.

        Model {
            source: "../shared/models/animal_with_lightmapuv1.mesh"
            scale: Qt.vector3d(50, 50, 50)
            eulerRotation.y: -80
            x: -80
            y: 100
            materials: [
                DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/oulu_2.jpeg"
                    }
                }
            ]
        }

        Model {
            property real modelScale: 100
            source: "../shared/models/animal_with_lightmapuv1.mesh"
            scale: Qt.vector3d(50, 50, 50)
            eulerRotation.y: -80
            x: 80
            y: -70
            materials: [
                DefaultMaterial {
                    diffuseMap: Texture {
                        source: "../shared/maps/oulu_2.jpeg"
                        indexUV: 1
                    }
                }
            ]
        }
    }
}
