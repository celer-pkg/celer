// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: rootItem

    Rectangle {
        anchors.fill: parent
        color: mainView.backgroundColor1
    }

    Flickable {
        id: helpFlickable
        anchors.fill: parent
        contentWidth: helpTextEdit.width
        contentHeight: helpTextEdit.height
        clip: true
        TextEdit {
            id: helpTextEdit
            padding: 20
            width: helpFlickable.width
            wrapMode: TextEdit.WordWrap
            color: mainView.foregroundColor2
            textFormat: TextEdit.RichText
            readOnly: true
            selectByMouse: true
            text: effectManager.getHelpTextString();
            font.pixelSize: 14
            onLinkActivated: (link)=> Qt.openUrlExternally(link)
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }
}
