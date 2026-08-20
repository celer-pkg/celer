// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

pragma ComponentBehavior: Bound

Rectangle {
    id: uvChecker
    color: "black"
    readonly property var cellColors: [
        { color: "#0C5D7A", textColor: "white", gridColor: "#548DA1" },
        { color: "#5FD6C2", textColor: "white", gridColor: "#8EE2D3" },
        { color: "#FFEDAB", textColor: "black", gridColor: "#FEF1C4" },
        { color: "#FD8F74", textColor: "white", gridColor: "#FCB19E" },
        { color: "#CF2C65", textColor: "white", gridColor: "#DE6B94" },
        { color: "#282828", textColor: "white", gridColor: "#686868" },
        { color: "#282828", textColor: "white", gridColor: "#A5A5A5" },
        { color: "#D6D6D6", textColor: "black", gridColor: "#E2E2E2" }
    ]

    component Cell : Rectangle {
        id: root
        required property int row
        required property int column
        required property color backgroundColor
        required property color textColor
        required property color gridColor

        color: gridColor

        Grid {
            columns: 8
            rows: 8
            rowSpacing: 1
            columnSpacing: 1
            Repeater {
                model: 64
                Rectangle {
                    width: (root.width - 8) / 8
                    height: (root.height - 8) / 8
                    color: root.backgroundColor
                }
            }

        }

        readonly property list<string> rowNames: ["A", "B", "C", "D", "E", "F", "G", "H"]

        Text {
            anchors.fill: parent
            anchors.margins: 10
            text: `${root.rowNames[root.row]}${root.column}\n\u2191`
            color: root.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            fontSizeMode: Text.Fit
            minimumPointSize: 1
            font.pointSize: 128
            font.hintingPreference: Font.PreferNoHinting
            renderType: Text.NativeRendering
        }
    }

    Grid {
        rows: 8
        columns: 8

        Repeater {
            model: 64
            Cell {
                required property int index
                width: uvChecker.width / 8
                height: uvChecker.height / 8
                row: Math.floor(index / 8)
                column: index % 8

                readonly property var cellColors: uvChecker.cellColors[(index - row) % uvChecker.cellColors.length]

                backgroundColor: cellColors.color
                textColor: cellColors.textColor
                gridColor: cellColors.gridColor
            }
        }
    }
}
