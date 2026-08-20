// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.Button {
    id: control

    opacity: control.enabled ? 1: 0.3

    contentItem: Text {
        text: control.text
        font.pixelSize: 16
        color: Constants.secondaryColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        color: "#4caf50"
        radius: 4
    }
}
