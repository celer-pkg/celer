// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

ToolTip {
    id: tooltip

    property real first: 157.2
    property real second: 300
    property real third: 5.9
    property string firstname: "Power (kW)"
    property string secondname: "Cap. (kW)"
    property string thirdname: "Wind (m/s)"
    property string firsticon: "../images/lightning_24px.png"
    property string secondicon: "../images/meter_24px.png"
    property string thirdicon: "../images/wind_24px.png"

    contentItem: Item {
        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true
                Image {
                    source: tooltip.firsticon
                }
                Label {
                    Layout.fillWidth: true
                    color: "#595959"
                    text: tooltip.firstname
                    font.pixelSize: GlobalSettings.fontsize20px
                }
                Label {
                    color: "#595959"
                    text: tooltip.first
                    font.pixelSize: GlobalSettings.fontsize20px
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Image {
                    source: tooltip.secondicon
                }
                Label {
                    Layout.fillWidth: true
                    color: "#595959"
                    text: tooltip.secondname
                    font.pixelSize: GlobalSettings.fontsize20px
                }
                Label {
                    color: "#595959"
                    text: tooltip.second
                    font.pixelSize: GlobalSettings.fontsize20px
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Image {
                    source: tooltip.thirdicon
                }
                Label {
                    Layout.fillWidth: true
                    color: "#595959"
                    text: tooltip.thirdname
                    font.pixelSize: GlobalSettings.fontsize20px
                }
                Label {
                    color: "#595959"
                    text: tooltip.third
                    font.pixelSize: GlobalSettings.fontsize20px
                }
            }
        }
    }
    background: Rectangle {
        color: "#F2F2F2"
        radius: GlobalSettings.quarterradius
    }
}
