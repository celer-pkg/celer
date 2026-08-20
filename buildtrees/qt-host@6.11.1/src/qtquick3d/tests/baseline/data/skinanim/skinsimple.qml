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
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 3)
            clipNear: 1
            clipFar: 100
        }


        PrincipledMaterial {
            id: material
            lighting: PrincipledMaterial.NoLighting
            baseColor: "orange"
        }

        Model {
            x : 5
            y : 5
            geometry: SkinGeometry {
                id: geometry

                positions: [
                    Qt.vector3d(0.0, 0.0, 0.0), // vertex 0
                    Qt.vector3d(1.0, 0.0, 0.0), // vertex 1
                    Qt.vector3d(0.0, 0.5, 0.0), // vertex 2
                    Qt.vector3d(1.0, 0.5, 0.0), // vertex 3
                    Qt.vector3d(0.0, 1.0, 0.0), // vertex 4
                    Qt.vector3d(1.0, 1.0, 0.0), // vertex 5
                    Qt.vector3d(0.0, 1.5, 0.0), // vertex 6
                    Qt.vector3d(1.0, 1.5, 0.0), // vertex 7
                    Qt.vector3d(0.0, 2.0, 0.0), // vertex 8
                    Qt.vector3d(1.0, 2.0, 0.0)  // vertex 9
                ]
                joints: [
                    0, 1, 0, 0, // vertex 0
                    0, 1, 0, 0, // vertex 1
                    0, 1, 0, 0, // vertex 2
                    0, 1, 0, 0, // vertex 3
                    0, 1, 0, 0, // vertex 4
                    0, 1, 0, 0, // vertex 5
                    0, 1, 0, 0, // vertex 6
                    0, 1, 0, 0, // vertex 7
                    0, 1, 0, 0, // vertex 8
                    0, 1, 0, 0  // vertex 9
                ]
                weights: [
                    1.00, 0.00, 0.0, 0.0, // vertex 0
                    1.00, 0.00, 0.0, 0.0, // vertex 1
                    0.75, 0.25, 0.0, 0.0, // vertex 2
                    0.75, 0.25, 0.0, 0.0, // vertex 3
                    0.50, 0.50, 0.0, 0.0, // vertex 4
                    0.50, 0.50, 0.0, 0.0, // vertex 5
                    0.25, 0.75, 0.0, 0.0, // vertex 6
                    0.25, 0.75, 0.0, 0.0, // vertex 7
                    0.00, 1.00, 0.0, 0.0, // vertex 8
                    0.00, 1.00, 0.0, 0.0  // vertex 9
                ]
                indexes: [
                    0, 1, 3, // triangle 0
                    0, 3, 2, // triangle 1
                    2, 3, 5, // triangle 2
                    2, 5, 4, // triangle 3
                    4, 5, 7, // triangle 4
                    4, 7, 6, // triangle 5
                    6, 7, 9, // triangle 6
                    6, 9, 8  // triangle 7
                ]
            }
            materials: [
                material
            ]

            skeleton: qmlskeleton
            inverseBindPoses: [
                Qt.matrix4x4(1, 0, 0, -0.5, 0, 1, 0, -1, 0, 0, 1, 0, 0, 0, 0, 1),
                Qt.matrix4x4(1, 0, 0, -0.5, 0, 1, 0, -1, 0, 0, 1, 0, 0, 0, 0, 1)
            ]
        }
        Skeleton {
            id: qmlskeleton
            Joint {
                id: joint0
                index: 0
                skeletonRoot: qmlskeleton
                Joint {
                    id: joint1
                    index: 1
                    skeletonRoot: qmlskeleton
                    eulerRotation.z: 45
                }
            }
        }
    }
}
