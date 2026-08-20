// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtLocation
import "utils.js" as Utils

TestCase {
    id: testCase

    name: "Ratings"

    property ratings emptyRatings

    function test_empty() {
        compare(emptyRatings.average, 0.0);
        compare(emptyRatings.maximum, 0.0);
        compare(emptyRatings.count, 0);
    }

    property ratings qmlRatings: ({ average: 3.5, maximum: 5.0, count: 7 })

    function test_qmlConstructedRatings() {
        compare(qmlRatings.average, 3.5);
        compare(qmlRatings.maximum, 5.0);
        compare(qmlRatings.count, 7);
    }
}
