// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [QtQuick import]
import QtQuick
//! [QtQuick import]
import QtPositioning
import QtLocation

Item {
    width: 1000
    height: 400

    Plugin {
        id: aPlugin
        name: "osm"
    }

    RouteQuery {
        id: aQuery
        waypoints: [
            { latitude: -27.575, longitude: 153.088},
            { latitude: -27.465, longitude: 153.023}
        ]
        travelModes: RouteQuery.CarTravel
        routeOptimizations: RouteQuery.ShortestRoute
    }

    //! [Route Maneuver List1]
    RouteModel {
        id: routeModel
        // model initialization
    //! [Route Maneuver List1]
        plugin: aPlugin
        autoUpdate: true
        query: aQuery
    //! [Route Maneuver List2]
    }


    ListView {
        id: listview
        anchors.fill: parent
        spacing: 10
        model: routeModel.status == RouteModel.Ready ? routeModel.get(0).segments : null
        visible: model ? true : false
        delegate: Row {
            width: parent.width
            spacing: 10
            property bool hasManeuver : modelData.maneuver && modelData.maneuver.valid
            visible: hasManeuver
            Text { text: (1 + index) + "." }
            Text { text: hasManeuver ? modelData.maneuver.instructionText : "" }
    //! [Route Maneuver List2]
            property routeManeuver routeManeuver: modelData.maneuver
            property routeSegment routeSegment: modelData

            //! [routeManeuver]
            Text {
                text: "Distance till next maneuver: " + routeManeuver.distanceToNextInstruction
                      + " meters, estimated time: " + routeManeuver.timeToNextInstruction + " seconds."
            }
            //! [routeManeuver]

            //! [routeSegment]
            Text {
                text: "Segment distance " + routeSegment.distance + " meters, " + routeSegment.path.length + " points."
            }
            //! [routeSegment]
    //! [Route Maneuver List3]
        }
    }
    //! [Route Maneuver List3]
}
