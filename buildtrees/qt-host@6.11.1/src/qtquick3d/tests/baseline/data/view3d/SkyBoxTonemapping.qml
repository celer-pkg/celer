// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D

import QtQuick3D.Helpers
Rectangle {
    width: 640
    height: 480
    color: "blue"

    CubeMapTexture {
        id: sRgbCubeMap
        source: "../shared/maps/clouds1_%p.jpg"
    }
    CubeMapTexture {
        id: linearCubeMap
        source: "../shared/maps/fishpond_bc1.ktx"
    }
    GridLayout {
        anchors.fill: parent
        columns: 2
        View3D {
            Layout.fillWidth : true
            Layout.fillHeight : true

            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBoxCubeMap
                skyBoxCubeMap: sRgbCubeMap
            }
            PerspectiveCamera {
            }
        }
        View3D {
            Layout.fillWidth : true
            Layout.fillHeight : true

            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBoxCubeMap
                skyBoxCubeMap: sRgbCubeMap
            }
            PerspectiveCamera {
            }
        }
        View3D {
            Layout.fillWidth : true
            Layout.fillHeight : true

            environment: ExtendedSceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBoxCubeMap
                skyBoxCubeMap: linearCubeMap
            }
            PerspectiveCamera {
            }
        }
        View3D {
            Layout.fillWidth : true
            Layout.fillHeight : true

            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.SkyBoxCubeMap
                skyBoxCubeMap: linearCubeMap
            }
            PerspectiveCamera {
            }
        }
    }
}
