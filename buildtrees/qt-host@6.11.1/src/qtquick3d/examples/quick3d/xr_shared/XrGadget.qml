// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

pragma ComponentBehavior: Bound

Model {
    pickable: parent?.visible

    property vector3d pressPos
    property vector3d originalPos

    property bool grabbable: true

    property vector3d delta

    property Node controlledObject
    property bool selected: false

    enum CursorStyle {
        Hidden,
        Flat,
        Sphere
    }

    property int cursorStyle: XrGadget.CursorStyle.Hidden

    signal pressed(pos: vector3d, dir: vector3d)
    signal moved(delta: vector3d, dir: vector3d)
    signal touched(uvPosition: vector2d, touchID: int, pressed: bool)

    function handleControllerPress(pos: vector3d, direction: vector3d) {
        pressPos = pos
        originalPos = controlledObject.position
        pressed(pressPos, direction)
    }

    function handleControllerMove(pos: vector3d, direction: vector3d) {
        delta = pos.minus(pressPos)
        moved(pos, direction)
    }

    function handleTouch(uvPosition: vector2d, touchID: int, pressed: bool) {
        touched(uvPosition, touchID, pressed)
    }
}
