// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(80, -140)

    id: map
    plugin: Plugin {
        name: "osm"
    }
    zoomLevel: 2
    copyrightsVisible: false

    MapCircle
    {
        center: map.center
        radius: 3000000
        color: "red"
        border.width: 1
        autoFadeIn: false
        opacity: 0.5
        referenceSurface: QtLocation.ReferenceSurface.Map
    }

    MapCircle
    {
        center: map.center
        radius: 3000000
        color: "blue"
        border.width: 1
        autoFadeIn: false
        opacity: 0.5
        referenceSurface: QtLocation.ReferenceSurface.Globe
    }
}
