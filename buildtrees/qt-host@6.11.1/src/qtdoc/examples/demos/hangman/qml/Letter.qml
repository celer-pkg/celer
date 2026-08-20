// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    property alias text: label.text

    Text {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        id: label
        color: "white"
        font.family: "Helvetica"
        font.pixelSize: parent.height * 0.75

        opacity: applicationData.lettersOwned.indexOf(text) >= 0 ? 1.0 : 0.0
        visible: opacity > 0.0

        anchors.horizontalCenterOffset: visible ? 0 : -topLevel.width / 2

        Behavior on anchors.horizontalCenterOffset {
            NumberAnimation {
                duration: 500
                easing.type: Easing.OutQuad
            }
        }
    }

    Rectangle {
        color: "white"
        width: parent.width
        anchors.bottom: parent.bottom
        anchors.top: label.bottom
    }
}
