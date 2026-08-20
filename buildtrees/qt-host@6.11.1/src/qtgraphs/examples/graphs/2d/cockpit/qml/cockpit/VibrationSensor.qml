// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGraphs

Item {
    id: root
    anchors.fill: parent

    property alias lineGraph: line

    Rectangle {
        color: "black"
        height: root.height * 0.1
        anchors.left: parent.left
        anchors.right: parent.right
        border.color: "white"
        border.width: 2
        z: 1

        Text {
            color: "white"
            antialiasing: true
            font.bold: true
            font.family: "Akshar"
            font.pointSize: 16
            anchors.centerIn: parent
            text: "ENGINE 1 VIBRATION"
        }
    }

    GraphsView {
        anchors.fill: parent
        anchors.leftMargin: -77
        anchors.rightMargin: -15
        anchors.bottomMargin: -40

        theme: GraphsTheme {
            backgroundVisible: false
            plotAreaBackgroundColor: "#11000000"
        }

        axisX: ValueAxis {
            lineVisible: false
            gridVisible: false
            subGridVisible: false
            labelsVisible: false
            visible: false
            id: xAxis
            max: 8
        }

        axisY: ValueAxis {
            lineVisible: false
            gridVisible: false
            subGridVisible: false
            labelsVisible: false
            visible: false
            tickInterval: 4
            id: yAxis
            max: 8
        }
        //! [1]
        LineSeries {
            id: line
            property int divisions: 500
            property real amplitude: 0.5
            property real resolution: 0.5

            FrameAnimation {
                running: true

                onTriggered: {
                    for (let i = 0; i < line.divisions; ++i) {
                        let y = Math.sin(line.resolution*i)
                        y *= Math.cos(i)
                        y *= Math.sin(i / line.divisions * 3.2) * 3 * line.amplitude * Math.random()

                       line.replace(i, (i/line.divisions) * 8.0, y + 4)
                    }
                }
            }

            Component.onCompleted: {
                for (let i = 1; i <= divisions; ++i) {
                    append((i/divisions) * 8.0, 4.0)
                }
            }

            function change(newDivs) {
                let delta = newDivs - divisions

                if (delta < 0) {
                    delta = Math.abs(delta)
                    removeMultiple(count - 1 - delta, delta)
                } else {
                    for (let i = 0; i < delta; ++i) {
                        append(((count + i)/divisions) * 8.0, 4.0)
                    }
                }

                divisions = newDivs
            }
        }
        //! [1]
    }

}
