// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtGraphs

Item {
    property int shadowQuality: Graphs3D.ShadowQuality.High
    property int cameraPreset: Graphs3D.CameraPreset.Front
    property alias meshSmooth: scatterSeries.meshSmooth
    property alias theme: scatterGraph.theme

    //! [0]
    Scatter3D {
        id: scatterGraph
        //! [0]
        anchors.fill: parent

        theme: themeQt
        shadowQuality: parent.shadowQuality
        cameraPreset: parent.cameraPreset

        //! [1]
        axisX.segmentCount: 3
        axisX.subSegmentCount: 2
        axisX.labelFormat: "%.2f"
        axisZ.segmentCount: 2
        axisZ.subSegmentCount: 2
        axisZ.labelFormat: "%.2f"
        axisY.segmentCount: 2
        axisY.subSegmentCount: 2
        axisY.labelFormat: "%.2f"
        //! [1]

        //! [2]
        Scatter3DSeries {
            id: scatterSeries
            itemLabelFormat: "Series 1: X:@xLabel Y:@yLabel Z:@zLabel"

            ItemModelScatterDataProxy {
                //! [3]
                itemModel: seriesData.model
                //! [3]
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
            }
        }
        //! [2]
        //! [4]
        Scatter3DSeries {
            id: scatterSeriesTwo
            //! [4]
            itemLabelFormat: "Series 2: X:@xLabel Y:@yLabel Z:@zLabel"
            itemSize: 0.05
            mesh: Abstract3DSeries.Mesh.Cube
            //! [5]
            ItemModelScatterDataProxy {
                itemModel: seriesData.modelTwo
                //! [5]
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
                //! [6]
            }
        }
        Scatter3DSeries {
            id: scatterSeriesThree
            //! [6]
            itemLabelFormat: "Series 3: X:@xLabel Y:@yLabel Z:@zLabel"
            itemSize: 0.1
            mesh: Abstract3DSeries.Mesh.Minimal

            //! [7]
            ItemModelScatterDataProxy {
                itemModel: seriesData.modelThree
                //! [7]
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
                //! [8]
            }
        }
        //! [8]
    }
}
