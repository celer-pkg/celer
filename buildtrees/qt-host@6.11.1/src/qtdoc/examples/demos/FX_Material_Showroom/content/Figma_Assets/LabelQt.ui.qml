// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: labelQt
    width: 248
    height: 80
    property alias built_with_QtText: built_with_Qt.text

    Rectangle {
        id: bg
        x: 0
        y: 0
        width: 248
        height: 80
        color: "#222222"
        radius: 8
    }

    Text {
        id: built_with_Qt
        x: 28
        y: 26
        width: 116
        height: 28
        color: "#ffffff"
        text: qsTr("Built with Qt")
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        font.family: "Archivo"
        font.weight: Font.Bold
    }

    Rectangle {
        id: qt_logo
        y: 21
        width: 51
        height: 38
        color: "transparent"
        anchors.right: parent.right
        anchors.rightMargin: 27
        clip: true
        Image {
            source: "assets/qtlogo.png"
        }
    }
}

/*##^##
Designer {
    D{i:0;uuid:"314987a3-c4c0-5399-9285-490b3fa1727f"}D{i:1;uuid:"4d65dfb8-1b3e-5585-a24e-a7c2d555dac2"}
D{i:2;uuid:"fbc07ec5-79f9-5b39-8692-73c80f372942"}D{i:4;uuid:"3e4a2a64-9faf-5c69-bbb0-07bd939efcf8"}
D{i:3;uuid:"864d959f-75d4-5ada-80c0-76c16bbf5fff"}
}
##^##*/

