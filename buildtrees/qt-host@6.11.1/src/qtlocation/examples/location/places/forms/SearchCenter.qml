// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtPositioning

SearchCenterForm {
    property string title;
    property var coordinate
    signal changeSearchCenter(var coordinate)
    signal closeForm()

    goButton.onClicked: {
        var coordinate = QtPositioning.coordinate(parseFloat(latitude.text),
                                                          parseFloat(longitude.text));
        if (coordinate.isValid)
            changeSearchCenter(coordinate)
    }

    clearButton.onClicked: {
        latitude.text = ""
        longitude.text = ""
    }

    cancelButton.onClicked: closeForm()

    Component.onCompleted: {
        latitude.text = "" + coordinate.latitude
        longitude.text = "" + coordinate.longitude
        if (title.length !== 0)
            tabTitle.text = title;
    }
}
