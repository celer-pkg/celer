// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.4
import QtPositioning 5.6
import QtLocation 5.9
import QtLocation.Test 5.6

MapItemGroup {
    id: itemGroup
    property double latitude : (mainRectangle.topLeft.latitude + mainRectangle.bottomRight.latitude) / 2.0
    property double longitude: (mainRectangle.topLeft.longitude + mainRectangle.bottomRight.longitude) / 2.0
    property double radius: 100 * 1000

    MapRectangle {
        id: mainRectangle
        topLeft: QtPositioning.coordinate(43, -3)
        bottomRight: QtPositioning.coordinate(37, 3)
        opacity: 0.05
        visible: true
        color: 'blue'
    }

    MapCircle {
        id: groupCircle
        center: QtPositioning.coordinate(parent.latitude, parent.longitude)
        radius: parent.radius
        color: 'crimson'
    }

    MapRectangle {
        id: groupRectangle
        topLeft: QtPositioning.coordinate(parent.latitude + 5, parent.longitude - 5)
        bottomRight: QtPositioning.coordinate(parent.latitude, parent.longitude )
        color: 'yellow'
    }
}
