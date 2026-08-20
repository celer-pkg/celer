// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: delegateItem

    required property url image
    required property string name
    required property real rating
    required property int originalPrice
    required property int discountPercent
    required property int index

    width: GridView.view.cellWidth
    height: GridView.view.cellHeight

    signal toySelected

    Rectangle {
        id: background
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        width: ApplicationConfig.responsiveSize(544)
        height: ApplicationConfig.responsiveSize(611)

        border.width: Math.round(ApplicationConfig.responsiveSize(4))
        border.color: "#D7D6E1"
        color: "#FFFFFF"
        radius: ApplicationConfig.responsiveSize(16)
    }

    ColumnLayout {
        id: content
        anchors {
            fill: parent
            margins: 10
        }
        spacing: 6
        ToyImage {
            id: modelImage
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: ApplicationConfig.responsiveSize(448)
            Layout.preferredHeight: ApplicationConfig.responsiveSize(448)
            sourceSize {
                width: ApplicationConfig.responsiveSize(448)
                height: ApplicationConfig.responsiveSize(448)
            }
            source: delegateItem.image
        }
        ToyLabel {
            id: modelName
            implicitHeight: ApplicationConfig.responsiveSize(64)
            textStyle: ApplicationConfig.TextStyle.H3_Bold
            text: delegateItem.name
        }
        RowLayout {
            id: priceItem
            Layout.fillWidth: true
            spacing: 4
            ToyLabel {
                id: modelPrice
                textStyle: ApplicationConfig.TextStyle.Price_M
                text: Number(delegateItem.originalPrice * (1 - delegateItem.discountPercent / 100)).toLocaleCurrencyString()
            }
            ColorIcon {
                source: "icons/currency.svg"
                color: modelPrice.color
                implicitWidth: 51
                implicitHeight: 12
                Layout.alignment: Qt.AlignBottom
            }
            Item {
                // filler item
                Layout.fillWidth: true
            }
            ToyLabel {
                id: modelRating
                Layout.alignment: Qt.AlignRight
                textStyle: ApplicationConfig.TextStyle.Body
                text: qsTr("★ %1").arg(Number(delegateItem.rating).toLocaleString())
            }
        }

        RowLayout {
            visible: delegateItem.discountPercent
            Layout.fillWidth: true
            ToyLabel {
                id: originalPrice
                textStyle: ApplicationConfig.TextStyle.Price_S
                font.strikeout: true
                color: "#6A6A8D"
                text: delegateItem.originalPrice
            }
            ToyLabel {
                id: discountPercent
                textStyle: ApplicationConfig.TextStyle.Price_S
                color: "#6A6A8D"
                text: qsTr("( -%1% )").arg(delegateItem.discountPercent)
            }
        }

        Item {
            implicitWidth: 2
            Layout.fillHeight: true
        }

        ToyButton {
            id: chooseButton
            implicitHeight: ApplicationConfig.responsiveSize(105)
            implicitWidth: ApplicationConfig.responsiveSize(448)
            type: ToyButton.Type.Secondary
            textStyle: ApplicationConfig.TextStyle.Button_M
            text: qsTr("Choose")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillWidth: true
            onClicked: delegateItem.toySelected()
        }
    }
}
