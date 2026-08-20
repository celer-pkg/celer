// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

pragma ComponentBehavior: Bound

ColumnLayout {
    id: root

    required property var base
    required property real maxMessageBoxWidth
    readonly property real margin: 7.5

    Rectangle {
        implicitWidth: Math.min(childrenRect.width, root.maxMessageBoxWidth)
        implicitHeight: childrenRect.height

        color: root.base.darkColor
        radius: 10
        clip: true

        RowLayout {
            Image {
                id: image
                Layout.margins: root.margin
                source: "../res/generic_file.svg"
                sourceSize.width: 30
                sourceSize.height: 30

                smooth: true
                antialiasing: true
                asynchronous: true
            }
            Text {
                Layout.margins: root.margin
                Layout.leftMargin: 0
                Layout.fillWidth: true
                Layout.maximumWidth: root.maxMessageBoxWidth - image.width - 4 * root.margin
                horizontalAlignment: Text.AlignLeft
                text: root.base.display.file.name
                color: hover.hovered? Qt.lighter(root.base.lightColor, 1.5) : root.base.lightColor
                font.pointSize: 14
                wrapMode: TextEdit.WrapAnywhere
            }
            clip: true
        }
        TapHandler { onTapped: ChatEngine.openUrl(root.base.display.file.content) }
        HoverHandler { id: hover }
    }
}
