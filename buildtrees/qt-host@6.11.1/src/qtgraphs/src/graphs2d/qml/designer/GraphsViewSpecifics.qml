// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import HelperWidgets
import QtQuick.Layouts

Column {
    anchors.left: parent.left
    anchors.right: parent.right

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Background Color")

        ColorEditor {
            caption: qsTr("Background Color")
            backendValue: backendValues.backgroundColor
            supportGradient: false
        }
    }

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Margins")

        SectionLayout {
            rows: 4
            Label {
                text: qsTr("Top")
                tooltip: qsTr("The amount of empty space on the top of the graph.")
                Layout.fillWidth: true
            }

            SecondColumnLayout {
                SpinBox {
                    backendValue: backendValues.marginTop
                    minimumValue: 0.0
                    maximumValue: 9999.0
                    stepSize: 1.0
                    decimals: 1
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Bottom")
                tooltip: qsTr("The amount of empty space on the bottom of the graph.")
                Layout.fillWidth: true
            }

            SecondColumnLayout {
                SpinBox {
                    backendValue: backendValues.marginBottom
                    minimumValue: 0.0
                    maximumValue: 9999.0
                    stepSize: 1.0
                    decimals: 1
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Left")
                tooltip: qsTr("The amount of empty space on the left of the graph.")
                Layout.fillWidth: true
            }

            SecondColumnLayout {
                SpinBox {
                    backendValue: backendValues.marginLeft
                    minimumValue: 0.0
                    maximumValue: 9999.0
                    stepSize: 1.0
                    decimals: 1
                    Layout.fillWidth: true
                }
            }

            Label {
                text: qsTr("Right")
                tooltip: qsTr("The amount of empty space on the right of the graph.")
                Layout.fillWidth: true
            }

            SecondColumnLayout {
                SpinBox {
                    backendValue: backendValues.marginRight
                    minimumValue: 0.0
                    maximumValue: 9999.0
                    stepSize: 1.0
                    decimals: 1
                    Layout.fillWidth: true
                }
            }
        }
    }
}
