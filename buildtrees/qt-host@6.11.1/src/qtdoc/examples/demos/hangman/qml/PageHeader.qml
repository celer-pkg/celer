// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: header

    property alias title: titleText.text
    property int buttonHeight: topLevel.buttonHeight
    signal clicked()


    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: buttonHeight + (topLevel.globalMargin * 2)
    SimpleButton {
        id: backButton
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: topLevel.globalMargin
        height: buttonHeight
        width: parent.width * 0.25
        text: "Back"
        onClicked: {
            pageStack.pop();
            header.clicked();
        }
    }
    Text {
        id: titleText
        anchors.left: backButton.right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: topLevel.globalMargin
        font.family: "Helvetica"
        color: "white"
        font.pointSize: 64
        fontSizeMode: Text.Fit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        minimumPointSize: 8
    }
}
