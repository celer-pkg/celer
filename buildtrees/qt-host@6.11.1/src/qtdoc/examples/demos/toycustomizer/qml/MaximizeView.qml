// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: maximizeView

    property alias toyIndex: bigToy.index
    property alias accessoryModel: bigToy.accessoryModel

    signal hideRequested

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: 40
            bottomMargin: 80
            leftMargin: ApplicationConfig.responsiveSize(-200)
            rightMargin: ApplicationConfig.responsiveSize(-200)
        }
        spacing: 30

        ToyLabel {
            Layout.alignment: Qt.AlignHCenter
            color: "#2269EE"
            text: qsTr("Full screen view")
            font {
                bold: true
                family: "DynaPuff"
                pixelSize: ApplicationConfig.responsiveSize(112)
            }
        }

        CurrentToyModel {
            id: bigToy
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
            isMaximized: true
        }

        ToyButton {
            id: backButton
            type: ToyButton.Type.Secondary
            textStyle: ApplicationConfig.TextStyle.Button_L
            text: qsTr("Back")
            icon.source: "icons/back.svg"

            Layout.alignment: Qt.AlignHCenter
            onClicked: maximizeView.hideRequested()
        }
    }
}
