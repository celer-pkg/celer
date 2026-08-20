// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import AerospaceHubExample

Rectangle {
    id: statusitem

    radius: GlobalSettings.defaultradius

    color: "#80181818"

    Column {
        anchors.fill: parent
        anchors.margins: GlobalSettings.doublespacing
        spacing: GlobalSettings.defaultspacing

        // Status
        Row {
            spacing: GlobalSettings.defaultspacing
            anchors.left: parent.left

            Label {
                color: "#EFEFEF"
                font.pixelSize: GlobalSettings.fontsize24px
                text: "Status"
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                color: "#1F52F1"
                radius: GlobalSettings.quarterradius
                width: statustext.implicitWidth * 1.5
                height: statustext.implicitHeight * 1.75

                Label {
                    id: statustext
                    anchors.centerIn: parent
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize20px
                    font.bold: true
                    text: "Active"
                }
            }
        }

        Row {
            spacing: GlobalSettings.defaultspacing

            Column {
                spacing: GlobalSettings.statusspacing

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Rotor"
                    font.bold: true
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Nacelle"
                    font.bold: true
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Converter"
                    font.bold: true
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Wind Speed"
                    font.bold: true
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Wind Dir."
                    font.bold: true
                }

            }

            Column {
                spacing: GlobalSettings.statusspacing

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Pitch Control"
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "On wind"
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: "Running"
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: GlobalSettings.windspeed.toFixed(1) + "m/s"
                }

                Label {
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize24px
                    text: GlobalSettings.winddir + "°"
                }
            }
        }
    }
}
