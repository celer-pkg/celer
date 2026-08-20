// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Item {
    id: root
    required property Node origin
    required property Camera camera

    property real speed: 0.1
    property vector2d __lastPos: Qt.vector2d(0, 0)

    DragHandler {
        id: dragHandler
        target: null
        enabled: root.enabled
        acceptedModifiers: Qt.NoModifier

        onActiveChanged: {
            if (active) {
                root.__lastPos = Qt.vector2d(centroid.position.x, centroid.position.y)
                root.forceActiveFocus()
            }
        }

        onCentroidChanged: {
            if (!active)
                return
            var rotationVector = root.origin.eulerRotation
            var delta = Qt.vector2d(root.__lastPos.x - centroid.position.x,
                                    root.__lastPos.y - centroid.position.y)
            // rotate x
            var rotateX = delta.x * root.speed * 1.5
            rotationVector.y -= rotateX

            root.origin.setEulerRotation(rotationVector)
            root.__lastPos = centroid.position
        }
    }
}
