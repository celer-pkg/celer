// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Window {
    width: 320
    height: 480
    color: "white"
    visible: true
    title: qsTr("Threaded fetch more example")
    Rectangle {
        color: "black"
        width: scrollbar.width
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
    ListView {
        id: listView
        anchors.fill: parent
        model: ThreadedFetchMoreModel {}
        delegate: ContactBookDelegate {
            required property var model
            width: listView.width - scrollbar.width
        }
        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            policy: ScrollBar.AlwaysOn
        }
    }
}
