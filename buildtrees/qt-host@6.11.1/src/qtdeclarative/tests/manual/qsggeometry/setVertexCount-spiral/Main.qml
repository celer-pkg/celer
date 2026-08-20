// Copyright (C) 2024 Stan Morris
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import VertexSpiral

ApplicationWindow {
    id: window

    property int frameCount: 0

    width: spiral.width + 40
    height: spiral.height + 40
    visible: true
    title: qsTr("QSGGeometry::setVertexCount() tester")
    color: "black"
    onFrameSwapped: ++window.frameCount

    // Component.onCompleted: {
    //     // kick off animation at startup
    //     button.clicked()
    // }

    SpiralItem {
        id: spiral
        anchors.centerIn: parent
        vertexCount: slider.value

        SequentialAnimation {
            id: seqAnim
            loops: Animation.Infinite
            onStarted: spiral.toggleMode()

            NumberAnimation {
                target: spiral
                property: "vertexCount"
                to: spiral.maxVertices
                duration: 2000
            }
            // Do not animate down to zero!
            //
            // Do not set "to:" equal to "0"... use "1" to avoid QTBUG-58177,
            // which regards geometries not rendering after vertex count is set 0.
            //
            NumberAnimation {
                target: spiral
                property: "vertexCount"

                // Make zero to see QTBUG-58177. Hover the top-tight "Quit"
                // button to see how it oddly temporarily fixes rendering.
                to: 1
                duration: 2000
            }
            ScriptAction { script: spiral.toggleMode() }

            // Pause long enough for the Renderer to process geometry with zero
            // vertices so QTBUG-58177 is consistently reproducible if you want
            // to see its behavior.
            PauseAnimation { duration: 20 }
        }
    }
    Slider {
        id: slider
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        orientation: Qt.Vertical
        from: spiral.maxVertices
        to: 0
        value: spiral.maxVertices
        focus: true
        Keys.onEscapePressed: Qt.quit()
    }
    Column {
        anchors.right: parent.right
        Button { id: hiddenButton; visible: false; text: "Animate Vertex Count" }
        Button {
            id: button
            text: hiddenButton.text
            width: hiddenButton.width
            Keys.onEscapePressed: Qt.quit()
            onClicked: {
                if (slider) {
                    slider.destroy();
                    text = "Quit";

                    // reset number of vertices in case user is starting from max
                    spiral.vertexCount = 1;
                    seqAnim.start();
                } else {
                    Qt.quit();
                }
            }
        }
        Column {
            id: stats
            anchors.horizontalCenter: parent.horizontalCenter

            component MyText: Text {
                font { bold: true; pixelSize: 16 }
                color: "white"
            }
            component Stat: Row {
                id: rowRoot
                property alias caption: label.text
                property alias value: valueText.text

                anchors.horizontalCenter: stats.horizontalCenter
                width: 2*label.width
                spacing: 8

                MyText { id: label }
                MyText { id: valueText }
            }

            MyText { anchors.horizontalCenter: parent.horizontalCenter; text: spiral.mode }
            Stat { caption: "FPS:"; value: spiral.fps }

            Timer {
                id: fpsTimer
                interval: 1000
                repeat: true
                running: stats.visible
                onTriggered: {
                    spiral.fps = window.frameCount;
                    window.frameCount = 0;
                }
            }
        }
    }
}
