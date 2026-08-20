// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

//! [qt-quick]
Window {
    id: root
    title: "Qt Quick"
    color: "#2CDE85"

    property alias calendarWindow: calendar.window

    property int contentsMargins: 20

    minimumWidth: calendarWindow.minimumWidth + contentsMargins * 2
    minimumHeight: calendarWindow.minimumHeight + contentsMargins * 2

    WindowContainer {
        id: calendar
        width: window.minimumWidth
        height: window.minimumHeight
        anchors.centerIn: parent
    }
}
//! [qt-quick]
