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
        referenceSurface: QtLocation.ReferenceSurface.Globe
        line.color: "green"
        line.width: 5
        path: [
            QtPositioning.coordinate(30, 1),
            QtPositioning.coordinate(30, 121),
            QtPositioning.coordinate(30, -119),
        ]
    }

    MapPolyline {
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        line.color: "green"
        line.width: 5
        path: [
            QtPositioning.coordinate(-30, 1),
            QtPositioning.coordinate(-30, 121),
            QtPositioning.coordinate(-30, -119),
        ]
    }

        MapPolyline {
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        line.color: "green"
        line.width: 5
        path: [
            QtPositioning.coordinate(-30, 60),
            QtPositioning.coordinate(-30, -120),
        ]
    }
}
