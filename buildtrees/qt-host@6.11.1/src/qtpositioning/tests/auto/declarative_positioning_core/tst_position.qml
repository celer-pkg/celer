// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtTest 1.0
import QtPositioning 5.3

TestCase {
    id: testCase

    name: "Position"

    Position { id: defaultPosition }

    function test_defaults() {
        compare(defaultPosition.latitudeValid, false);
        compare(defaultPosition.longitudeValid, false);
        compare(defaultPosition.altitudeValid, false);
        compare(defaultPosition.speedValid, false);
        compare(defaultPosition.horizontalAccuracyValid, false);
        compare(defaultPosition.verticalAccuracyValid, false);
        verify(!defaultPosition.directionValid);
        verify(isNaN(defaultPosition.direction));
        verify(!defaultPosition.verticalSpeedValid);
        verify(isNaN(defaultPosition.verticalSpeed));
    }
}
