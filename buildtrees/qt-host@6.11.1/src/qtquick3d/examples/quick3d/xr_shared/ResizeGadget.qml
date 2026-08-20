// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

pragma ComponentBehavior: Bound

AxisGadget {
    source: "qrc:/qt/qml/xr_shared/meshes/resize_mesh.mesh"

    property vector3d originalScale
    property real minExtent
    property real maxExtent

    property bool proportionalResize: false

    function scaleFactor(delta: real, min: real, max: real) : real {
        let d = max - min
        let newD = Math.max(0.1, d + delta)
        return newD / d
    }

    onPressed: (pos, dir) => {
        originalScale = controlledObject.scale
        const min = objectBounds.minimum.times(originalScale)
        const max = objectBounds.maximum.times(originalScale)
        switch (axis) {
            case 0:
                minExtent = min.x
                maxExtent = max.x
                break
            case 1:
                minExtent = min.y
                maxExtent = max.y
                break
            case 2:
                minExtent = min.z
                maxExtent = max.z
                break
        }
    }

    onMoved: (pos, dir) => {
        let moveDirection = delta.normalized()
        let mapped_axis = controlledObject.mapDirectionToScene(axisDirection).normalized()
        let dot = mapped_axis.dotProduct(moveDirection)
        let offset = delta.length() * dot
        let sf = scaleFactor(offset, minExtent, maxExtent)
        let s = originalScale.times(1) // deep copy
        if (proportionalResize) {
            s = s.times(sf)
        } else {
            switch (axis) {
                case 0:
                    s.x *= sf
                    break
                case 1:
                    s.y *= sf
                    break
                case 2:
                    s.z *= sf
                    break
            }
        }
        controlledObject.scale = s
    }
}
