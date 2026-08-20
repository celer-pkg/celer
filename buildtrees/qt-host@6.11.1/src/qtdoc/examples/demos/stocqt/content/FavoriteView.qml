// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import StocQt
import QtQuick.Layouts
import "components"

Rectangle {
    id: rectangle
    color: "#101010"

    property int updatedModels: 0
    property bool portrait: width < height
    property bool fullscreen: false
    property alias currentIndex: favoriteList.currentIndex

    Component.onCompleted: StockEngine.onFavoritesChanged.connect(updateListModels)

    function updateListModels(){
        stockLegendModel.clear()
        favoriteListModel.clear()
        favoriteList.currentIndex = 0
        for (let i = 0; i < StockEngine.favoritesModel.count(); i++){
            let stock = StockEngine.favoritesModel.atIndex(i)
            let stockId = stock.stockId()
            let lastOpen = stock.openPrice(0)
            let lastClose = stock.closePrice(0)
            let color = StockEngine.favoritesModel.color(i)

            stockLegendModel.append({"stockId": stockId,
                                        "keyColor": color})
            favoriteListModel.append({"stockId": stockId,
                                      "open":lastOpen,
                                      "close":lastClose})
        }

        profile.updateDetails()
        chart.updateData()
    }

    Rectangle {
        id: banner
        width: parent.width
        height: rectangle.portrait ? 42 : 0
        color: parent.color
        anchors.top: parent.top
        anchors.topMargin: 3
        visible: rectangle.portrait

        Image {
            id: logo2
            anchors.verticalCenter: parent.verticalCenter
            source: "images/qtLogo2.png"
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
        }
    }
    FavTab {
        id: tab
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: banner.bottom
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        priceButton.onClicked: {
            tab.state = "PriceActive"
            chart.state = "Price"
        }
        volumeButton.onClicked: {
            tab.state = "VolumeActive"
            chart.state = "Volume"
        }
    }

    GridLayout {
        id: gridLayout
        width: parent.width
        anchors.top: tab.bottom
        anchors.bottom: parent.bottom
        layoutDirection: Qt.RightToLeft
        rows: rectangle.portrait? 4 : 2
        columns: rectangle.portrait? 1 : 2

        StockDetail {
            id: profile
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            visible: !rectangle.fullscreen

            function updateDetails() {
                if (rectangle.currentIndex === -1 || StockEngine.favoritesModel.count() === 0)
                    return
                var stock = StockEngine.favoritesModel.atIndex(rectangle.currentIndex)
                stockId = stock.stockId()
                stockName = stock.name()
                price = qsTr("%L1$").arg(stock.price(0).toFixed(1))
                priceChange = qsTr("%L1%").arg(stock.changePercentage(0).toFixed(2))
                rising = stock.change(0) >= 0
                timeBar.date = qsTr("%1").arg(new Date(stock.historyDate(0)).
                                              toLocaleDateString(Locale.ShortFormat))
            }
        }

        Rectangle {
            id: graphItems
            width: parent.width
            height: 200
            Layout.preferredWidth: 360
            Layout.preferredHeight: 200
            Layout.rowSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: rectangle.portrait || rectangle.visible? parent.width : parent.width / 2

            TimeBar {
                id: timeBar
                width: parent.width
                anchors.top: parent.top
            }

            Rectangle {
                id: legendRect
                height: 34
                color: "#1d1d1d"
                width: parent.width
                anchors.bottom: parent.bottom

                ListView {
                    id: stockLegend
                    anchors.fill: parent
                    anchors.topMargin: 10
                    anchors.rightMargin: 12
                    anchors.leftMargin: 12
                    interactive: false
                    orientation: ListView.Horizontal
                    model: ListModel{
                        id: stockLegendModel
                    }

                    delegate: Rectangle {
                        id: legendItem
                        width: legendRect.width / 5
                        color: "transparent"
                        required property color keyColor
                        RowLayout {
                            id: layout
                            anchors.fill: parent
                            Rectangle {
                                id: tag_color
                                width: 8
                                height: 8
                                radius: 180
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                color: legendItem.keyColor
                            }

                            Text {
                                id: tag_name
                                color: "#BFBFBF"
                                text: StockEngine.currentStockId()
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                font.family: "Roboto"
                            }
                        }
                    }
                }
            }

            Item {
                id: graph
                width: parent.width
                anchors.top: timeBar.bottom
                anchors.bottom: legendRect.top

                FavoriteChart {
                    timeFrame: timeBar.selectedTimeframe
                    id: chart
                    width: parent.width
                    height: parent.height
                }

                Image {
                    id: fullscreenId
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.bottomMargin: 5
                    anchors.rightMargin: 5
                    source: "images/fullscreen.png"

                    MouseArea {
                        id: fullscreenButton
                        anchors.fill: parent
                        onClicked: rectangle.fullscreen = !rectangle.fullscreen
                    }
                }
            }
        }
        Rectangle {
            id: overviewRect
            height: 233
            color: "#00ffffff"
            width: parent.width
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.fillWidth: true
            Layout.fillHeight: !rectangle.portrait
            visible: !rectangle.fullscreen

            Text {
                id: overview
                x: 8
                y: 0
                width: 58
                color: "#f2f2f2"
                text: qsTr("Overview")
                font.pixelSize: 14
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                anchors.verticalCenterOffset: 1
                font.weight: Font.DemiBold
                font.family: "Titillium Web"
            }

            Rectangle {
                id: addId
                width: 64
                height: 22

                property bool full
                color: "transparent"
                border.color: full? "#787878" : "#f2f2f2"
                border.width: 1
                anchors.right: parent.right
                anchors.rightMargin: 20
                radius: 4
                Component.onCompleted: StockEngine.onFavoritesChanged.connect(setFavoritesFull)
                function setFavoritesFull(full) {
                    addId.full = full
                }
                Text{
                    anchors.fill: parent
                    anchors.rightMargin: 20
                    id: addText
                    text: qsTr("ADD")
                    width: 25
                    height: 14
                    color: addId.full? "#787878" : "#f2f2f2"
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    id: plusIcon
                    width: 24
                    height: 24
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    property color plusColor: addId.full? "#787878" : "#f2f2f2"
                    Rectangle {
                        width: 2
                        height: 8
                        radius: 2
                        color: parent.plusColor
                        anchors.centerIn: parent
                    }
                    Rectangle {
                        width: 8
                        height: 2
                        radius: 2
                        color: parent.plusColor
                        anchors.centerIn: parent
                    }
                }

                MouseArea {
                    id: addButtonId
                    anchors.fill: parent
                    enabled: !addId.full
                    onClicked: {
                        StockEngine.filterModel.setFilterFixedString("")
                        addPopup.open()
                    }
                }
            }

            Rectangle {
                id: favStats
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.topMargin: 30
                color: "#1D1D1D"
                radius: 10
                height: 185
                anchors.rightMargin: 10
                anchors.leftMargin: 10

                ListView {
                    id: favoriteList
                    width: 320
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: 11
                    anchors.leftMargin: 12
                    anchors.topMargin: 8
                    anchors.bottomMargin: 4
                    interactive: false
                    model: ListModel {
                        id: favoriteListModel
                    }
                    delegate: FavStatsDelegate {
                        id: favStatsDelegate
                        width: favoriteList.width
                        required property int index
                        selectButton.onClicked: {
                            favoriteList.currentIndex = favStatsDelegate.index
                        }
                    }
                    onCurrentIndexChanged: {
                        profile.updateDetails()
                    }

                    highlight: Rectangle {
                        color: "black"
                        border.color: "black"
                        border.width: 1
                        height: 32
                        width: 311
                    }
                }
            }
        }
        Popup {
            id: addPopup
            width: 184
            height: 220
            modal: true
            anchors.centerIn: parent
            background: Rectangle {
                id: backgroundId
                radius: 5
                color: "#1D1D1D"
            }
            dim: true
            Overlay.modal: Rectangle {
               color: Qt.rgba(0,0,0,0.8)
            }

            Search {
                id: search
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.top: parent.top
                placeHolderText: ""
            }
            ListView {
                id: addListView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.top: search.top
                anchors.topMargin: 50
                anchors.bottom: parent.bottom
                clip: true
                boundsBehavior: Flickable.DragAndOvershootBounds
                model: StockEngine.filterModel
                spacing: 5
                delegate: AddDelegate {
                    width: addListView.width
                    popup: addPopup
                }
            }
        }
    }
}
