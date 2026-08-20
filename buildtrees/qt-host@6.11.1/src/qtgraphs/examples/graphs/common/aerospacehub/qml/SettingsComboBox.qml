// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal

ComboBox {
    id: combobox

    width: GlobalSettings.settingscomboboxwidth

    background: Rectangle {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: GlobalSettings.settingscomboboxheight
        radius: height / 10
        color: parent.hovered || parent.popup.visible ? "#80181818" : "transparent"
        border.width: 1
        border.color: "#EFEFEF"
    }

    contentItem: Text {
        text: combobox.displayText
        color: "#EFEFEF"
        font.pixelSize: GlobalSettings.fontsize12px
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        width: parent.background.width
        anchors.left: parent.background.left
        anchors.leftMargin: GlobalSettings.halfspacing
    }

    indicator: Image {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: GlobalSettings.halfspacing
        source: combobox.popup.visible ? "../images/arrow-head-up-s.png"
                                       : "../images/arrow-head-down-s.png"
        height: 6
        width: 10
    }

    popup: Popup {
        y: parent.background.height
        x: parent.background.x
        width: combobox.background.width
        implicitHeight: contentItem.implicitHeight + GlobalSettings.defaultspacing

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }

        background: Rectangle {
            radius: combobox.background.radius
            color: "#80181818"
            border.color: "#EFEFEF"
        }

        contentItem: ListView {
            clip: true
            model: combobox.delegateModel
            implicitHeight: contentHeight
            currentIndex: combobox.highlightedIndex

            delegate: ItemDelegate {
                required property string modelData
                width: combobox.background.width
                height: GlobalSettings.fontsize18px
                background: Rectangle {
                    color: parent.hovered ? "#80EFEFEF" : "transparent"
                }
                contentItem: Text {
                    text: parent.modelData
                    anchors.fill: parent
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize12px
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
