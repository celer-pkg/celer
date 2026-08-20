// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick 2.15
import QtQuick.Layouts
import StocQt
import "components"

Rectangle {
    id: rectangle
    color: "#101010"
    required property StateGroup stateGroup
    property bool portrait: width < height
    property bool fullscreen: false

    Component.onCompleted: {
        StockEngine.onStockModelChanged.connect(update)
        StockEngine.favoritesModel.onFavoritesChanged.connect(updateFavorite)
    }

    function update() {
        stockDetail.stockId = StockEngine.currentStockId()
        stockDetail.stockName = StockEngine.currentName()
        stockDetail.price = qsTr("%L1").arg(StockEngine.stockModel.price(0).toFixed(1))
        stockDetail.priceChange = qsTr("%L1%").arg(StockEngine.stockModel.changePercentage(0).toFixed(2))
        star.isFavorite = StockEngine.isFavorite(stockDetail.stockId)
        keyStats.open = qsTr("%L1$").arg(StockEngine.stockModel.openPrice(0))
        keyStats.close = qsTr("%L1$").arg(StockEngine.stockModel.closePrice(0))
        keyStats.volume = qsTr("%L1").arg(StockEngine.stockModel.volume(0))
        keyStats.averageVolume = qsTr("%L1").arg(StockEngine.stockModel.avgVolume())
        timeBar.date = qsTr("%1").arg(new Date(StockEngine.stockModel.historyDate(0)).toLocaleDateString(Locale.ShortFormat))
        chart.update()
    }


    function updateFavorite() {
        star.isFavorite = StockEngine.isFavorite(stockDetail.stockId)
        star.favoritesFull = StockEngine.favoritesModel.count() >= 5
    }

    Rectangle {
        id: banner
        x: 0
        width: parent.width
        height: rectangle.portrait? 42 : 0
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

        Star {
            id: star
            x: 319
            y: 10
            anchors.right: parent.right
            anchors.rightMargin: 24
            stock: stockDetail.stockId
        }

        Image {
            id: arrowleft
            y: 10
            anchors.left: parent.left
            source: "images/arrowLeft.svg"
            anchors.leftMargin: 24
            fillMode: Image.PreserveAspectFit

            MouseArea {
                x: 0
                y: 0
                width: 24
                height: 24
                onClicked: rectangle.stateGroup.state = "ListView"
            }
        }
    }

    TabMenu {
        id: tab
        anchors.top: banner.bottom
        width: parent.width
        historyButton.onClicked: {
            tab.state = "History"
            chart.state = "History"
        }
        volumeButton.onClicked: {
            tab.state = "Volume"
            chart.state = "Volume"
        }
        liveButton.onClicked: {
            tab.state = "LiveData"
            chart.state = "Live"
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
            id: stockDetail
            width: parent.width
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            stockId: StockEngine.currentStockId()
            stockName: StockEngine.currentName()
            price: StockEngine.stockModel.closePrice(0);
            priceChange: StockEngine.stockModel.changePercentage(0) + "%"
            rising: StockEngine.stockModel.changePercentage(0) >= 0
            visible: !rectangle.fullscreen
        }

        Item {
            id: graphItems
            width: parent.width
            height: 200
            Layout.preferredWidth: 360
            Layout.preferredHeight: 200
            Layout.rowSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: rectangle.portrait || rectangle.fullscreen ? parent.width : parent.width / 2

            TimeBar {
                id: timeBar
                anchors.top: parent.top
                width: parent.width
                height: chart.state !== "Live"? 42 : 0
            }

            Rectangle {
                id: graph
                anchors.bottom: legend.top
                anchors.top: timeBar.bottom
                anchors.bottomMargin: 5
                width: parent.width


                StockChart {
                    id: chart
                    width: parent.width
                    height: parent.height
                    timeFrame: timeBar.selectedTimeframe
                    highVisible: legend.high.checked
                    lowVisible: legend.low.checked
                    openVisible: legend.open.checked
                    closeVisible: legend.close.checked
                }

                Image {
                    id: fullscreenImage
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

            Legend {
                id: legend
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
                width: parent.width
                visible: tab.state === "History"
            }
        }
        Rectangle {
            id: overviewRect
            width: parent.width
            height: 200
            color: "#00ffffff"
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.fillWidth: true
            Layout.fillHeight: !rectangle.portrait
            visible: !rectangle.fullscreen
            Text {
                id: overview
                width: 58
                color: "#f2f2f2"
                text: qsTr("Overview")
                anchors.left: parent.left
                anchors.top: parent.top
                font.pixelSize: 14
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                anchors.leftMargin: 12
                anchors.topMargin: 5
                anchors.verticalCenterOffset: 1
                font.weight: Font.DemiBold
                font.family: "Titillium Web"
            }

            Keystats {
                id: keyStats
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: overview.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 0
                anchors.bottomMargin: 10
                anchors.rightMargin: 10
                anchors.leftMargin: 10
            }
        }
    }
}
