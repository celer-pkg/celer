// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//![0]
import QtQuick
import QtRemoteObjects
import TimeExample // import types from the plugin

Rectangle {
    width: 200
    height: 400
    color: "blue"

    Node {
        id: node
        registryUrl: "local:registry"
    }

    Time { // this class is defined in C++
        id: time
        node: node
    }

    Text {
        id: validity; font.bold: true; font.pixelSize: 14; y:200; color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        text: time.state == Time.Valid ? "Click to see a clock!" : "Server not found"
    }
}
//![0]
