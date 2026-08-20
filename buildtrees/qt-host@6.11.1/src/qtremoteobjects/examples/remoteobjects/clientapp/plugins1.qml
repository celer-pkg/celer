// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtRemoteObjects
import QtQuick
import TimeExample // import types from the plugin

Rectangle {
    width: 200
    height: 400
    color: "blue"
    Clock {
        id: clock1
        anchors.top: parent.top

        Time { // this class is defined in C++
            id: time
            node: node
        }

        Node {
            id: node
            registryUrl: "local:registry"
        }

        hours: time.hour
        minutes: time.minute
        valid: time.state == Time.Valid

    }
}
