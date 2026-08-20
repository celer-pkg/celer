// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import "about_effect"

CustomPopup {
    id: rootItem
    modal: true
    width: 400
    height: 260
    padding: 1

    AboutEffect1 {
        anchors.fill: parent
        timeRunning: rootItem.visible
        opacity: 0.5
    }

    Item {
        anchors.fill: parent
        anchors.margins: 10
        Column {
            width: parent.width
            spacing: 10
            Label {
                text: qsTr("Qt Quick Effect Maker")
                font.bold: true
                font.pixelSize: 22
                color: mainView.foregroundColor2
            }
            Label {
                text: qsTr("VERSION %1 (Built with Qt %2)").arg(Qt.application.version).arg(buildQtVersion);
                width: parent.width
                font.pixelSize: 18
                color: mainView.foregroundColor2
            }
        }
        Column {
            anchors.bottom: parent.bottom
            width: parent.width
            spacing: 10
            Label {
                text: qsTr("Copyright (C) The Qt Company Ltd and other contributors.")
                width: parent.width
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: 14
                color: mainView.foregroundColor2
            }

        }
    }
}
