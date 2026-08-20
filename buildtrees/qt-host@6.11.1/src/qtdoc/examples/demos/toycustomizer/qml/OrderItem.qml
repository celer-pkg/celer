// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: orderItem

    required property real oldPrice
    required property real newPrice
    property real __discountPercent: Math.round(Math.max(oldPrice - newPrice, 0) / oldPrice * 100)
    property string name
    property bool priceVisible: true
    property bool isSelected: false
    property alias label: itemLabel.text
    property alias image: toyImage.source

    implicitHeight: mainLayout.implicitHeight
    implicitWidth: mainLayout.implicitWidth

    RowLayout {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: separator.top
            topMargin: ApplicationConfig.responsiveSize(20)
            bottomMargin: ApplicationConfig.responsiveSize(40)
        }
        ColumnLayout {
            id: layout
            spacing: ApplicationConfig.responsiveSize(20)
            Item {
                implicitWidth: itemLabel.implicitWidth
                implicitHeight: itemLabel.implicitHeight
                Layout.fillWidth: true
                Layout.bottomMargin: ApplicationConfig.responsiveSize(24)
                ToyLabel {
                    id: itemLabel
                    textStyle: ApplicationConfig.TextStyle.H3_Light
                    color: "#6A6A8D"
                }
            }
            RowLayout {
                ToyLabel {
                    id: itemName
                    wrapMode: Text.WordWrap
                    textStyle: ApplicationConfig.TextStyle.H4
                    lineHeight: 1
                    text: orderItem.isSelected ? orderItem.name : qsTr("Not Selected")
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.fillWidth: true
                }
                Item {
                   implicitHeight: 2
                   visible: orderItem.isSelected
                   Layout.fillWidth: true
                   Layout.fillHeight: true
                }
                LayoutItemProxy {
                    target: selectedItem
                    visible: orderItem.isSelected && orderItem.priceVisible
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
            Item {
               implicitWidth: 2
               Layout.fillHeight: true
            }
        }
        Item {
            implicitWidth: ApplicationConfig.responsiveSize(190)
            implicitHeight: ApplicationConfig.responsiveSize(190)
            Layout.topMargin: ApplicationConfig.responsiveSize(48)
            Image {
                id: toyImage
                visible: orderItem.isSelected
                sourceSize {
                    width: ApplicationConfig.responsiveSize(190)
                    height: ApplicationConfig.responsiveSize(190)
                }
                anchors.centerIn: parent
            }
        }
    }

    Rectangle {
        id: separator
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: ApplicationConfig.responsiveSize(5)
        color: "#A0AFC5"
    }

    ColumnLayout {
        id: selectedItem
        visible: orderItem.isSelected && orderItem.priceVisible
        spacing: 0
        RowLayout {
            id: priceRow
            spacing: ApplicationConfig.responsiveSize(16)
            ToyLabel {
                id: toyPrice
                textStyle: ApplicationConfig.TextStyle.Price_M
                text: `${orderItem.newPrice}`
            }
            ColorIcon {
                implicitWidth: 51
                implicitHeight: 12
                source: "icons/currency.svg"
                color: toyPrice.color
            }
        }
        RowLayout {
            visible: orderItem.__discountPercent > 0
            spacing: ApplicationConfig.responsiveSize(16)
            ToyLabel {
                id: originalPrice
                textStyle: ApplicationConfig.TextStyle.Price_S
                color: "#6A6A8D"
                font.strikeout: true
                text: `${orderItem.oldPrice}`
            }
            ToyLabel {
                id: discount
                textStyle: ApplicationConfig.TextStyle.Price_S
                color: "#6A6A8D"
                text: qsTr("%1%").arg(-orderItem.__discountPercent)
            }
        }
        Item {
            // filler
            Layout.fillHeight: true
        }
    }
}
