// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(0, 0)

    plugin: Plugin {
        name: "itemsoverlay"
    }
    zoomLevel: 1
    copyrightsVisible: false

    MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 2
        path: [
            QtPositioning.coordinate(60, 100),
            QtPositioning.coordinate(55, -100),
            QtPositioning.coordinate(50,  0),
            QtPositioning.coordinate(45, 100),
            QtPositioning.coordinate(40,  0),
            QtPositioning.coordinate(35, -100),
            QtPositioning.coordinate(30, 100),
            QtPositioning.coordinate(25, -100),
            QtPositioning.coordinate(20, 0),
            QtPositioning.coordinate(15, 100),
            QtPositioning.coordinate(10, 0),
            QtPositioning.coordinate( 5, -100)
        ]
    }

}
