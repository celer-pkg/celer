// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtGraphs

Rectangle {
    //! [0]
    property GraphsTheme theme
    property Bar3DSeries series
    //! [0]
    property point previousSelection

    id: legendItem
    state: "unselected"

    // Workaround for a layout bug that in some situations causes changing from fully opaque color
    // to a transparent one to use black background instead of what is actually under the items.
    // Having the control always slighthly transparent forces the background to be refreshed
    // properly.
    opacity: 0.999

    //! [1]
    RowLayout {
        anchors.fill: parent
        spacing: 0
        clip: true
        Item {
            id: markerSpace
            Layout.minimumWidth: 20
            Layout.minimumHeight: 20
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            Rectangle {
                x: parent.x + parent.width / 4
                y: parent.y + parent.height / 4
                width: parent.width / 2
                height: width
                border.color: "black"
                color: series.baseColor
            }
        }
        Item {
            height: markerSpace.height
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            Layout.minimumWidth: 100
            Text {
                anchors.fill: parent
                text: series.name
                verticalAlignment: Text.AlignVCenter
                clip: true
                color: theme.labelTextColor
                font: theme.labelFont
            }
        }
    }
    //! [1]

    //! [2]
    MouseArea {
        id: mouseArea
        anchors.fill: legendItem
        onClicked: {
            if (legendItem.state === "selected") {
                series.selectedBar = series.invalidSelectionPosition
            } else {
                series.selectedBar = previousSelection
            }
        }
    }
    //! [2]

    //! [4]
    Connections {
        target: series
        function onSelectedBarChanged(position) {
            if (position !== series.invalidSelectionPosition) {
                previousSelection = position
            }
        }
    }
    //! [4]

    //! [3]
    states: [
        State  {
            name: "selected"
            when: series.selectedBar != series.invalidSelectionPosition
            PropertyChanges {
                target: legendItem
                color: series.singleHighlightColor
            }
        },
        State  {
            name: "unselected"
            when: series.selectedBar == series.invalidSelectionPosition
            PropertyChanges {
                target: legendItem
                color: theme.labelBackgroundColor
            }
        }
    ]
    //! [3]
}
