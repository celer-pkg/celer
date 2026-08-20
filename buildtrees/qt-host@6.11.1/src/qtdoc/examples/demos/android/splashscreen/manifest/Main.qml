// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls

Window {
    visible: true
    title: qsTr("Qt Splash Screen Example")
    color: "#2CDE85"

    Text {
        id: textLabel
        text: "First View"
        anchors.centerIn: parent
    }
    Button {
        id: closeButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: textLabel.bottom
        anchors.topMargin: 5
        background: Rectangle {
            color: "#00414A"
            radius: 2
        }
        text: "Close"
        contentItem: Text {
            color: "#FFFFFF"
            text: closeButton.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        onClicked: {
            Qt.callLater(Qt.quit)
        }
    }
}
