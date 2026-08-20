// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//![0]
import QtQuick
import QtRemoteObjects
import TimeExample // import types from the plugin

Rectangle {
    width: 200
    height: 400
    Clock { // this class is defined in QML
        id: clock1
        anchors.top: parent.top

        Time { // this class is defined in C++
            id: time
            node: sharedNode
        }

        hours: time.hour
        minutes: time.minute
        valid: time.state == Time.Valid

    }
    Clock { // this class is defined in QML
        id: clock2
        anchors.top: clock1.bottom

        Time { // this class is defined in C++
            id: time2
            node: sharedNode
        }

        hours: time2.hour
        minutes: time2.minute
        valid: time2.state == Time.Valid

    }

    Node {
        id: sharedNode
        registryUrl: "local:registry"
    }

}
//![0]
