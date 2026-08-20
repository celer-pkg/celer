// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: door_button
    width: 48
    height: 48
    color: "transparent"
    radius: 8
    property alias downloadBase: icons.downloadBase

    Icons {
        id: icons
        width: 36
        height: 36
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 6
        anchors.topMargin: 6
        state: "state_Active_Name_Door"
        clip: true
    }
}
