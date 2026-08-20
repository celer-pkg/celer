// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtLocation
import QtPositioning

Map {
    width: 512
    height: 512

    center: QtPositioning.coordinate(0, 47.4)

    id: map
    plugin: Plugin {
        name: "itemsoverlay"
    }
    zoomLevel: 1
    copyrightsVisible: false

    Repeater {
        id: c
        property var colors: [
            "#30ff0000",
            "#30ffa500",
            "#30ffff00",
            "#3000ff00",
            "#300000ff",
            "#30ee82ee"
        ]
        model: 20
        MapRectangle
        {
            property int rHeight: ((c.count-index-1)/(c.count-1)*(180-5)+5)
            property int rWidth: rHeight*5

            topLeft
            {
                longitude: Math.min(175, Math.max(-180, 20 - rWidth/2))
                latitude: Math.min(90, Math.max(-90, 20 - rHeight/2))
            }
            bottomRight
            {
                longitude: Math.min(175, Math.max(-180, 20 + rWidth/2))
                latitude: Math.min(90, Math.max(-90, 20 + rHeight/2))
            }
            color: c.colors[Math.floor(index%c.colors.length)]
            border.width: 1
            autoFadeIn: false
            opacity: 1
        }
    }
}
