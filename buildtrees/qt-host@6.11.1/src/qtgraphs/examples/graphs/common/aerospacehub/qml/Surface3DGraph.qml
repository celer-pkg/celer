// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import AerospaceHubExample

Surface3D {
    id: graph

    property alias series: surfaceseries3d
    property var dataModel: GlobalSettings.surfaceDataModel
    property bool dataready: GlobalSettings.surface3ddataready

    msaaSamples: 8

    theme: GlobalSettings.theme

    visible: opacity > 0.5

    rotationEnabled: false
    zoomEnabled: false

    axisX.title: "Air speed (m/s)"
    axisY.title: "Power (MW)"
    axisZ.title: "Air density (kg/m3)"

    axisX.titleVisible: true
    axisY.titleVisible: true
    axisZ.titleVisible: true

    axisX.min: 0
    axisX.max: 30
    axisY.min: 0
    axisY.max: 10
    axisZ.min: 0
    axisZ.max: 1.5

    horizontalAspectRatio: 1.0

    Surface3DSeries {
        id: surfaceseries3d

        colorStyle: GlobalSettings.colorstyle

        itemLabelFormat: "Air speed: @xLabel m/s\nAir density: @zLabel kg/m3\nPower: @yLabel MW"

        ItemModelSurfaceDataProxy {
            itemModel: graph.dataModel
            rowRole: "row"
            columnRole: "column"
            yPosRole: "y"
        }
    }
}
