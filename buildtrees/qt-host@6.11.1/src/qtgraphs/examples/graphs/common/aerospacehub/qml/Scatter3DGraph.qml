// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtGraphs
import QtQuick
import AerospaceHubExample

Scatter3D {
    id: graph

    property alias series: scatterseries3d
    property var dataModel: GlobalSettings.scatterDataModel
    property bool dataready: GlobalSettings.scatter3ddataready

    msaaSamples: 8

    theme: GlobalSettings.theme

    axisX.title: "Capacity factor (%)"
    axisY.title: "Mean Power (MW)"
    axisZ.title: "Availability (%)"

    axisX.titleVisible: true
    axisY.titleVisible: true
    axisZ.titleVisible: true

    axisX.min: 0
    axisX.max: 1
    axisY.min: 0
    axisY.max: 10
    axisZ.min: 0
    axisZ.max: 1

    visible: opacity > 0.5

    rotationEnabled: false
    zoomEnabled: false

    Scatter3DSeries {
        id: scatterseries3d

        itemLabelFormat: "Capacity factor: @xLabel %\nAvailability: @zLabel %\nMean Power: @yLabel MW"

        colorStyle: GlobalSettings.colorstyle

        ItemModelScatterDataProxy {
            itemModel: graph.dataModel
            xPosRole: "x"
            yPosRole: "y"
            zPosRole: "z"
        }
    }
}
