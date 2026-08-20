// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick 6.5
import QtGraphs
import StocQt

Rectangle {
    id: chart

    property var startDate: new Date()
    property var endDate: new Date()
    property string timeFrame: "1M"
    property int currentLiveIndex: -1

    property alias highVisible: highSeries.visible
    property alias lowVisible: lowSeries.visible
    property alias openVisible: openSeries.visible
    property alias closeVisible: closeSeries.visible

    onTimeFrameChanged: update()
    function update(){
        volumeModel.clear()
        highModel.clear()
        lowModel.clear()
        openModel.clear()
        closeModel.clear()
        updateHistory()
        currentLiveIndex = -1
        updateLive()
    }

    function updateStartDate() {
        const  timeFrames = new Map([
                                    ["1w", 7],
                                    ["1M", 30],
                                    ["3M", 90],
                                    ["6M",180]])

        let tf = timeFrames.get(timeFrame)
        endDate =  StockEngine.useLiveData()? new Date() : new Date(StockEngine.stockModel.historyDate(0) + 1)
        startDate = new Date(endDate.getTime())
        startDate.setDate(endDate.getDate() -tf);
    }

    function updateHistory(){
        updateStartDate()
        let startPoint = StockEngine.stockModel.indexOf(startDate)
        let totalPoints = StockEngine.stockModel.historyCount()

        let width = startPoint / 50
        for (let i = 0; i < totalPoints; i++) {
            let epochInDays = StockEngine.stockModel.historyDate(i, false) / 86400
            appendSurfacePoint(openModel, width, epochInDays, StockEngine.stockModel.openPrice(i))
            appendSurfacePoint(closeModel,width, epochInDays, StockEngine.stockModel.closePrice(i))
            appendSurfacePoint(highModel,width, epochInDays, StockEngine.stockModel.highPrice(i))
            appendSurfacePoint(lowModel,width, epochInDays, StockEngine.stockModel.lowPrice(i))
        }

        for (let j = startPoint - 1; j >= 0; j--) {
            let date = new Date(StockEngine.stockModel.historyDate(j)).toLocaleDateString(Locale.ShortFormat)
            volumeModel.append({"row": StockEngine.stockModel.name(),
                                   "column": date,
                                   "value": StockEngine.stockModel.volume(j) / 1000000})
        }

        historyGraph.axisX.min = (startDate.getTime() / 1000).toFixed() / 86400
        historyGraph.axisX.max = (endDate.getTime() / 1000).toFixed() / 86400
    }

    function appendSurfacePoint(listModel, width, time, value) {
        listModel.append({"row": 0,
                         "column": time,
                         "value": value})
        listModel.append({"row": width,
                         "column": time,
                         "value": value})
    }

    function updateLive(){
        currentLiveIndex = liveSeries.selectedItem
        priceModel.clear()
        for (let i= 0; i < StockEngine.stockModel.quoteCount(); i++){
            let date = new Date(StockEngine.stockModel.quoteTime(i))
            let dayS = date.getHours() * 3600 + date.getMinutes() * 60 + date.getSeconds()
            priceModel.insert(0,{"row" : 0,
                                  "column": dayS,
                                  "value" : StockEngine.stockModel.price(i)});
        }
    }

    ListModel {
        id: volumeModel
    }
    ListModel {
        id: highModel
    }
    ListModel {
        id: lowModel
    }
    ListModel {
        id: openModel
    }
    ListModel {
        id: closeModel
    }
    ListModel {
        id: priceModel
    }

    Surface3D {
        id: historyGraph
        clip: true
        visible: true
        width: parent.width
        height: parent.height
        cameraZoomLevel: 170
        maxCameraZoomLevel:  400
        minCameraZoomLevel:  80

        axisX: Value3DAxis {
            autoAdjustRange: true
            title: chart.startDate.toDateString() + " - " + chart.endDate.toDateString()
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
                id: highSeries
                visible: true
                baseColor: "green"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel High:@yLabel$"
                ItemModelSurfaceDataProxy {
                    itemModel: highModel
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "value"
                }
            },
            Surface3DSeries {
                id: lowSeries
                visible: true
                baseColor: "red"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Low:@yLabel$"
                ItemModelSurfaceDataProxy {
                    itemModel: lowModel
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "value"
                }
            },
            Surface3DSeries {
                id: openSeries
                visible: true
                baseColor: "yellow"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Open:@yLabel$"
                ItemModelSurfaceDataProxy {
                    itemModel: openModel
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "value"
                }
            },
            Surface3DSeries {
                id: closeSeries
                visible: true
                baseColor: "blue"
                shading: Surface3DSeries.Shading.Flat
                drawMode: Surface3DSeries.DrawSurface
                itemLabelFormat: "Time: @xLabel Close:@yLabel$"
                ItemModelSurfaceDataProxy {
                    itemModel: closeModel
                    rowRole: "row"
                    columnRole: "column"
                    yPosRole: "value"
                }
            }
        ]
    }

    Bars3D{
        id: volumeGraph
        clip: true
        width: parent.width
        height: parent.height
        visible: false
        cameraZoomLevel: 140
        maxCameraZoomLevel:  400
        minCameraZoomLevel:  80
        orthoProjection: true

        valueAxis: Value3DAxis {
            labelFormat: "%.1f M"
            title: "Volume"
            titleVisible: true
        }
        columnAxis: Category3DAxis {
            title: chart.startDate.toDateString() + " - " + chart.endDate.toDateString()
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
                id: volumeSeries
                itemLabelFormat: "Date:@colLabel : Volume: @valueLabel"
                ItemModelBarDataProxy {
                    itemModel: volumeModel
                    rowRole: "row"
                    columnRole: "column"
                    valueRole: "value"
                }
            }
        ]
    }

    Scatter3D {
        id: liveGraph
        clip: true
        width: parent.width
        height: parent.height
        visible: false
        cameraZoomLevel: 140
        maxCameraZoomLevel:  400
        minCameraZoomLevel:  80
        orthoProjection: true

        axisX: Value3DAxis {
            title: chart.startDate.toDateString() + " - " + chart.endDate.toDateString()
            formatter: TimeFormatter {
                id: minuteFormatter
                selectionFormat: "hh:mm:ss"
                epochFormat: TimeFormatter.S
            }
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
            Scatter3DSeries {
                id: liveSeries
                baseColor: "green"
                meshSmooth: true
                itemLabelFormat: "@xLabel: @yLabel$"
                ItemModelScatterDataProxy {
                    itemModel: priceModel
                    xPosRole: "column"
                    yPosRole: "value"
                    zPosRole: "row"
                    onItemCountChanged: {
                        let i = liveSeries.selectedItem
                        liveSeries.selectedItem = chart.currentLiveIndex
                    }
                }
            }
        ]
    }

    states : [
        State {
            name: "History"
        },
        State {
            name: "Volume"
            PropertyChanges {
                target: volumeGraph
                visible: true
            }
            PropertyChanges {
                target: historyGraph
                visible: false
            }
        },
        State {
            name: "Live"
            PropertyChanges {
                target: liveGraph
                visible: true
            }
            PropertyChanges {
                target: historyGraph
                visible: false
            }
        }
    ]
}
