// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtQuick.Controls.Basic

Rectangle {
    id: mainView
    color: "#404040"

    // Reset the set colors to transparent
    // so theme color is used instead.
    function resetCustomSetColors() {
        set1.color = Qt.rgba(0, 0, 0, 0)
        set2.color = Qt.rgba(0, 0, 0, 0)
        set3.color = Qt.rgba(0, 0, 0, 0)
        set4.color = Qt.rgba(0, 0, 0, 0)
        set1.borderColor = Qt.rgba(0, 0, 0, 0)
        set2.borderColor = Qt.rgba(0, 0, 0, 0)
        set3.borderColor = Qt.rgba(0, 0, 0, 0)
        set4.borderColor = Qt.rgba(0, 0, 0, 0)
        set1.borderWidth = -1
        set2.borderWidth = -1
        set3.borderWidth = -1
        set4.borderWidth = -1
    }

    function resetCustomGraphTheme() {
        myTheme.axisYLabelFont.family = Qt.application.font.family
        graphsView.gridSmoothing = 1
    }

    FontLoader {
        id: customFont
        source: "images/Sevillana-Regular.ttf"
    }

    Text {
        id: graphToolbarTitle
        anchors.horizontalCenter: graphToolbar.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        text: "Graph Theme"
        font.pixelSize: 12
        color: "#ffffff"
    }

    Row {
        id: graphToolbar
        anchors.top: graphToolbarTitle.bottom
        anchors.margins: 10
        anchors.left: parent.left
        anchors.leftMargin: 60
        spacing: 10
        Button {
            text: "Automatic"
            onClicked: {
                mainView.resetCustomGraphTheme()
                graphsView.theme = myTheme
                myTheme.colorScheme = GraphsTheme.ColorScheme.Automatic
            }
        }
        Button {
            text: "Dark"
            onClicked: {
                mainView.resetCustomGraphTheme()
                graphsView.theme = myTheme
                myTheme.colorScheme = GraphsTheme.ColorScheme.Dark
            }
        }
        Button {
            text: "Light"
            onClicked: {
                mainView.resetCustomGraphTheme()
                graphsView.theme = myTheme
                myTheme.colorScheme = GraphsTheme.ColorScheme.Light
            }
        }
        Button {
            text: "CustomBlue"
            onClicked: {
                mainView.resetCustomGraphTheme()
                graphsView.theme = customBlueTheme
            }
        }
        Button {
            text: "Customize!"
            onClicked: {
                graphsView.theme = myTheme
                myTheme.backgroundColor = Qt.rgba(Math.random(),
                                                  Math.random(),
                                                  Math.random(), 1.0)
                myTheme.plotAreaBackgroundColor = Qt.rgba(Math.random(),
                                                  Math.random(),
                                                  Math.random(), Math.random())
                myTheme.grid.mainColor = Qt.rgba(Math.random(),
                                                     Math.random(),
                                                     Math.random(), 1.0)
                myTheme.grid.subColor = Qt.rgba(Math.random(),
                                                     Math.random(),
                                                     Math.random(), 1.0)
                graphsView.gridSmoothing = 2 * Math.random()
                myTheme.axisY.mainColor = Qt.rgba(Math.random(), Math.random(),
                                                  Math.random(), 1.0)
                myTheme.axisX.mainColor = Qt.rgba(Math.random(), Math.random(),
                                                  Math.random(), 1.0)
                myTheme.axisY.subColor = Qt.rgba(Math.random(), Math.random(),
                                                  Math.random(), 1.0)
                myTheme.axisX.subColor = Qt.rgba(Math.random(), Math.random(),
                                                  Math.random(), 1.0)
                myTheme.labelBackgroundColor = Qt.rgba(Math.random(),
                                                   Math.random(),
                                                   Math.random(), 1.0)
                myTheme.axisYLabelFont.family = customFont.font.family
                graphsView.axisYSmoothing = 2 * Math.random()
                // Generic label color, should affect titles and axes
                myTheme.labelTextColor = Qt.rgba(Math.random(),
                                                 Math.random(),
                                                 Math.random(), 1.0)
                // Override one axis, but not its title
                myTheme.axisX.labelTextColor = Qt.rgba(Math.random(),
                                                   Math.random(),
                                                   Math.random(), 1.0)
            }
        }
    }

    Text {
        id: seriesToolbarTitle
        anchors.horizontalCenter: seriesToolbar.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        text: "Series Theme"
        font.pixelSize: 12
        color: "#ffffff"
    }

    Row {
        id: seriesToolbar
        anchors.top: seriesToolbarTitle.bottom
        anchors.margins: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        spacing: 10
        Button {
            text: "Theme1"
            onClicked: {
                mainView.resetCustomSetColors()
                mySeries.seriesColors = []
                mySeries.borderColors = []
                myTheme.theme = GraphsTheme.Theme.QtGreen
            }
        }
        Button {
            text: "Theme2"
            onClicked: {
                mainView.resetCustomSetColors()
                mySeries.seriesColors = []
                mySeries.borderColors = []
                myTheme.theme = GraphsTheme.Theme.QtGreenNeon
            }
        }
        Button {
            text: "CustomBW"
            onClicked: {
                mainView.resetCustomSetColors()
                mySeries.seriesColors = ["#444444", "#555555", "#666666", "#777777"]
                mySeries.borderColors = ["#888888", "#999999", "#aaaaaa", "#bbbbbb"]
            }
        }
        Button {
            text: "Customize!"
            onClicked: {
                set1.color = Qt.rgba(Math.random(), Math.random(),
                                     Math.random(), 1.0)
                set2.color = Qt.rgba(Math.random(), Math.random(),
                                     Math.random(), 1.0)
                set3.color = Qt.rgba(Math.random(), Math.random(),
                                     Math.random(), 1.0)
                set4.color = Qt.rgba(Math.random(), Math.random(),
                                     Math.random(), 1.0)
                // Borders a bit lighter/darker than main colors
                var borderMod = 0.5 + Math.random()
                set1.borderColor = Qt.lighter(set1.color, borderMod)
                set2.borderColor = Qt.lighter(set2.color, borderMod)
                set3.borderColor = Qt.lighter(set3.color, borderMod)
                set4.borderColor = Qt.lighter(set4.color, borderMod)
                set1.borderWidth = 2
                set2.borderWidth = 2
                set3.borderWidth = 2
                set4.borderWidth = 2
            }
        }
    }

    GraphsView {
        id: graphsView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: graphToolbar.bottom
        anchors.margins: 20
        marginLeft: 100 // Space for legend
        axisX: BarCategoryAxis {
            categories: ["2007", "2008", "2009", "2010", "2011", "2012"]
            titleVisible: true
            titleText: "Year"
        }
        axisY: ValueAxis {
            subTickCount: 4
            max: 20
            titleVisible: true
            titleText: "Value"
        }
        theme: myTheme
        GraphsTheme {
            id: myTheme
            theme: GraphsTheme.Theme.QtGreen
            axisXLabelFont.pixelSize: 20
            axisYLabelFont.pixelSize: 16
            colorScheme: GraphsTheme.ColorScheme.Automatic
            labelFont.pixelSize: 20
        }
        GraphsTheme {
            id: customBlueTheme
            theme: myTheme.theme
            colorScheme: GraphsTheme.ColorScheme.Dark
            labelFont.pixelSize: 14
            axisXLabelFont.pixelSize: 20
            axisYLabelFont.pixelSize: 14
            backgroundColor: "#303438"
            plotAreaBackgroundColor: "#60000000"
            grid.mainColor: "#405060"
            grid.subColor: "#606060"
            grid.mainWidth: 1.6
            grid.subWidth: 0.6
            axisX.mainColor: "#405060"
            axisX.subColor: "#606060"
            // axisX.labelTextColor: "#50d090"
            axisX.mainWidth: 1.6
            axisX.subWidth: 0.6
            axisY.mainColor: "#405060"
            axisY.subColor: "#606060"
            // axisY.labelTextColor: "#00d090"
            axisY.mainWidth: 1.6
            axisY.subWidth: 0.6
            labelTextColor: "#d0d090" // this should affect all axes
        }
        BarSeries {
            id: mySeries
            BarSet {
                id: set1
                label: "Bob"
                values: [1, 2, 3, 4, 5, 6]
            }
            BarSet {
                id: set2
                label: "Susan"
                values: [5, 1, 2, 4, 1, 7]
            }
            BarSet {
                id: set3
                label: "James"
                values: [4 + 3 * Math.sin(
                        fA.elapsedTime), 5 + 5 * Math.sin(fA.elapsedTime), 6 + 2 * Math.sin(
                        fA.elapsedTime), 13 + 2 * Math.sin(fA.elapsedTime), 4 + 3 * Math.sin(
                        fA.elapsedTime), 8 + 4 * Math.sin(fA.elapsedTime)]
            }
            BarSet {
                id: set4
                label: "Frank"
                values: [3, 3, 5, 8, 4, 2]
            }
        }
        FrameAnimation {
            id: fA
            running: true
        }
    }
    Column {
        id: legendColumn
        anchors.left: graphsView.left
        anchors.leftMargin: 20
        anchors.verticalCenter: graphsView.verticalCenter
        spacing: 2
        Repeater {
            model: mySeries.legendData.length
            Row {
                spacing: 6
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 16
                    width: 16
                    color: mySeries.legendData[index].color
                    border.color: graphsView.theme.grid.mainColor
                    border.width: 2
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: mySeries.legendData[index].label
                    font.pixelSize: 16
                    color: graphsView.theme.axisY.labelTextColor
                }
            }
        }
    }

    MouseArea {
        anchors.fill: graphsView
        onClicked: fA.paused = !fA.paused
    }
}
