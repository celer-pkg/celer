// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: root
    width: 1920
    height: 1080

    property vector3d pos: Qt.vector3d(0, 0, 0)
    property vector3d rot: Qt.vector3d(0, 0, 0)

    // TODO: maybe a dead code?
    function resetPosition(pos) {
        root.pos = pos
    }

    function resetRotation(rot) {
        root.rot = rot
    }
}
