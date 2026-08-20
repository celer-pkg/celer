// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: mainPage

    property alias isCurrent: showcaseView.isAnimationRunning

    signal startRequested()

    ColumnLayout {
        spacing: ApplicationConfig.responsiveSize(60)
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: ApplicationConfig.responsiveSize(80)
        }

        ToyImage {
            source: "images/appLogo.svg"
            sourceSize {
                height: ApplicationConfig.responsiveSize(460)
                width: ApplicationConfig.responsiveSize(1000)
            }
            Layout.alignment: Qt.AlignHCenter
        }
        ToyImage {
            source: "images/builtWithQt.svg"
            sourceSize {
                height: ApplicationConfig.responsiveSize(120)
                width: ApplicationConfig.responsiveSize(384)
            }
            Layout.alignment: Qt.AlignHCenter
        }
    }

    ColumnLayout {
        width: ApplicationConfig.responsiveSize(1760)

        anchors {
            fill: parent
            bottomMargin: ApplicationConfig.responsiveSize(160)
            topMargin: ApplicationConfig.responsiveSize(420)
        }

        ShowcaseView {
            id: showcaseView
            implicitWidth:  ApplicationConfig.responsiveSize(2089)
            implicitHeight: ApplicationConfig.responsiveSize(1980)
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ToyButton {
            implicitWidth: ApplicationConfig.responsiveSize(881)
            implicitHeight: ApplicationConfig.responsiveSize(288)
            textStyle: ApplicationConfig.TextStyle.H2_Bold
            text: qsTr("Tap to Start")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            onClicked: mainPage.startRequested()
        }
    }

}
