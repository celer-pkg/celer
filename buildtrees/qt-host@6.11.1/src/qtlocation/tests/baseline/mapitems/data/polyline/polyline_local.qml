// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(52, 22)

    plugin: Plugin {
        name: "itemsoverlay"
    }
    zoomLevel: 4
    copyrightsVisible: false

    MapPolyline {
        autoFadeIn: false
        line.color: "yellow"
        line.width: 15
        path: [
            QtPositioning.coordinate(51, 17),
            QtPositioning.coordinate(60, 12),
            QtPositioning.coordinate(58, 26),
            QtPositioning.coordinate(45, 32),
            QtPositioning.coordinate(52, 21)
        ]
    }
    MapPolyline {
        autoFadeIn: false
        line.color: "green"
        line.width: 2
        path: [
            QtPositioning.coordinate(53, 15),
            QtPositioning.coordinate(49, 32),
            QtPositioning.coordinate(57, 27)
        ]
    }
        MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 2
        path: [
            QtPositioning.coordinate(49, 15),
            QtPositioning.coordinate(47, 32),
            QtPositioning.coordinate(54, 24),
            QtPositioning.coordinate(49, 15)
        ]
    }
}
