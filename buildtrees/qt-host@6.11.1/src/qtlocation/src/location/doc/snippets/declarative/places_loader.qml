// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning
import QtLocation

Rectangle {
    width: 360
    height: 360
    property variant startCoordinate: QtPositioning.coordinate(-27.46778, 153.02778)

    Plugin {
        id: myPlugin
        name: "osm"
        //specify plugin parameters if necessary
        //PluginParameter {...}
        //PluginParameter {...}
        //...
    }

    PlaceSearchModel {
        id: searchModel

        plugin: myPlugin

        searchTerm: "pizza"
        searchArea: QtPositioning.circle(startCoordinate)

        Component.onCompleted: update()
    }

    //! [Handle Result Types]
    Component {
        id: resultDelegate
        Loader {
            Component {
                id: placeResult

                Column {
                    Text { text: title }
                    Text { text: place.location.address.text }
                }
            }

            Component {
                id: otherResult
                Text { text: title }
            }

            sourceComponent: type == PlaceSearchModel.PlaceResult ? placeResult :
                                                                    otherResult
        }
    }
    //! [Handle Result Types]

    ListView {
        anchors.fill: parent
        model: searchModel
        delegate:  resultDelegate
        spacing: 10
    }

    Connections {
        target: searchModel
        onStatusChanged: {
            if (searchModel.status == PlaceSearchModel.Error)
                console.log(searchModel.errorString());
        }
    }
}
