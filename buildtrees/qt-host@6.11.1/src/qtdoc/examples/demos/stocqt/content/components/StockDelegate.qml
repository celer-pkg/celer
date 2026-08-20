// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import StocQt

Rectangle {
    id: stockDelegate
    width: 360
    height: 81
    color: "transparent"
    property alias selectButton: mouseArea

    required property string stockId
    required property real price
    required property real change
    required property real changePercentage
    required property string name
    required property real date
    required property bool favorite

    state: change >= 0 ? "Rising" : "Falling"

    RowLayout {
        anchors.fill: parent
        spacing: 51

        Rectangle {
            x: 8
            y: 12
            width: 178
            height: 53
            Layout.preferredWidth: 178
            Layout.preferredHeight: 53
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 10
                Image {
                    id: logoBackground
                    y: 15
                    width: 24
                    height: 24
                    Layout.leftMargin: 13
                    Layout.rightMargin: 0
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    source: "../images/logos/" + stockDelegate.stockId + ".svg"
                }

                Rectangle {
                    id: profileDescription
                    x: 34
                    width: 144
                    height: 53
                    Layout.preferredWidth: 144
                    Layout.preferredHeight: 53
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    color: "transparent"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 5
                        Text {
                            id: idText
                            width: 35
                            height: 16
                            color: "#f2f2f2"
                            text: stockDelegate.stockId
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignTop
                            wrapMode: Text.NoWrap
                            font.weight: Font.Bold
                            font.family: "Titillium Web"
                            Layout.preferredWidth: 35
                            Layout.preferredHeight: 16
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        }

                        Text {
                            id: nameText
                            y: 21
                            width: 145
                            height: 16
                            color: "#bfbfbf"
                            text: stockDelegate.name
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignTop
                            wrapMode: Text.NoWrap
                            font.weight: Font.Normal
                            font.family: "Roboto"
                            Layout.preferredWidth: 145
                            Layout.preferredHeight: 16
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        }

                        Image {
                            id: timeIcon
                            y: 42
                            width: 12
                            height: 12
                            clip: true
                            Layout.preferredWidth: 12
                            Layout.preferredHeight: 12
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            source: "../images/timeGreen.svg"
                        }
                    }

                    Text {
                        id: dateText
                        x: 19
                        y: 43
                        width: 66
                        height: 11
                        color: "#bfbfbf"
                        text: qsTr("%1").arg(new Date(stockDelegate.date).toLocaleString(Locale.ShortFormat))
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        font.weight: Font.Medium
                        font.family: "Roboto"
                    }
                }
            }
        }

        Rectangle {
            id: priceRect
            x: 261
            y: 8
            width: 71
            height: 61
            Layout.rightMargin: 18
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: 71
            Layout.preferredHeight: 61
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                spacing: 4
                Star {
                    id: star
                    x: 53
                    width: 24
                    height: 24
                    clip: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    isFavorite: stockDelegate.favorite
                    stock: stockDelegate.stockId
                }

                Text {
                    id: priceText
                    x: 35
                    y: 22
                    width: 37
                    height: 21
                    color: "#f2f2f2"
                    text: qsTr("%L1").arg(stockDelegate.price.toFixed(1))
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Bold
                    font.family: "Titillium Web"
                    Layout.preferredWidth: 37
                    Layout.preferredHeight: 21
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }

                Text {
                    id: priceChangeText
                    y: 47
                    width: 72
                    height: 14
                    color: "#5ecca3"
                    text: qsTr("%L1 (%L2%)").arg(stockDelegate.change.toFixed()).arg(stockDelegate.changePercentage.toFixed(1))
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        height: 81
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 60
        onClicked: {
            StockEngine.updateStockView(stockDelegate.stockId)
        }
    }

    states: [
        State {
            name: "Rising"
        },
        State {
            name: "Falling"
            PropertyChanges {
                target: priceChangeText
                color: "#fa8a8a"
            }
        }
    ]
}
