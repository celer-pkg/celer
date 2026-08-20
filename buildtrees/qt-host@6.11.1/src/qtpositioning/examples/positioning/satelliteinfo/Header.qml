// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool simulation: false
    property string statusString
    property bool redStatus: false

    color: Theme.backgroundColor

    implicitWidth: Math.max(applicationName.implicitWidth,
                            applicationMode.implicitWidth,
                            applicationStatus.implicitWidth)
                   + rootLayout.anchors.leftMargin
                   + rootLayout.anchors.rightMargin
    implicitHeight: applicationName.implicitHeight
                    + applicationMode.implicitHeight
                    + applicationStatus.implicitHeight
                    + 2 * rootLayout.spacing
                    + rootLayout.anchors.topMargin
                    + rootLayout.anchors.bottomMargin

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        spacing: 0


        Text {
            id: applicationName
            text: qsTr("Satellite Info")
            color: Theme.textMainColor
            font.pixelSize: Theme.largeFontSize
            font.weight: Theme.fontDefaultWeight
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            id: applicationMode
            text: root.simulation ? qsTr("Simulation mode") : qsTr("Realtime mode")
            color: Theme.textGrayColor
            font.pixelSize: Theme.smallFontSize
            font.weight: Theme.fontDefaultWeight
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            id: applicationStatus
            text: root.statusString
            color: root.redStatus ? Theme.redColor : Theme.greenColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontDefaultWeight
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
