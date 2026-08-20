// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

Rectangle {
    id: verticaltoolbar
    color: "#181818"
    radius: GlobalSettings.defaultradius / 2

    // Grouping for vertical toolbar buttons
    ButtonGroup {
        buttons: viewbuttons.children
    }

    ColumnLayout {
        id: viewbuttons
        spacing: GlobalSettings.defaultspacing
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: GlobalSettings.defaultspacing * 1.5
        anchors.leftMargin: GlobalSettings.defaultspacing
        anchors.rightMargin: GlobalSettings.defaultspacing

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: width
            icon.source: "../images/dashboard-icon.png"
            icon.color: checked ? "#181818" : "#F2F2F2"
            autoExclusive: true
            checkable: true
            checked: root.state !== "ShowSettings"
            enabled: root.state === "ShowSettings"
            background: Rectangle {
                anchors.centerIn: parent
                color: parent.checked ? "#F2F2F2" : "#181818"
                radius: width / 9
                width: verticaltoolbar.width * 0.7
                height: width
            }
            onClicked: {
                root.state = "ShowDashboard"
            }
        }

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: width
            icon.source: "../images/settings-icon.png"
            icon.color: checked ? "#181818" : "#F2F2F2"
            autoExclusive: true
            checkable: true
            checked: root.state === "ShowSettings"
            background: Rectangle {
                anchors.centerIn: parent
                color: parent.checked ? "#F2F2F2" : "#181818"
                radius: width / 9
                width: verticaltoolbar.width * 0.7
                height: width
            }
            onClicked: root.state = "ShowSettings"
        }

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: width
            icon.source: "../images/split-icon.png"
            icon.color: "gray"
            autoExclusive: true
            checkable: true
            enabled: false
            background: Rectangle {
                anchors.centerIn: parent
                color: parent.checked ? "#EFEFEF" : "#181818"
                radius: width / 9
                width: verticaltoolbar.width * 0.7
                height: width
            }
        }
    }
}
