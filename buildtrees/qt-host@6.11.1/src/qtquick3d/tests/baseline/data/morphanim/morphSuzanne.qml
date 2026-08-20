// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 500
    height: 500

    View3D {
        anchors.fill: parent

        PointLight {
            id: light
            position: Qt.vector3d(5, 5, 5)
            color: "#ffffffff"
        }

        PerspectiveCamera {
            id: camera
            z: 10
            clipNear: 0.1
            clipFar: 100
        }

        MorphTarget {
            id: morphtarget
            weight: 0
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        MorphTarget {
            id: morphtarget1
            weight: 0.33
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        MorphTarget {
            id: morphtarget2
            weight: 0.67
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        MorphTarget {
            id: morphtarget3
            weight: 1
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        Model {
            id: suzanne
            source: "../shared/models/suzanne.mesh"
            x: -3
            y: 3

            morphTargets: [
                morphtarget,
                morphtarget3
            ]

            PrincipledMaterial {
                id: _material
                metalness: 1
                roughness: 1
                alphaMode: PrincipledMaterial.Opaque
            }
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne1
            source: "../shared/models/suzanne.mesh"
            y: 3
            morphTargets: [
                morphtarget,
                morphtarget2
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne2
            source: "../shared/models/suzanne.mesh"
            x: 3
            y: 3
            morphTargets: [
                morphtarget,
                morphtarget1
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne3
            source: "../shared/models/suzanne.mesh"
            x: -3
            morphTargets: [
                morphtarget,
                morphtarget
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne4
            source: "../shared/models/suzanne.mesh"
            morphTargets: [
                morphtarget1,
                morphtarget
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne5
            source: "../shared/models/suzanne.mesh"
            x: 3
            morphTargets: [
                morphtarget2,
                morphtarget
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne6
            source: "../shared/models/suzanne.mesh"
            x: -3
            y: -3
            morphTargets: [
                morphtarget3,
                morphtarget
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne7
            source: "../shared/models/suzanne.mesh"
            y: -3
            morphTargets: [
                morphtarget3,
                morphtarget2
            ]
            materials: [
                _material
            ]
        }

        Model {
            id: suzanne8
            source: "../shared/models/suzanne.mesh"
            x: 3
            y: -3
            morphTargets: [
                morphtarget3,
                morphtarget3
            ]
            materials: [
                _material
            ]
        }
    }
}
