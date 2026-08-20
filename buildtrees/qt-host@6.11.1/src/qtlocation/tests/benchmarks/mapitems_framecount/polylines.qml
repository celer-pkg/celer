// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Map {
    width: 1024
    height: 1024

    property double lonPos: 30
    center: QtPositioning.coordinate(0, lonPos)

    NumberAnimation on lonPos
    {
        loops: Animation.Infinite
        from: -180
        to: 180
        duration: 30000
    }

    id: map
    plugin: Plugin {
        name: "osm"
    }
    zoomLevel: 1
    copyrightsVisible: false

    Repeater {
        id: polylines
        property var colors: [
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet"
        ]
        property int gridSize: polylines.colors.length*10
        model: gridSize*gridSize
        MapPolyline
        {
            property double center_longitude: -180+360*(index%polylines.gridSize+0.5)/(polylines.gridSize)
            property double center_latitude: -90+180*(Math.floor(index/polylines.gridSize)+0.5)/(polylines.gridSize)
            property double a: 1
            path:  [
                QtPositioning.coordinate(Math.min(90, Math.max(-90, a*center_latitude-a*40)),
                                         Math.min(180, Math.max(-180, a*center_longitude-a*40))),
                QtPositioning.coordinate(Math.min(90, Math.max(-90, a*center_latitude+a*40)),
                                         Math.min(180, Math.max(-180, a*center_longitude-a*40))),
                QtPositioning.coordinate(Math.min(90, Math.max(-90, a*center_latitude+a*40)),
                                         Math.min(180, Math.max(-180, a*center_longitude+a*40))),
                QtPositioning.coordinate(Math.min(90, Math.max(-90, a*center_latitude-a*40)),
                                         Math.min(180, Math.max(-180, a*center_longitude+a*80)))
            ]
            line.color: polylines.colors[Math.floor(index%polylines.colors.length)]
            line.width: 2
            autoFadeIn: false
            opacity: 0.5
            NumberAnimation on a
            {
                loops: Animation.Infinite
                from: 0
                to: 1
                duration: 10000
            }
        }
    }



    Keys.onPressed: (event)=> {
        Qt.quit()
    }
}
