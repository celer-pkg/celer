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
        property double rWidth: 70
        property double rHeight: 50
        property int gridCount: 4
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
            color: 'green'
            border.width: 1
            autoFadeIn: false
            opacity: 0.3
        }
    }

    Repeater {
        id: c2
        property double rWidth: 40
        property double rHeight: 40
        property int gridCount: 4
        model: gridCount*gridCount
        MapRectangle
        {
            topLeft
            {
                longitude: Math.min(180, Math.max(-180, -180+360*(index%c2.gridCount+0.5)/(c2.gridCount) - c2.rWidth/2))
                latitude: Math.min(90, Math.max(-90, -90+180*(Math.floor(index/c2.gridCount)+0.5)/(c2.gridCount) - c2.rHeight/2))
            }
            bottomRight
            {
                longitude: Math.min(180, Math.max(-180, -180+360*(index%c2.gridCount+0.5)/(c2.gridCount) + c2.rWidth/2))
                latitude: Math.min(90, Math.max(-90, -90+180*(Math.floor(index/c2.gridCount)+0.5)/(c2.gridCount) + c2.rHeight/2))
            }
            color: 'blue'
            border.width: 1
            autoFadeIn: false
            opacity: 0.3
        }
    }
}
