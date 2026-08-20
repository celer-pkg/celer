// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property int currentIndex: 0
    readonly property int tableViewIndex: 1
    readonly property int settingsIndex: 3

    implicitHeight: rootLayout.implicitHeight + 2 * rootLayout.anchors.margins
    color: Theme.backgroundColor

    RowLayout {
        id: rootLayout
        anchors {
            fill: parent
            margins: Theme.defaultSpacing
        }
        spacing: 0
        property real itemWidth: rootLayout.width / repeater.model.length
        Repeater {
            id: repeater
            model: [
                {"name" : qsTr("Sky View"), "source": "icons/skyview.svg"},
                {"name" : qsTr("Table View"), "source": "icons/tableview.svg"},
                {"name" : qsTr("RSSI View"), "source": "icons/rssiview.svg"},
                {"name" : qsTr("Settings"), "source": "icons/settings.svg"},
            ]
            PageButton {
                required property var modelData
                required property int index
                implicitWidth: rootLayout.itemWidth
                text: modelData.name
                source: modelData.source
                selected: root.currentIndex === index
                onClicked: root.currentIndex = index
            }
        }
    }
}
