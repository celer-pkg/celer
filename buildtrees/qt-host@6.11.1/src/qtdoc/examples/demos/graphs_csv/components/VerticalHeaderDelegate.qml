// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import qtgraphscsv

Rectangle {
    id: verticalViewDelegate

    required property string display
    property color textColor
    property color borderColor

    color: "transparent"
    implicitHeight: 31 * Units.px
    implicitWidth: 91 * Units.px
    border.color: borderColor
    border.width: 1 * Units.px

    Text {
        id: txv
        topPadding: 5 * Units.px
        rightPadding: 25 * Units.px
        leftPadding: 8 * Units.px
        bottomPadding: 6 * Units.px
        color: verticalViewDelegate.textColor
        text: verticalViewDelegate.display

        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
    }
}
