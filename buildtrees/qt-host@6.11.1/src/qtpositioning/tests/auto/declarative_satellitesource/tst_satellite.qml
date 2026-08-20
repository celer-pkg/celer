// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtPositioning
import QtQml
import QtQuick
import QtTest

TestCase {
    name: "Satellite"
    id: top
    property geoSatelliteInfo sat

    function test_default() {
        compare(sat.satelliteSystem, GeoSatelliteInfo.Undefined)
        compare(sat.satelliteIdentifier, -1)
        compare(sat.signalStrength, -1)
        compare(sat.hasAttribute(GeoSatelliteInfo.Azimuth), false)
        compare(sat.attribute(GeoSatelliteInfo.Azimuth), -1)
        compare(sat.hasAttribute(GeoSatelliteInfo.Elevation), false)
        compare(sat.attribute(GeoSatelliteInfo.Elevation), -1)
    }
}
