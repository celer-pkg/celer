// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

pragma ComponentBehavior: Bound

XrGadget {
    id: root
    required property int axis // x == 0, y == 1, z == 2
    readonly property var objectBounds: (controlledObject as Model)?.bounds
    materials: PrincipledMaterial {
        property color axisColor: axis === 0 ? "red" : axis === 1 ? "lime" : "deepskyblue"
        baseColor: root.selected ? axisColor : Qt.darker(axisColor)
        roughness: 0.5
    }
    eulerRotation: axis === 0 ? Qt.vector3d(0, 0, -90)
                              :  axis === 1 ? Qt.vector3d(0, 0, 0)
                                            : Qt.vector3d(90, 0, 0)

    property vector3d axisDirection: axis === 0 ? Qt.vector3d(1, 0, 0)
                                                : axis === 1 ? Qt.vector3d(0, 1, 0)
                                                             : Qt.vector3d(0, 0, 1)
}
