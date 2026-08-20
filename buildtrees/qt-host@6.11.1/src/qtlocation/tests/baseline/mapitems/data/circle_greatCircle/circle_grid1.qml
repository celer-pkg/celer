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

    Repeater {
        id: c
        property var colors: [
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet"
        ]
        property int gridCount: c.colors.length
        model: gridCount*gridCount
        MapCircle
        {
            center
            {
                longitude: -180+360*(index%c.gridCount+0.5)/(c.gridCount)
                latitude: -90+180*(Math.floor(index/c.gridCount)+0.5)/(c.gridCount)
            }
            radius: 2000*1000
            color: c.colors[Math.floor(index%c.colors.length)]
            border.width: 1
            autoFadeIn: false
            referenceSurface: QtLocation.ReferenceSurface.Globe
            opacity: 0.3
        }
    }
}
