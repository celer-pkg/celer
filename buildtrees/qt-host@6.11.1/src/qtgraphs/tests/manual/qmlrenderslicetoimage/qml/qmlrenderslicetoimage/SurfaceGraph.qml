// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs

Item {
    property real fontSize: 12
    property real windowRatio: 4
    property real maxAxisSegmentCount: 20
    property var graph: surfaceLayers

    id: surfaceView

    //! [0]
    Gradient {
        id: layerOneGradient
        GradientStop { position: 0.0; color: "black" }
        GradientStop { position: 0.31; color: "tan" }
        GradientStop { position: 0.32; color: "green" }
        GradientStop { position: 0.40; color: "darkslategray" }
        GradientStop { position: 1.0; color: "white" }
    }

    Gradient {
        id: layerTwoGradient
        GradientStop { position: 0.315; color: "blue" }
        GradientStop { position: 0.33; color: "white" }
    }

    Gradient {
        id: layerThreeGradient
        GradientStop { position: 0.0; color: "red" }
        GradientStop { position: 0.15; color: "black" }
    }
    //! [0]

    Surface3D {
        id: surfaceLayers
        width: surfaceView.width
        height: surfaceView.height
        theme: GraphsTheme {
            theme: GraphsTheme.Theme.QtGreen
            labelFont.pointSize: 35
            colorStyle: GraphsTheme.ColorStyle.RangeGradient
        }
        shadowQuality: Graphs3D.ShadowQuality.None
        selectionMode: Graphs3D.SelectionFlag.Row | Graphs3D.SelectionFlag.Slice | Graphs3D.SelectionFlag.MultiSeries
        cameraPreset: Graphs3D.CameraPreset.IsometricLeft
        axisY.min: 20
        axisY.max: 200
        axisX.segmentCount: 5
        axisX.subSegmentCount: 2
        axisX.labelFormat: "%i"
        axisZ.segmentCount: 5
        axisZ.subSegmentCount: 2
        axisZ.labelFormat: "%i"
        axisY.segmentCount: 5
        axisY.subSegmentCount: 2
        axisY.labelFormat: "%i"

        Surface3DSeries {
            id: layerOneSeries
            baseGradient: layerOneGradient
            HeightMapSurfaceDataProxy {
                heightMapFile: ":/heightmaps/layer_1.png"
            }
            shading: Surface3DSeries.Shading.Smooth
            drawMode: Surface3DSeries.DrawSurface
        }

        Surface3DSeries {
            id: layerTwoSeries
            baseGradient: layerTwoGradient
            HeightMapSurfaceDataProxy {
                heightMapFile: ":/heightmaps/layer_2.png"
            }
            shading: Surface3DSeries.Shading.Smooth
            drawMode: Surface3DSeries.DrawSurface
        }

        Surface3DSeries {
            id: layerThreeSeries
            baseGradient: layerThreeGradient
            HeightMapSurfaceDataProxy {
                heightMapFile: ":/heightmaps/layer_3.png"
            }
            shading: Surface3DSeries.Shading.Smooth
            drawMode: Surface3DSeries.DrawSurface
        }
    }
}
