// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import qtgraphscsv

Rectangle {
    id: horizontalHeaderViewDelegate

    property color textColor
    property color borderColor
    required property string display

    readonly property font horizontalTitleFont: ({
            family: "Inter",
            weight: 600 * Units.px,
            pixelSize: 12 * Units.px,
            letterSpacing: 0 * Units.px,
            bold: false
        })

    color: "transparent"
    implicitHeight: 31 * Units.px
    implicitWidth: 63 * Units.px
    Text {
        id: txv

        anchors.right: horizontalHeaderViewDelegate.right
        anchors.rightMargin: 2 * Units.px
        anchors.bottom: horizontalHeaderViewDelegate.bottom
        anchors.bottomMargin: 10 * Units.px
        color: horizontalHeaderViewDelegate.textColor
        text: horizontalHeaderViewDelegate.display
        font: horizontalHeaderViewDelegate.horizontalTitleFont

        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
    }
}
