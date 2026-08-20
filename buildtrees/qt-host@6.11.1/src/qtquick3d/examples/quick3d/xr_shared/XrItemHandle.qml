// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Xr

XrGadget {
    id: grabHandle

    controlledObject: parent as Node

    property XrItem item: parent as XrItem

    property real objectWidth: item?.width
    property real objectHeight: item?.height

    property real margin: 10

    property real width: objectWidth + margin
    property real height: objectHeight + margin

    cursorStyle: XrGadget.CursorStyle.Flat

    objectName: "grab_handle"

    x: objectWidth/2
    y: -objectHeight/2
    z: -0.5

    source: "#Rectangle"
    scale: Qt.vector3d(width / 100, height / 100, 1)

    materials: PrincipledMaterial {
        id: grabMat
        property real ef: 0.2
        baseColor: "white"
        emissiveFactor: Qt.vector3d(ef, ef, ef)
    }
    opacity: selected ? 0.4 : 0.0
}
