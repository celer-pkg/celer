// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

//! [dashboard]
Flickable {
    property alias contentheight: content.height

    clip: true

    contentHeight: contentheight

    boundsBehavior: Flickable.StopAtBounds
    interactive: true
    visible: opacity > 0.5

    Item {
        id: content
        width: parent.width

        GridLayout {
            id: grid
            columns: 2
            //! [dashboard]
            rowSpacing: GlobalSettings.doublespacing
            columnSpacing: GlobalSettings.doublespacing
            anchors.fill: parent

            //! [previews]
            // Graph 1: Power output over time
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Area2D"
                linesimulator: linesim
            }

            // Graph 2: Energy production x Turbine
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Bar2D"
                barsimulator: barsim
            }
            //! [previews]

            // Graph 3: Power curve visualization
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Surface3D"
            }

            // Graph 4: Fleet-level performance distribution
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Scatter3D"
            }

            // Graph 5: Component failure distribution
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Pie2D"
                piesimulator: piesim
            }

            // Graph 6: Energy production breakdown x Turbine
            SmallGraphItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                state: "Bar3D"
            }
        }
    }
}
