// Copyright (C) 2017 Lorn Potter.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import Collector
import QtSensors

Rectangle {
    Collector {
        id: writer
    }

    Text {
        id: label
        text: "Sensor Clerk<br> push to start and stop<br> sensor dump";
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Button {
        id: startCollectingButton
        text: depressed ? "Stop" : "Start"
        property bool depressed: false
        anchors.top: label.bottom
        enabled: true;
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            if (!depressed) {
                writer.startCollecting()
                depressed = true
            } else {
                writer.stopCollecting()
                depressed = false
            }
        }
    }

}


