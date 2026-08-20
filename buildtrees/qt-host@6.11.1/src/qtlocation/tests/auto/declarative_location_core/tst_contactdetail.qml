// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtLocation

TestCase {
    id: testCase

    name: "ContactDetail"

    property contactDetail emptyContactDetail

    function test_empty() {
        compare(emptyContactDetail.label, "");
        compare(emptyContactDetail.value, "");
    }

    property contactDetail qmlContactDetail
    qmlContactDetail {
        label: "Phone"
        value: "12345"
    }

    function test_qmlConstructedContactDetail() {
        compare(qmlContactDetail.label, "Phone");
        compare(qmlContactDetail.value, "12345");
    }
}
