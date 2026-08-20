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
        line.color: "yellow"
        line.width: 10
        path: [
            QtPositioning.coordinate(44, 137),
            QtPositioning.coordinate(40, 159),
            QtPositioning.coordinate(52, 172),
            QtPositioning.coordinate(48, -173),
            QtPositioning.coordinate(53, -124),
            QtPositioning.coordinate(45, -66),
            QtPositioning.coordinate(55, -17)
        ]
    }
    MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 5
        path: [
            QtPositioning.coordinate(35, 90),
            QtPositioning.coordinate(35, -90)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 5
        path: [
            QtPositioning.coordinate(30, -91),
            QtPositioning.coordinate(30, 91)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 5
        path: [
            QtPositioning.coordinate(25, 120),
            QtPositioning.coordinate(25, -120)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "blue"
        line.width: 5
        path: [
            QtPositioning.coordinate(20, 179),
            QtPositioning.coordinate(20, -179)
        ]
    }

    MapPolyline {
    autoFadeIn: false
    line.color: "green"
    line.width: 5
    path: [
        QtPositioning.coordinate(15, 180),
        QtPositioning.coordinate(15, -180)
    ]
    }

    MapPolyline {
    autoFadeIn: false
    line.color: "green"
    line.width: 5
    path: [
        QtPositioning.coordinate(10, -179),
        QtPositioning.coordinate(10, 179)
    ]
    }

    MapPolyline {
    autoFadeIn: false
    line.color: "green"
    line.width: 5
    path: [
        QtPositioning.coordinate(5, -179),
        QtPositioning.coordinate(5, 179)
    ]
    }
}
