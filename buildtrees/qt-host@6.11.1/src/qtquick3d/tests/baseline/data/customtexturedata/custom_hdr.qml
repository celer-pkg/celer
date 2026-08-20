// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Lancelot

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: DynamicTextureData {
                    Component.onCompleted: {
                        generateRGBA32FTexture();
                    }
                }
            }
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 150)
            clipNear: 10
            clipFar: 300
        }

        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                metalness: 1.0
                roughness: 0.0
            }
        }
    }
}
