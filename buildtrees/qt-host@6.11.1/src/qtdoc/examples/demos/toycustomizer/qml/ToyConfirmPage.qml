// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property int toyIndex: -1
    property var __modelData: ToyModel.get(root.toyIndex) ?? null
    property int __price: root.__modelData ? root.__modelData.originalPrice : 0
    property int __discount: root.__modelData ? root.__modelData.discountPercent : 0
    property real __imageSourceSize: ApplicationConfig.responsiveSize(1150)

    signal cancelled
    signal confirmed

    Item {
        id: contentItem

        readonly property real horizontalMargins: ApplicationConfig.responsiveSize(200)
        readonly property real minimumWidth: ApplicationConfig.responsiveSize(1760)
        readonly property real paddings: ApplicationConfig.responsiveSize(128)

        width: {
            const maximumWidth = ApplicationConfig.responsiveSize(2848)
            const preferredWidth = parent.width - 2 * horizontalMargins
            return Math.min(Math.max(minimumWidth, preferredWidth), maximumWidth)
        }

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            leftMargin: horizontalMargins
            topMargin: ApplicationConfig.responsiveSize(150)
        }

        state: width < root.width ? "" : "narrow"
        states: State {
            name: "narrow"
            AnchorChanges {
                target: contentItem
                anchors {
                    left: contentItem.parent.left
                    horizontalCenter: undefined
                }
            }
            PropertyChanges {
                target: contentItem
                anchors.leftMargin: 0
            }
        }

        ColumnLayout {
            readonly property real topMargin: ApplicationConfig.responsiveSize(176)
            spacing: ApplicationConfig.responsiveSize(56)
            height: contentLayout.height + parent.paddings - topMargin
            width: contentLayout.width + 2 * parent.paddings
            anchors {
                top: parent.top
                left: parent.left
                topMargin: topMargin
            }
            ToyButton {
                type: ToyButton.Type.Secondary
                textStyle: ApplicationConfig.TextStyle.Button_L
                text: qsTr("Back")
                icon.source: "icons/back.svg"
                onClicked: root.cancelled()
            }
            Rectangle {
                id: gridBackgroundRect
                radius: ApplicationConfig.responsiveSize(56)
                color: "white"
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        ColumnLayout {
            id: contentLayout
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                leftMargin: parent.paddings
                rightMargin: parent.paddings
            }
            LayoutItemProxy {
                target: portraitGridLayout
                visible: ApplicationConfig.isPortrait
                Layout.fillWidth: true
            }
            LayoutItemProxy {
                target: landscapeGridLayout
                visible: !ApplicationConfig.isPortrait
                Layout.fillWidth: true
            }
        }
    }

    // GridLayout for portrait mode
    GridLayout {
        id: portraitGridLayout

        visible: ApplicationConfig.isPortrait
        columns: 2
        columnSpacing: ApplicationConfig.responsiveSize(64)

        Item {
            implicitHeight: root.__imageSourceSize
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumWidth: toyImage.sourceSize.width
            Layout.minimumHeight: toyImage.sourceSize.height
            Layout.alignment: Qt.AlignCenter
            LayoutItemProxy {
                target: toyImage
                anchors.fill: parent
                anchors.margins: 10
            }
        }

        ColumnLayout {
            spacing: ApplicationConfig.responsiveSize(48)
            Layout.fillWidth: true
            Layout.fillHeight: true
            LayoutItemProxy {
                target: toyNameLabel
            }
            LayoutItemProxy {
                target: reviewsRow
            }
            LayoutItemProxy {
                target: descriptionLabel
                Layout.fillWidth: true
                Layout.minimumHeight: descriptionLabel.implicitHeight
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            LayoutItemProxy {
                visible: root.__discount > 0
                target: discountRow
            }
            LayoutItemProxy {
                id: portraitPriceLayoutItem
                target: priceRow
                Layout.minimumWidth: target.implicitWidth
            }
            LayoutItemProxy {
                target: confirmButton
                Layout.topMargin: ApplicationConfig.responsiveSize(100)
                Layout.preferredWidth: Math.max(portraitPriceLayoutItem.width,
                                                confirmButton.implicitWidth)
            }
        }
    }

    // GridLayout for landscape mode
    GridLayout {
        id: landscapeGridLayout
        visible: !ApplicationConfig.isPortrait
        columns: 3

        Item {
            implicitHeight: root.__imageSourceSize
            Layout.fillWidth: true
            Layout.minimumWidth: toyImage.sourceSize.width
            Layout.minimumHeight: toyImage.sourceSize.height
            Layout.alignment: Qt.AlignCenter
            LayoutItemProxy {
                target: toyImage
                anchors.fill: parent
            }
        }
        Item {
            implicitHeight: 2
            Layout.fillWidth: true
        }
        ColumnLayout {
            Layout.topMargin: ApplicationConfig.responsiveSize(522)
            spacing: 16
            LayoutItemProxy {
                visible: root.__discount > 0
                target: discountRow
                Layout.alignment: Qt.AlignLeft
            }
            LayoutItemProxy {
                id: landscapePriceLayoutItem
                target: priceRow
                Layout.alignment: Qt.AlignLeft
            }
            LayoutItemProxy {
                target: confirmButton
                Layout.topMargin: ApplicationConfig.responsiveSize(80)
                Layout.preferredWidth: landscapePriceLayoutItem.width
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.columnSpan: 3
            spacing: ApplicationConfig.responsiveSize(64)
            LayoutItemProxy {
                target: toyNameLabel
            }
            LayoutItemProxy {
                target: reviewsRow
            }
            LayoutItemProxy {
                target: descriptionLabel
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }

    // Items
    ToyImage {
        id: toyImage
        source: root.__modelData ? root.__modelData.image : ""
        sourceSize {
            width: root.__imageSourceSize
            height: root.__imageSourceSize
        }
    }
    ToyLabel {
        id: toyNameLabel
        text: root.__modelData ? root.__modelData.name : ""
        textStyle: ApplicationConfig.TextStyle.H2_Bold
    }
    Row {
        id: reviewsRow
        spacing: 8
        ToyLabel {
            text: qsTr("%1 reviews").arg(root.__modelData ? root.__modelData.reviews : 0)
            textStyle: ApplicationConfig.TextStyle.H3_Light
        }
        ToyLabel {
            text: qsTr("★%1").arg(root.__modelData ? root.__modelData.rating : 0)
            textStyle: ApplicationConfig.TextStyle.H3_Light
        }
    }
    ToyLabel {
        id: descriptionLabel
        wrapMode: Text.WordWrap
        textStyle: ApplicationConfig.TextStyle.Body_L
        color: "#6A6A8D"
        text: root.__modelData ? root.__modelData.description : ""
    }
    Row {
        id: discountRow
        spacing: 8
        ToyLabel {
            id: originalPriceLabel
            anchors.verticalCenter: parent.verticalCenter
            textStyle: ApplicationConfig.isPortrait ? ApplicationConfig.TextStyle.Price_ML
                                                    : ApplicationConfig.TextStyle.Price_L
            text: qsTr("%1").arg(root.__price)
            font.strikeout: true
            color: "#6A6A8D"
        }
        ToyLabel {
            id: discountLabel
            anchors.verticalCenter: parent.verticalCenter
            textStyle: ApplicationConfig.isPortrait ? ApplicationConfig.TextStyle.Price_M
                                                    : ApplicationConfig.TextStyle.Price_ML
            text: qsTr("%1%").arg(-root.__discount)
            color: "#6A6A8D"
        }
    }
    Row {
        id: priceRow
        spacing: 8

        ToyLabel {
            id: priceLabel
            textStyle: ApplicationConfig.isPortrait ? ApplicationConfig.TextStyle.Price_XL
                                                    : ApplicationConfig.TextStyle.Price_XXL
            text: root.__discount > 0 ? `${root.__price * (1 - root.__discount / 100)}`
                                      : `${root.__price}`
        }

        ColorIcon {
            implicitWidth: ApplicationConfig.isPortrait ? 102 : 153
            implicitHeight: ApplicationConfig.isPortrait ? 24 : 36
            anchors.bottom: parent.bottom
            source: "icons/currency.svg"
        }
    }
    ToyButton {
        id: confirmButton
        textStyle: ApplicationConfig.TextStyle.Button_L
        text: qsTr("Confirm choice")
        onClicked: root.confirmed()
    }

    Connections {
        target: ToyModel
        function onDataChanged() {
            root.__modelDataChanged()
        }
    }
}
