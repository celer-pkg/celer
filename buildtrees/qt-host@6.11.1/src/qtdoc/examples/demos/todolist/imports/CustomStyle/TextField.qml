// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Templates as T
import ToDoList

T.TextField {
    id: control

    implicitWidth: 300
    implicitHeight: 56
    leftPadding: 16
    rightPadding: 16
    topPadding: 0
    bottomPadding: 0
    font.pixelSize: 16
    verticalAlignment: Text.AlignVCenter
    placeholderTextColor: Constants.secondaryColor

    color: Constants.secondaryColor
    opacity: control.hovered || control.text ? 1 : 0.4

    background: Rectangle {
        color: Constants.mainColor
        border.color: control.text ? "#41CD52" : Constants.secondaryColor
        radius: 6
    }

    Text {
        id: placeholder

        x: control.leftPadding
        anchors.verticalCenter: control.verticalCenter
        font: control.font
        text: control.placeholderText
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText
                 && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }
}
