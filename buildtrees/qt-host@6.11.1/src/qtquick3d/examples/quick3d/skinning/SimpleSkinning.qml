// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import Example
import QtQuick.Timeline

Node {
    PrincipledMaterial {
        id: material
        lighting: PrincipledMaterial.NoLighting
        baseColor: "orange"
    }

    Model {
        geometry: SkinGeometry {
            id: geometry

            //! [positions]
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
            //! [positions]
            //! [joints]
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
            //! [joints]
            //! [weights]
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
            //! [weights]
            //! [triangles]
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
            //! [triangles]
        }
        materials: [
            material
        ]
        //! [skeleton in Model]
        skeleton: qmlskeleton
        //! [skeleton in Model]
        //! [poses]
        inverseBindPoses: [
            Qt.matrix4x4(1, 0, 0, -0.5,
                         0, 1, 0, -1,
                         0, 0, 1, 0,
                         0, 0, 0, 1),
            Qt.matrix4x4(1, 0, 0, -0.5,
                         0, 1, 0, -1,
                         0, 0, 1, 0,
                         0, 0, 0, 1)
        ]
        //! [poses]
    }
    //! [skeleton]
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
    //! [skeleton]

    //! [animation]
    Timeline {
        id: timeline0
        startFrame: 0
        endFrame: 1000
        currentFrame: 0
        enabled: true
        animations: [
            TimelineAnimation {
                duration: 5000
                from: 0
                to: 1000
                running: true
            }
        ]

        KeyframeGroup {
            target: joint1
            property: "eulerRotation.z"

            Keyframe {
                frame: 0
                value: 0
            }
            Keyframe {
                frame: 250
                value: 90
            }
            Keyframe {
                frame: 750
                value: -90
            }
            Keyframe {
                frame: 1000
                value: 0
            }
        }
    }
    //! [animation]
}
