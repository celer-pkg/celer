// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtGraphs
import SingletonTheme

Window {
    width: 1000
    height: 1500
    visible: true

    Item {
        id: mainView
        anchors.fill: parent

        property var activetheme: SingletonTheme.theme

        onActivethemeChanged: {
            switch (activetheme.theme) {
            case GraphsTheme.Theme.QtGreen:
                themecombobox.currentIndex = 0
                break
            case GraphsTheme.Theme.QtGreenNeon:
                themecombobox.currentIndex = 1
                break
            case GraphsTheme.Theme.MixSeries:
                themecombobox.currentIndex = 2
                break
            case GraphsTheme.Theme.OrangeSeries:
                themecombobox.currentIndex = 3
                break
            case GraphsTheme.Theme.YellowSeries:
                themecombobox.currentIndex = 4
                break
            case GraphsTheme.Theme.BlueSeries:
                themecombobox.currentIndex = 5
                break
            case GraphsTheme.Theme.PurpleSeries:
                themecombobox.currentIndex = 6
                break
            case GraphsTheme.Theme.GreySeries:
                themecombobox.currentIndex = 7
                break
            }
        }

        GraphsTheme {
            id: commontheme
            theme: GraphsTheme.Theme.BlueSeries
            labelFont.pointSize: 40
            plotAreaBackgroundVisible: true
            labelsVisible: true
            gridVisible: true
            backgroundVisible: true
            labelTextColor: "green"
        }

        GridLayout {
            id: gridLayout
            columns: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            anchors.top: mainView.top
            anchors.bottom: mainView.bottom
            anchors.left: mainView.left
            anchors.right: mainView.right

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: mainView.activetheme.backgroundColor

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Singleton theme"
                            color: mainView.activetheme.labelTextColor
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignRight
                            checked: mainView.activetheme === SingletonTheme.theme
                            onCheckedChanged: mainView.activetheme = checked
                                              ? SingletonTheme.theme : commontheme
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Global theme preset"
                            color: mainView.activetheme.labelTextColor
                        }

                        ComboBox {
                            id: themecombobox
                            Layout.alignment: Qt.AlignRight

                            currentIndex: 2

                            model: [
                                "QtGreen",
                                "QtGreenNeon",
                                "MixSeries",
                                "OrangeSeries",
                                "YellowSeries",
                                "BlueSeries",
                                "PurpleSeries",
                                "GreySeries"
                            ]

                            onCurrentIndexChanged: {
                                switch (currentIndex) {
                                case 0:
                                    mainView.activetheme.theme = GraphsTheme.Theme.QtGreen
                                    break
                                case 1:
                                    mainView.activetheme.theme = GraphsTheme.Theme.QtGreenNeon
                                    break
                                case 2:
                                    mainView.activetheme.theme = GraphsTheme.Theme.MixSeries
                                    break
                                case 3:
                                    mainView.activetheme.theme = GraphsTheme.Theme.OrangeSeries
                                    break
                                case 4:
                                    mainView.activetheme.theme = GraphsTheme.Theme.YellowSeries
                                    break
                                case 5:
                                    mainView.activetheme.theme = GraphsTheme.Theme.BlueSeries
                                    break
                                case 6:
                                    mainView.activetheme.theme = GraphsTheme.Theme.PurpleSeries
                                    break
                                case 7:
                                    mainView.activetheme.theme = GraphsTheme.Theme.GreySeries
                                    break
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Dark color scheme"
                            color: mainView.activetheme.labelTextColor
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignRight
                            checked: true
                            onCheckedChanged: mainView.activetheme.colorScheme = checked
                                              ? GraphsTheme.ColorScheme.Dark
                                              : GraphsTheme.ColorScheme.Light
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Background visibility"
                            color: mainView.activetheme.labelTextColor
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignRight
                            checked: mainView.activetheme.plotAreaBackgroundVisible
                            onCheckedChanged: mainView.activetheme.plotAreaBackgroundVisible = checked
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Grid visibility"
                            color: mainView.activetheme.labelTextColor
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignRight
                            checked: mainView.activetheme.gridVisible
                            onCheckedChanged: mainView.activetheme.gridVisible = checked
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            Layout.fillWidth: true
                            text: "Label visibility"
                            color: mainView.activetheme.labelTextColor
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignRight
                            checked: mainView.activetheme.labelsVisible
                            onCheckedChanged: mainView.activetheme.labelsVisible = checked
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: "#00000000"

                GraphsView {
                    anchors.fill: parent

                    theme: mainView.activetheme

                    axisX: ValueAxis {}
                    axisY: ValueAxis {}

                    PieSeries {
                        PieSlice { label: "Slice1"; value: 13.5 }
                        PieSlice { label: "Slice2"; value: 10.9 }
                        PieSlice { label: "Slice3"; value: 8.6 }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: "#00000000"

                GraphsView {
                    anchors.fill: parent

                    theme: mainView.activetheme

                    axisX: ValueAxis {}
                    axisY: ValueAxis {}

                    BarSeries {
                        BarSet { id: set1; label: "Set1"; values: [2, 2, 3] }
                        BarSet { id: set2; label: "Set2"; values: [5, 1, 2] }
                        BarSet { id: set3; label: "Set3"; values: [3, 5, 8] }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: "#00000000"

                Surface3D {
                    anchors.fill: parent

                    theme: mainView.activetheme
                    cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh

                    Surface3DSeries {
                        ItemModelSurfaceDataProxy {
                            itemModel: ListModel {
                                ListElement{ row: "1"; column: "1"; y: "1"; }
                                ListElement{ row: "1"; column: "2"; y: "2"; }
                                ListElement{ row: "2"; column: "1"; y: "3"; }
                                ListElement{ row: "2"; column: "2"; y: "4"; }
                            }

                            rowRole: "row"
                            columnRole: "column"
                            yPosRole: "y"
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: "#00000000"

                Scatter3D {
                    anchors.fill: parent

                    theme: mainView.activetheme
                    cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh

                    Scatter3DSeries {
                        ItemModelScatterDataProxy {
                            itemModel: ListModel {
                                ListElement{ x: "1"; y: "2"; z: "3"; }
                                ListElement{ x: "2"; y: "3"; z: "4"; }
                                ListElement{ x: "3"; y: "4"; z: "1"; }
                            }

                            xPosRole: "x"
                            yPosRole: "y"
                            zPosRole: "z"
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                border.color: mainView.activetheme.grid.mainColor
                border.width: 2
                color: "#00000000"

                Bars3D {
                    anchors.fill: parent

                    theme: mainView.activetheme
                    selectionMode: Graphs3D.SelectionFlag.ItemAndRow | Graphs3D.SelectionFlag.Slice
                    cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh

                    Bar3DSeries {
                        ItemModelBarDataProxy {
                            itemModel: ListModel {
                                ListElement{ row: "row 1"; column: "column 1"; value: "1"; }
                                ListElement{ row: "row 1"; column: "column 2"; value: "2"; }
                                ListElement{ row: "row 1"; column: "column 3"; value: "3"; }
                            }

                            rowRole: "row"
                            columnRole: "column"
                            valueRole: "value"
                        }
                    }
                }
            }
        }
    }
}
