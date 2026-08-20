// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQml.Models
import QtGraphs
import qtgraphscsv

Window {
    id: mainWindow
    width: 1200
    height: 720
    visible: true
    title: qsTr("QtGraphs with CSV Demo")

    Binding {
        target: Units
        property: "window"
        value: mainWindow
    }

    property var seriesBaseColors: ["#99A500", "#361EAB", "#1F9B5D", "#BC7A19", "#B00F36", "#5A1FAA", "#077947", "#4B523C"]
    property var allSeriesColors: ["#99A500", "#361EAB", "#1F9B5D", "#BC7A19", "#B00F36", "#5A1FAA", "#077947", "#4B523C"]

    property var activeTheme: themes.lightTheme

    Item {
        id: themes
        property alias lightTheme: lightTheme
        property alias graphsLightTheme: lightTheme.graphsTheme
        property alias darkTheme: darkTheme
        property alias graphsDarkTheme: darkTheme.graphsTheme

        Item {
            id: lightTheme
            property color primaryTextColor: "#2D2D2D"
            property color secondaryTextColor: "#595959"
            property color backgroundColor: "#FCFCFC"
            property color selectionColor: "#E3E3E3"
            property color borderColor: "#CDCDCD"
            property color legendLabelTextColor: "#FCFCFC"
            property alias graphsTheme: graphsLightTheme

            GraphsTheme {
                id: graphsLightTheme

                backgroundColor: "transparent"
                colorScheme: GraphsTheme.ColorScheme.Light
                labelTextColor: lightTheme.primaryTextColor
            }
        }

        Item {
            id: darkTheme
            property color primaryTextColor: "#F2F2F2"
            property color secondaryTextColor: "#AEAEAE"
            property color backgroundColor: "#1F1F1F"
            property color selectionColor: "#353535"
            property color borderColor: "#3F3F3F"
            property color legendLabelTextColor: "#FCFCFC"
            property alias graphsTheme: graphsDarkTheme

            GraphsTheme {
                id: graphsDarkTheme

                backgroundColor: "transparent"
                colorScheme: GraphsTheme.ColorScheme.Dark
                labelTextColor: darkTheme.primaryTextColor
            }
        }
        Component.onCompleted: {
            // Generate more colors for the series
            for (let i = 0; i < mainWindow.seriesBaseColors.length; ++i) {
                mainWindow.allSeriesColors[i + 8] = Qt.lighter(mainWindow.seriesBaseColors[i]);
                mainWindow.allSeriesColors[i + 16] = Qt.darker(mainWindow.seriesBaseColors[i]);
                mainWindow.allSeriesColors[i + 24] = Qt.lighter(Qt.lighter(mainWindow.seriesBaseColors[i]));
                mainWindow.allSeriesColors[i + 32] = Qt.darker(Qt.darker(mainWindow.seriesBaseColors[i]));
            }
            graphsDarkTheme.seriesColors = mainWindow.allSeriesColors;
            graphsLightTheme.seriesColors = mainWindow.allSeriesColors;
        }
    }

    Rectangle {
        id: background
        property alias dataModel: dataView.dataModel

        anchors.fill: parent
        color: mainWindow.activeTheme.backgroundColor

        Item {
            id: dataView

            property alias dataModel: dataModel
            anchors.top: background.top
            anchors.bottom: background.bottom
            anchors.left: background.left
            implicitWidth: 400 * Units.px
            implicitHeight: 719 * Units.px

            CustomTableView {
                id: customTableView

                implicitHeight: 320 * Units.px
                tableViewModel: dataModel
                horizontalHeaderViewModel: hHeaderModel
                anchors.top: dataView.top
                anchors.topMargin: 40
                anchors.left: dataView.left
                anchors.right: dataView.right
                anchors.leftMargin: 24 * Units.px
                anchors.bottom: dataView.bottom
                anchors.bottomMargin: 294 * Units.px

                selectionColor: mainWindow.activeTheme.selectionColor
                backgroundColor: mainWindow.activeTheme.backgroundColor
                primaryTextColor: mainWindow.activeTheme.primaryTextColor
                secondaryTextColor: mainWindow.activeTheme.secondaryTextColor
                borderColor: mainWindow.activeTheme.borderColor
                title: qsTr("Medals")

                Connections {
                    target: customTableView.dataSelectionModel
                    function onSelectionChanged(selected, deselected) {
                        if (customTableView.dataSelectionModel.selectedIndexes.length < 1) {
                            graphView.graphsItem.clearGraph();
                            return;
                        }

                        const first = customTableView.mainTableView.cellAtIndex(customTableView.dataSelectionModel.selectedIndexes[0]);
                        const last = customTableView.mainTableView.cellAtIndex(
                            customTableView.dataSelectionModel.selectedIndexes[
                                customTableView.dataSelectionModel.selectedIndexes.length - 1]);

                        graphView.graphsItem.updateModelMapper(first, last);

                        const firstCategoryIndex = graphView.graphsItem.modelMapper.first;
                        const lastCategoryIndex = graphView.graphsItem.modelMapper.count;

                        graphView.graphsItem.categoryAxis.categories = customTableView.extractBarSetGategories(
                            firstCategoryIndex, lastCategoryIndex);
                    }
                }
            }

            SelectionRectangle {
                id: selectionRectangle
                target: customTableView.mainTableView
            }

            ListModel {
                id: hHeaderModel
            }

            LegendItem {
                id: legendItem
                anchors.top: customTableView.bottom
                anchors.left: dataView.left
                anchors.leftMargin: 24 * Units.px
                anchors.topMargin: 70 * Units.px
                labelTextColor: mainWindow.activeTheme.legendLabelTextColor
                titleTextColor: mainWindow.activeTheme.secondaryTextColor
                series: graphView.graphsItem.series
            }

            CsvDataModel {
                id: dataModel
                onModelReset: () => {
                    const first = graphView.graphsItem.modelMapper.first;
                    const count = graphView.graphsItem.modelMapper.count;
                    graphView.graphsItem.categoryAxis.categories = customTableView.extractBarSetGategories(first, count);

                    customTableView.fillHorizontalHeaderModel(dataModel.columnCount());
                }

                Component.onCompleted: () => {
                    dataModel.readCsv("qrc:/data/medals.csv");
                }
            }
        }

        Item {
            id: graphView

            property alias graphsItem: graphsItem

            anchors.left: dataView.right
            anchors.right: background.right
            anchors.top: background.top
            anchors.bottom: background.bottom
            width: 800 * Units.px

            Graph {
                id: graphsItem

                anchors.fill: graphView

                chartView.marginLeft: 71 * Units.px
                chartView.marginRight: 43 * Units.px
                chartView.marginTop: 47 * Units.px
                chartView.marginBottom: 80 * Units.px

                theme: mainWindow.activeTheme.graphsTheme
                labelDelegateTextColor: mainWindow.activeTheme.secondaryTextColor
            }
        }
    }

    Component.onCompleted: () => {
        switch (Application.styleHints.colorScheme) {
        case Qt.Light:
            activeTheme = themes.lightTheme;
            break;
        case Qt.Dark:
            activeTheme = themes.darkTheme;
            break;
        }
        graphView.graphsItem.modelMapper.model = background.dataModel;
    }
}
