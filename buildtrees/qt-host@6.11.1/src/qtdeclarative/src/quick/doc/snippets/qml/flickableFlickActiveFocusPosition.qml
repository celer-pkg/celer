// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![0]
import QtQuick
import QtQuick.Controls
import QtQuick.Window

Flickable {
    id: flickable

    width: 200
    height: 200
    contentWidth: width
    contentHeight: column.height

    // Will flick to the beginning of the activeFocusItem every time it changes
    property Item activeFocusItem: Window.activeFocusItem
    onActiveFocusItemChanged: flickable.flickToChild(activeFocusItem, Flickable.AlignTop)

    Column {
        id: column

        spacing: 10

        Repeater {
            model: 10
            TextArea {}
        }
    }
}
//![0]
