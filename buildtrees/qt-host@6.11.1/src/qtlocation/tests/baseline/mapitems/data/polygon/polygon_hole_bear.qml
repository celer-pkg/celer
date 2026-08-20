// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(52, 22)
    bearing: 54

    plugin: Plugin {
        name: "itemsoverlay"
    }
    zoomLevel: 4
    copyrightsVisible: false

    MapPolygon {
        color: "green"
        autoFadeIn: false
        geoShape: QtPositioning.polygon(
            [
                QtPositioning.coordinate(51.11, 17.13),
                QtPositioning.coordinate(50.42, 30.54),
                QtPositioning.coordinate(58.36, 26.70)
            ],
            [
                [
                   QtPositioning.coordinate(54.36, 23.46),
                   QtPositioning.coordinate(51.91, 20.52),
                   QtPositioning.coordinate(51.50, 28.25),
                   QtPositioning.coordinate(54.36, 26.80)
               ]
            ]
        )
    }

    MapPolygon {
        color: "blue"
        autoFadeIn: false
        geoShape: QtPositioning.polygon(
            [
                QtPositioning.coordinate(54.36, 33.46),
                QtPositioning.coordinate(51.91, 30.52),
                QtPositioning.coordinate(51.50, 38.25),
                QtPositioning.coordinate(54.36, 36.80)
            ]
        )
    }
}
