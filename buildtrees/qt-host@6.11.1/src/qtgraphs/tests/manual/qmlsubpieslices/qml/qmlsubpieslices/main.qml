// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import QtGraphs
import "."

Item {
    id: mainview

    width: 1280
    height: 1024

    GridLayout {
        anchors.fill: parent
        columns: 2

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            seriesList: cppPieSeries
        }

        GraphsView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            PieSeries {
                holeSize: 0.2
                selectable: true
                hoverable: true

                PieSlice {
                    value: 1
                    color: objectName
                    objectName: "cyan"
                    label: objectName
                    labelVisible: true
                }
                PieSlice {
                    value: 2
                    color: objectName
                    objectName: "darkgreen"
                    label: objectName
                    labelVisible: true
                    PieSlice {
                        value: 1
                        color: objectName
                        objectName: "salmon"
                        label: objectName
                        labelVisible: true
                    }
                    PieSlice {
                        value: 2
                        color: objectName
                        objectName: "lightgrey"
                        label: objectName
                        labelVisible: true
                        PieSlice {
                            value: 2
                            color: objectName
                            objectName: "blue"
                            label: objectName
                            labelVisible: true
                        }
                        PieSlice {
                            value: 2
                            color: objectName
                            objectName: "orange"
                            label: objectName
                            labelVisible: true
                        }
                    }
                }
                PieSlice {
                    value: 1
                    exploded: true
                    color: objectName
                    objectName: "pink"
                    label: objectName
                    labelVisible: true
                    PieSlice {
                        value: 1
                        color: objectName
                        objectName: "red"
                        label: objectName
                        labelVisible: true
                    }
                    PieSlice {
                        value: 2
                        color: objectName
                        objectName: "yellow"
                        label: objectName
                        exploded: true
                        labelVisible: true
                    }
                }
                PieSlice {
                    value: 2
                    color: objectName
                    objectName: "violet"
                    label: objectName
                    labelVisible: true
                }

                onClicked: function(slice) {
                    console.log("Clicked", slice.objectName)
                }
                onHoverEnter: function(name, position, value){
                    console.log(name, position, value)
                }
            }
        }
    }
}
