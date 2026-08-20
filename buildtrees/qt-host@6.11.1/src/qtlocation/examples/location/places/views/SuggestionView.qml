// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

//! [PlaceSearchSuggestionModel view 1]
ListView {
    id: suggestionView
    property var suggestionModel
    signal suggestionSelected(string text)
//! [PlaceSearchSuggestionModel view 1]
    snapMode: ListView.SnapToItem
//! [PlaceSearchSuggestionModel view 2]
    model: suggestionModel
    delegate: Item {
        width: ListView.view.width
        height: label.height * 1.5
        Label {
            id: label
            text: suggestion
        }
        MouseArea {
            anchors.fill: parent
            onClicked: suggestionSelected(suggestion)
        }
    }
}
//! [PlaceSearchSuggestionModel view 2]

