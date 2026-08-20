// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: legend
    width: 359
    height: 22
    color: "transparent"

    property alias high: highCheckbox
    property alias low: lowCheckbox
    property alias open: openCheckbox
    property alias close: closeCheckbox

    RowLayout {
        anchors.fill: parent
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        spacing: 8

        StockCheckbox {
            id: highCheckbox
            cbText: "high"
            activeColor: "#33279646"
            activeTextColor: "#279646"
            width: 43
            height: 22
            Layout.preferredHeight: 22
            Layout.preferredWidth: 43
            checked: true
        }
        StockCheckbox {
            id: lowCheckbox
            cbText: "low"
            activeColor: "#33cc5e87"
            activeTextColor: "#CC5E87"
            width: 43
            height: 22
            Layout.preferredHeight: 22
            Layout.preferredWidth: 43
            checked: true
        }
        StockCheckbox {
            id: openCheckbox
            cbText: "open"
            activeColor: "#33cca35e"
            activeTextColor: "#CCA35E"
            width: 43
            height: 22
            Layout.preferredHeight: 22
            Layout.preferredWidth: 43
        }
        StockCheckbox {
            id: closeCheckbox
            cbText: "close"
            activeColor: "#325e87cc"
            activeTextColor: "#5E87CC"
            width: 43
            height: 22
            Layout.preferredHeight: 22
            Layout.preferredWidth: 43
        }

        Item {
            width: 43
            height: 22
            Layout.fillWidth: true
            Layout.preferredHeight: 22
            Layout.preferredWidth: 43
        }
    }
}
