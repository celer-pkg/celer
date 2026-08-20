// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtPositioning

Location {
    address: TestAddress { }
    boundingShape : QtPositioning.rectangle(QtPositioning.coordinate(20, 10, 30), 40, 30)
    coordinate {
        longitude: 10.0
        latitude: 20.0
        altitude: 30.0
    }
}
