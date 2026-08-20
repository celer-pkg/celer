// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: overViewPage

    property bool buttonsVisible: true
    property alias toyIndex: orderGrid.toyIndex
    property alias accessoryModel: orderGrid.accessoryModel
    readonly property bool narrowWidth: width < 1112

    signal confirmed
    signal cancelled

    /* Column Layout for portrait mode */
    ColumnLayout {
        id: portraitLayout

        spacing: ApplicationConfig.responsiveSize(60)
        visible: ApplicationConfig.isPortrait

        anchors {
            fill: parent
            topMargin: ApplicationConfig.responsiveSize(200)
        }

        Item {
            implicitWidth: 2
            Layout.fillHeight: true
        }

        LayoutItemProxy {
            target: orderGrid
            // Give the OrderGrid 20 pixel margins on horizontal sides (left+right=40).
            Layout.preferredWidth: Math.min(orderGrid.implicitWidth, portraitLayout.width - 40)
            Layout.minimumWidth: 472
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            implicitWidth: 2
            Layout.fillHeight: true
        }

        LayoutItemProxy {
            target: backButton
            visible: overViewPage.buttonsVisible
            Layout.leftMargin: ApplicationConfig.responsiveSize(80)
        }
        Item {
            implicitHeight: backButton.implicitHeight
            visible: !overViewPage.buttonsVisible
        }

        LayoutItemProxy {
            target: whiteAreaItem
            Layout.fillWidth: true
        }
    }

    /* Grid Layout for landscape mode */
    GridLayout {
        id: landscapeLayout

        visible: !ApplicationConfig.isPortrait
        columns: 2
        rowSpacing: ApplicationConfig.responsiveSize(80)
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        columnSpacing: {
            const prefSpacing = ApplicationConfig.responsiveSize(400)
            const prefWidth = whiteAreaItem.width + breakdownAndConfirmItem.width + prefSpacing
            const minMargins = ApplicationConfig.responsiveSize(60)
            const parentWidth = parent.width - 2 * minMargins
            const availableWidth = parentWidth - prefWidth
            if (availableWidth > 0)
                return prefSpacing
            const minSpacing = ApplicationConfig.responsiveSize(40)
            return Math.max(prefSpacing + availableWidth, minSpacing)
        }

        state: height < parent.height ? "" : "narrow"
        states: State {
            name: "narrow"
            AnchorChanges {
                target: landscapeLayout
                anchors {
                    verticalCenter: undefined
                    top: landscapeLayout.parent.top
                }
            }
        }

        LayoutItemProxy {
            target: backButton
            visible: overViewPage.buttonsVisible
        }

        LayoutItemProxy {
            target: breakdownAndConfirmItem
            Layout.rowSpan: 2
            Layout.row: 0
            Layout.column: 1
        }

        LayoutItemProxy {
            target: whiteAreaItem
            Layout.row: overViewPage.buttonsVisible ? 1 : 0
            Layout.column: 0
        }
    }

    Item {
        id: whiteAreaItem

        readonly property real horizontalPaddings: ApplicationConfig.responsiveSize(80)
        readonly property real verticalPaddings: overViewPage.buttonsVisible ?
                                             ApplicationConfig.responsiveSize(80) :
                                             ApplicationConfig.responsiveSize(200)

        // For the smaller width of the page the white area should have fixed width,
        // as the minimum width, to force layouting and text wrapping.
        implicitWidth: overViewPage.narrowWidth ? 576 : (whiteAreaLayout.implicitWidth +
                                                         2 * horizontalPaddings)
        implicitHeight: whiteAreaLayout.implicitHeight + 2 * verticalPaddings
        visible: false

        Rectangle {
            id: whiteAreaRect
            radius: ApplicationConfig.responsiveSize(64)
            color: "white"
            anchors.fill: parent

            Binding {
                when: ApplicationConfig.isPortrait
                whiteAreaRect {
                    bottomLeftRadius: 0
                    bottomRightRadius: 0
                }
            }
        }

        ColumnLayout {
            id: whiteAreaLayout
            anchors.centerIn: parent
            width: parent.width - 2 * parent.horizontalPaddings
            LayoutItemProxy {
                target: orderGrid
                visible: !ApplicationConfig.isPortrait
                Layout.fillWidth: true
            }
            LayoutItemProxy {
                target: breakdownAndConfirmItem
                visible: ApplicationConfig.isPortrait
                Layout.alignment: Qt.AlignCenter
            }
        }
    }

    Column {
        id: breakdownAndConfirmItem
        spacing: ApplicationConfig.responsiveSize(152)
        visible: false
        LayoutItemProxy {
            target: orderBreakdownLayout
        }
        LayoutItemProxy {
            target: confirmButton
            visible: overViewPage.buttonsVisible
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    GridLayout {
        id: orderBreakdownLayout

        columns: 4

        ToyLabel {
            id: subTotal
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: qsTr("SubTotal")
        }
        Item {
            implicitHeight: 2
            implicitWidth: ApplicationConfig.responsiveSize(720)
            Layout.fillWidth: true
        }
        ToyLabel {
            id: subTotalPrice
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: `${orderGrid.totalPrice}`
        }
        ToyImage {
            Layout.alignment: Qt.AlignRight
            source: "icons/currency.svg"
            color: subTotalPrice.color
            colorize: true
            sourceSize {
                width: ApplicationConfig.responsiveSize(142)
                height: ApplicationConfig.responsiveSize(32)
            }
        }

        ToyLabel {
            id: delivery
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: qsTr("Delivery")
            Layout.row: 1
            Layout.column: 0
        }
        ToyLabel {
            id: deliveryPrice
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: qsTr("0")
            Layout.row: 1
            Layout.column: 2
        }
        ToyImage {
            Layout.alignment: Qt.AlignRight
            source: "icons/currency.svg"
            color: deliveryPrice.color
            colorize: true
            sourceSize {
                width: ApplicationConfig.responsiveSize(142)
                height: ApplicationConfig.responsiveSize(32)
            }
            Layout.row: 1
            Layout.column: 3
        }

        Rectangle {
            id: separator
            color: "#162655"
            implicitHeight: ApplicationConfig.responsiveSize(8)
            Layout.fillWidth: true
            Layout.columnSpan: 4
            Layout.row: 2
            Layout.column: 0
        }

        ToyLabel {
            id: total
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: qsTr("Total")
            Layout.row: 3
            Layout.column: 0
        }
        ToyLabel {
            id: totalPrice
            textStyle: ApplicationConfig.TextStyle.H3_Light
            text: `${orderGrid.totalPrice}`
            Layout.row: 3
            Layout.column: 2
        }
        ToyImage {
            Layout.alignment: Qt.AlignRight
            source: "icons/currency.svg"
            color: totalPrice.color
            colorize: true
            sourceSize {
                width: ApplicationConfig.responsiveSize(142)
                height: ApplicationConfig.responsiveSize(32)
            }
            Layout.row: 3
            Layout.column: 3
        }
    }

    OrderGrid {
        id: orderGrid

        // This is a break-point width when there is not enough width to show
        // all the items with their preferred widths.
        readonly property real narrowWidth: ApplicationConfig.isPortrait ? 640 : 584

        visible: false
        implicitWidth: ApplicationConfig.responsiveSize(2280)
        implicitHeight: ApplicationConfig.responsiveSize(1170)
        accessoryModel: overViewPage.accessoryModel
        wrap: width < narrowWidth
    }

    ToyButton {
        id: confirmButton
        visible: false
        type: ToyButton.Type.Primary
        textStyle: ApplicationConfig.TextStyle.Button_L
        text: qsTr("Confirm order")
        onClicked: overViewPage.confirmed()
    }

    ToyButton {
        id: backButton
        visible: false
        type: ToyButton.Type.Secondary
        textStyle: ApplicationConfig.TextStyle.Button_L
        icon.source: "icons/back.svg"
        text: qsTr("Back")
        onClicked: overViewPage.cancelled()
    }
}
