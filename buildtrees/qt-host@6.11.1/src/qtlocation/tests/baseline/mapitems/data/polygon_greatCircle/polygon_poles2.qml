// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(0, 20)

    id: map
    plugin: Plugin {
        name: "itemsoverlay"
    }
    zoomLevel: 1
    copyrightsVisible: false

    MapPolygon {
        color: 'red'
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        geoShape: QtPositioning.polygon(
            [
                QtPositioning.coordinate(-30, 61),
                QtPositioning.coordinate(0, -179),
                QtPositioning.coordinate(30, -59)
            ]
        )
        opacity: 0.5
    }

    MapPolygon {
        color: 'green'
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        geoShape: QtPositioning.polygon(
            [
                QtPositioning.coordinate(30, 61),
                QtPositioning.coordinate(0, -179),
                QtPositioning.coordinate(-30, -59)
            ]
        )
        opacity: 0.5
    }
}
