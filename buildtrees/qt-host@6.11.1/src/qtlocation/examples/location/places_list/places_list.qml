// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [Imports]
import QtQuick
import QtPositioning
import QtLocation
//! [Imports]

Rectangle {
    width: 360
    height: 640
    property variant startCoordinate: QtPositioning.coordinate( 59.9485, 10.7686) // The Qt Company in Oslo

    //! [Initialize Plugin]
    Plugin {
        id: myPlugin
        name: "osm"
        //specify plugin parameters as necessary
        //PluginParameter {...}
        //PluginParameter {...}
        //...
    }
    //! [Initialize Plugin]

    //! [PlaceSearchModel]
    PlaceSearchModel {
        id: searchModel

        plugin: myPlugin

        searchTerm: "food"
        searchArea: QtPositioning.circle(startCoordinate, 5000 /* 5 km radius */);

        Component.onCompleted: update()

    }
    //! [PlaceSearchModel]

    //! [Places ListView]
    ListView {
        anchors.fill: parent
        model: searchModel
        delegate: Component {
            Row {
                spacing: 5
                Marker { height: parent.height }
                Column {
                    Text { text: title; font.bold: true }
                    Text { text: place.location.address.text }
                }
            }
        }
    }
    //! [Places ListView]

    Connections {
        target: searchModel
        function onStatusChanged() {
            if (searchModel.status == PlaceSearchModel.Error)
                console.log(searchModel.errorString());
        }
    }
}
