// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning

SearchBoundingBoxForm {
    property var searchRegion
    signal changeSearchBoundingBox(var coordinate, real widthDeg, real heightDeg)
    signal closeForm()

    goButton.onClicked: {
        var coordinate = QtPositioning.coordinate(parseFloat(latitude.text),
                                                  parseFloat(longitude.text));
        if (coordinate.isValid)
            changeSearchBoundingBox(coordinate,parseFloat(widthDeg.text),parseFloat(heightDeg.text))
    }

    clearButton.onClicked: {
        latitude.text = ""
        longitude.text = ""
        widthDeg.text = ""
        heightDeg.text = ""
    }

    cancelButton.onClicked: closeForm()

    Component.onCompleted: {
        latitude.text = "" + searchRegion.center.latitude
        longitude.text = "" + searchRegion.center.longitude
        widthDeg.text = searchRegion.width ? "" + searchRegion.width : "0.0"
        heightDeg.text = searchRegion.height ? "" + searchRegion.height: "0.0"
    }
}
