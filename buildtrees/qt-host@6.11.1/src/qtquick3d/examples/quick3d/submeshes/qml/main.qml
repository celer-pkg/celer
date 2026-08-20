// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 640
    height: 640
    visible: true
    title: "Sub-mesh example"

    View3D {
        id: view
        anchors.fill: parent
        camera: camera
        visible: true

        PerspectiveCamera {
            id: camera
            z: 20
        }

        DistortedCube {
            x: 4
            scale: Qt.vector3d(2, 2, 2)

            SequentialAnimation on eulerRotation {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 0, 360)
                }
            }
        }

        //! [DistortedCube Left]
        DistortedCube {
            x: -4
            scale: Qt.vector3d(2, 2, 2)
            materials: [ PrincipledMaterial {
                baseColor: "red"
                lighting: PrincipledMaterial.NoLighting
            },
            PrincipledMaterial {
                            baseColor: "green"
                            lighting: PrincipledMaterial.NoLighting
                        } ]
            //! [DistortedCube Left]
            SequentialAnimation on eulerRotation {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 0, 360)
                }
            }
        }
    }
}
