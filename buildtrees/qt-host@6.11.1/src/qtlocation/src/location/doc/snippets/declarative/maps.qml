// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [QtQuick import]
import QtQuick
//! [QtQuick import]
//! [QtLocation import]
import QtPositioning
import QtLocation
//! [QtLocation import]

Item {
    id: page

    Plugin {
        id: myPlugin
    }

    //! [MapRoute]
    Map {
        RouteModel {
            id: routeModel
        }

        MapItemView {
            model: routeModel
            delegate: routeDelegate
        }

        Component {
            id: routeDelegate

            MapRoute {
                route: routeData
                line.color: "blue"
                line.width: 5
                smooth: true
                opacity: 0.8
            }
        }
    }
    //! [MapRoute]

    //! [Map addMapItem MapCircle at current position]
    PositionSource {
        id: positionSource
    }

    Map {
        id: map
        property MapCircle circle

        Component.onCompleted: {
            circle = Qt.createQmlObject('import QtLocation; MapCircle {}', page)
            circle.center = positionSource.position.coordinate
            circle.radius = 5000.0
            circle.color = 'green'
            circle.border.width = 3
            map.addMapItem(circle)
        }
    }
    //! [Map addMapItem MapCircle at current position]
}
