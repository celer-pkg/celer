// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                // the pre-baked ktx file included in scenegrabber should give identical results to
                //source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
                source: "qrc:/OpenfootageNET_lowerAustria01-1024.ktx"
                mappingMode: Texture.LightProbe
            }
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600)
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(5, 5, 5)
            materials: PrincipledMaterial {
                metalness: 0.5
            }
        }
    }
}
