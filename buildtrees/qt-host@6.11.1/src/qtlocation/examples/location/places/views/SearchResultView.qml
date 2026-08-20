// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls
import QtQuick.Layouts

//! [PlaceSearchModel place list]
ListView {
    id: searchView

    property var placeSearchModel
    signal showPlaceDetails(var place, var distance)
    signal showMap()

    model: placeSearchModel
    delegate: SearchResultDelegate {
        width: ListView.view.width
        onShowPlaceDetails: function (place, distance) { searchView.showPlaceDetails(place, distance) }
        onSearchFor: function (query) { placeSearchModel.searchForText(query) }
    }

    footer: RowLayout {
        width: parent.width

        Button {
            text: qsTr("Previous")
            enabled: placeSearchModel.previousPagesAvailable
            onClicked: placeSearchModel.previousPage()
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            text: qsTr("Clear")
            onClicked: {
                placeSearchModel.reset()
                showMap()
            }
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            text: qsTr("Next")
            enabled: placeSearchModel.nextPagesAvailable
            onClicked: placeSearchModel.nextPage()
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
//! [PlaceSearchModel place list]
