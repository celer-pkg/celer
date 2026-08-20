// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
import QtGraphs
//! [0]

//! [1]
import QtQuick
import QtGraphs

View3D {
    width: 640
    height: 480

    Bars3DNode {
        width: parent.width
        height: parent.height

        Bar3DSeries {
            itemLabelFormat: "@colLabel, @rowLabel: @valueLabel"

            ItemModelBarDataProxy {
                itemModel: dataModel
                // Mapping model roles to bar series rows, columns, and values.
                rowRole: "year"
                columnRole: "city"
                valueRole: "expenses"
            }
        }
    }

    ListModel {
        id: dataModel
        ListElement{ year: "2022"; city: "Oulu";     expenses: "4200"; }
        ListElement{ year: "2022"; city: "Rauma";    expenses: "2100"; }
        ListElement{ year: "2022"; city: "Helsinki"; expenses: "7040"; }
        ListElement{ year: "2022"; city: "Tampere";  expenses: "4330"; }
        ListElement{ year: "2023"; city: "Oulu";     expenses: "3960"; }
        ListElement{ year: "2023"; city: "Rauma";    expenses: "1990"; }
        ListElement{ year: "2023"; city: "Helsinki"; expenses: "7230"; }
        ListElement{ year: "2023"; city: "Tampere";  expenses: "4650"; }
    }
}
//! [1]

//! [2]
import QtQuick
import QtGraphs

View3D {
    width: 640
    height: 480

    Scatter3DNode {
        width: parent.width
        height: parent.height
        Scatter3DSeries {
            ItemModelScatterDataProxy {
                itemModel: dataModel
                // Mapping model roles to scatter series item coordinates.
                xPosRole: "xPos"
                yPosRole: "yPos"
                zPosRole: "zPos"
            }
        }
    }

    ListModel {
        id: dataModel
        ListElement{ xPos: "2.754"; yPos: "1.455"; zPos: "3.362"; }
        ListElement{ xPos: "3.164"; yPos: "2.022"; zPos: "4.348"; }
        ListElement{ xPos: "4.564"; yPos: "1.865"; zPos: "1.346"; }
        ListElement{ xPos: "1.068"; yPos: "1.224"; zPos: "2.983"; }
        ListElement{ xPos: "2.323"; yPos: "2.502"; zPos: "3.133"; }
    }
}
//! [2]

//! [3]
import QtQuick
import QtGraphs

View3D {
    width: 640
    height: 480

    Surface3DNode {
        width: parent.width
        height: parent.height
        Surface3DSeries {
            itemLabelFormat: "Pop density at (@xLabel N, @zLabel E): @yLabel"
            ItemModelSurfaceDataProxy {
                itemModel: dataModel
                // Mapping model roles to surface series rows, columns, and values.
                rowRole: "longitude"
                columnRole: "latitude"
                yPosRole: "pop_density"
            }
        }

        //! [4]
        onTapped: {
            // Disable the default input handler
            unsetDefaultTapHandler()
            // Implement own custom event handler
            console.log("Custom tap event handler")
        }
        //! [4]
    }
    ListModel {
        id: dataModel
        ListElement{ longitude: "20"; latitude: "10"; pop_density: "4.75"; }
        ListElement{ longitude: "21"; latitude: "10"; pop_density: "3.00"; }
        ListElement{ longitude: "22"; latitude: "10"; pop_density: "1.24"; }
        ListElement{ longitude: "23"; latitude: "10"; pop_density: "2.53"; }
        ListElement{ longitude: "20"; latitude: "11"; pop_density: "2.55"; }
        ListElement{ longitude: "21"; latitude: "11"; pop_density: "2.03"; }
        ListElement{ longitude: "22"; latitude: "11"; pop_density: "3.46"; }
        ListElement{ longitude: "23"; latitude: "11"; pop_density: "5.12"; }
        ListElement{ longitude: "20"; latitude: "12"; pop_density: "1.37"; }
        ListElement{ longitude: "21"; latitude: "12"; pop_density: "2.98"; }
        ListElement{ longitude: "22"; latitude: "12"; pop_density: "3.33"; }
        ListElement{ longitude: "23"; latitude: "12"; pop_density: "3.23"; }
        ListElement{ longitude: "20"; latitude: "13"; pop_density: "4.34"; }
        ListElement{ longitude: "21"; latitude: "13"; pop_density: "3.54"; }
        ListElement{ longitude: "22"; latitude: "13"; pop_density: "1.65"; }
        ListElement{ longitude: "23"; latitude: "13"; pop_density: "2.67"; }
    }
}
//! [3]
