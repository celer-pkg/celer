// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick 6.5
import QtGraphs
import StocQt

Rectangle {
    id: favoriteChart

    property var startDate: new Date()
    property var endDate: new Date()
    property string timeFrame: "1M"

    onTimeFrameChanged: updateData()

    function updateData() {
        updateStartDate()
        const priceModels = [priceModel1, priceModel2, priceModel3, priceModel4, priceModel5]
        priceModels.forEach((model => model.clear()))
        volumeModel.clear()
        for (let i = 0; i < StockEngine.favoritesModel.count(); i++) {
            updateStock(i, priceModels[i])
        }
    }

    function updateStartDate() {
        const timeFrames = new Map([
                                    ["1w", 7],
                                    ["1M", 30],
                                    ["3M", 90],
                                    ["6M",180]])
        let tf = timeFrames.get(timeFrame)

        if (StockEngine.favoritesModel.count() === 0) {
            endDate = new Date()
        } else {
            let stock = StockEngine.favoritesModel.atIndex(0)
            endDate =  StockEngine.useLiveData()? new Date() : new Date(stock.historyDate(0) + 1)
        }

        startDate = new Date(endDate.getTime())
        startDate.setDate(endDate.getDate() -tf);
        volumeGraph.barSpacing.height = tf / 10
    }

    function updateStock(index, priceModel){
        let stock = StockEngine.favoritesModel.atIndex(index)
        let startPoint = stock.indexOf(startDate)
        let totalPoints = stock.historyCount()
        let width = startPoint / 30

        for (let i = 0; i < totalPoints; i++) {
            let epochInDays = stock.historyDate(i, false) / 86400
            priceModel.append({"column": epochInDays,
                                  "row": index * width,
                                  "close": stock.closePrice(i)})
            priceModel.append({"column": epochInDays,
                                  "row": (index * width) + width - (width / 10),
                                  "close": stock.closePrice(i)})
        }

        for (let j = startPoint - 1; j >= 0; j--){
            let date = new Date(stock.historyDate(j)).toLocaleDateString(Locale.ShortFormat)
            volumeModel.append({"row": stock.stockId(),
                                   "column": date,
                                   "volume": stock.volume(j) / 1000000})
        }

        priceGraph.axisX.min = (startDate.getTime() / 1000).toFixed() / 86400
        priceGraph.axisX.max = (endDate.getTime() / 1000).toFixed() / 86400
    }

    ListModel {
        id: volumeModel
    }
    ListModel {
        id: priceModel1
    }
    ListModel {
        id: priceModel2
    }
    ListModel {
        id: priceModel3
    }
    ListModel {
        id: priceModel4
    }
    ListModel {
        id: priceModel5
    }

    Bars3D {
        id: volumeGraph;
        clip: true
        width: parent.width
        height: parent.height
        visible: false
        multiSeriesUniform: true
        cameraZoomLevel: 150
        maxCameraZoomLevel:  400
        minCameraZoomLevel:  80
        orthoProjection: true

        valueAxis: Value3DAxis {
            labelFormat: "%.1f M"
            title: "Volume"
            titleVisible: true
        }
        columnAxis: Category3DAxis {
            id: columnAxisId
            title: favoriteChart.startDate.toDateString() + " - " + favoriteChart.endDate.toDateString()
            titleVisible: true
        }
        rowAxis: Category3DAxis {
            id: rowAxisId
            title: "Stock"
            titleVisible: true
        }

        ambientLightStrength: 1

        theme: GraphsTheme {
            theme: GraphsTheme.Theme.QtGreen
            backgroundColor: "#101010"
            backgroundVisible: false
            grid.mainColor: Qt.rgba(0.2,0.2,0.2,1)
            labelTextColor: "white"
            labelBackgroundColor: "black"
            labelFont.pointSize: 9
            labelFont.family: "Roboto"
        }

        seriesList: [
            Bar3DSeries {
                id: volume
                baseColor: "red"
                itemLabelFormat: "@rowLabel Date:@colLabel: Volume:@valueLabel"
                ItemModelBarDataProxy {
                    itemModel: volumeModel
                    rowRole: "row"
                    columnRole: "column"
                    valueRole: "volume"
                }
                rowColors: [
                    Color { color: "red"},
                    Color { color: "green"},
                    Color { color: "blue"},
                    Color { color: "yellow"},
                    Color { color: Qt.rgba(1,0,1,1)}
                ]
            }
        ]
    }

    Surface3D {
        id: priceGraph
        clip: true
        width: parent.width
        height: parent.height
        visible: true
        cameraZoomLevel: 180
        maxCameraZoomLevel:  400
        minCameraZoomLevel:  80

        axisX: Value3DAxis {
            autoAdjustRange: true
            title: favoriteChart.startDate.toDateString() + " - " + favoriteChart.endDate.toDateString()
            formatter: TimeFormatter {
                id: monthFormatter
                selectionFormat: "dd-MM-yyyy"
                epochFormat: TimeFormatter.Day
            }

            labelFormat: "dd-MM-yyyy"
            titleVisible: true
        }

        axisZ: Value3DAxis {
            segmentCount: 1
        }

        ambientLightStrength: 1

        theme: GraphsTheme {
            theme: GraphsTheme.Theme.QtGreen
            backgroundColor: "#101010"
            backgroundVisible: false
            grid.mainColor: Qt.rgba(0.2,0.2,0.2,1)
            labelTextColor: "white"
            labelBackgroundColor: "black"
            labelFont.pointSize: 9
            labelFont.family: "Roboto"
        }

        seriesList: [
            Surface3DSeries {
                id: price1
                baseColor: "red"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Price:@yLabel $"
                ItemModelSurfaceDataProxy {
                    itemModel: priceModel1
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "close"
                }
            },
            Surface3DSeries {
                id: price2
                baseColor: "green"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Price:@yLabel $"
                ItemModelSurfaceDataProxy {
                    itemModel: priceModel2
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "close"
                }
            },
            Surface3DSeries {
                id: price3
                baseColor: "blue"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Price:@yLabel $"
                ItemModelSurfaceDataProxy {
                    itemModel: priceModel3
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "close"
                }
            },
            Surface3DSeries {
                id: price4
                baseColor: "yellow"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Price:@yLabel $"
                ItemModelSurfaceDataProxy {
                    itemModel: priceModel4
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "close"
                }
            },
            Surface3DSeries {
                id: price5
                baseColor: "purple"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Price:@yLabel $"
                ItemModelSurfaceDataProxy {
                    itemModel: priceModel5
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "close"
                }
            }
        ]
    }

    states : [
        State {
            name: "Price"
        },
        State {
            name: "Volume"
            PropertyChanges {
                target: volumeGraph
                visible: true
            }
            PropertyChanges {
                target: priceGraph
                visible: false
            }
        }
    ]

}
