// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: PerspectiveCamera {
            z: 300
        }

        DirectionalLight {}

        Model {
            source: "../shared/models/suzanne_vertexcolor.mesh"

            instancing: InstanceList {
                InstanceListEntry {
                    scale: Qt.vector3d(100,100,100);
                }
            }

            materials: CustomMaterial {
                shadingMode: CustomMaterial.Shaded
                depthDrawMode: Material.OpaquePrePassDepthDraw
            }
        }
    }
}
