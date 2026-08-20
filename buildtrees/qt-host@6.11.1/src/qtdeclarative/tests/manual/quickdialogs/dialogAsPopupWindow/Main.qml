// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Window {
    width: 300
    height: 200
    visible: true
    title: "Window"

    Dialog {
        visible: true
        popupType: Popup.Window
        focus: true
        title: "Dialog"

        Label {
            anchors.fill: parent
            text: "Move and resize me!"
        }
    }
    Label {
        anchors.fill: parent
        text: "Move and resize the child dialog using mouse."
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}
