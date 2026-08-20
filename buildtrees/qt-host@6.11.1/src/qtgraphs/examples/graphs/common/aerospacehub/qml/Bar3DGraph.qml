// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import AerospaceHubExample

//! [bargraph]
Bars3D {
    id: graph
    visible: opacity > 0.5
    //! [bargraph]

    property alias series: barseries3d
    property var dataModel: GlobalSettings.barDataModel
    property bool dataready: GlobalSettings.bar3ddataready

    msaaSamples: 8

    theme: GlobalSettings.theme

    rotationEnabled: false
    zoomEnabled: false

    rowAxis.title: "Grid X"
    valueAxis.title: "Power (MW)"
    columnAxis.title: "Grid Z"

    valueAxis.min: 0
    valueAxis.max: 10

    Bar3DSeries {
        id: barseries3d

        colorStyle: GlobalSettings.colorstyle

        itemLabelFormat: "@rowLabel/@colLabel: @valueLabel MW"

        ItemModelBarDataProxy {
            itemModel: graph.dataModel
            rowRole: "row"
            columnRole: "column"
            valueRole: "value"
        }

        onSelectedBarChanged: (position) => {
            let count = GlobalSettings.turbineCountPerAxis
            GlobalSettings.turbineid = position.x + "/" + position.y
        }
    }
}
