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
        line.width: 5
        path: [
            QtPositioning.coordinate(-90, -10),
            QtPositioning.coordinate(90, -10)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "yellow"
        line.width: 10
        path: [
            QtPositioning.coordinate(90, 10),
            QtPositioning.coordinate(-90, 10)
        ]
    }


    MapPolyline {
        autoFadeIn: false
        line.color: "magenta"
        line.width: 15
        path: [
            QtPositioning.coordinate(-90, -180),
            QtPositioning.coordinate(90, -180)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "cyan"
        line.width: 5
        path: [
            QtPositioning.coordinate(-90, 180),
            QtPositioning.coordinate(90, 180)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "green"
        line.width: 5
        path: [
            QtPositioning.coordinate(-90, -170),
            QtPositioning.coordinate(90, 170)
        ]
    }

    MapPolyline {
        autoFadeIn: false
        line.color: "green"
        line.width: 5
        path: [
            QtPositioning.coordinate(-90, 170),
            QtPositioning.coordinate(90, -170)
        ]
    }
}
