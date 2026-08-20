// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick3D.Helpers
import QtQuick3D
import QtQuick3D.Xr

Node {
    DirectionalLight {
    }
    XrItem {
        width: 100
        height: 100
        position: Qt.vector3d(-120, 120, 5)
        contentItem: Rectangle {
            width: 200
            height: 200
            color: "red"
        }
    }
    Node {
        position: Qt.vector3d(-50, 50, 0)
        View3D {
            id: nestedView3D
            width: 100
            height: 100

            environment: SceneEnvironment {
                clearColor: "khaki"
                backgroundMode: SceneEnvironment.Color
            }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial {
                    baseColor: "goldenrod"
                }
            }
            DirectionalLight {}
            PerspectiveCamera {
                z: 100
            }
        }
    }
    XrItem {
        width: 100
        height: 100
        position: Qt.vector3d(20, -20, -5)
        contentItem: Rectangle {
            width: 200
            height: 200
            color: "green"
        }
    }
}
