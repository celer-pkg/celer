// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(0, 30)

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
        property double rWidth: 55
        property double rHeight: 55
        property int gridCount: c.colors.length
        model: gridCount*gridCount
        MapRectangle
        {
            topLeft
            {
                longitude: Math.min(180, Math.max(-180, -180+360*(index%c.gridCount+0.5)/(c.gridCount) - c.rWidth/2))
                latitude: Math.min(90, Math.max(-90, -90+180*(Math.floor(index/c.gridCount)+0.5)/(c.gridCount) - c.rHeight/2))
            }
            bottomRight
            {
                longitude: Math.min(180, Math.max(-180, -180+360*(index%c.gridCount+0.5)/(c.gridCount) + c.rWidth/2))
                latitude: Math.min(90, Math.max(-90, -90+180*(Math.floor(index/c.gridCount)+0.5)/(c.gridCount) + c.rHeight/2))
            }
            color: c.colors[Math.floor(index%c.colors.length)]
            border.width: 1
            autoFadeIn: false
			referenceSurface: QtLocation.ReferenceSurface.Globe
            opacity: 0.3
        }
    }
}
