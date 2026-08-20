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
    MapCircle
    {
        center
        {
            longitude: 0
            latitude: -90
        }
        radius: 0.25*Math.PI*6371*1000
        color: 'red'
        smooth: true
        border.width: 1
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        opacity: 0.5
    }
    MapCircle
    {
        center
        {
            longitude: 0
            latitude: 90
        }
        radius: 0.25*Math.PI*6371*1000
        color: 'green'
        smooth: true
        border.width: 1
        autoFadeIn: false
        referenceSurface: QtLocation.ReferenceSurface.Globe
        opacity: 0.5
    }
}
