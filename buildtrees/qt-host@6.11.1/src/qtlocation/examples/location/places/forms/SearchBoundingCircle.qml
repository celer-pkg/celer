// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning

SearchBoundingCircleForm {
    property var searchRegion
    signal changeSearchBoundingCircle(var coordinate, real radius)
    signal closeForm()

    goButton.onClicked: {
        var coordinate = QtPositioning.coordinate(parseFloat(latitude.text),
                                                  parseFloat(longitude.text));
        if (coordinate.isValid)
            changeSearchBoundingCircle(coordinate,parseFloat(radius.text))
    }

    clearButton.onClicked: {
        latitude.text = ""
        longitude.text = ""
        radius.text = ""
    }

    cancelButton.onClicked: closeForm()

    Component.onCompleted: {
        latitude.text = "" + searchRegion.center.latitude
        longitude.text = "" + searchRegion.center.longitude
        radius.text = searchRegion.radius ? "" + searchRegion.radius : "0.0"
    }
}
