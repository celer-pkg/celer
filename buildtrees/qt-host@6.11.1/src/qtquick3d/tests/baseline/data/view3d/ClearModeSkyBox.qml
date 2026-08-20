// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

import "../shared/"

Rectangle {
    width: 320
    height: 480
    color: "blue"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "../shared/maps/TestEnvironment-512.hdr"
                mappingMode: Texture.LightProbe
            }
        }
        PerspectiveCamera {
            id: dummyCamera
        }
    }
}
