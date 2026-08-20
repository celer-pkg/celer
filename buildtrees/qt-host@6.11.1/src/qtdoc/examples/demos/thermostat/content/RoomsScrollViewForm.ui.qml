// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause


/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Thermostat

ScrollView {
    id: scrollView

    clip: true
    bottomPadding: 10
    contentWidth: availableWidth

    required property list<Room> roomsList

    property alias columns: gridLayout.columns
    property alias gridWidth: gridLayout.width
    property alias gridHeight: gridLayout.height

    required property int delegatePreferredHeight
    required property int delegatePreferredWidth

    GridLayout {
        id: gridLayout

        columnSpacing: 24
        rowSpacing: 25

        Repeater {
            id: repeater

            model: scrollView.roomsList
            RoomItem {
                id: roomItem

                required property Room modelData
                room: modelData
                Layout.preferredHeight: scrollView.delegatePreferredHeight
                Layout.preferredWidth: scrollView.delegatePreferredWidth
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}
