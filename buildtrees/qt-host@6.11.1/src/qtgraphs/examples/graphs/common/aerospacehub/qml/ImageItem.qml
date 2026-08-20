// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: maskitem

    // Bluish alpha rectangle for masking a part of the background
    Rectangle {
        anchors.fill: parent
        radius: GlobalSettings.defaultradius
        color: "#5500A1FF"
    }

    // Border around the alpha rectangle
    Rectangle {
        anchors.fill: parent
        radius: GlobalSettings.defaultradius
        border.color: "#EFEFEF"
        border.width: 1
        color: "transparent"
    }
}
