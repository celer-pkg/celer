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
import QtQuick.Layouts
import Thermostat

Item {
    id: root

    required property list<Room> roomsList

    property bool isOneColumn: false
    property int currentRoomIndex

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentRoomIndex

        Repeater {
            id: repeater

            model: root.roomsList
            StatisticsScrollView {
                required property int index
                required property Room modelData

                room: modelData
                width: root.width
                height: root.height

                isOneColumn: root.isOneColumn
            }
        }
    }
}
