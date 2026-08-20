// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import "../shared"
import "../shared/materials"

Rectangle {
    width: 400
    height: 400
    color: "lightgrey"

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
            probeExposure: 3
            lightProbe: Texture {
                source: "../shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
            }
        }

        DirectionalLight {

        }

        PerspectiveCamera {
            z: 150
        }

        Model {
            source: "#Cube"
            eulerRotation.y: 25
            materials: [GoldLattice {}]
        }
    }
}
