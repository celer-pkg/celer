// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import QtGraphs

Item {
    id: settingsitem

    property alias settingsstate: settingsgraph.state
    property PieSimulator piesimulator
    property BarSimulator barsimulator
    property LineSimulator linesimulator

    visible: opacity > 0.5

    //! [settingsview]
    // Graph detail
    BigGraphItem {
        id: settingsgraph
        width: GlobalSettings.settingsgraphwidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: GlobalSettings.graphdetailmargin
        radius: GlobalSettings.defaultradius
        opacity: parent.opacity
        piesimulator: settingsitem.piesimulator
        barsimulator: settingsitem.barsimulator
        linesimulator: settingsitem.linesimulator
    }

    // Settings pane
    StackLayout {
        id: settingspane
        //! [settingsview]
        anchors.left: settingsgraph.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: GlobalSettings.doublespacing

        currentIndex: bar.currentIndex

        Rectangle {
            radius: GlobalSettings.defaultradius
            color: "#80181818"
            border.width: 1
            border.color: "#EFEFEF"

            // General Settings
            ColumnLayout {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: GlobalSettings.doublemargin * 1.5
                anchors.leftMargin: GlobalSettings.doublemargin
                anchors.rightMargin: GlobalSettings.doublemargin
                spacing: GlobalSettings.halfspacing

                Label {
                    Layout.fillWidth: true
                    text: "Theme Settings"
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize22px
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 2
                    color: "#FCFCFC"
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: "Global theme preset"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsComboBox {
                        Layout.alignment: Qt.AlignRight

                        currentIndex: 5

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
                            // Reset color style, in case we have overridden it in 3D graph settings
                            GlobalSettings.colorstyle = GraphsTheme.ColorStyle.Uniform
                            // Reset 2D custom color button, if it is on
                            GlobalSettings.activegraph.customcolors = false
                            switch (currentIndex) {
                            case 0:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.QtGreen
                                break
                            case 1:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.QtGreenNeon
                                break
                            case 2:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.MixSeries
                                break
                            case 3:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.OrangeSeries
                                break
                            case 4:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.YellowSeries
                                break
                            case 5:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.BlueSeries
                                break
                            case 6:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.PurpleSeries
                                break
                            case 7:
                                GlobalSettings.theme.theme = GraphsTheme.Theme.GreySeries
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
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: true
                        onCheckedChanged: GlobalSettings.theme.colorScheme
                                          = (checked ? GraphsTheme.ColorScheme.Dark
                                                    : GraphsTheme.ColorScheme.Light)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: "Background visibility"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.theme.plotAreaBackgroundVisible
                        onCheckedChanged: GlobalSettings.theme.plotAreaBackgroundVisible
                                          = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate !== "ShowPie2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Grid visibility"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.theme.gridVisible
                        onCheckedChanged: GlobalSettings.theme.gridVisible = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Label visibility"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.theme.labelsVisible
                        onCheckedChanged: GlobalSettings.theme.labelsVisible = checked
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: !GlobalSettings.hidelivedata
                    text: "Data Settings"
                    color: "#EFEFEF"
                    font.pixelSize: GlobalSettings.fontsize22px
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: !GlobalSettings.hidelivedata
                    height: 2
                    color: "#FCFCFC"
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: !GlobalSettings.hidelivedata

                    Label {
                        Layout.fillWidth: true
                        text: "Live data"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.livedata
                        onCheckedChanged: GlobalSettings.livedata = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: !GlobalSettings.hidelivedata

                    Label {
                        Layout.fillWidth: true
                        text: "Update interval"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 1.0
                        to: 100.0
                        value: GlobalSettings.updateinterval
                        stepSize: 1.0
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.updateinterval = value
                        }
                    }
                }
            }
        }

        Rectangle {
            radius: GlobalSettings.defaultradius
            color: "#80181818"
            border.width: 1
            border.color: "#EFEFEF"

            // Graph Specific Settings
            ColumnLayout {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: GlobalSettings.doublemargin * 1.5
                anchors.leftMargin: GlobalSettings.doublemargin
                anchors.rightMargin: GlobalSettings.doublemargin
                spacing: GlobalSettings.halfspacing

                // All 2D graphs
                RowLayout {
                    Layout.fillWidth: true
                    visible: !GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Custom colors"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.customcolors
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.customcolors = checked
                            if (root.settingsstate === "ShowPie2D")
                                GlobalSettings.activegraph.updateSliceVisuals()
                        }
                    }
                }

                // BarSeries and SplineSeries
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"
                             || root.settingsstate === "ShowArea2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Grid smoothing"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 5.0
                        value: GlobalSettings.activegraph.gridSmoothing
                        stepSize: 0.5
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.axisXSmoothing = value
                            GlobalSettings.activegraph.axisYSmoothing = value
                            GlobalSettings.activegraph.gridSmoothing = value
                            GlobalSettings.activegraph.shadowSmoothing = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"
                             || root.settingsstate === "ShowArea2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Show grid shadows"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.shadowVisible
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.shadowVisible = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: (root.settingsstate === "ShowBar2D"
                             || root.settingsstate === "ShowArea2D")
                             && GlobalSettings.activegraph.shadowVisible

                    Label {
                        Layout.fillWidth: true
                        text: "Shadow width"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 5.0
                        value: GlobalSettings.activegraph.shadowBarWidth
                        stepSize: 0.5
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.shadowBarWidth = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: (root.settingsstate === "ShowBar2D"
                             || root.settingsstate === "ShowArea2D")
                             && GlobalSettings.activegraph.shadowVisible

                    Label {
                        Layout.fillWidth: true
                        text: "Shadow offset"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: -10.0
                        to: 10.0
                        value: GlobalSettings.activegraph.shadowXOffset
                        stepSize: 1.0
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.shadowXOffset = value
                            GlobalSettings.activegraph.shadowYOffset = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"
                             || root.settingsstate === "ShowArea2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Horizontal orientation"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.orientation === Qt.Horizontal
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.orientation
                                          = (checked ? Qt.Horizontal : Qt.Vertical)
                        }
                    }
                }

                // BarSeries only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Stacked bars"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].barsType
                                 === BarSeries.BarsType.Stacked
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].barsType
                                          = (checked ? BarSeries.BarsType.Stacked
                                                     : BarSeries.BarsType.Groups)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Bar width"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.1
                        to: 1.0
                        value: GlobalSettings.activegraph.seriesList[0].barWidth
                        stepSize: 0.1
                        onValueChanged:{
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].barWidth = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Labels visible"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].labelsVisible
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].labelsVisible
                                          = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar2D"
                             && GlobalSettings.activegraph.seriesList[0].labelsVisible

                    Label {
                        Layout.fillWidth: true
                        text: "Label angle"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 360.0
                        value: GlobalSettings.activegraph.seriesList[0].labelsAngle
                        stepSize: 5.0
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].labelsAngle = value
                        }
                    }
                }

                // SplineSeries only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowArea2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Line width"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.1
                        to: 5.0
                        value: GlobalSettings.activegraph.series.width
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.series.width = value
                        }
                    }
                }

                // PieSeries only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowPie2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Labels outside"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.labelposition === PieSlice.LabelPosition.Outside
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.labelposition
                                          = (checked ? PieSlice.LabelPosition.Outside
                                                     : PieSlice.LabelPosition.InsideHorizontal)
                            GlobalSettings.activegraph.updateSliceVisuals()
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowPie2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Exploded"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.explode
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.explode = checked
                            GlobalSettings.activegraph.updateSliceVisuals()
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowPie2D"

                    Label {
                        Layout.fillWidth: true
                        text: "Label hide angle"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 360.0
                        value: GlobalSettings.activegraph.series.angleSpanVisibleLimit
                        stepSize: 1.0
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.series.angleSpanVisibleLimit = value
                            GlobalSettings.activegraph.updateSliceVisuals()
                        }
                    }
                }

                // All 3D graphs
                RowLayout {
                    Layout.fillWidth: true
                    visible: GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Custom colors"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GraphsTheme.ColorStyle.RangeGradient === GlobalSettings.colorstyle
                        onCheckedChanged: {
                            GlobalSettings.colorstyle = (checked
                                                         ? GraphsTheme.ColorStyle.RangeGradient
                                                         : GraphsTheme.ColorStyle.Uniform)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Orthographic"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.orthoProjection
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.orthoProjection = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Shader grid"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.gridLineType
                                 === Graphs3D.GridLineType.Shader
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.gridLineType = (checked
                                          ? Graphs3D.GridLineType.Shader
                                          : Graphs3D.GridLineType.Geometry)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible:  GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Margin"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: -1.0
                        to: 1.0
                        value: GlobalSettings.activegraph.margin
                        stepSize: 1.0
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.margin = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible:  GlobalSettings.is3dgraph

                    Label {
                        Layout.fillWidth: true
                        text: "Light strength"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.1
                        to: 10.0
                        value: GlobalSettings.activegraph.lightStrength
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.lightStrength = value
                        }
                    }
                }

                // Bars3D only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Relative bar spacing"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.barSpacingRelative
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.barSpacingRelative = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Bar spacing"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 1.0
                        value: GlobalSettings.activegraph.barSpacing.x
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.barSpacing = Qt.size(value, value)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Bar thickness"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.5
                        to: 2.0
                        value: GlobalSettings.activegraph.barThickness
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.barThickness = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowBar3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Series margin"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 0.9
                        value: GlobalSettings.activegraph.barSeriesMargin.x
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.barSeriesMargin
                                        = Qt.size(value, value)
                        }
                    }
                }

                // Scatter3D only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowScatter3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Smooth mesh"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].meshSmooth
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].meshSmooth
                                          = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowScatter3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Point mesh"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].mesh
                                 === Abstract3DSeries.Mesh.Point
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].mesh
                                          = (checked ? Abstract3DSeries.Mesh.Point
                                                     : Abstract3DSeries.Mesh.Sphere)
                        }
                    }
                }

                // Surface3D only
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Smooth shading"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].shading
                                 === Surface3DSeries.Shading.Smooth
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].shading
                                          = (checked ? Surface3DSeries.Shading.Smooth
                                                     : Surface3DSeries.Shading.Flat)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Lighting"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].lightingMode
                                 === Abstract3DSeries.LightingMode.Shaded
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].lightingMode
                                          = (checked ? Abstract3DSeries.LightingMode.Shaded
                                                     : Abstract3DSeries.LightingMode.Unshaded)
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Show wireframe"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.seriesList[0].drawMode
                                 === Surface3DSeries.DrawSurfaceAndWireframe
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.seriesList[0].drawMode
                                          = (checked ? Surface3DSeries.DrawSurfaceAndWireframe
                                                     : Surface3DSeries.DrawSurface)
                        }
                    }
                }

                // Scatter3D and Surface3D
                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowScatter3D"
                             || root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Polar axes"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsCheckBox {
                        Layout.alignment: Qt.AlignRight
                        checked: GlobalSettings.activegraph.polar
                        onCheckedChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.polar = checked
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowScatter3D"
                             || root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Ambient light strength"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.0
                        to: 1.0
                        value: GlobalSettings.activegraph.ambientLightStrength
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.ambientLightStrength = value
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: root.settingsstate === "ShowScatter3D"
                             || root.settingsstate === "ShowSurface3D"

                    Label {
                        Layout.fillWidth: true
                        text: "Aspect ratio"
                        color: "#EFEFEF"
                        font.pixelSize: GlobalSettings.fontsize14px
                    }

                    SettingsSlider {
                        Layout.alignment: Qt.AlignRight
                        from: 0.5
                        to: 4.0
                        value: GlobalSettings.activegraph.aspectRatio
                        stepSize: 0.1
                        onValueChanged: {
                            if (!GlobalSettings.activegraph)
                                return
                            GlobalSettings.activegraph.aspectRatio = value
                        }
                    }
                }
            }
        }
    }

    TabBar {
        id: bar
        anchors.left: settingspane.left
        anchors.right: settingspane.right
        anchors.top: settingspane.top
        anchors.topMargin: GlobalSettings.defaultspacing
        background: Item {}
        currentIndex: 0

        TabButton {
            text: "General Settings"

            background: Rectangle {
                y: bar.height
                anchors.horizontalCenter: parent.contentItem.horizontalCenter
                width: parent.contentItem.width
                height: 2
                color: parent.hovered || settingspane.currentIndex === 0 ? "#00A1FF"
                                                                         : "transparent"
            }

            contentItem: Label {
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                text: parent.text
                font.pixelSize: GlobalSettings.fontsize24px
                font.bold: true
                color: settingspane.currentIndex === 0 ? "#EFEFEF" : "gray"
            }
        }

        TabButton {
            text: "Graph Settings"

            background: Rectangle {
                y: bar.height
                anchors.horizontalCenter: parent.contentItem.horizontalCenter
                width: parent.contentItem.width
                height: 2
                color: parent.hovered || settingspane.currentIndex === 1 ? "#00A1FF"
                                                                         : "transparent"
            }

            contentItem: Label {
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                text: parent.text
                font.pixelSize: GlobalSettings.fontsize24px
                font.bold: true
                color: settingspane.currentIndex === 1 ? "#EFEFEF" : "gray"
            }
        }
    }
}
