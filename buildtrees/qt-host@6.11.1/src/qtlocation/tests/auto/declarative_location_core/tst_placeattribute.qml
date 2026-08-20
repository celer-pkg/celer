// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation

TestCase {
    id: testCase

    name: "PlaceAttribute"

    property placeAttribute testAttribute

    function test_default() {
        compare(testAttribute.label, "")
        compare(testAttribute.text, "")
    }

    property placeAttribute qmlAttribute
    qmlAttribute {
        label: "Label"
        text: "Text"
    }

    function test_initialized() {
        compare(qmlAttribute.label, "Label")
        compare(qmlAttribute.text, "Text")
    }
}
