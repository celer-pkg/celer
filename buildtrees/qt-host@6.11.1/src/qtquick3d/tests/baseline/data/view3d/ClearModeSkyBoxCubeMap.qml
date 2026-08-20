// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

//import "../shared/"
import "."

Rectangle {
    width: 320
    height: 480
    color: "blue"

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.SkyBoxCubeMap
            skyBoxCubeMap: CubeMapTexture {
                source: "../shared/maps/clouds1_%p.jpg"
            }
        }
        PerspectiveCamera {
            id: dummyCamera
        }
    }
}
