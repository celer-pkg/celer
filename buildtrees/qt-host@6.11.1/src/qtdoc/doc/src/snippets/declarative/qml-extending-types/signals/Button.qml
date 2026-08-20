// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick

//![0]
// Button.qml
Rectangle {
    id: rect
    width: 100; height: 100

    signal buttonClicked(int xPos, int yPos)

    MouseArea {
        anchors.fill: parent
        onClicked: rect.buttonClicked(mouse.x, mouse.y)
    }
}
//![0]
