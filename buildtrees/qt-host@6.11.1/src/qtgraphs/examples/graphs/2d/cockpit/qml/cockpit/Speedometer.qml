// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGraphs

Item {
    id: root
    anchors.fill: parent

    Text {
        id: txt
        color: "white"
        antialiasing: true
        font.bold: true
        font.family: "Akshar"
        anchors.centerIn: parent
        transform: Translate { y: -20 }
        text: "AIRSPEED"
    }

    //! [1]
    GraphsView {
        id: chart
        anchors.fill: parent
        anchors.margins: 20

        theme: GraphsTheme {
            backgroundVisible: false
            borderColors: ["#252525"]
        }

        PieSeries {
            id: pieOuter
            pieSize: 1
            holeSize: 0.8
            startAngle: -120
            endAngle: 120

            PieSlice { label: "Stall"; value: 1; color: "#ff0000"; labelVisible: false }
            PieSlice { label: "Optimal"; value: 4; color: "#00ff00"; labelVisible: false }
            PieSlice { label: "Overspeed"; value: 1; color: "#ffaa22"; labelVisible: false }
        }

        PieSeries {
            pieSize: 0.8
            holeSize: 0.6
            startAngle: -120
            endAngle: 120

            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
            PieSlice { value: 4; color: "#252525"; }
            PieSlice { value: 1; color: "#ffffff"; }
        }

        PieSeries {
            pieSize: 0.6
            holeSize: 1.0
            startAngle: -120
            endAngle: 120
            verticalPosition: 1

            PieSlice { label: "Stall"; value: 1; color: "#ff0000"; labelVisible: false }
            PieSlice { label: "Optimal"; value: 4; color: "#00ff00"; labelVisible: false  }
            PieSlice { label: "Overspeed"; value: 1; color: "#ffaa22"; labelVisible: false  }
        }
    }
    //! [1]

    Rectangle {
        id: needle
        property double sAngle: 0

        FrameAnimation {
            id: anim
            running: true

            onTriggered: {
                let v = Math.sin (elapsedTime) + Math.sin(3.142 * elapsedTime);
                v = v * 45.0 - 90.0
                needle.sAngle = v
            }
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: chart.width * 0.3
        height: 5
        transform: [
            Translate {x: 55},
            Rotation {angle: needle.sAngle; origin.x: 55; origin.y: 2.5}
        ]
        color: "grey"
    }

    Rectangle {
        width: 20
        height: 20
        anchors.centerIn: parent
        color: "#454545"
        radius: 20
    }

    Rectangle {
        id: needle2
        property double sAngle: 0

        FrameAnimation {
            id: anim2
            running: true

            onTriggered: {
                let v = Math.sin (elapsedTime) + Math.sin(3.142 * 2 * elapsedTime);
                v = v * 45.0 - 90.0
                needle2.sAngle = v
            }
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: chart.width * 0.1
        height: 5
        transform: [
            Translate {x: 20; y: 90},
            Rotation {angle: needle2.sAngle; origin.x: 20; origin.y: 90+2.5}
        ]
        color: "white"
    }
}
