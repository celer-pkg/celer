// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Item {
    property var graph: barGraph
    Bars3D {
        id: barGraph
        anchors.fill: parent
        shadowQuality: Graphs3D.ShadowQuality.SoftHigh
        selectionMode: Graphs3D.SelectionFlag.Row | Graphs3D.SelectionFlag.Slice
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            labelBorderVisible: true
            labelFont.pointSize: 35
            labelBackgroundVisible: true
            colorStyle: GraphsTheme.ColorStyle.RangeGradient
            singleHighlightGradient: customGradient

            Gradient {
                id: customGradient
                GradientStop { position: 1.0; color: "#FFFF00" }
                GradientStop { position: 0.0; color: "#808000" }
            }
        }
        barThickness: 0.7
        barSpacing: Qt.size(0.5, 0.5)
        barSpacingRelative: false
        cameraPreset: Graphs3D.CameraPreset.IsometricLeftHigh
        columnAxis: graphAxes.column
        rowAxis: graphAxes.row
        valueAxis: graphAxes.value

        Bar3DSeries {
            id: secondarySeries
            visible: true
            itemLabelFormat: "Expenses, @colLabel, @rowLabel: -@valueLabel"
            baseGradient: secondaryGradient

            ItemModelBarDataProxy {
                id: secondaryProxy
                itemModel: graphData.model
                rowRole: "timestamp"
                columnRole: "timestamp"
                valueRole: "expenses"
                rowRolePattern: /^(\d\d\d\d).*$/
                columnRolePattern: /^.*-(\d\d)$/
                valueRolePattern: /-/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\1"
                multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
            }

            Gradient {
                id: secondaryGradient
                GradientStop { position: 1.0; color: "#FF0000" }
                GradientStop { position: 0.0; color: "#600000" }
            }
        }

        Bar3DSeries {
            id: barSeries
            itemLabelFormat: "Income, @colLabel, @rowLabel: @valueLabel"
            baseGradient: barGradient

            ItemModelBarDataProxy {
                id: modelProxy
                itemModel: graphData.model
                rowRole: "timestamp"
                columnRole: "timestamp"
                valueRole: "income"
                rowRolePattern: /^(\d\d\d\d).*$/
                columnRolePattern: /^.*-(\d\d)$/
                rowRoleReplace: "\\1"
                columnRoleReplace: "\\1"
                multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
            }

            Gradient {
                id: barGradient
                GradientStop { position: 1.0; color: "#00FF00" }
                GradientStop { position: 0.0; color: "#006000" }
            }
        }
    }

    Item {
        id: graphAxes
        property alias column: columnAxis
        property alias row: rowAxis
        property alias value: valueAxis
        property alias total: totalAxis

        // Custom labels for columns, since the data contains abbreviated month names.
        Category3DAxis {
            id: columnAxis
            labels: ["January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October", "November", "December"]
            labelAutoAngle: 30
        }
        Category3DAxis {
            id: totalAxis
            labels: ["Yearly total"]
            labelAutoAngle: 30
        }
        Category3DAxis {
            // For row labels we can use row labels from data proxy, no labels defined for rows.
            id: rowAxis
            labelAutoAngle: 30
        }

        Value3DAxis {
            id: valueAxis
            min: 0
            max: 35
            labelFormat: "%.2f M\u20AC"
            title: "Monthly income"
            labelAutoAngle: 90
        }
    }

    Item {
        id: graphData

        property alias model: dataModel

        property var modelAsJsArray: {
            var arr = [];
            for (var i = 0; i < dataModel.count; i++) {
                var row = dataModel.get(i);
                arr.push({
                             timestamp: row.timestamp,
                             expenses: row.expenses,
                             income: row.income
                         });
            }
            return arr;
        }

        ListModel {
            id: dataModel
            ListElement{ timestamp: "2016-01"; expenses: "-4"; income: "5" }
            ListElement{ timestamp: "2016-02"; expenses: "-5"; income: "6" }
            ListElement{ timestamp: "2016-03"; expenses: "-7"; income: "4" }
            ListElement{ timestamp: "2016-04"; expenses: "-3"; income: "2" }
            ListElement{ timestamp: "2016-05"; expenses: "-4"; income: "1" }
            ListElement{ timestamp: "2016-06"; expenses: "-2"; income: "2" }
            ListElement{ timestamp: "2016-07"; expenses: "-1"; income: "3" }
            ListElement{ timestamp: "2016-08"; expenses: "-5"; income: "1" }
            ListElement{ timestamp: "2016-09"; expenses: "-2"; income: "3" }
            ListElement{ timestamp: "2016-10"; expenses: "-5"; income: "2" }
            ListElement{ timestamp: "2016-11"; expenses: "-8"; income: "5" }
            ListElement{ timestamp: "2016-12"; expenses: "-3"; income: "3" }

            ListElement{ timestamp: "2017-01"; expenses: "-3"; income: "1" }
            ListElement{ timestamp: "2017-02"; expenses: "-4"; income: "2" }
            ListElement{ timestamp: "2017-03"; expenses: "-12"; income: "4" }
            ListElement{ timestamp: "2017-04"; expenses: "-13"; income: "6" }
            ListElement{ timestamp: "2017-05"; expenses: "-14"; income: "11" }
            ListElement{ timestamp: "2017-06"; expenses: "-7"; income: "7" }
            ListElement{ timestamp: "2017-07"; expenses: "-6"; income: "4" }
            ListElement{ timestamp: "2017-08"; expenses: "-4"; income: "15" }
            ListElement{ timestamp: "2017-09"; expenses: "-2"; income: "18" }
            ListElement{ timestamp: "2017-10"; expenses: "-29"; income: "25" }
            ListElement{ timestamp: "2017-11"; expenses: "-23"; income: "29" }
            ListElement{ timestamp: "2017-12"; expenses: "-5"; income: "9" }

            ListElement{ timestamp: "2018-01"; expenses: "-3"; income: "8" }
            ListElement{ timestamp: "2018-02"; expenses: "-8"; income: "14" }
            ListElement{ timestamp: "2018-03"; expenses: "-10"; income: "20" }
            ListElement{ timestamp: "2018-04"; expenses: "-12"; income: "24" }
            ListElement{ timestamp: "2018-05"; expenses: "-10"; income: "19" }
            ListElement{ timestamp: "2018-06"; expenses: "-5"; income: "8" }
            ListElement{ timestamp: "2018-07"; expenses: "-1"; income: "4" }
            ListElement{ timestamp: "2018-08"; expenses: "-7"; income: "12" }
            ListElement{ timestamp: "2018-09"; expenses: "-4"; income: "16" }
            ListElement{ timestamp: "2018-10"; expenses: "-22"; income: "33" }
            ListElement{ timestamp: "2018-11"; expenses: "-16"; income: "25" }
            ListElement{ timestamp: "2018-12"; expenses: "-2"; income: "7" }

            ListElement{ timestamp: "2019-01"; expenses: "-4"; income: "5" }
            ListElement{ timestamp: "2019-02"; expenses: "-4"; income: "7" }
            ListElement{ timestamp: "2019-03"; expenses: "-11"; income: "14" }
            ListElement{ timestamp: "2019-04"; expenses: "-16"; income: "22" }
            ListElement{ timestamp: "2019-05"; expenses: "-3"; income: "5" }
            ListElement{ timestamp: "2019-06"; expenses: "-4"; income: "8" }
            ListElement{ timestamp: "2019-07"; expenses: "-7"; income: "9" }
            ListElement{ timestamp: "2019-08"; expenses: "-9"; income: "13" }
            ListElement{ timestamp: "2019-09"; expenses: "-1"; income: "6" }
            ListElement{ timestamp: "2019-10"; expenses: "-14"; income: "25" }
            ListElement{ timestamp: "2019-11"; expenses: "-19"; income: "29" }
            ListElement{ timestamp: "2019-12"; expenses: "-5"; income: "7" }

            ListElement{ timestamp: "2020-01"; expenses: "-14"; income: "22" }
            ListElement{ timestamp: "2020-02"; expenses: "-5"; income: "7" }
            ListElement{ timestamp: "2020-03"; expenses: "-1"; income: "9" }
            ListElement{ timestamp: "2020-04"; expenses: "-1"; income: "12" }
            ListElement{ timestamp: "2020-05"; expenses: "-5"; income: "9" }
            ListElement{ timestamp: "2020-06"; expenses: "-5"; income: "8" }
            ListElement{ timestamp: "2020-07"; expenses: "-3"; income: "7" }
            ListElement{ timestamp: "2020-08"; expenses: "-1"; income: "5" }
            ListElement{ timestamp: "2020-09"; expenses: "-2"; income: "4" }
            ListElement{ timestamp: "2020-10"; expenses: "-10"; income: "13" }
            ListElement{ timestamp: "2020-11"; expenses: "-12"; income: "17" }
            ListElement{ timestamp: "2020-12"; expenses: "-6"; income: "9" }

            ListElement{ timestamp: "2021-01"; expenses: "-2"; income: "6" }
            ListElement{ timestamp: "2021-02"; expenses: "-4"; income: "8" }
            ListElement{ timestamp: "2021-03"; expenses: "-7"; income: "12" }
            ListElement{ timestamp: "2021-04"; expenses: "-9"; income: "15" }
            ListElement{ timestamp: "2021-05"; expenses: "-7"; income: "19" }
            ListElement{ timestamp: "2021-06"; expenses: "-9"; income: "18" }
            ListElement{ timestamp: "2021-07"; expenses: "-13"; income: "17" }
            ListElement{ timestamp: "2021-08"; expenses: "-5"; income: "9" }
            ListElement{ timestamp: "2021-09"; expenses: "-3"; income: "8" }
            ListElement{ timestamp: "2021-10"; expenses: "-13"; income: "15" }
            ListElement{ timestamp: "2021-11"; expenses: "-8"; income: "17" }
            ListElement{ timestamp: "2021-12"; expenses: "-7"; income: "10" }

            ListElement{ timestamp: "2022-01"; expenses: "-12"; income: "16" }
            ListElement{ timestamp: "2022-02"; expenses: "-24"; income: "28" }
            ListElement{ timestamp: "2022-03"; expenses: "-27"; income: "22" }
            ListElement{ timestamp: "2022-04"; expenses: "-29"; income: "25" }
            ListElement{ timestamp: "2022-05"; expenses: "-27"; income: "29" }
            ListElement{ timestamp: "2022-06"; expenses: "-19"; income: "18" }
            ListElement{ timestamp: "2022-07"; expenses: "-13"; income: "17" }
            ListElement{ timestamp: "2022-08"; expenses: "-15"; income: "19" }
            ListElement{ timestamp: "2022-09"; expenses: "-3"; income: "8" }
            ListElement{ timestamp: "2022-10"; expenses: "-3"; income: "6" }
            ListElement{ timestamp: "2022-11"; expenses: "-4"; income: "8" }
            ListElement{ timestamp: "2022-12"; expenses: "-5"; income: "9" }
        }
    }
}
