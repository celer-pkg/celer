// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property int currentStep: 0
    property alias model: repeater.model

    function __stepCircleSize(portrait) {
        return ApplicationConfig.responsiveSize(portrait ? 64 : 84)
    }

    function __stepIndicatorSize(portrait) {
        return ApplicationConfig.responsiveSize(portrait ? 25 : 26)
    }

    function __stepItemSize() {
        return stepsRowItem.width / repeater.count
    }

    implicitHeight: stepsRowItem.implicitHeight
    implicitWidth: ApplicationConfig.responsiveSize(2288)

    Item {
        id: bar
        implicitHeight: ApplicationConfig.responsiveSize(16)
        anchors {
            left: stepsRowItem.left
            right: stepsRowItem.right
            leftMargin: root.__stepItemSize() / 2
            rightMargin: root.__stepItemSize() / 2
        }

        Rectangle {
            color: "#2269EE"
            anchors {
                fill: parent
                leftMargin: root.__stepCircleSize(ApplicationConfig.isPortrait) / 2
                rightMargin: root.__stepCircleSize(ApplicationConfig.isPortrait) / 2
            }
        }
    }

    Row {
        id: stepsRowItem
        spacing: 0
        anchors {
            top: bar.top
            left: parent.left
            right: parent.right
            topMargin: (bar.height - root.__stepCircleSize(ApplicationConfig.isPortrait)) / 2
        }
        Repeater {
            id: repeater
            ColumnLayout {
                id: stepItem
                required property int index
                required property int step
                required property string text
                readonly property bool current: step === root.currentStep
                width: root.__stepItemSize()
                Rectangle {
                    id: stepCircle
                    implicitHeight: root.__stepCircleSize(ApplicationConfig.isPortrait)
                    implicitWidth: root.__stepCircleSize(ApplicationConfig.isPortrait)
                    radius: height / 2
                    color: "#2269EE"
                    Rectangle {
                        color: "white"
                        implicitHeight: root.__stepIndicatorSize(ApplicationConfig.isPortrait)
                        implicitWidth: root.__stepIndicatorSize(ApplicationConfig.isPortrait)
                        radius: height / 2
                        visible: stepItem.current
                        anchors.centerIn: parent
                    }
                    Layout.alignment: Qt.AlignHCenter
                }
                ToyLabel {
                    id: stepLabel
                    text: stepItem.text
                    textStyle: stepItem.current ? ApplicationConfig.TextStyle.Body_Bold
                                                : ApplicationConfig.TextStyle.Caption
                    font.pixelSize: ApplicationConfig.responsiveFontSize(72)
                    color: stepItem.current ? "#2269EE" : "#162655"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
