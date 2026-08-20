// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: rootWindow
    width: 100
    height: 100

    property alias dialog: dialog

    property alias rootMArea: rootMouseArea
    property alias childMArea: childMouseArea
    property alias childWindow: childWindow

    MouseArea {
        id: rootMouseArea
        anchors.fill: parent
    }

    ApplicationWindow {
        id: childWindow
        width: 100
        height: 100
        MouseArea {
            id: childMouseArea
            anchors.fill: parent
        }
    }

    FolderDialog {
        id: dialog
        objectName: "FolderDialog"
    }
}
